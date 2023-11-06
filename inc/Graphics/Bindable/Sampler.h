#pragma once
#include "Bindable.h"

#include <Graphics/KDRenderer.h>

#include <Debug/DXGIInfoManager.h>
#include <Debug/GfxExcept.h>

#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>

namespace KDE
{
	class Sampler : public Bindable
	{
		public:
			Sampler(KDRenderer& renderer)
			{
				INFOMAN(renderer);

				D3D11_SAMPLER_DESC sDesc{};
				sDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				sDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
				sDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
				sDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

				KDGFX_THROW_INFO( GetDevice(renderer).CreateSamplerState(&sDesc, &pSampler) );
			}

			void Bind(KDRenderer& renderer) const override
			{
				GetContext(renderer).PSSetSamplers(0, 1, pSampler.GetAddressOf());
			}

		private:
			Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
	};
}