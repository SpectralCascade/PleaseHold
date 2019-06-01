#ifndef GAME_H
#define GAME_H

#include "../Ossium/src/Ossium.h"
#include "switchboard.h"

using namespace Ossium;

extern Font* font;

class Game : public Component
{
public:
    DECLARE_COMPONENT(Game);

    void OnInitGraphics(Renderer* renderer, int layer);

    void SetupInput(InputController* ic);

    ActionOutcome HandlePointer(const MouseInput& data);

    vector<TelephoneNode*> nodes;

    vector<Connection*> connections;

private:
    InputContext context;

    MouseHandler* mouse;

};

#endif // GAME_H
