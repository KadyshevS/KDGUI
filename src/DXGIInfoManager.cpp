#include <precomp.h>

#include <Debug/DXGIInfoManager.h>
#include <Debug/WndExcept.h>
#include <Debug/GfxExcept.h>

namespace KDE
{
    DXGIInfoManager::DXGIInfoManager()
    {
    #ifndef NDEBUG
        typedef HRESULT (WINAPI* DXGIGetDebugInterface)(REFIID,void **);

        const auto hModDxgiDebug = LoadLibraryEx("dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(hModDxgiDebug == nullptr)
            throw KDWND_LAST_EXCEPT;

        const auto DxgiGetDebugInterface = (DXGIGetDebugInterface)GetProcAddress(hModDxgiDebug,"DXGIGetDebugInterface");
        if(DxgiGetDebugInterface == nullptr)
            throw KDWND_LAST_EXCEPT;

        HRESULT hr;
        KDGFX_THROW_NOINFO( DxgiGetDebugInterface(__uuidof(IDXGIInfoQueue), &pDxgiInfoQueue) );
    #endif
    }

    void DXGIInfoManager::Set() noexcept
    {
    #ifndef NDEBUG
        next = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
    #endif
    }

    std::vector<std::string> DXGIInfoManager::GetMessages() const
    {
    #ifndef NDEBUG
        std::vector<std::string> messages;
        const auto end = pDxgiInfoQueue->GetNumStoredMessages(DXGI_DEBUG_ALL);
        for(auto i = next; i < end; i++)
        {
            HRESULT hr;
            SIZE_T messageLength;

            KDGFX_THROW_NOINFO( pDxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL,i,nullptr,&messageLength) );
            
            auto bytes = std::make_unique<byte[]>(messageLength);
            auto pMessage = (DXGI_INFO_QUEUE_MESSAGE*)bytes.get();
            
            KDGFX_THROW_NOINFO( pDxgiInfoQueue->GetMessage(DXGI_DEBUG_ALL, i, pMessage, &messageLength) );
            messages.emplace_back(pMessage->pDescription);
        }
        return messages;
    #else
        return {};
    #endif
    }
}