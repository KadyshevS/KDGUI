#pragma once
#include "Drawable.h"

#include <random>

namespace KDE
{
    class TexturedCube : public Drawable
    {
        public:
            TexturedCube(KDRenderer& renderer, std::mt19937& rng,
                std::uniform_real_distribution<float>& adist,
                std::uniform_real_distribution<float>& bdist,
                std::uniform_real_distribution<float>& cdist,
                std::uniform_real_distribution<float>& ddist);

            void Update(KDRenderer& renderer, float deltaTime) override;
            DirectX::XMMATRIX GetModelMatrix() const override;

        private:
            struct CBuffer
            {
                DirectX::XMMATRIX model;
            };

            float r;
            float lRoll = 0.0f, lPitch = 0.0f, lYaw = 0.0f;
            float gRoll = 0.0f, gPitch = 0.0f, gYaw = 0.0f;

            float lRollD = 0.0f, lPitchD = 0.0f, lYawD = 0.0f;
            float gRollD = 0.0f, gPitchD = 0.0f, gYawD = 0.0f;
    };
}