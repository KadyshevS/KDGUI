#include <precomp.h>
#include <Graphics/Bindable/Texture.h>

#include <Debug/DXGIInfoManager.h>
#include <Debug/GfxExcept.h>

#define STB_IMAGE_IMPLEMENTATION
#include <Utils/stb/stb_image.h>

namespace KDE
{
    Texture::Texture(KDRenderer& renderer, const char* filePath)
	{
        namespace wrl = Microsoft::WRL;

        INFOMAN(renderer);

        int imageWidth;
        int imageHeight;
        int imageChannels;
        int imageDesiredChannels = 4;

        unsigned char* imageData = nullptr;
        imageData = stbi_load(filePath,
            &imageWidth,
            &imageHeight,
            &imageChannels, imageDesiredChannels);

        assert("Failed to load image" && imageData != nullptr);

        int imagePitch = imageWidth * 4;

        D3D11_TEXTURE2D_DESC textureDesc{};

        textureDesc.Width = imageWidth;
        textureDesc.Height = imageHeight;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA textureSD{};

        textureSD.pSysMem = imageData;
        textureSD.SysMemPitch = imagePitch;

        wrl::ComPtr<ID3D11Texture2D> pImageTexture;
        KDGFX_THROW_INFO( GetDevice(renderer).CreateTexture2D(&textureDesc,
            &textureSD,
            &pImageTexture) );

        free(imageData);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        KDGFX_THROW_INFO( GetDevice(renderer).CreateShaderResourceView(
            pImageTexture.Get(), &srvDesc, &pSRView) );
	}

	void Texture::Bind(KDRenderer& renderer) const
	{
        GetContext(renderer).PSSetShaderResources(0, 1, pSRView.GetAddressOf());
	}
}