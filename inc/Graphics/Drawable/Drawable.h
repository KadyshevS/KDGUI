#pragma once

#include <Graphics/KDRenderer.h>

#include <Graphics/Bindable/Buffers.h>

#include <DirectXMath.h>
#include <vector>
#include <memory>

namespace KDE
{
    class Drawable
    {
        public:
            Drawable() = default;
			Drawable(const Drawable&) = delete;
			Drawable& operator = (const Drawable&) = delete;
            ~Drawable();

            void AddBind(std::unique_ptr<Bindable> bind);

            void Draw(KDRenderer& renderer);
            virtual void Update(KDRenderer& renderer, float deltaTime) = 0;

            virtual DirectX::XMMATRIX GetModelMatrix() const = 0;

        protected:
            std::vector<std::unique_ptr<Bindable>> m_Binds;
            const IndexBuffer* m_IndexBuffer = nullptr;
    };
}