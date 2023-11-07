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

namespace KDE
{
    struct KDRendererData
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
    struct KDWindowData
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
}

namespace KDE
{
    KDRenderer::KDRenderer(KDWindow* outputWindow)
    {
        HRESULT hr;

        m_Data = new KDRendererData{};

        KDWindowData* riWinData = (KDWindowData*)outputWindow->m_Data;

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

        m_Data->pOutputWindow = outputWindow;
        KDGFX_THROW_INFO( D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            creationFlags, nullptr, 0, D3D11_SDK_VERSION, &scd,
            &m_Data->pSwapChain, &m_Data->pDevice,
            nullptr, &m_Data->pContext
        ) );

        assert( "Failed to create D3D11 Device and DXGI SwapChain" && SUCCEEDED(hr) );

        wrl::ComPtr<ID3D11Resource> pBackBuffer;
        KDGFX_THROW_INFO( m_Data->pSwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer) );
        KDGFX_THROW_INFO( m_Data->pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_Data->pTarget) );

     ////////////////////   Depth Stencil stuff  ////////////////////
        D3D11_DEPTH_STENCIL_DESC dsDesc{};
        dsDesc.DepthEnable = TRUE;
        dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
        wrl::ComPtr<ID3D11DepthStencilState> pDSState;
        KDGFX_THROW_INFO( m_Data->pDevice->CreateDepthStencilState(&dsDesc, &pDSState) );

        m_Data->pContext->OMSetDepthStencilState(pDSState.Get(), 1);

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
        KDGFX_THROW_INFO( m_Data->pDevice->CreateTexture2D(&depthDesc, nullptr, &pDepthStencil) );

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsvDesc.Texture2D.MipSlice = 0;
        KDGFX_THROW_INFO( m_Data->pDevice->CreateDepthStencilView(
            pDepthStencil.Get(), 
            &dsvDesc, &m_Data->pDSView) );
     ////////////////////////////////////////////////////////////

        m_Data->pContext->OMSetRenderTargets(1, m_Data->pTarget.GetAddressOf(), m_Data->pDSView.Get());

        D3D11_VIEWPORT vp{};
        vp.Width = (FLOAT)outputWindow->GetWidth();
        vp.Height = (FLOAT)outputWindow->GetHeight();
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        m_Data->pContext->RSSetViewports(1, &vp);

        m_Data->pDefaultTexture = new Texture(*this, "assets/images/cube.jpg");

        UpdateCamera();
    }
    KDRenderer::~KDRenderer()
    {
        delete (KDRendererData*)m_Data;
    }

    DXGIInfoManager& KDRenderer::GetInfoManager() const
    {
        return ( (KDRendererData*)m_Data )->m_InfoManager;
    }
    ID3D11Device& KDRenderer::GetDevice() const
    {
        return *( (KDRendererData*)m_Data )->pDevice.Get();
    }
    ID3D11DeviceContext& KDRenderer::GetContext() const
    {
        return *( (KDRendererData*)m_Data )->pContext.Get();
    }
    
    void KDRenderer::MoveCameraTo(float offX, float offY, float offZ)
    {
        dx::XMFLOAT3 translation{offX, offY, offZ};

        dx::XMStoreFloat3( 
            &translation, 
            dx::XMVector3Transform( 
                dx::XMLoadFloat3(&translation),
                dx::XMMatrixRotationRollPitchYaw(m_Data->m_CameraLook.y, m_Data->m_CameraLook.x, 0.0f) )
        );

        m_Data->m_CameraPosition.x += translation.x;
        m_Data->m_CameraPosition.y += translation.y;
        m_Data->m_CameraPosition.z += translation.z;
    }
    void KDRenderer::MoveCameraAt(float posX, float posY, float posZ)
    {
        m_Data->m_CameraPosition.x = posX;
        m_Data->m_CameraPosition.y = posY;
        m_Data->m_CameraPosition.z = posZ;
    }
    void KDRenderer::RotateCameraTo(float offX, float offY)
    {
        m_Data->m_CameraLook.x = WrapAngle(m_Data->m_CameraLook.x + offX);
        m_Data->m_CameraLook.y = std::clamp(m_Data->m_CameraLook.y + offY, -3.14f / 2.0f, 3.14f / 2.0f);
    }
    void KDRenderer::RotateCameraAt(float rotX, float rotY)
    {
        m_Data->m_CameraLook.x = WrapAngle(rotX);
        m_Data->m_CameraLook.y = std::clamp(rotY, -3.14f / 2.0f, 3.14f / 2.0f);
    }
    void KDRenderer::UpdateCamera()
    {
        auto riWinData = m_Data->pOutputWindow->m_Data;

        static bool isFreeLookMode = true;
        static bool isFreeLookMode_Press = false;

        m_Data->pOutputWindow->ShowCursor();

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

        if (!isFreeLookMode) { m_Data->pOutputWindow->ShowCursor(); return; }
        else { m_Data->pOutputWindow->HideCursor(); }

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
            dx::XMMatrixRotationRollPitchYaw(m_Data->m_CameraLook.y, m_Data->m_CameraLook.x, 0.0f)
        );
        const dx::XMVECTOR lookVecNorm = dx::XMVector3Normalize(lookVec);
        const dx::XMVECTOR camPos = dx::XMLoadFloat3(&m_Data->m_CameraPosition);
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
        auto& infoManager = GetInfoManager();

        if ( FAILED(hr = m_Data->pSwapChain->Present((int)m_Data->m_VSync, 0)) )
        {
            if (hr == DXGI_ERROR_DEVICE_REMOVED)
                throw KDGFX_DEVICE_REMOVED_EXCEPT(m_Data->pDevice->GetDeviceRemovedReason());
            else
                throw KDGFX_EXCEPT(hr);
        }

        m_Data->m_DeltaTime = m_Data->pTimer.Mark();
    }
    void KDRenderer::DrawIndexed(unsigned int count)
    {
        auto& infoManager = GetInfoManager();

        KDGFX_THROW_INFO_ONLY( m_Data->pContext->DrawIndexed((UINT)count, 0, 0) );
    }
    void KDRenderer::ClearBuffer(float red, float green, float blue)
    {
        const FLOAT rgba[4] = {red,green,blue,1.0f};
        m_Data->pContext->ClearRenderTargetView(m_Data->pTarget.Get(), rgba);
        m_Data->pContext->ClearDepthStencilView(m_Data->pDSView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    float KDRenderer::GetDeltaTime() const
    {
        return m_Data->m_DeltaTime;
    }
}