#include <precomp.h>

#include <WinBase/KDApplication.h>

#include <Events/Event.h>
#include <Events/ApplicationEvent.h>
#include <Events/MouseEvent.h>
#include <Events/KeyEvent.h>

#include <Input/Input.h>

namespace KDE
{
    void KDApplication::Run()
    {
        window = std::make_shared<KDWindow>(800, 600, "Test app");
        window->SetEventFunc(std::bind(&KDApplication::OnEvent, this, std::placeholders::_1));

        renderer = std::make_shared<KDRenderer>(window);

        OnCreate();
        while(!window->ShouldClose())
        {
            renderer->BeginFrame();
            OnUpdate();
            renderer->EndFrame();

            window->Update();
        }
        OnDestroy();
    }
    void KDApplication::OnEvent(Event& e)
    {
        switch(e.GetEventType())
        {
            case EventType::KeyPressed:
                {
                    Input::OnKeyPressed( ((KeyPressedEvent&)e).GetKeyCode() );
                    break;
                }
            case EventType::KeyReleased:
                {
                    Input::OnKeyReleased( ((KeyReleasedEvent&)e).GetKeyCode() );
                    break;
                }
            case EventType::KeyChar:
                {
                    // TODO
                    break;
                }
            case EventType::MouseButtonPressed:
                {
                    Input::OnMouseButtonPressed( ((MouseButtonPressedEvent&)e).GetMouseButton() );
                    break;
                }
            case EventType::MouseButtonReleased:
                {
                    Input::OnMouseButtonReleased( ((MouseButtonReleasedEvent&)e).GetMouseButton() );
                    break;
                }
            case EventType::MouseScrolled:
                {
                    Input::OnMouseWheelScroll( ((MouseScrolledEvent&)e).GetDeltaY() );
                    break;
                }
            case EventType::MouseMoved:
                {
                    auto& riE = (MouseMovedEvent&)e;
                    Input::OnMouseMoved( (int)riE.GetX(), (int)riE.GetY() );
                    break;
                }
            case EventType::MouseRaw:
                {
                    auto& riE = (MouseRawEvent&)e;
                    Input::OnMouseRaw( (int)riE.GetDeltaX(), (int)riE.GetDeltaY() );
                    break;
                }
            case EventType::AppUpdate:
                {
                    Input::OnMouseRaw(0, 0);
                    Input::OnMouseWheelScroll( 0 );
                    Input::OnMouseStopped();
                    break;
                }
            default:
                break;
        }
    }
}
