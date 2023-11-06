#pragma once
#include "Bindable.h"

#include <Graphics/KDRenderer.h>

#include <Debug/DXGIInfoManager.h>
#include <Debug/GfxExcept.h>

#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>

#include <d3dcompiler.h>
#include <string>

namespace KDE
{
	class VertexShader : public Bindable
	{
		public:
			VertexShader(KDRenderer& renderer, const std::string& path)
			{
				INFOMAN(renderer);

				wchar_t* wide_string = new wchar_t[path.length() + 1];
				std::copy(path.begin(), path.end(), wide_string);
				wide_string[path.length()] = 0;

				KDGFX_THROW_INFO( D3DReadFileToBlob(wide_string, &pBlob) );
				KDGFX_THROW_INFO(
					GetDevice(renderer).CreateVertexShader(
						pBlob->GetBufferPointer(),
						pBlob->GetBufferSize(),
						nullptr, &pVShader)
				);
			}
			
			void Bind(KDRenderer& renderer) const override
			{
				GetContext(renderer).VSSetShader(pVShader.Get(), nullptr, 0);
			}

			Microsoft::WRL::ComPtr<ID3DBlob>& GetBytecode() const
			{
				return pBlob;
			}

		private:
			Microsoft::WRL::ComPtr<ID3D11VertexShader> pVShader;
			mutable Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	};
	class PixelShader : public Bindable
	{
	public:
		PixelShader(KDRenderer& renderer, const std::string& path)
		{
			INFOMAN(renderer);

			wchar_t* wide_string = new wchar_t[path.length() + 1];
			std::copy(path.begin(), path.end(), wide_string);
			wide_string[path.length()] = 0;

			KDGFX_THROW_INFO( D3DReadFileToBlob(wide_string, &pBlob) );
			KDGFX_THROW_INFO(
				GetDevice(renderer).CreatePixelShader(
					pBlob->GetBufferPointer(),
					pBlob->GetBufferSize(),
					nullptr, &pPShader)
			);
		}

		void Bind(KDRenderer& renderer) const override
		{
			GetContext(renderer).PSSetShader(pPShader.Get(), nullptr, 0);
		}

	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPShader;
		Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	};
}