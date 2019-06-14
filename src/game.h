#ifndef GAME_H
#define GAME_H

#include <queue>

#include "../Ossium/src/Ossium.h"
#include "../Ossium/src/Ossium/randutils.h"
#include "switchboard.h"

using namespace std;
using namespace Ossium;

extern Font* font;

extern int MAX_WAIT_TIME;
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

extern AudioSource jack;
extern AudioSource caller;
extern AudioSource scoreAudio;
extern AudioBus master;

extern AudioClip plugin;
extern AudioClip plugout;
extern AudioClip ding;
extern AudioClip badscore;
extern AudioClip goodscore;

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

    Rand* rng = nullptr;

    void OnDestroy();

    void Log(string message, SDL_Color colour = Colors::WHITE);

    void ClearLog();

    void SetTutorial(Tutorial* t);

    bool IsTutorial();

    void PruneClients();

    void Restart();

    int GetTutorialStage();

    Tutorial* tutorial = nullptr;

    /// This is the parent clock; all other clocks in the game should use this clock's delta time.
    /// That means that all clocks will pause when this clock pauses and so on.
    Clock gameTime;

    bool isTutorial = false;

    Image* regularButton = nullptr;

private:
    int oldScore = 0;
    float changeAlpha = 0;
    Uint8 alpha = 0;

    int clickRestartHandle = 0;

    Text* scoreChangeText = nullptr;

    GraphicRect r;

    Entity* restartButton = nullptr;

    Renderer* render = nullptr;

    bool finished = false;

    void EndGame();

    Text* endText = nullptr;

    Text* scoreText = nullptr;

    Text* timeLeftText = nullptr;

    CircularBuffer<string>* messageStream;
    Text** messageTexts = nullptr;

    bool cycleMessages = false;

    Clock timeLeft;

    Clock eventTimer;

    set<NodeClient*> clients;

    set<int> targetNodes;

    Uint32 eventTimeThreshold = 5000;

    InputContext context;

    InputGUI* inputGui = nullptr;

    MouseHandler* mouse = nullptr;

    bool doRestart = false;

};

extern Game* theGame;

#endif // GAME_H
