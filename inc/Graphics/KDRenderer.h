#pragma once 

#include <WinBase/KDWindow.h>

#include <memory>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace KDE
{
    class KDRenderer
    {
        friend class Bindable;
        friend class Drawable;
        friend class KDApplication;

        public:
            KDRenderer(std::shared_ptr<KDWindow> outputWindow);
            KDRenderer(const KDRenderer&) = delete;
            KDRenderer& operator = (const KDRenderer&) = delete;
            ~KDRenderer();

            float GetDeltaTime() const;

        private:
            struct KDRendererData* m_Data;

        private:
            class DXGIInfoManager& GetInfoManager() const;
            ID3D11Device& GetDevice() const;
            ID3D11DeviceContext& GetContext() const;

            void MoveCameraTo(float offX, float offY, float offZ);
            void MoveCameraAt(float posX, float posY, float posZ);
            void RotateCameraTo(float offX, float offY);
            void RotateCameraAt(float rotX, float rotY);
            
            void ClearBuffer(float red, float green, float blue);
            void BeginFrame();
            void EndFrame();
            void UpdateCamera();
            void DrawIndexed(unsigned int count);
    };
}