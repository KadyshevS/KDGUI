#include <precomp.h>
#include <Graphics/Drawable/TexturedCube.h>

#include <Graphics/Bindable/Bindables.h>

namespace KDE
{
    TexturedCube::TexturedCube(KDRenderer& renderer, std::mt19937& rng,
        std::uniform_real_distribution<float>& adist,
        std::uniform_real_distribution<float>& bdist,
        std::uniform_real_distribution<float>& cdist,
        std::uniform_real_distribution<float>& ddist)
        :
        r(adist(rng)),
        lRollD(bdist(rng)),
        lPitchD(bdist(rng)),
        lYawD(bdist(rng)),
        gRollD(cdist(rng)),
        gPitchD(cdist(rng)),
        gYawD(cdist(rng)),
        gRoll(ddist(rng)),
        gPitch(ddist(rng)),
        gYaw(ddist(rng))
    {
        namespace dx = DirectX;

        struct Vertex
        {
            dx::XMFLOAT3 pos;
            dx::XMFLOAT2 tex;
        };

        std::vector<Vertex> vertices =
        {
            { {-1, -1, -1}, {0, 1} },
            { {-1,  1, -1}, {0, 0} },
            { { 1,  1, -1}, {1, 0} },
            { { 1, -1, -1}, {1, 1} },

            { {-1, -1,  1}, {0, 1} },
            { {-1,  1,  1}, {0, 0} },
            { {-1,  1, -1}, {1, 0} },
            { {-1, -1, -1}, {1, 1} },

            { { 1, -1,  1}, {0, 1} },
            { { 1,  1,  1}, {0, 0} },
            { {-1,  1,  1}, {1, 0} },
            { {-1, -1,  1}, {1, 1} },

            { { 1, -1, -1}, {0, 1} },
            { { 1,  1, -1}, {0, 0} },
            { { 1,  1,  1}, {1, 0} },
            { { 1, -1,  1}, {1, 1} },

            { {-1,  1, -1}, {0, 1} },
            { {-1,  1,  1}, {0, 0} },
            { { 1,  1,  1}, {1, 0} },
            { { 1,  1, -1}, {1, 1} },

            { {-1, -1,  1}, {0, 1} },
            { {-1, -1, -1}, {0, 0} },
            { { 1, -1, -1}, {1, 0} },
            { { 1, -1,  1}, {1, 1} },
        };
        std::vector<uint32_t> indices =
        {
            0, 1, 2,    0, 2, 3,
            4, 5, 6,    4, 6, 7,
            8, 9, 10,   8, 10, 11,
            12, 13, 14,   12, 14, 15,
            16, 17, 18,   16, 18, 19,
            20, 21, 22,   20, 22, 23,
        };

        AddBind(std::make_unique<VertexConstantBuffer<CBuffer>>(renderer));

        AddBind(std::make_unique<VertexBuffer<Vertex>>(renderer, vertices));

        AddBind(std::make_unique<IndexBuffer>(renderer, indices));
        
        auto vShader = std::make_unique<VertexShader>(renderer, "assets/shaders/VSTexture.cso");
        auto vShaderBlob = vShader->GetBytecode();
        AddBind(std::move(vShader));

        AddBind(std::make_unique<PixelShader>(renderer, "assets/shaders/PSTexture.cso"));

        AddBind(std::make_unique<InputLayout>(renderer,
            std::vector<LayoutElement>
            {
                { "Position", InputType_Float3 },
                { "TexCoord", InputType_Float2 }
            },
            vShaderBlob)
        );

        AddBind(std::make_unique<Topology>(renderer, TopologyType_TriangleList));

        AddBind(std::make_unique<Sampler>(renderer));

        AddBind(std::make_unique<Texture>(renderer, "assets/images/cube.jpg"));
    }

    void TexturedCube::Update(KDRenderer& renderer, float deltaTime)
    {
        lRoll += lRollD * deltaTime;
        lPitch += lPitchD * deltaTime;
        lYaw += lYawD * deltaTime;
        gRoll += gRollD * deltaTime;
        gPitch += gPitchD * deltaTime;
        gYaw += gYawD * deltaTime;

        ( (VertexConstantBuffer<CBuffer>*)m_Binds[0].get() )->Update(renderer, 
        {
            GetModelMatrix()
        });
    }
    DirectX::XMMATRIX TexturedCube::GetModelMatrix() const
    {
        namespace dx = DirectX;

        return dx::XMMatrixRotationRollPitchYaw(lRoll, lPitch, lYaw) *
            dx::XMMatrixTranslation(r, 0.0f, 0.0f) *
            dx::XMMatrixRotationRollPitchYaw(gRoll, gPitch, gYaw) *
            dx::XMMatrixTranslation(0.0f, 0.0f, 20.0f);
    }
}