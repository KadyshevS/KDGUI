#pragma once 

#include <Codes/KeyCodes.h>
#include <Codes/MouseCodes.h>

#include <utility>
#include <map>

namespace KDE
{
    class Input
    {
        friend class KDApplication;

        private:
            static std::map<int, bool> m_KeyStates;
            static std::map<int, bool> m_MouseStates;
            static std::pair<int, int> m_MousePos;
            static std::pair<int, int> m_MouseRaw;
            static int m_MouseWheelDelta;
            static bool m_MouseIsMoving;

            static void OnKeyPressed(int keycode);
            static void OnKeyReleased(int keycode);
            static void OnMouseButtonPressed(int mousecode);
            static void OnMouseButtonReleased(int mousecode);
            static void OnMouseWheelScroll(int delta);
            static void OnMouseMoved(int xPos, int yPos);
            static void OnMouseRaw(int xDelta, int yDelta);
            static void OnMouseStopped();

        public:
            static bool IsKeyPressed(KeyCode keycode) { return m_KeyStates[keycode]; }
            static bool IsMouseButtonPressed(MouseCode mousecode) { return m_MouseStates[mousecode]; }
            static bool IsMouseMoving() { return m_MouseIsMoving; }
            static const std::pair<int,int>& GetMousePosition() { return m_MousePos; }
            static int GetMousePositionX() { return m_MousePos.first; }
            static int GetMousePositionY() { return m_MousePos.second; }
            static const std::pair<int, int>& GetMouseRaw() { return m_MouseRaw; }
            static int GetMouseRawX() { return m_MouseRaw.first; }
            static int GetMouseRawY() { return m_MouseRaw.second; }
            static int GetMouseWheelDelta() { return m_MouseWheelDelta; }
    };
}