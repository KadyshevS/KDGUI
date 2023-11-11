#pragma once

#include <WinBase/KDWindow.h>
#include <Graphics/KDRenderer.h>

#include <memory>

namespace KDE
{
    class KDApplication
    {
        protected:
            std::shared_ptr<KDWindow> window;
            std::shared_ptr<KDRenderer> renderer;

        protected:
            void OnEvent(class Event& e);

        public:
            void Run();

            virtual void OnCreate() {}
            virtual void OnUpdate() {}
            virtual void OnDestroy() {}
    };
}