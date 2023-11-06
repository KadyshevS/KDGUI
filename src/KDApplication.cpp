#include <precomp.h>

#include <WinBase/KDApplication.h>
#include <WinBase/KDWindow.h>
#include <Graphics/KDRenderer.h>

#include <Events/KDEvent.h>
#include <Events/ApplicationEvent.h>
#include <Events/MouseEvent.h>
#include <Events/KeyEvent.h>

#include <Input/Input.h>

namespace KDE
{
    void KDApplication::Run()
    {
        window = new KDWindow(800, 600, "Test app");
        window->SetEventFunc(std::bind(&KDApplication::OnEvent, this, std::placeholders::_1));

        renderer = new KDRenderer(window);

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
    void KDApplication::OnEvent(KDEvent& e)
    {
        switch(e.GetEventType())
        {
            case KDEventType::KeyPressed:
                {
                    Input::OnKeyPressed( ((KeyPressedEvent&)e).GetKeyCode() );
                    break;
                }
            case KDEventType::KeyReleased:
                {
                    Input::OnKeyReleased( ((KeyReleasedEvent&)e).GetKeyCode() );
                    break;
                }
            case KDEventType::KeyChar:
                {
                    // TODO
                    break;
                }
            case KDEventType::MouseButtonPressed:
                {
                    Input::OnMouseButtonPressed( ((MouseButtonPressedEvent&)e).GetMouseButton() );
                    break;
                }
            case KDEventType::MouseButtonReleased:
                {
                    Input::OnMouseButtonReleased( ((MouseButtonReleasedEvent&)e).GetMouseButton() );
                    break;
                }
            case KDEventType::MouseScrolled:
                {
                    Input::OnMouseWheelScroll( ((MouseScrolledEvent&)e).GetDeltaY() );
                    break;
                }
            case KDEventType::MouseMoved:
                {
                    auto& riE = (MouseMovedEvent&)e;
                    Input::OnMouseMoved( (int)riE.GetX(), (int)riE.GetY() );
                    break;
                }
            case KDEventType::MouseRaw:
                {
                    auto& riE = (MouseRawEvent&)e;
                    Input::OnMouseRaw( (int)riE.GetDeltaX(), (int)riE.GetDeltaY() );
                    break;
                }
            case KDEventType::AppUpdate:
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
