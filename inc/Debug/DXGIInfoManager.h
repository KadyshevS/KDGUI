#pragma once 

#include <dxgidebug.h>
#include <vector>
#include <string>
#include <wrl.h>

namespace KDE
{
    class DXGIInfoManager
    {
    public:
        DXGIInfoManager();
        ~DXGIInfoManager() = default;
        DXGIInfoManager(const DXGIInfoManager&) = delete;
        DXGIInfoManager& operator = (const DXGIInfoManager&) = delete;

        void Set() noexcept;
        
        std::vector<std::string> GetMessages() const;

    private:
        unsigned long long next = 0u;
        Microsoft::WRL::ComPtr<IDXGIInfoQueue> pDxgiInfoQueue;
    };
}