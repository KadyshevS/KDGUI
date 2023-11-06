#pragma once

namespace KDE
{
    class KDApplication
    {
        protected:
            class KDWindow* window;
            class KDRenderer* renderer;

        protected:
            void OnEvent(class KDEvent& e);

        public:
            void Run();

            virtual void OnCreate() {}
            virtual void OnUpdate() {}
            virtual void OnDestroy() {}
    };
}