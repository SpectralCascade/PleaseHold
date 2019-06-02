#ifndef GAME_H
#define GAME_H

#include "../Ossium/src/Ossium.h"
#include "../Ossium/src/Ossium/randutils.h"
#include "switchboard.h"

using namespace Ossium;

extern Font* font;

const int MAX_WAIT_TIME = 20000;
const int MAX_PATIENCE = 20;
const int MIN_WAIT_TIME = 5000;

class Game : public Component
{
public:
    DECLARE_COMPONENT(Game);

    void OnInitGraphics(Renderer* renderer, int layer);

    void SetupInput(InputController* ic);

    ActionOutcome HandlePointer(const MouseInput& data);

    vector<TelephoneNode*> nodes;

    vector<Connection*> connections;

    void Update();

    int score = 0;

    Rand* rng;

private:
    Text* scoreText;

    Clock eventTimer;

    set<NodeClient*> clients;

    set<int> targetNodes;

    Uint32 eventTimeThreshold = 5000;

    InputContext context;

    MouseHandler* mouse;

};

#endif // GAME_H
