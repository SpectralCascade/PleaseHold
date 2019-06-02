#ifndef GAME_H
#define GAME_H

#include <queue>

#include "../Ossium/src/Ossium.h"
#include "../Ossium/src/Ossium/randutils.h"
#include "switchboard.h"

using namespace std;
using namespace Ossium;

extern Font* font;

const int MAX_WAIT_TIME = 20000;
const int MAX_PATIENCE = 20;
const int MIN_WAIT_TIME = 5000;

enum TutorialStages
{
    INTRO = 0,
    AWAIT_CALL,
    AWAIT_LINKAGE,
    LINKAGE,
    AWAIT_CONNECTION,
    CONNECTION,
    AWAIT_CALL_END,
    CALL_END,
    AWAIT_DISCONNECT,
    END,
    FINISHED
};

class GraphicRect : public Rect, public Graphic
{
    void Render(Renderer& renderer);
};

class Tutorial : public GraphicComponent
{
public:
    DECLARE_COMPONENT(Tutorial);

    void OnInitGraphics(Renderer* renderer, int layer);

    void Render(Renderer& renderer);

    void SetShroudActive(bool active);

    void SetShroud(Rect targetArea);

    void SetPopupActive(bool active);

    void SetPopupPosition(Point position);

    void SetPopupText(string text);

    void SetCentral();

    void AddPopup(string text);

    void AddPopup(string text, Point p);

    void AddPopup(string text, Point p, Rect targetArea);

    void GoNext();

    int GetStage();

    bool HasPopups();

    void ShowPopup(bool immediate = false);

    void Update();

    Button* okay;

    InputGUI* gui;

    int stage = TutorialStages::INTRO;

private:
    Rect shrouds[4];

    bool onNextUpdate = false;

    Image buttonTexture;

    queue<string> texts;
    queue<Rect> targets;
    queue<Point> positions;

    Text* popupText;
    Text* buttonText;

    bool shroudActive = false;
    bool popupActive = false;

};

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

    void OnDestroy();

    void Log(string message, SDL_Color colour = colours::WHITE);

    void SetTutorial(Tutorial* t);

    bool IsTutorial();

    int GetTutorialStage();

    Tutorial* tutorial = nullptr;

    /// This is the parent clock; all other clocks in the game should use this clock's delta time.
    /// That means that all clocks will pause when this clock pauses and so on.
    Clock gameTime;

private:
    bool isTutorial = false;

    int oldScore = 0;
    float changeAlpha = 0;
    Uint8 alpha = 0;

    Text* scoreChangeText;

    GraphicRect r;

    Renderer* render;

    bool finished = false;

    void EndGame();

    Text* endText;

    Text* scoreText;

    Text* timeLeftText;

    CircularBuffer<string>* messageStream;
    Text** messageTexts;

    bool cycleMessages = false;

    Clock timeLeft;

    Clock eventTimer;

    set<NodeClient*> clients;

    set<int> targetNodes;

    Uint32 eventTimeThreshold = 5000;

    InputContext context;

    MouseHandler* mouse;

};

extern Game* theGame;

#endif // GAME_H
