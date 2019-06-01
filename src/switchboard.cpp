#include "switchboard.h"
#include "game.h"

using namespace Ossium;

void NodeClient::OnLink()
{
}

void NodeClient::OnUnlink()
{
}

REGISTER_COMPONENT(TrunkLine);

Image TrunkLine::head;
Image TrunkLine::body;
Image TrunkLine::pluggedIn;
Image TrunkLine::holder;

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
            head.LoadAndInit("Textures/trs_jack.png", *renderer, SDL_GetWindowPixelFormat(renderer->GetWindow()->GetWindow()));
        }
        if (!body.Initialised())
        {
            body.LoadAndInit("Textures/trs_jack_body.png", *renderer, SDL_GetWindowPixelFormat(renderer->GetWindow()->GetWindow()));
        }
        if (!pluggedIn.Initialised())
        {
            pluggedIn.LoadAndInit("Textures/trs_jack_plugged_in.png", *renderer, SDL_GetWindowPixelFormat(renderer->GetWindow()->GetWindow()));
        }
        if (!holder.Initialised())
        {
            holder.LoadAndInit("Textures/trs_jack_holder.png", *renderer, SDL_GetWindowPixelFormat(renderer->GetWindow()->GetWindow()));
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

void TrunkLine::LinkTo(TelephoneNode* node)
{
    Unlink();
    SetPosition(node->position + socket_offset);
    linked = node->GetId();
    node = node;
    node->Link(this);
    SetSource(&pluggedIn);
    SetMod(colour);
    /// Make body sprite invisible
    bodySprite->SetSource(nullptr);
}

void TrunkLine::Unlink()
{
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

void TrunkLine::OnClient(NodeClient* c)
{
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
    if (IsLinked())
    {
        Unlink();
        /// play pull out sound
    }
    SetPosition(Point(data.x, data.y));
}

void TrunkLine::OnPointerUp(const MouseInput& data)
{
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
                    /// play plug in sound
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
    SetPosition(Point(data.x, data.y));
}

REGISTER_COMPONENT(TelephoneNode);

void TelephoneNode::OnInitGraphics(Renderer* renderer, int layer)
{
    GraphicComponent::OnInitGraphics(renderer, layer);
    socketSprite = entity->AddComponent<Texture>(renderer, layer);
    if (!socket.Initialised())
    {
        socket.LoadAndInit("Textures/socket.png", *renderer, SDL_GetWindowPixelFormat(renderer->GetWindow()->GetWindow()));
    }
    if (!lamp.Initialised())
    {
        lamp.LoadAndInit("Textures/socket_lamp.png", *renderer, SDL_GetWindowPixelFormat(renderer->GetWindow()->GetWindow()));
    }
    SetSource(&lamp);
    SetClip(0, 0, 64, 36);
    socketSprite->SetSource(&socket);
    numberText = entity->AddComponent<Text>(renderer, 2);
    numberText->SetColor(colours::BLACK);
    numberText->SetRenderMode(RENDERTEXT_BLEND);
    numberText->SetText(ToString(id));
    numberText->TextToTexture(*renderer, font, 12);
}

void TelephoneNode::SetPos(Point p)
{
    socketSprite->position = p;
    position = Point(p.x, p.y + height);
    numberText->position = Point(p.x + 8, p.y);
}

void TelephoneNode::Link(TrunkLine* trunkLine)
{
    Unlink();
    link = trunkLine;
    if (link != nullptr)
    {
        link->node = this;
        if (client != nullptr)
        {
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
    client = c;
    if (IsLinked())
    {
        link->OnClient(client);
        SetClip(0, 36, 64, 36);
    }
    else
    {
        SetClip(0, 0, 64, 36);
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
}

void Connection::SetRoot(Point p)
{
    line1->SetRoot(Point(p.x - 16, p.y));
    line2->SetRoot(Point(p.x + 16, p.y));
}

void Connection::Reset()
{
    line1->Reset();
    line2->Reset();
}

void Connection::SetupInput(InputContext* ic)
{
}
