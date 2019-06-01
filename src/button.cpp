#include "button.h"

using namespace Ossium;
using namespace Ossium::input;
using namespace Ossium::structs;

inline namespace UI
{

    REGISTER_COMPONENT(Button);

    void Button::OnInitGraphics(Renderer* renderer, int layer)
    {
        GraphicComponent::OnInitGraphics(renderer, layer);
        /// We can't inherit from StateSprite as we already inherit indirectly from GraphicComponent,
        /// but we can add our own instance to the ECS.
        sprite = entity->GetComponent<StateSprite>();
        if (sprite == nullptr)
        {
            sprite = entity->AddComponent<StateSprite>(renderer, layer);
        }
    }

    void Button::Render(Renderer& renderer)
    {
    }

    bool Button::ContainsPointer(Point position)
    {
        return sprite->GetRect().Contains(position);
    }

    void Button::OnClick()
    {
        sprite->ChangeSubState(1);
        OnClicked(*this);
    }

    void Button::OnPointerDown()
    {
        sprite->ChangeSubState(2);
    }

    void Button::OnPointerUp()
    {
        sprite->ChangeSubState(0);
    }

    void Button::OnHoverBegin()
    {
        if (!IsPressed())
        {
            sprite->ChangeSubState(1);
        }
    }

    void Button::OnHoverEnd()
    {
        if (!IsPressed())
        {
            sprite->ChangeSubState(0);
        }
    }

}
