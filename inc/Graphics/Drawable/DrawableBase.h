#pragma once
#include "Drawable.h"

namespace KDE
{
    template <typename T>
    class DrawableBase : Drawable
    {
        public:
            bool IsStaticInitialized() const
            {

            }

        private:
            static std::vector<std::unique<Bindable>> m_StaticBinds;
    };
}