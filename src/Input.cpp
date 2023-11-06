#include <precomp.h>

#include <Input/Input.h>

namespace KDE
{
    std::map<int, bool> Input::m_KeyStates = {};
    std::map<int, bool> Input::m_MouseStates = {};
    std::pair<int, int> Input::m_MousePos = {0,0};
    std::pair<int, int> Input::m_MouseRaw = {0,0};
    int Input::m_MouseWheelDelta = 0;
    bool Input::m_MouseIsMoving = false;

    void Input::OnKeyPressed(int keycode)
    {
        m_KeyStates[keycode] = true;
    }
    void Input::OnKeyReleased(int keycode)
    {
        m_KeyStates[keycode] = false;
    }
    void Input::OnMouseButtonPressed(int mousecode)
    {
        m_MouseStates[mousecode] = true;
    }
    void Input::OnMouseButtonReleased(int mousecode)
    {
        m_MouseStates[mousecode] = false;
    }
    void Input::OnMouseWheelScroll(int delta)
    {
        m_MouseWheelDelta = delta;
    }
    void Input::OnMouseMoved(int xPos, int yPos)
    {
        m_MousePos.first = xPos;
        m_MousePos.second = yPos;
        m_MouseIsMoving = true;
    }
    void Input::OnMouseRaw(int xDelta, int yDelta)
    {
        m_MouseRaw.first = xDelta;
        m_MouseRaw.second = yDelta;
    }
    void Input::OnMouseStopped()
    {
        m_MouseIsMoving = false;
    }
}
