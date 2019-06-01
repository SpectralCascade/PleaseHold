#include "game.h"

using namespace Ossium;

Font* font = nullptr;

REGISTER_COMPONENT(Game);

void Game::OnInitGraphics(Renderer* renderer, int layer)
{
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