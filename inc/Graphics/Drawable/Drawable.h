#pragma once

#include <Graphics/KDRenderer.h>

#include <Graphics/Bindable/Buffers.h>

#include <DirectXMath.h>
#include <vector>

namespace KDE
{
    class Drawable
    {
        public:
            Drawable() = default;
			Drawable(const Drawable&) = delete;
			Drawable& operator = (const Drawable&) = delete;
            virtual ~Drawable() = default;

            void AddBind(class Bindable* bind);

            void Draw(KDRenderer& renderer);
            virtual void Update(KDRenderer& renderer, float deltaTime) = 0;

            virtual DirectX::XMMATRIX GetModelMatrix() const = 0;

        protected:
            std::vector<Bindable*> m_Binds;
            class IndexBuffer* m_IndexBuffer;
    };
}