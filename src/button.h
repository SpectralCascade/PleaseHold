#ifndef BUTTON_H
#define BUTTON_H

#include "../../Ossium/src/Ossium.h"

using namespace Ossium;
using namespace Ossium::input;
using namespace Ossium::graphics;

inline namespace UI
{

    class Button : public InteractableGUI
    {
    public:
        DECLARE_COMPONENT(Button);

        /// Adds a sprite component and sets up a direct reference to it.
        void OnInitGraphics(Renderer* renderer, int layer = -1);

        /// Does nothing because we have a separate sprite component for the actual graphics.
        void Render(Renderer& renderer);

        /// Override to use sprite rect.
        bool ContainsPointer(Point position);

        /// Callback registry that is called whenever the button is clicked
        Callback<Button> OnClicked;

        /// Reference to the sprite component this button requires.
        StateSprite* sprite = nullptr;

    protected:
        void OnClick();
        void OnHoverBegin();
        void OnHoverEnd();
        void OnPointerDown();
        void OnPointerUp();

    };

}

#endif // BUTTON_H
