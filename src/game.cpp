#include "game.h"

using namespace Ossium;

Font* font = nullptr;

REGISTER_COMPONENT(Game);

void Game::OnInitGraphics(Renderer* renderer, int layer)
{
    rng = new Rand();
    renderer->SetDrawColour(200, 200, 200, 255);
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
        connections.push_back(c);
    }
    scoreText = entity->AddComponent<Text>(renderer, 4);
    scoreText->SetColor(colours::WHITE);
    scoreText->SetRenderMode(RENDERTEXT_BLEND);
    scoreText->SetText(string("Score: ") + ToString(score));
    scoreText->TextToTexture(*renderer, font, 48);
    scoreText->position = Point(((renderer->GetWidth() / 8) * 7), 100);
}

void Game::SetupInput(InputController* ic)
{
    ic->AddContext("game_input", &context);
    mouse = context.AddHandler<MouseHandler>();
    mouse->AddBindlessAction([&] (const MouseInput& data) { return this->HandlePointer(data); });
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

void Game::Update()
{
    scoreText->SetText(string("Score: ") + ToString(score));
    eventTimer.Update(delta.Time());
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
                    if (targetNodes.find(target) == targetNodes.end())
                    {
                        targetNodes.insert(target);
                        break;
                    }
                }
                NodeClient* c = new NodeClient(rng->Int(0, MAX_PATIENCE), this, node, target);
                switch (rng->Int(0, 4))
                {
                case 0:
                    c->requestMessage = string("Hello there. Please connect me to extension number ") + ToString(c->GetTargetExt()) + string(".");
                    break;
                case 1:
                    c->requestMessage = string("Extension ") + ToString(c->GetTargetExt()) + string(" please.");
                    break;
                case 2:
                    c->requestMessage = ToString(c->GetTargetExt()) + string(". Hurry up.");
                    break;
                default:
                    c->requestMessage = string("Could you connect me to extension ") + ToString(c->GetTargetExt()) + string(" please?");
                    break;
                }
                clients.insert(c);
                break;
            }
        }
        eventTimer.SetTime(0);
    }
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
        if (c->node != nullptr)
        {
            c->node->SetClient(nullptr);
        }
        SDL_Log("%s", c->deathMessage.c_str());
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
