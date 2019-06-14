#include <chrono>

#include "game.h"

using namespace Ossium;

Font* font = nullptr;
Game* theGame = nullptr;

int MAX_WAIT_TIME = 12000;

void GraphicRect::Render(Renderer& renderer)
{
    renderer.SetDrawColor(Colors::BLACK);
    DrawFilled(renderer);
}

REGISTER_COMPONENT(Tutorial);

void Tutorial::OnInitGraphics(Renderer* renderer, int layer)
{
    GraphicComponent::OnInitGraphics(renderer, layer);
    /// popup text/box
    popupText = entity->AddComponent<Text>(renderer, layer + 1);
    popupText->SetAlphaMod(0);
    popupText->SetRenderMode(RENDERTEXT_BLEND);
    popupText->SetBox(true);
    popupText->SetBoxPaddingWidth(12);
    popupText->SetBoxPaddingHeight(40);
    popupText->SetBackgroundColor(Colors::CYAN);
    popupText->SetColor(Colors::BLACK);
    popupText->SetText("TUTORIAL");
    popupText->TextToTexture(*renderer, font, 18);
    /// button
    okay = entity->CreateChild()->AddComponent<Button>(renderer, layer + 2);
    okay->OnClicked += [&] (Button& bcaller) { this->GoNext(); };
    buttonText = entity->AddComponent<Text>(renderer, layer + 3);
    if (!buttonTexture.LoadAndInit("Textures/popupbutton.png", *renderer))
    {
        SDL_Log("Failed to load popup button!");
    }
    else
    {
        okay->sprite->AddState("default", &buttonTexture, true, 3);
    }
    buttonText->SetText("Okay");
    buttonText->SetColor(Colors::BLACK);
    buttonText->SetRenderMode(TextRenderModes::RENDERTEXT_BLEND);
    buttonText->TextToTexture(*renderer, font, 14);
}

void Tutorial::Render(Renderer& renderer)
{
    if (shroudActive)
    {
        renderer.SetDrawColor((SDL_Color){0, 0, 0, 220});
        for (int i = 0; i < 4; i++)
        {
            shrouds[i].DrawFilled(renderer);
        }
    }
}

void Tutorial::SetShroudActive(bool active)
{
    shroudActive = active;
}

void Tutorial::SetShroud(Rect targetArea)
{
    /// Left
    shrouds[0] = Rect(0, 0, targetArea.x, 768);
    /// Top
    shrouds[1] = Rect(targetArea.x, 0, targetArea.w, targetArea.y);
    /// Right
    shrouds[2] = Rect(targetArea.x + targetArea.w, 0, 1024 - (targetArea.x + targetArea.w), 768);
    /// Bottom
    shrouds[3] = Rect(targetArea.x, targetArea.y + targetArea.h, targetArea.w, 768 - (targetArea.y + targetArea.h));
}

void Tutorial::SetCentral()
{
    SetPopupPosition(Point(1024 / 2, 768 / 2));
    Rect shroud = Rect(popupText->position.x - (popupText->GetWidth() / 2), popupText->position.y - (popupText->GetHeight() / 2), popupText->GetWidth(), popupText->GetHeight());
    SetShroud(shroud);
}

void Tutorial::SetPopupActive(bool active)
{
    popupActive = active;
    if (active)
    {
        popupText->SetAlphaMod(255);
        popupText->SetBox(true);
        okay->sprite->SetAlphaMod(255);
        buttonText->SetAlphaMod(255);
        gui->SetActive(true);
    }
    else
    {
        popupText->SetAlphaMod(0);
        popupText->SetBox(false);
        okay->sprite->SetAlphaMod(0);
        buttonText->SetAlphaMod(0);
        gui->SetActive(false);
    }
}

bool Tutorial::HasPopups()
{
    return !texts.empty();
}

void Tutorial::AddPopup(string text)
{
    AddPopup(text, Point(1024 / 2, 768 / 2), Rect(0, 0, 1024, 768));
}

void Tutorial::AddPopup(string text, Point p)
{
    AddPopup(text, p, Rect(0, 0, 1024, 768));
}

void Tutorial::AddPopup(string text, Point p, Rect targetArea)
{
    texts.push(text);
    positions.push(p);
    targets.push(targetArea);
}

void Tutorial::SetPopupPosition(Point position)
{
    popupText->position = position;
    okay->sprite->position = Point(position.x, position.y + 32);
    buttonText->position = okay->sprite->position;
}

void Tutorial::SetPopupText(string text)
{
    popupText->SetText(text);
}

void Tutorial::Update()
{
    if (onNextUpdate)
    {
        onNextUpdate = false;
        ShowPopup(true);
    }
    if ((stage == AWAIT_LINKAGE || stage == AWAIT_CONNECTION || stage == AWAIT_DISCONNECT) && !theGame->gameTime.IsPaused())
    {
        theGame->gameTime.SetPaused(true);
    }
}

void Tutorial::ShowPopup(bool immediate)
{
    if (immediate && HasPopups())
    {
        GoNext();
        SetPopupActive(true);
        SetShroudActive(true);
        theGame->gameTime.SetPaused(true);
    }
    else
    {
        onNextUpdate = true;
    }
}

void Tutorial::GoNext()
{
    if (HasPopups())
    {
        SetPopupText(texts.front());
        SetPopupPosition(positions.front());
        SetShroud(targets.front());
        texts.pop();
        positions.pop();
        targets.pop();
    }
    else
    {
        SetPopupActive(false);
        SetShroudActive(false);
        stage++;
        theGame->gameTime.SetPaused(false);
    }
}

AudioSource jack;
AudioSource caller;
AudioBus master;

AudioClip plugin;
AudioClip plugout;
AudioClip ding;

REGISTER_COMPONENT(Game);

const int MESSAGE_OUTPUT_HEIGHT = (768 / 5);
const int MESSAGE_OUTPUT_Y_POS = MESSAGE_OUTPUT_HEIGHT * 4;
const int MESSAGE_HEIGHT = 80;

bool Game::IsTutorial()
{
    return isTutorial;
}

int Game::GetTutorialStage()
{
    if (!IsTutorial())
    {
        return -1;
    }
    else
    {
        return tutorial->stage;
    }
}

const SDL_Color LINE_COLOURS[8] = {Colors::WHITE, Colors::RED, (SDL_Color){200, 100, 0, 255}, Colors::YELLOW, Colors::GREEN, Colors::CYAN, Colors::BLUE, Colors::MAGENTA};

void Game::OnInitGraphics(Renderer* renderer, int layer)
{
    MAX_WAIT_TIME = 12000;
    r.x = 0;
    r.y = 0;
    r.w = 1024;
    r.h = 768;
    render = renderer;
    rng = new Rand(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    renderer->SetDrawColor(200, 200, 200, 255);
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            TelephoneNode* node = entity->AddComponent<TelephoneNode>(renderer, 1);
            node->SetId(100 + ToInt(ToString(j + 1) + ToString(i)));
            node->SetPos(Point(32 + ((renderer->GetWidth() / 2) - (64 * (4 - j))), 18 + (i * 72)));
            nodes.push_back(node);
        }
    }
    for (int i = 0; i < 8; i++)
    {
        Connection* c = entity->AddComponent<Connection>(renderer, 3);
        c->SetRoot(Point((renderer->GetWidth() / 2) - (64 * (4 - i)) + 32, 7 * 72 + 36));
        c->Reset();
        c->game = this;
        c->line1->SetColour(LINE_COLOURS[i]);
        c->line2->SetColour(LINE_COLOURS[i]);
        connections.push_back(c);
    }
    scoreText = entity->AddComponent<Text>(renderer, 8);
    scoreText->SetColor(Colors::WHITE);
    scoreText->SetRenderMode(RENDERTEXT_BLEND);
    scoreText->SetText(string("Score: ") + ToString(score));
    scoreText->TextToTexture(*renderer, font, 48);
    scoreText->position = Point(((renderer->GetWidth() / 8) * 7), 100);
    scoreChangeText = entity->AddComponent<Text>(renderer, 4);
    scoreChangeText->SetColor(Colors::GREEN);
    scoreChangeText->SetRenderMode(RENDERTEXT_BLEND);
    scoreChangeText->SetText("+0");
    scoreChangeText->TextToTexture(*renderer, font, 48);
    scoreChangeText->position = Point(((renderer->GetWidth() / 8) * 7), (768 / 6) * 5);
    scoreChangeText->SetAlphaMod(0);
    timeLeftText = entity->AddComponent<Text>(renderer, 4);
    timeLeftText->SetColor(Colors::RED);
    timeLeftText->SetRenderMode(RENDERTEXT_BLEND);
    timeLeftText->SetText("0:00s");
    timeLeftText->TextToTexture(*renderer, font, 36);
    timeLeftText->position = Point(scoreText->position.x, 200);
    messageStream = new CircularBuffer<string>(MESSAGE_OUTPUT_Y_POS / MESSAGE_HEIGHT);
    messageTexts = new Text*[messageStream->maxSize()];
    for (int i = 0, counti = messageStream->maxSize(); i < counti; i++)
    {
        Text* t = entity->AddComponent<Text>(renderer, 6);
        t->SetColor(Colors::WHITE);
        t->SetRenderMode(RENDERTEXT_BLEND);
        t->position = Point(1024 / 2, MESSAGE_OUTPUT_Y_POS + ((MESSAGE_OUTPUT_HEIGHT / counti) + (MESSAGE_OUTPUT_HEIGHT / (counti + 1)) * i));
        messageTexts[i] = t;
    }
    /// audio
    jack.Link(&master);
    caller.Link(&master);
    scoreAudio.Link(&master);
    if (!ding.Load("Audio/ding1.wav") || !plugin.Load("Audio/plugin.wav") || !plugout.Load("Audio/plugout.wav") || !badscore.Load("Audio/badscore.wav") || !goodscore.Load("Audio/goodscore.wav"))
    {
        SDL_Log("Error loading sounds! Mix_Error: %s", Mix_GetError());
    }
    theGame = this;
}

void Game::SetupInput(InputController* ic)
{
    inputGui = entity->AddComponent<InputGUI>(render, 9);
    ic->AddContext("game_input", &context);
    ic->AddContext("restart_gui", inputGui);
    mouse = context.AddHandler<MouseHandler>();
    mouse->AddBindlessAction([&] (const MouseInput& data) { return this->HandlePointer(data); });
    /// Each game will last 5 minutes
    timeLeft.SetTime(60000 * 5);
    /// Reverse the timer so it counts down
    timeLeft.Stretch(-1);
}

void Game::SetTutorial(Tutorial* t)
{
    tutorial = t;
    isTutorial = true;
    gameTime.SetPaused(true);
    MAX_WAIT_TIME = 20000;
}

ActionOutcome Game::HandlePointer(const MouseInput& data)
{
    if (data.type != MOUSE_UNKNOWN)
    {
        for (auto connection : connections)
        {
            connection->line1->OnPointerEvent(data);
            connection->line2->OnPointerEvent(data);
        }
    }
    /// Ignored as this is a bindless action.
    return Ignore;
}

void Game::Restart()
{
    gameTime.SetPaused(false);
    timeLeft.SetPaused(false);
    timeLeft.SetTime(60000 * 5);
    if (endText != nullptr)
    {
        endText->SetAlphaMod(0, true);
    }
    scoreText->position = Point(((1024 / 8) * 7), 100);
    finished = false;
    for (auto c : connections)
    {
        c->Reset();
    }
    for (auto c : clients)
    {
        c->alive = false;
    }
    PruneClients();
    oldScore = 0;
    score = 0;
    restartButton->GetComponent<Button>()->OnClicked -= clickRestartHandle;
    restartButton->Destroy();
    context.SetActive(true);
    inputGui->SetActive(false);
    doRestart = false;
    ClearLog();
}

AudioClip badscore;
AudioClip goodscore;
AudioSource scoreAudio;

void Game::PruneClients()
{
    vector<NodeClient*> toRemove;
    for (auto c : clients)
    {
        c->Update();
        if (!c->alive)
        {
            toRemove.push_back(c);
        }
    }
    for (auto c : toRemove)
    {
        Log("[ext " + ToString(c->node->GetId()) + "] > \"" + c->deathMessage + "\"");
        if (IsTutorial() && !tutorial->HasPopups())
        {
            if (tutorial->stage == TutorialStages::AWAIT_CALL_END)
            {
                tutorial->stage = TutorialStages::CALL_END;
                tutorial->AddPopup("Great, the call has ended and both lamps have gone out under our line connection.");
                tutorial->AddPopup("The more successful calls, the more points you will score.");
                tutorial->AddPopup("Now we can go ahead and disconnect the lines because it's not an active connection anymore.");
                tutorial->AddPopup("To do that, click \"Okay\" and right click both lines to disconnect them.");
                tutorial->ShowPopup();
            }
        }
        if (c->node != nullptr)
        {
            c->node->SetClient(nullptr);
        }
        clients.erase(c);
        auto itr = targetNodes.find(c->GetTargetExt());
        if (itr != targetNodes.end())
        {
            targetNodes.erase(itr);
        }
        delete c;
        c = nullptr;
    }
}

void Game::Update()
{
    gameTime.Update(delta.Time());
    if (finished)
    {
        if (doRestart)
        {
            Restart();
        }
        else
        {
            render->Enqueue(&r, 7);
        }
        return;
    }
    if (oldScore != score)
    {
        alpha = 255;
        changeAlpha = 1;
        int difference = score - oldScore;
        if (score - oldScore < 0)
        {
            scoreChangeText->SetText(ToString(difference));
            scoreChangeText->SetColor(Colors::RED);
            scoreAudio.Play(&badscore, 0.4f);
        }
        else
        {
            scoreChangeText->SetText("+" + ToString(difference));
            scoreChangeText->SetColor(Colors::GREEN);
            scoreAudio.Play(&goodscore, 0.4f);
        }
        scoreChangeText->position = Point(((render->GetWidth() / 8) * 7), (768 / 4) * 3);
        oldScore = score;
    }
    else if (alpha > 0)
    {
        changeAlpha = clamp(changeAlpha - (gameTime.GetDeltaTime()), 0.0f, 1.0f);
        alpha = (Uint8)(changeAlpha * 255.0f);
        scoreChangeText->position = Point(((render->GetWidth() / 8) * 7), scoreChangeText->position.y - ((gameTime.GetDeltaTime() * 64) * (scoreChangeText->GetColor().g == 255 ? 1.0f : -1.0f)));
    }
    scoreChangeText->SetAlphaMod(alpha);

    timeLeft.Update(gameTime.GetDeltaTime());
    int secs = (int)((timeLeft.GetTime() / 1000) % 60);
    timeLeftText->SetText(ToString((int)((timeLeft.GetTime() / 1000) / 60)) + string(":") + (secs < 10 ? string("0") + ToString(secs) : ToString(secs)) + string("s"));
    if (timeLeft.GetTime() == 0)
    {
        /// Game ends
        EndGame();
    }
    scoreText->SetText(string("Score: ") + ToString(score));
    eventTimer.Update(gameTime.GetDeltaTime());
    if (eventTimer.GetTime() > eventTimeThreshold)
    {
        eventTimeThreshold = (Uint32)rng->Int(2000, MAX_WAIT_TIME);
        for (int i = 0; i < 10; i++)
        {
            TelephoneNode* node = *PickRandom(nodes, rng);
            if (!node->IsActive() && !node->IsLinked() && targetNodes.find(node->GetId()) == targetNodes.end())
            {
                int target = 110;
                for (int j = 0; j < 10; j++)
                {
                    target = 100 + ToInt(ToString(rng->Int(0, 7) + 1) + ToString(rng->Int(0, 5)));
                    if (targetNodes.find(target) == targetNodes.end() && target != node->GetId())
                    {
                        targetNodes.insert(target);
                        break;
                    }
                }
                NodeClient* c = new NodeClient(rng->Int(0, MAX_PATIENCE), this, node, target, &gameTime);
                switch (rng->Int(0, 4))
                {
                case 0:
                    c->requestMessage = string("Hello there. Please connect me to extension number ") + ToString(c->GetTargetExt()) + string(".");
                    break;
                case 1:
                    c->requestMessage = string("Extension number ") + ToString(c->GetTargetExt()) + string(" please.");
                    break;
                case 2:
                    c->requestMessage = string("Extension ") + ToString(c->GetTargetExt()) + string(". Hurry up.");
                    break;
                default:
                    c->requestMessage = string("Could you connect me to extension ") + ToString(c->GetTargetExt()) + string(" please?");
                    break;
                }
                clients.insert(c);
                if (IsTutorial() && tutorial->stage == TutorialStages::AWAIT_CALL && !tutorial->HasPopups())
                {
                    tutorial->AddPopup("Look! We have an incoming call on extension " + ToString(node->GetId()) + ".",
                                       Point(1024 / 2, 768 / 4 * 3),
                                       Rect(node->position.x - (node->GetSourceWidth() / 2),
                                            node->position.y - (node->GetSourceHeight() / 4 * 3),
                                            node->GetSourceWidth(),
                                            node->GetSourceHeight()
                                       )
                    );
                    tutorial->AddPopup("We need to connect a trunk line to this extension.");
                    tutorial->AddPopup("Down here we have several colour coded pairs of trunk lines.",
                                       Point(1024 / 2, 768 / 8 * 7),
                                       Rect(100, 450, 800, 250)
                    );
                    tutorial->AddPopup("Go ahead and connect one to the caller on extension " + ToString(node->GetId()) + " (drag and drop any trunk line).",
                                       Point(1024 / 2, 768 / 8 * 7)
                    );
                    tutorial->ShowPopup();
                }
                break;
            }
        }
        eventTimer.SetTime(0);
    }
    PruneClients();
}

void Game::EndGame()
{
    if (!finished)
    {
        SDL_Log("Game Over!");
        gameTime.SetPaused(true);
        scoreText->position = Point(1024 / 2, 768 / 2);
        if (endText == nullptr)
        {
            endText = entity->AddComponent<Text>(render, 8);
            endText->SetText("Thanks for playing! :)");
            endText->SetColor(Colors::GREEN);
            endText->SetRenderMode(RENDERTEXT_BLEND);
            endText->TextToTexture(*render, font, 36);
            endText->position = Point(scoreText->position.x, scoreText->position.y + 64);
        }
        if (endText != nullptr)
        {
            endText->SetAlphaMod(255, true);
        }
        restartButton = entity->CreateChild();
        Button* actualButton = restartButton->AddComponent<Button>(render, 8);
        Text* restartText = restartButton->AddComponent<Text>(render, 9);
        restartText->SetText("Replay");
        restartText->SetColor(Colors::BLACK);
        restartText->TextToTexture(*render, font, 36);
        restartText->position = Point(1024 / 2, (768 / 4 * 3));
        actualButton->sprite->position = restartText->position;
        actualButton->sprite->AddState("default", regularButton, true, 3);
        clickRestartHandle = actualButton->OnClicked += [&] (const Button& bcaller) {
            this->doRestart = true;
        };
        context.SetActive(false);
        inputGui->RemoveAll();
        inputGui->AddInteractable("restart button", *actualButton);
        inputGui->SetActive(true);
        finished = true;
    }
}

void Game::OnDestroy()
{
    for (auto client : clients)
    {
        if (client != nullptr)
        {
            delete client;
            client = nullptr;
        }
    }
    clients.clear();
    targetNodes.clear();
}

void Game::Log(string message, SDL_Color colour)
{
    static int counter;
    message = "> " + message;
    counter++;
    if (messageStream->size() == messageStream->maxSize())
    {
        float cachedy = messageTexts[messageStream->maxSize() - 1]->position.y;
        for (int i = messageStream->maxSize() - 1; i > 0; i--)
        {
            messageTexts[i]->position = Point(messageTexts[i]->position.x, messageTexts[wrap(i, -1, 0, messageStream->maxSize() - 1)]->position.y);
        }
        messageTexts[0]->position = Point(messageTexts[0]->position.x, cachedy);
    }
    messageStream->push_back(message);
    messageTexts[messageStream->GetBackIndex()]->SetColor(colour);
    messageTexts[messageStream->GetBackIndex()]->SetText(message);
    messageTexts[messageStream->GetBackIndex()]->TextToTexture(*render, font, 16);
}

void Game::ClearLog()
{
    for (int i = 0, counti = messageStream->size(); i < counti; i++)
    {
        messageTexts[i]->SetColor(Colors::TRANSPARENT);
        messageTexts[i]->TextToTexture(*render, font, 16);
        messageStream->pop_back();
    }
}

