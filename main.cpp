#include "Ossium/src/Ossium.h"
#include "src/Button.h"
#include "src/game.h"

using namespace std;

using namespace Ossium;
using namespace Ossium::global;

int main(int argc, char* argv[])
{
    bool quit = false;
    if (InitialiseOssium() < 0)
    {
        printf("ERROR: Failed to initialise Ossium.\n");
    }
    else
    {
        /// Load configuration settings
        Config settings;
        LoadConfig(&settings);

        /// Create the window
        Window mainWindow("Please Hold", 1024, 768, settings.fullscreen, SDL_WINDOW_SHOWN);

        /// Create renderer
        Renderer mainRenderer(&mainWindow, 15, settings.vsync ? SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC : SDL_RENDERER_ACCELERATED);
        mainRenderer.SetAspectRatio(4, 3);

        /// Input and ECS
        InputController mainInput;
        EntityComponentSystem ECS;

        /// Load resources
        Image buttonTexture;
        if (!buttonTexture.LoadAndInit("Textures/menubutton.png", mainRenderer))
        {
            SDL_Log("Failed to load menu button!");
            return -1;
        }
        Font mainFont;
        int psizes = -1;
        if (!mainFont.Load("Fonts/Orkney Regular.ttf", &psizes))
        {
            SDL_Log("Failed to load font!");
            return 0xFFFF;
        }

        font = &mainFont;

        /// Button + GUI system
        Entity& guiSys = *ECS.CreateEntity();
        Entity& button = *ECS.CreateEntity(&guiSys);
        InputGUI& gui = *guiSys.AddComponent<InputGUI>(&mainRenderer);
        Button& b = *button.AddComponent<Button>(&mainRenderer);
        mainInput.AddContext("gui", &gui);
        /// Button text
        Text& text = *button.AddComponent<Text>(&mainRenderer, 1);
        b.sprite->position = Point(mainRenderer.GetWidth() / 2, (mainRenderer.GetHeight() / 6) * 4);
        text.position = b.sprite->position;
        text.SetText("Play");
        text.SetColor(colours::BLACK);
        text.SetRenderMode(TextRenderModes::RENDERTEXT_BLEND);
        text.TextToTexture(mainRenderer, &mainFont, 48);
        b.sprite->AddState("default", &buttonTexture, true, 3);
        /// Clone stuff to make another button, then change certain bits
        Entity& quitButton = *button.Clone();
        quitButton.GetComponent<StateSprite>()->position = Point(mainRenderer.GetWidth() / 2, (mainRenderer.GetHeight() / 6) * 5);
        quitButton.GetComponent<Text>()->position = quitButton.GetComponent<StateSprite>()->position;
        quitButton.GetComponent<Text>()->SetText("Quit");

        Entity& tutorialButton = *button.Clone();
        tutorialButton.GetComponent<StateSprite>()->position = Point(mainRenderer.GetWidth() / 2, (mainRenderer.GetHeight() / 6) * 3);
        tutorialButton.GetComponent<Text>()->position = tutorialButton.GetComponent<StateSprite>()->position;
        tutorialButton.GetComponent<Text>()->SetText("Tutorial");

        gui.AddInteractable("menuhome", *tutorialButton.GetComponent<Button>());
        gui.AddInteractable("menuhome", b);
        gui.AddInteractable("menuhome", *quitButton.GetComponent<Button>());

        Entity& world = *ECS.CreateEntity();
        Game* game = nullptr;
        InputGUI& tutorialGui = *world.AddComponent<InputGUI>(&mainRenderer);
        mainInput.AddContext("tutorial", &tutorialGui);

        int quitButtonHandle = quitButton.GetComponent<Button>()->OnClicked += [&] (Button& bcaller) { quit = true; };
        int clickHandle = b.OnClicked += [&] (Button& bcaller) {
            SDL_Log("Starting game...");
            if (game == nullptr)
            {
                gui.SetActive(false);
                mainRenderer.Unregister(quitButton.GetComponent<StateSprite>());
                mainRenderer.Unregister(button.GetComponent<StateSprite>());
                mainRenderer.Unregister(tutorialButton.GetComponent<StateSprite>());
                game = world.AddComponent<Game>(&mainRenderer);
                game->SetupInput(&mainInput);
            }
        };
        bool setupTutorial = false;

        int tutorialHandle = tutorialButton.GetComponent<Button>()->OnClicked += [&] (Button& bcaller) {
            SDL_Log("Starting tutorial...");
            if (game == nullptr)
            {
                mainRenderer.Unregister(quitButton.GetComponent<StateSprite>());
                mainRenderer.Unregister(button.GetComponent<StateSprite>());
                mainRenderer.Unregister(tutorialButton.GetComponent<StateSprite>());
                game = world.AddComponent<Game>(&mainRenderer);
                game->SetupInput(&mainInput);
                Tutorial* t = world.AddComponent<Tutorial>(&mainRenderer, 9);
                tutorialGui.AddInteractable("tutorial", *t->okay);
                game->SetTutorial(t);
                gui.SetActive(false);
                t->gui = &tutorialGui;
                t->SetCentral();
                t->SetPopupActive(true);
                t->SetShroudActive(true);
                t->SetPopupText("Welcome to PLEASE HOLD, the switchboard operator game!");
                t->AddPopup("This is your switchboard. Your job is to connect telephone lines.", Point(1024 / 2, 768 / 4 * 3));
                t->AddPopup("The main section has several 6 rows of extension number connections.", Point(1024 / 2, 768 / 4 * 3));
                t->AddPopup("When someone makes a call, a light will turn on under the corresponding extension.", Point(1024 / 2, 768 / 4 * 3));
                t->AddPopup("Let's wait a few moments for someone to call.", Point(1024 / 2, 768 / 4 * 3));
                setupTutorial = true;
            }
        };

        SDL_Event e;

        delta.Init(settings);
        while (!quit)
        {
            /// Input handling phase
            while (SDL_PollEvent(&e) != 0)
            {
                mainWindow.handle_events(e);
                if (e.type == SDL_QUIT)
                {
                    quit = true;
                    break;
                }
                mainInput.HandleEvent(e);
            }
            /// Logic phase
            ECS.UpdateComponents();

            /// Rendering phase
            mainRenderer.RenderPresent();

            /// Time update
            delta.Update();
        }
        quitButton.GetComponent<Button>()->OnClicked -= quitButtonHandle;
        tutorialButton.GetComponent<Button>()->OnClicked -= tutorialHandle;
        b.OnClicked -= clickHandle;
    }

    TerminateOssium();
    return 0;
}
