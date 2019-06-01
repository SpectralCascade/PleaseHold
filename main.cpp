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
        Renderer mainRenderer(&mainWindow, 5, settings.vsync ? SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC : SDL_RENDERER_ACCELERATED);
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
        gui.AddInteractable("menuhome", b);
        mainInput.AddContext("gui", &gui);
        /// Button text
        Text& text = *button.AddComponent<Text>(&mainRenderer, 1);
        b.sprite->position = Point(mainRenderer.GetWidth() / 2, mainRenderer.GetHeight() / 2);
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
        gui.AddInteractable("menuhome", *quitButton.GetComponent<Button>());

        Entity& world = *ECS.CreateEntity();
        Game* game = nullptr;

        int quitButtonHandle = quitButton.GetComponent<Button>()->OnClicked += [&] (Button& bcaller) { quit = true; };
        int clickHandle = b.OnClicked += [&] (Button& bcaller) {
            SDL_Log("Starting game...");
            if (game == nullptr)
            {
                gui.SetActive(false);
                game = world.AddComponent<Game>(&mainRenderer);
                game->SetupInput(&mainInput);
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
        b.OnClicked -= clickHandle;
    }

    TerminateOssium();
    return 0;
}
