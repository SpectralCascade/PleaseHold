#ifndef SWITCHBOARD_H
#define SWITCHBOARD_H

#include "../Ossium/src/Ossium.h"
#include "button.h"

using namespace Ossium;

/// Forward declaration
class TelephoneNode;

/// This is the person on the other end of the line.
class NodeClient
{
public:
    void OnLink();
    void OnUnlink();

};

const Point socket_offset = {-16, -36};

/// Forward declaration
class Connection;

class TrunkLine : public Texture
{
public:
    DECLARE_COMPONENT(TrunkLine);

    void OnInitGraphics(Renderer* renderer, int layer);

    /// Links this trunkLine to a specified node
    void LinkTo(TelephoneNode* node);
    /// Unlinks this trunkLine.
    void Unlink();

    /// Is this trunkLine linked to a node?
    bool IsLinked();
    /// Is there a caller coming through this trunkLine?
    bool IsActive();

    void OnClient(NodeClient* c);

    void SetColour(SDL_Color c);

    TelephoneNode* node = nullptr;

    void SetRoot(Point p);

    void SetPosition(Point p);

    void Render(Renderer& renderer);

    void Reset();

    void OnPointerEvent(const MouseInput& data);

    void OnHoverBegin(){};
    void OnHoverEnd(){};

    void OnPointerDown(const MouseInput& data);
    void OnPointerUp(const MouseInput& data);
    void OnDrag(const MouseInput& data);

    ActionOutcome HandlePointer(const MouseInput& data);

    Connection* connection = nullptr;

private:
    /// Where the trunkLine sits
    Point root;

    bool hovered = false;
    bool pressed = false;

    /// The node this trunkLine is linked to
    int linked = -1;

    SDL_Color colour = colours::RED;

    static Image head;
    static Image body;
    static Image pluggedIn;
    static Image holder;

    Texture* holderSprite = nullptr;
    Texture* bodySprite = nullptr;

};

class TelephoneNode : public StateSprite
{
public:
    DECLARE_COMPONENT(TelephoneNode);

    void OnInitGraphics(Renderer* renderer, int layer);

    bool IsLinked();

    void Link(TrunkLine* trunkLine);
    void Unlink();

    /// Is there a client trying to use this node?
    bool IsActive();

    /// Adds a client to this node; overrides any current client
    void SetClient(NodeClient* c);

    void SetPos(Point p);

    /// Returns this node's client
    NodeClient* GetClient();

    void SetId(int v);
    int GetId();

    Texture* socketSprite;

private:
    int id = 0;

    TrunkLine* link = nullptr;

    NodeClient* client = nullptr;

    Text* numberText;

    static Image socket;
    static Image lamp;

};

class Connection : public Texture
{
public:
    DECLARE_COMPONENT(Connection);

    void OnInitGraphics(Renderer* renderer, int layer);

    TrunkLine* line1;
    TrunkLine* line2;

    /// Are both trunk lines linked to client nodes?
    bool IsActive();

    void SetRoot(Point p);

    /// Resets the connection
    void Reset();

    void SetupInput(InputContext* ic);

    void* game;

private:
    StateSprite* lamp1;
    StateSprite* lamp2;

    static Image lamp;

};

#endif // SWITCHBOARD_H