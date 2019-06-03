#include "switchboard.h"
#include "game.h"

using namespace Ossium;

NodeClient::NodeClient(int patience, void* g, TelephoneNode* n, int targetId, Clock* mainClock)
{
    game = g;
    score = clamp(MAX_PATIENCE - patience, 5, MAX_PATIENCE) * 10;
    waitTime = MIN_WAIT_TIME + (patience * 1000);
    node = n;
    targetExt = targetId;
    node->SetClient(this);
    gameClock = mainClock;
    deathMessage = "*Call Finished*";
}

int NodeClient::GetTargetExt()
{
    return targetExt;
}

void NodeClient::Update()
{
    if (!alive)
    {
        return;
    }
    if (theGame->IsTutorial() && !theGame->tutorial->HasPopups() && theGame->tutorial->stage == TutorialStages::AWAIT_CALL_END && clock.GetTime() < waitTime - 3000)
    {
        clock.SetTime(waitTime - 2000);
    }
    if (linkChanges > 2)
    {
        clock.SetTime(waitTime + 1);
        switch (((Game*)game)->rng->Int(0, 4))
        {
        case 0:
            deathMessage = "Ugh, I've been interrupted far too many times, I'm off!";
            break;
        case 1:
            deathMessage = "Interrupted AGAIN?! Terrible! I'm off...";
            break;
        case 2:
            deathMessage = "Damn, the line keeps cutting out, I'm off.";
            break;
        default:
            deathMessage = "I can't cope with these interruptions, I'm off!";
            break;
        }
        connected = false;
    }
    clock.Update(gameClock->GetDeltaTime());
    if (clock.GetTime() > waitTime)
    {
        if (connected)
        {
            /// Increment score for a successful call
            ((Game*)game)->score += score;
            connected = false;
        }
        else
        {
            /// Decrement score for an unsuccessful call
            ((Game*)game)->score -= (MAX_PATIENCE / 3) * 2 * 10;
        }
        alive = false;
    }
}

void NodeClient::OnLink()
{
    clock.SetTime(0);
}

void NodeClient::OnUnlink()
{
    clock.SetTime(0);
    linkChanges++;
}

void NodeClient::SetConnected(bool connect)
{
    connected = connect;
    if (connected)
    {
        clock.SetTime(0);
        linkChanges++;
    }
}

REGISTER_COMPONENT(TrunkLine);

Image TrunkLine::head;
Image TrunkLine::body;
Image TrunkLine::pluggedIn;
Image TrunkLine::holder;

TrunkLine* TrunkLine::tutorialOther = nullptr;

void TrunkLine::OnInitGraphics(Renderer* renderer, int layer)
{
    GraphicComponent::OnInitGraphics(renderer, layer);
    bodySprite = entity->AddComponent<Texture>(renderer, layer);
    holderSprite = entity->AddComponent<Texture>(renderer, layer - 1);
    bodySprite->SetMod(colour);
    if (renderer != nullptr)
    {
        if (!head.Initialised())
        {
            head.LoadAndInit("Textures/trs_jack.png", *renderer);
        }
        if (!body.Initialised())
        {
            body.LoadAndInit("Textures/trs_jack_body.png", *renderer);
        }
        if (!pluggedIn.Initialised())
        {
            pluggedIn.LoadAndInit("Textures/trs_jack_plugged_in.png", *renderer);
        }
        if (!holder.Initialised())
        {
            holder.LoadAndInit("Textures/trs_jack_holder.png", *renderer);
        }
        bodySprite->SetSource(&body);
        holderSprite->SetSource(&holder);
        SetSource(&head);
    }
    else
    {
        SDL_Log("Failed to initialise trunkLine, renderer is null!");
    }
}

void TrunkLine::SetRoot(Point p)
{
    root = p;
    holderSprite->position = p;
}

void TrunkLine::LinkTo(TelephoneNode* n)
{
    if (IsLinked())
    {
        Unlink();
    }
    SetPosition(n->position + socket_offset);
    linked = n->GetId();
    node = n;
    node->Link(this);
    SetSource(&pluggedIn);
    SetMod(colour);
    /// Make body sprite invisible
    bodySprite->SetSource(nullptr);
    if (client != nullptr)
    {
        theGame->Log("[ext " + ToString(client->node->GetId()) + "] > \"" + client->requestMessage + "\"", colours::YELLOW);
    }
    else
    {
        TrunkLine* other = connection->GetOtherLine(this);
        if (IsLinked() && other->client != nullptr)
        {
            if (node->GetId() == other->client->GetTargetExt())
            {
                /// Reset connection
                theGame->Log("CONNECTION ESTABLISHED between " + ToString(node->GetId()) + " and " + ToString(other->node->GetId()), colours::GREEN);
                other->client->SetConnected(true);
                connection->lamp2->ChangeSubState(1);
                connection->lamp1->ChangeSubState(1);
            }
            else
            {
                other->client->alive = false;
                other->client->deathMessage = "Who is this? What?! I'm hanging up >:(";
                theGame->score -= (MAX_PATIENCE / 3) * 10;
            }
        }
    }
}

void TrunkLine::Unlink()
{
    OnClient(nullptr);
    if (node != nullptr)
    {
        node->Unlink();
        node = nullptr;
    }
    SetMod(colours::WHITE);
    SetSource(&head);
    bodySprite->SetSource(&body);
}

bool TrunkLine::IsLinked()
{
    return node != nullptr;
}

void TrunkLine::SetColour(SDL_Color c)
{
    colour = c;
    if (IsLinked())
    {
        SetMod(c);
    }
    bodySprite->SetMod(c);
}

bool TrunkLine::IsActive()
{
    return client != nullptr;
}

void TrunkLine::OnClient(NodeClient* c)
{
    client = c;
    if (client != nullptr)
    {
        if (connection->line1 == this)
        {
            connection->lamp1->ChangeSubState(1);
        }
        else
        {
            connection->lamp2->ChangeSubState(1);
        }
    }
    else
    {
        if (connection->line1 == this)
        {
            connection->lamp1->ChangeSubState(0);
        }
        else
        {
            connection->lamp2->ChangeSubState(0);
        }
    }
}

void TrunkLine::SetPosition(Point p)
{
    position = p;
    bodySprite->position = Point(p.x, p.y + (bodySprite->height / 2) + (height / 2));
}

void TrunkLine::Render(Renderer& renderer)
{
    Texture::Render(renderer);
    renderer.SetDrawColour(colour);
    if (!IsLinked())
    {
        Line(Point(root.x, root.y - 3), Point(bodySprite->position.x, bodySprite->position.y + (bodySprite->height / 2))).Draw(renderer);
    }
    else
    {
        Line(Point(root.x, root.y - 3), position).Draw(renderer);
    }
}

void TrunkLine::Reset()
{
    if (IsLinked())
    {
        Unlink();
    }
    SetPosition(Point(root.x, root.y - 3 - (bodySprite->height) - (height / 2)));
}

void TrunkLine::OnPointerEvent(const MouseInput& data)
{
    Point mpos;
    if (rendererInstance != nullptr)
    {
        /// Offsets to account for the aspect ratio / viewport of the window
        SDL_Rect vrect = rendererInstance->GetViewportRect();
        mpos = Point((float)(data.x - vrect.x), (float)(data.y - vrect.y));
    }
    else
    {
        mpos = Point((float)data.x, (float)data.y);
    }
    if (GetRect().Contains(mpos) || (!IsLinked() && bodySprite->GetRect().Contains(mpos)))
    {
       if (!hovered)
       {
           hovered = true;
           OnHoverBegin();
       }
    }
    else if (hovered)
    {
        hovered = false;
        OnHoverEnd();
    }
    switch (data.type)
    {
    case MOUSE_BUTTON_LEFT:
    case MOUSE_BUTTON_RIGHT:
    case MOUSE_BUTTON_MIDDLE:
        if (data.state)
        {
            if (hovered && !pressed)
            {
                pressed = true;
                OnPointerDown(data);
            }
        }
        else
        {
            if (pressed)
            {
                pressed = false;
                OnPointerUp(data);
            }
        }
        break;
    case MOUSE_MOTION:
        if (pressed)
        {
            OnDrag(data);
        }
        break;
    default:
        break;
    }
}

void TrunkLine::OnPointerDown(const MouseInput& data)
{
    if (theGame->IsTutorial())
    {
        int stage = theGame->tutorial->stage;
        if ((stage != TutorialStages::FINISHED && stage != TutorialStages::AWAIT_LINKAGE && stage != TutorialStages::AWAIT_CONNECTION && stage != TutorialStages::AWAIT_DISCONNECT) || (data.type == MOUSE_BUTTON_LEFT && stage == TutorialStages::AWAIT_DISCONNECT))
        {
            return;
        }
    }
    if (IsLinked())
    {
        Unlink();
        /// play pull out sound
    }
    SetPosition(Point(data.x, data.y));
}

void TrunkLine::OnPointerUp(const MouseInput& data)
{
    if (theGame->IsTutorial())
    {
        int stage = theGame->tutorial->stage;
        Tutorial* t = theGame->tutorial;
        if (stage != TutorialStages::FINISHED && stage != TutorialStages::AWAIT_LINKAGE && stage != TutorialStages::AWAIT_DISCONNECT && stage != TutorialStages::AWAIT_CONNECTION)
        {
            return;
        }
        else if (stage == TutorialStages::AWAIT_DISCONNECT)
        {
            if (data.type != MOUSE_BUTTON_LEFT && !connection->GetOtherLine(this)->IsLinked())
            {
                stage = TutorialStages::END;
                t->AddPopup("Congratulations! You've learned the basics of being a switchboard operator!");
                t->AddPopup("Now, show us what you've got and try n' score big!");
                theGame->isTutorial = false;
                MAX_WAIT_TIME = 12000;
                t->ShowPopup();
            }
            else if (data.type == MOUSE_BUTTON_LEFT)
            {
                return;
            }
        }
    }
    if (data.type == MOUSE_BUTTON_LEFT)
    {
        /// Link up with node if touching one
        bool hit = false;
        for (auto node : ((Game*)(connection->game))->nodes)
        {
            if (node->socketSprite->GetRect().Contains(position) || node->GetRect().Contains(position))
            {
                if (!node->IsLinked())
                {
                    LinkTo(node);
                    Tutorial* t = theGame->tutorial;
                    if (theGame->IsTutorial() && !t->HasPopups())
                    {
                        if (t->stage == TutorialStages::AWAIT_LINKAGE && node->IsActive())
                        {
                            t->stage = TutorialStages::LINKAGE;
                            t->AddPopup("Excellent! Now we're connected to the caller we can listen to their request.");
                            t->AddPopup("Below is the message board, showing recent messages from callers and other information.",
                                        Point(1024 / 2, 768 / 3 * 2),
                                        Rect(100, 600, 800, 300)
                            );
                            t->AddPopup("This particular caller wants us to connect them to extension " + ToString(node->GetClient()->GetTargetExt()) + ".",
                                        Point(1024 / 2, 768 / 3 * 2),
                                        Rect(100, 600, 800, 300)
                            );
                            t->AddPopup("In order to do this, we must use the matching trunk line next to our caller's trunk line.",
                                        Point(1024 / 2, 768 / 3 * 2),
                                        Rect(100, 600, 800, 300)
                            );
                            t->AddPopup("Click \"Okay\" and then connect the matching trunk line to extension " + ToString(node->GetClient()->GetTargetExt()) + ".",
                                        Point(1024 / 2, 768 / 3 * 2),
                                        Rect(100, 600, 800, 300)
                            );
                            t->ShowPopup();
                            tutorialOther = this;
                        }
                        if (t->stage == TutorialStages::AWAIT_CONNECTION)
                        {
                            if (connection->GetOtherLine(this) == tutorialOther && tutorialOther->IsActive())
                            {
                                if (node->GetId() == tutorialOther->node->GetClient()->GetTargetExt())
                                {
                                    t->stage = TutorialStages::CONNECTION;
                                    t->AddPopup("Good job! Now both lines are connected we can sit back (for now).");
                                    t->AddPopup("We know both lines have an active connection because their lamps are on.",
                                                Point(1024 / 2, 768 / 2),
                                                Rect(connection->lamp1->position.x - connection->lamp1->GetSourceWidth() / 2,
                                                     connection->lamp1->position.y - connection->lamp1->GetSourceHeight() / 4,
                                                     connection->lamp1->GetSourceWidth() * 2,
                                                     connection->lamp1->GetSourceHeight() / 2
                                                )
                                    );
                                    t->AddPopup("In the main game, you should try and make these connections as soon as a caller appears.");
                                    t->AddPopup("You lose points if a caller gets connected to the wrong extension or if the call ends before you connect the caller.");
                                    t->AddPopup("On the contrary, you gain points when a call ends if the caller is connected to their target extension.");
                                    t->AddPopup("Let's wait for the call to end.");
                                }
                                else
                                {
                                    t->AddPopup("Oof! Unfortunately, that was the wrong extension and you've lost some points as a result.");
                                    t->AddPopup("The caller has gone, so we'll have to wait for another call.");
                                    connection->Reset();
                                    tutorialOther = nullptr;
                                    t->stage = TutorialStages::INTRO;
                                }
                                t->ShowPopup();
                            }
                        }
                    }
                    /// play plug-in sound
                    hit = true;
                }
                break;
            }
        }
        if (!hit)
        {
            Reset();
        }
    }
    else
    {
        Reset();
    }
}

void TrunkLine::OnDrag(const MouseInput& data)
{
    if (theGame->IsTutorial())
    {
        int stage = theGame->tutorial->stage;
        if (stage != TutorialStages::FINISHED && stage != TutorialStages::AWAIT_LINKAGE && stage != TutorialStages::AWAIT_CONNECTION)
        {
            return;
        }
    }
    SetPosition(Point(data.x, data.y));
}

REGISTER_COMPONENT(TelephoneNode);

void TelephoneNode::OnInitGraphics(Renderer* renderer, int layer)
{
    GraphicComponent::OnInitGraphics(renderer, layer);
    socketSprite = entity->AddComponent<Texture>(renderer, layer);
    if (!socket.Initialised())
    {
        socket.LoadAndInit("Textures/socket.png", *renderer);
    }
    if (!lamp.Initialised())
    {
        lamp.LoadAndInit("Textures/socket_lamp.png", *renderer);
    }
    AddState("lamp", &lamp, true, 2);
    socketSprite->SetSource(&socket);
    numberText = entity->AddComponent<Text>(renderer, 2);
    numberText->SetColor(colours::BLACK);
    numberText->SetRenderMode(RENDERTEXT_BLEND);
    numberText->SetText(ToString(id));
    numberText->TextToTexture(*renderer, font, 12);
}

NodeClient* TelephoneNode::GetClient()
{
    return client;
}

void TelephoneNode::SetPos(Point p)
{
    socketSprite->position = p;
    position = Point(p.x, p.y + height);
    numberText->position = Point(p.x + 8, p.y);
}

void TelephoneNode::Link(TrunkLine* trunkLine)
{
    if (IsLinked())
    {
        Unlink();
    }
    link = trunkLine;
    if (link != nullptr)
    {
        if (client != nullptr)
        {
            link->OnClient(client);
            client->OnLink();
        }
    }
}

void TelephoneNode::Unlink()
{
    if (link != nullptr)
    {
        if (client != nullptr)
        {
            client->OnUnlink();
        }
        link = nullptr;
    }
}

bool TelephoneNode::IsActive()
{
    return client != nullptr;
}

bool TelephoneNode::IsLinked()
{
    return link != nullptr;
}

void TelephoneNode::SetClient(NodeClient* c)
{
    if (client != nullptr)
    {
        if (IsLinked())
        {
            link->OnClient(nullptr);
        }
        client->alive = false;
    }
    client = c;
    if (client != nullptr)
    {
        if (IsLinked())
        {
            link->OnClient(client);
        }
        ChangeSubState(1);
    }
    else
    {
        ChangeSubState(0);
        if (IsLinked())
        {
            if (link == link->connection->line1)
            {
                link->connection->lamp2->ChangeSubState(0);
            }
            else
            {
                link->connection->lamp1->ChangeSubState(0);
            }
            link->OnClient(nullptr);
        }
    }
}

void TelephoneNode::SetId(int v)
{
    id = v;
    numberText->SetText(ToString(v));
}

int TelephoneNode::GetId()
{
    return id;
}

Image TelephoneNode::socket;
Image TelephoneNode::lamp;

REGISTER_COMPONENT(Connection);

Image Connection::lamp;

void Connection::OnInitGraphics(Renderer* renderer, int layer)
{
    line1 = entity->AddComponent<TrunkLine>(renderer, layer >= 0 ? layer + 1 : -1);
    line2 = entity->AddComponent<TrunkLine>(renderer, layer >= 0 ? layer + 1 : -1);
    line1->connection = this;
    line2->connection = this;
    lamp1 = entity->AddComponent<StateSprite>(renderer, layer);
    lamp2 = entity->AddComponent<StateSprite>(renderer, layer);
    if (!lamp.Initialised())
    {
        lamp.LoadAndInit("Textures/line_lamp.png", *renderer);
    }
    lamp1->AddState("lamp", &lamp, true, 2);
    lamp2->AddState("lamp", &lamp, true, 2);
}

void Connection::SetRoot(Point p)
{
    line1->SetRoot(Point(p.x - 16, p.y));
    line2->SetRoot(Point(p.x + 16, p.y));
    lamp1->position = Point(p.x - 16, p.y + 21);
    lamp2->position = Point(lamp1->position.x + 32, lamp1->position.y);
}

TrunkLine* Connection::GetOtherLine(TrunkLine* line)
{
    return line == line1 ? line2 : line1;
}

void Connection::Reset()
{
    line1->Reset();
    line2->Reset();
}

bool Connection::IsActive()
{
    return line1 != nullptr && line2 != nullptr && line1->IsActive() && line2->IsActive();
}
