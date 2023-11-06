#include <precomp.h>
#include <Graphics/KDRenderer.h>

#include <WinBase/KDWindow.h>
#include <Graphics/Bindable/Bindables.h>
#include <Input/Input.h>
#include <Utils/FrameTimer.h>

#include <Debug/DXGIInfoManager.h>
#include <Debug/GfxExcept.h>

#include <Events/KDEvent.h>

float WrapAngle(float x)
{
    x = fmod(x, 3.14f * 2.0f);
    if (x < 0)
        x += 3.14f * 2.0f;
    return x;
}

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

struct KDRendererData_dx11
{
    KDE::KDWindow* pOutputWindow = nullptr;
    KDE::DXGIInfoManager m_InfoManager;

    wrl::ComPtr<ID3D11Device>               pDevice;
    wrl::ComPtr<ID3D11DeviceContext>        pContext;
    wrl::ComPtr<IDXGISwapChain>             pSwapChain;
    wrl::ComPtr<ID3D11RenderTargetView>     pTarget;
    wrl::ComPtr<ID3D11DepthStencilView>     pDSView;

    KDE::Texture* pDefaultTexture;

    KDE::Timer pTimer;
    float m_DeltaTime = 0.0f;
    bool m_VSync = true;

    dx::XMFLOAT3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
    dx::XMFLOAT2 m_CameraLook = { 0.0f, 0.0f };
};
struct KDWindowData_win32
{
    bool IsShouldClose;
    bool IsCursorEnabled;
    int Width, Height;
    std::string Title;
    std::function<void(KDE::KDEvent&)> EventFunc;
    std::vector<BYTE> RawBuffer;

    HINSTANCE hInstance;
    HWND hWnd;
    MSG msg;
};

namespace KDE
{
    KDRenderer::KDRenderer(KDWindow* outputWindow)
    {
        HRESULT hr;

        m_Data = new KDRendererData_dx11{};

        KDRendererData_dx11* riData = (KDRendererData_dx11*)m_Data;
        KDWindowData_win32* riWinData = (KDWindowData_win32*)outputWindow->m_Data;

        DXGI_SWAP_CHAIN_DESC scd{};
        scd.BufferDesc.Width = 0;
        scd.BufferDesc.Height = 0;
        scd.BufferDesc.RefreshRate.Numerator = 0;
        scd.BufferDesc.RefreshRate.Denominator = 0;
        scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

        scd.SampleDesc.Count = 1;
        scd.SampleDesc.Quality = 0;

        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.BufferCount = 1;
        scd.OutputWindow = riWinData->hWnd;
        scd.Windowed = TRUE;
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        scd.Flags = 0;
        
        UINT creationFlags = 0;

        #ifndef NDEBUG
            creationFlags = D3D11_CREATE_DEVICE_DEBUG;
        #endif

        auto& infoManager = GetInfoManager();

        riData->pOutputWindow = outputWindow;
        KDGFX_THROW_INFO( D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            creationFlags, nullptr, 0, D3D11_SDK_VERSION, &scd,
            &riData->pSwapChain, &riData->pDevice,
            nullptr, &riData->pContext
        ) );

        assert( "Failed to create D3D11 Device and DXGI SwapChain" && SUCCEEDED(hr) );

        wrl::ComPtr<ID3D11Resource> pBackBuffer;
        KDGFX_THROW_INFO( riData->pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer) );
        KDGFX_THROW_INFO( riData->pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &riData->pTarget) );

     ////////////////////   Depth Stencil stuff  ////////////////////
        D3D11_DEPTH_STENCIL_DESC dsDesc{};
        dsDesc.DepthEnable = TRUE;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
        wrl::ComPtr<ID3D11DepthStencilState> pDSState;
        KDGFX_THROW_INFO( riData->pDevice->CreateDepthStencilState(&dsDesc, &pDSState) );

        riData->pContext->OMSetDepthStencilState(pDSState.Get(), 1);

        wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
        D3D11_TEXTURE2D_DESC depthDesc{};
        depthDesc.Width = riWinData->Width;
        depthDesc.Height = riWinData->Height;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.SampleDesc.Quality = 0;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthDesc.CPUAccessFlags = 0;
        depthDesc.MiscFlags = 0;
        KDGFX_THROW_INFO( riData->pDevice->CreateTexture2D(&depthDesc, nullptr, &pDepthStencil) );

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        KDGFX_THROW_INFO( riData->pDevice->CreateDepthStencilView(
            pDepthStencil.Get(), 
            &dsvDesc, &riData->pDSView) );
     ////////////////////////////////////////////////////////////

        riData->pContext->OMSetRenderTargets(1, riData->pTarget.GetAddressOf(), riData->pDSView.Get());

        D3D11_VIEWPORT vp{};
        vp.Width = (FLOAT)outputWindow->GetWidth();
        vp.Height = (FLOAT)outputWindow->GetHeight();
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        riData->pContext->RSSetViewports(1, &vp);

        riData->pDefaultTexture = new Texture(*this, "assets/images/cube.jpg");

        UpdateCamera();
    }
    KDRenderer::~KDRenderer()
    {
        delete (KDRendererData_dx11*)m_Data;
    }

    DXGIInfoManager& KDRenderer::GetInfoManager() const
    {
        return ( (KDRendererData_dx11*)m_Data )->m_InfoManager;
    }
    ID3D11Device& KDRenderer::GetDevice() const
    {
        return *( (KDRendererData_dx11*)m_Data )->pDevice.Get();
    }
    ID3D11DeviceContext& KDRenderer::GetContext() const
    {
        return *( (KDRendererData_dx11*)m_Data )->pContext.Get();
    }
    
    void KDRenderer::MoveCameraTo(float offX, float offY, float offZ)
    {
        auto riData = (KDRendererData_dx11*)m_Data;

        dx::XMFLOAT3 translation{offX, offY, offZ};

        dx::XMStoreFloat3( 
            &translation, 
            dx::XMVector3Transform( 
                dx::XMLoadFloat3(&translation),
                dx::XMMatrixRotationRollPitchYaw(riData->m_CameraLook.y, riData->m_CameraLook.x, 0.0f) )
        );

        riData->m_CameraPosition.x += translation.x;
        riData->m_CameraPosition.y += translation.y;
        riData->m_CameraPosition.z += translation.z;
    }
    void KDRenderer::MoveCameraAt(float posX, float posY, float posZ)
    {
        auto riData = (KDRendererData_dx11*)m_Data;

        riData->m_CameraPosition.x = posX;
        riData->m_CameraPosition.y = posY;
        riData->m_CameraPosition.z = posZ;
    }
    void KDRenderer::RotateCameraTo(float offX, float offY)
    {
        auto riData = (KDRendererData_dx11*)m_Data;
        riData->m_CameraLook.x = WrapAngle(riData->m_CameraLook.x + offX);
        riData->m_CameraLook.y = std::clamp(riData->m_CameraLook.y + offY, -3.14f / 2.0f, 3.14f / 2.0f);
    }
    void KDRenderer::RotateCameraAt(float rotX, float rotY)
    {
        auto riData = (KDRendererData_dx11*)m_Data;
        riData->m_CameraLook.x = WrapAngle(rotX);
        riData->m_CameraLook.y = std::clamp(rotY, -3.14f / 2.0f, 3.14f / 2.0f);
    }
    void KDRenderer::UpdateCamera()
    {
        KDRendererData_dx11* riData = (KDRendererData_dx11*)m_Data;
        KDWindowData_win32* riWinData = (KDWindowData_win32*)riData->pOutputWindow->m_Data;

        static bool isFreeLookMode = true;
        static bool isFreeLookMode_Press = false;

        riData->pOutputWindow->ShowCursor();

        if (Input::IsKeyPressed(Key::Escape))
        {
            if (!isFreeLookMode_Press)
            {
                isFreeLookMode = !isFreeLookMode;
                isFreeLookMode_Press = true; 
            }
        }
        else
        {
            isFreeLookMode_Press = false;
        }

        if (!isFreeLookMode) { riData->pOutputWindow->ShowCursor(); return; }
        else { riData->pOutputWindow->HideCursor(); }

        bool cPress = Input::IsMouseButtonPressed(KDE::Mouse::LeftMouseBtn);
        
        float xRaw = (float)Input::GetMouseRawX();
        float yRaw = (float)Input::GetMouseRawY();

        bool isWalkForward = Input::IsKeyPressed(KDE::Key::W);
        bool isWalkBack = Input::IsKeyPressed(KDE::Key::S);
        bool isWalkLeft = Input::IsKeyPressed(KDE::Key::A);
        bool isWalkRight = Input::IsKeyPressed(KDE::Key::D);
        bool isWalkUp = Input::IsKeyPressed(KDE::Key::R);
        bool isWalkDown = Input::IsKeyPressed(KDE::Key::F);

        float camVelocity = 5.0f * GetDeltaTime();
        float camRotVelocity = 2.0f * GetDeltaTime();

        RotateCameraTo(camRotVelocity * xRaw, camRotVelocity * yRaw);

        if (isWalkForward)
            MoveCameraTo(0.0f, 0.0f, camVelocity);
        if (isWalkBack)
            MoveCameraTo(0.0f, 0.0f, -camVelocity);
        if (isWalkLeft)
            MoveCameraTo(-camVelocity, 0.0f, 0.0f);
        if (isWalkRight)
            MoveCameraTo(camVelocity, 0.0f, 0.0f);
        if (isWalkUp)
            MoveCameraTo(0.0f, camVelocity, 0.0f);
        if (isWalkDown)
            MoveCameraTo(0.0f, -camVelocity, 0.0f);

        float aspect = (float)riWinData->Height / riWinData->Width;

        const dx::XMVECTOR forwardVec = dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
        const dx::XMVECTOR lookVec = dx::XMVector3Transform(
            forwardVec,
            dx::XMMatrixRotationRollPitchYaw(riData->m_CameraLook.y, riData->m_CameraLook.x, 0.0f)
        );
        const dx::XMVECTOR lookVecNorm = dx::XMVector3Normalize(lookVec);
        const dx::XMVECTOR camPos = dx::XMLoadFloat3(&riData->m_CameraPosition);
        const dx::XMVECTOR camDir = dx::XMVectorAdd(camPos, lookVecNorm);

        struct CameraCBuffer
        {
            dx::XMMATRIX view;
            dx::XMMATRIX projection;
        };

        const CameraCBuffer camCBuffer =
        {
            dx::XMMatrixLookAtLH(camPos, camDir, dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)),

            dx::XMMatrixPerspectiveLH(1.0f, aspect, 0.5f, 100.0f)
        };

        VertexConstantBuffer<CameraCBuffer> camVCBuf(*this, camCBuffer, 1);
        camVCBuf.Bind(*this);
    }

    void KDRenderer::BeginFrame()
    {
        UpdateCamera();
        ClearBuffer(0.0f, 0.2f, 0.3f);
    }
    void KDRenderer::EndFrame()
    {
        HRESULT hr;
        auto riData = (KDRendererData_dx11*)m_Data;
        auto& infoManager = GetInfoManager();

        if ( FAILED(hr = riData->pSwapChain->Present((int)riData->m_VSync, 0)) )
        {
            if (hr == DXGI_ERROR_DEVICE_REMOVED)
                throw KDGFX_DEVICE_REMOVED_EXCEPT(riData->pDevice->GetDeviceRemovedReason());
            else
                throw KDGFX_EXCEPT(hr);
        }

        riData->m_DeltaTime = riData->pTimer.Mark();
    }
    void KDRenderer::DrawIndexed(unsigned int count)
    {
        KDRendererData_dx11* riData = (KDRendererData_dx11*)m_Data;
        auto& infoManager = GetInfoManager();

        KDGFX_THROW_INFO_ONLY( riData->pContext->DrawIndexed((UINT)count, 0, 0) );
    }
    void KDRenderer::ClearBuffer(float red, float green, float blue)
    {
        KDRendererData_dx11* riData = (KDRendererData_dx11*)m_Data;

        const FLOAT rgba[4] = {red,green,blue,1.0f};
        riData->pContext->ClearRenderTargetView(riData->pTarget.Get(), rgba);
        riData->pContext->ClearDepthStencilView(riData->pDSView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    }
    void KDRenderer::DrawTestScene()
    {
        KDRendererData_dx11* riData = (KDRendererData_dx11*)m_Data;
        KDWindowData_win32* riWinData = (KDWindowData_win32*)riData->pOutputWindow->m_Data;

        struct Vertex
        {
            dx::XMFLOAT3 pos;
            dx::XMFLOAT3 color;
        };
        struct CBuffer
        {
            dx::XMMATRIX model;
            dx::XMMATRIX view;
            dx::XMMATRIX projection;
        };

        std::vector<Vertex> vertices =
        {
            { {-1.0f, -1.0f,  1.0}, {0.0f, 0.0f, 0.0f} },
            { { 1.0f, -1.0f,  1.0}, {0.0f, 0.0f, 1.0f} },
            { {-1.0f,  1.0f,  1.0}, {0.0f, 1.0f, 0.0f} },
            { { 1.0f,  1.0f,  1.0}, {0.0f, 1.0f, 1.0f} },
            { {-1.0f, -1.0f, -1.0}, {1.0f, 0.0f, 0.0f} },
            { { 1.0f, -1.0f, -1.0}, {1.0f, 0.0f, 1.0f} },
            { {-1.0f,  1.0f, -1.0}, {1.0f, 1.0f, 0.0f} },
            { { 1.0f,  1.0f, -1.0}, {1.0f, 1.0f, 1.0f} } 
        };
        std::vector<uint32_t> indices = 
        {
            4,6,5,  5,6,7,
            0,2,6,  0,6,4,
            0,3,2,  0,1,3,
            5,7,3,  5,3,1,
            4,6,5,  5,6,7,
            6,2,3,  6,3,7,
            4,1,0,  4,5,1,
        };
        
        static float modelRot = 0.0f;
        float camRotVelocity = 2.0f * GetDeltaTime();
        modelRot = WrapAngle(modelRot + camRotVelocity);
        
        const CBuffer cBuffer =
        {
            dx::XMMatrixScaling(0.5f, 0.5f, 0.5f) *
            dx::XMMatrixRotationRollPitchYaw(modelRot, 0.0f, modelRot) *
            dx::XMMatrixTranslation(0.0f, 1.0f, 4.0f)
        };

        VertexBuffer<Vertex> pVBuffer(*this, vertices);
        pVBuffer.Bind(*this);

        IndexBuffer pIBuffer(*this, indices);
        pIBuffer.Bind(*this);

        VertexConstantBuffer<CBuffer> vCBuffer(*this, cBuffer);
        vCBuffer.Bind(*this);

        VertexShader vShader(*this, "assets/shaders/VSDefault.cso");
        vShader.Bind(*this);

        PixelShader pShader(*this, "assets/shaders/PSDefault.cso");
        pShader.Bind(*this);

        InputLayout iLayout(*this,
            {
                { "Position", InputType_Float3 },
                { "Color", InputType_Float3 }
            },
            vShader.GetBytecode());
        iLayout.Bind(*this);

        Topology iTopology(*this, TopologyType_TriangleList);
        iTopology.Bind(*this);

        DrawIndexed( pIBuffer.GetCount() );
    }
    void KDRenderer::DrawTestTexturedScene()
    {
        KDRendererData_dx11* riData = (KDRendererData_dx11*)m_Data;
        KDWindowData_win32* riWinData = (KDWindowData_win32*)riData->pOutputWindow->m_Data;

        struct Vertex
        {
            dx::XMFLOAT3 pos;
            dx::XMFLOAT2 tex;
        };
        struct CBuffer
        {
            dx::XMMATRIX model;
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

        const float cubeRotVelocity = 1.5f * GetDeltaTime();
        static float modelRot = 0.0f;
        modelRot = WrapAngle(modelRot + cubeRotVelocity);

        const CBuffer cBuffer =
        {
            dx::XMMatrixScaling(0.5f, 0.5f, 0.5f) *
            dx::XMMatrixRotationRollPitchYaw(modelRot, 0.0f, modelRot) *
            dx::XMMatrixTranslation(0.0f, 1.0f, 4.0f)
        };

        VertexBuffer<Vertex> pVBuffer(*this, vertices);
        pVBuffer.Bind(*this);

        IndexBuffer pIBuffer(*this, indices);
        pIBuffer.Bind(*this);

        VertexConstantBuffer<CBuffer> vCBuffer(*this, cBuffer);
        vCBuffer.Bind(*this);

        VertexShader vShader(*this, "assets/shaders/VSTexture.cso");
        vShader.Bind(*this);

        PixelShader pShader(*this, "assets/shaders/PSTexture.cso");
        pShader.Bind(*this);

        InputLayout iLayout(*this,
            {
                { "Position", InputType_Float3 },
                { "TexCoord", InputType_Float2 }
            },
            vShader.GetBytecode());
        iLayout.Bind(*this);

        Topology iTopology(*this, TopologyType_TriangleList);
        iTopology.Bind(*this);

        Sampler iSampler(*this);
        iSampler.Bind(*this);

        riData->pDefaultTexture->Bind(*this);

        DrawIndexed(pIBuffer.GetCount());
    }
    void KDRenderer::DrawTestPlane()
    {
        KDRendererData_dx11* riData = (KDRendererData_dx11*)m_Data;
        KDWindowData_win32* riWinData = (KDWindowData_win32*)riData->pOutputWindow->m_Data;

        struct KDVertex
        {
            dx::XMFLOAT3 pos;
            dx::XMFLOAT3 color;
        };
        struct KDCBuffer
        {
            dx::XMMATRIX model;
        };

        std::vector<KDVertex> vertices;
        std::vector<uint32_t> indices;

        vertices.resize(440);
        indices.resize(660);

        {
            int v = 0;
            int i = 0;
            bool isBlack = true;
            dx::XMFLOAT3 currColor;
            for (int z = -5; z <= 5; z++)
            {
                for (int x = -5; x <= 5 && v < vertices.size(); x++)
                {
                
                    if (isBlack)
                    {
                        currColor.x = 1.0f;
                        currColor.y = 1.0f;
                        currColor.z = 1.0f;
                    }
                    else
                    {
                        currColor.x = 0.0f;
                        currColor.y = 0.0f;
                        currColor.z = 0.0f;
                    }

                    vertices[v].pos.x       = (float)x;
                    vertices[v].pos.x       = (float)x;
                    vertices[v].pos.y       = (float)0.0f;
                    vertices[v].pos.y       = (float)0.0f;
                    vertices[v].pos.z       = (float)z;
                    vertices[v].pos.z       = (float)z;
                    vertices[v].color.x     = (float)currColor.x;
                    vertices[v].color.x     = (float)currColor.x;
                    vertices[v].color.y     = (float)currColor.y;
                    vertices[v].color.y     = (float)currColor.y;
                    vertices[v].color.z     = (float)currColor.z;
                    vertices[v].color.z     = (float)currColor.z;
                                                              
                    vertices[v+1].pos.x     = (float)x;
                    vertices[v+1].pos.x     = (float)x;
                    vertices[v+1].pos.y     = (float)0.0f;
                    vertices[v+1].pos.y     = (float)0.0f;
                    vertices[v+1].pos.z     = (float)z+1;
                    vertices[v+1].pos.z     = (float)z+1;
                    vertices[v+1].color.x   = (float)currColor.x;
                    vertices[v+1].color.x   = (float)currColor.x;
                    vertices[v+1].color.y   = (float)currColor.y;
                    vertices[v+1].color.y   = (float)currColor.y;
                    vertices[v+1].color.z   = (float)currColor.z;
                    vertices[v+1].color.z   = (float)currColor.z;            
                                              
                    vertices[v+2].pos.x     = (float)x+1;
                    vertices[v+2].pos.x     = (float)x+1;
                    vertices[v+2].pos.y     = (float)0.0f;
                    vertices[v+2].pos.y     = (float)0.0f;
                    vertices[v+2].pos.z     = (float)z+1;
                    vertices[v+2].pos.z     = (float)z+1;
                    vertices[v+2].color.x   = (float)currColor.x;
                    vertices[v+2].color.x   = (float)currColor.x;
                    vertices[v+2].color.y   = (float)currColor.y;
                    vertices[v+2].color.y   = (float)currColor.y;
                    vertices[v+2].color.z   = (float)currColor.z;
                    vertices[v+2].color.z   = (float)currColor.z;                 
                                              
                    vertices[v+3].pos.x     = (float)x+1;
                    vertices[v+3].pos.x     = (float)x+1;
                    vertices[v+3].pos.y     = (float)0.0f;
                    vertices[v+3].pos.y     = (float)0.0f;
                    vertices[v+3].pos.z     = (float)z;
                    vertices[v+3].pos.z     = (float)z;
                    vertices[v+3].color.x   = (float)currColor.x;
                    vertices[v+3].color.x   = (float)currColor.x;
                    vertices[v+3].color.y   = (float)currColor.y;
                    vertices[v+3].color.y   = (float)currColor.y;
                    vertices[v+3].color.z   = (float)currColor.z;
                    vertices[v+3].color.z   = (float)currColor.z;

                    indices[i]            = v;
                    indices[i+1]    = v+1;
                    indices[i+2]    = v+2;
                    indices[i+3]    = v;
                    indices[i+4]    = v+2;
                    indices[i+5]    = v+3;
                    v += 4; 
                    i += 6; 
                    isBlack = !isBlack;
                }
            }
        }

        const KDCBuffer cBuffer =
        {
            dx::XMMatrixScaling(0.5f, 0.5f, 0.5f) *
            dx::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f) *
            dx::XMMatrixTranslation(0.0f, 0.0f, 4.0f)
        };

        VertexBuffer<KDVertex> pVBuffer(*this, vertices);
        pVBuffer.Bind(*this);

        IndexBuffer pIBuffer(*this, indices);
        pIBuffer.Bind(*this);

        VertexConstantBuffer<KDCBuffer> vCBuffer(*this, cBuffer);
        vCBuffer.Bind(*this);

        VertexShader vShader(*this, "assets/shaders/VSDefault.cso");
        vShader.Bind(*this);

        PixelShader pShader(*this, "assets/shaders/PSDefault.cso");
        pShader.Bind(*this);

        InputLayout iLayout(*this,
            {
                { "Position", InputType_Float3 },
                { "Color", InputType_Float3 }
            },
            vShader.GetBytecode());
        iLayout.Bind(*this);

        Topology iTopology(*this, TopologyType_TriangleList);
        iTopology.Bind(*this);

        DrawIndexed(pIBuffer.GetCount());
    }

    float KDRenderer::GetDeltaTime() const
    {
        return ( (KDRendererData_dx11*)m_Data )->m_DeltaTime;
    }
}