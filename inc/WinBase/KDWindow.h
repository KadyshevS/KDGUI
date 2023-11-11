#pragma once

namespace std
{
    template <class _Fty>
    class function;
}

namespace KDE
{
    class KDWindow
    {
        friend class KDRenderer;

        private:
            mutable struct KDWindowData* m_Data;

        public:
            KDWindow(int width, int height, const char* title);
            KDWindow(const KDWindow&) = delete;
            KDWindow& operator = (const KDWindow&) = delete;
            ~KDWindow();

            void SetEventFunc(std::function<void(class Event&)> eventFunc);
            inline KDWindowData* GetNativeData() const { return m_Data; }

            void Update();
            bool ShouldClose();
            int GetWidth() const;
            int GetHeight() const;

            void ShowCursor();
            void HideCursor();
            bool IsCursorEnabled() const;

            static void ShowMessageBox(const char* text, const char* caption);
    };
}