#pragma once
#include "Bindable.h"

#include <Graphics/KDRenderer.h>

#include <Debug/DXGIInfoManager.h>
#include <Debug/GfxExcept.h>

#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <wrl.h>

#include <string>
#include <vector>
#include <exception>

namespace KDE
{
	enum InputType
	{
		InputType_Unknown = 0,
		InputType_Float2,
		InputType_Float3,
		InputType_Float4,
		InputType_Int2,
		InputType_Int3,
		InputType_Int4,
		InputType_UInt2,
		InputType_UInt3,
		InputType_UInt4
	};
	struct LayoutElement
	{
		std::string Name;
		InputType Type;
	};

	class InputLayout : public Bindable
	{
		public:
			InputLayout(
				KDRenderer& renderer, 
				const std::vector<LayoutElement>& layout, 
				const Microsoft::WRL::ComPtr<ID3DBlob>& vertexShaderBytecode)
			{
				INFOMAN(renderer);

				std::vector<D3D11_INPUT_ELEMENT_DESC> ied;
				
				for (auto& l : layout)
				{
					ied.emplace_back( 
						l.Name.c_str(), 0, KDTypeToDXGIFormat(l.Type), 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 );
				}
				
				KDGFX_THROW_INFO(
					GetDevice(renderer).CreateInputLayout(
						ied.data(), (UINT)ied.size(),
						vertexShaderBytecode->GetBufferPointer(),
						vertexShaderBytecode->GetBufferSize(),
						&pInputLayout)
				);
			}

			void Bind(KDRenderer& renderer) const override
			{
				GetContext(renderer).IASetInputLayout(pInputLayout.Get());
			}

		private:
			static DXGI_FORMAT KDTypeToDXGIFormat(const InputType& type)
			{
				switch (type)
				{
					case InputType_Unknown:
					{
						throw std::exception("Unknown input type is not supported");
						return DXGI_FORMAT_UNKNOWN;
						break;
					}
					case InputType_Float2:
					{
						return DXGI_FORMAT_R32G32_FLOAT;
						break;
					}
					case InputType_Float3:
					{
						return DXGI_FORMAT_R32G32B32_FLOAT;
						break;
					}
					case InputType_Float4:
					{
						return DXGI_FORMAT_R32G32B32A32_FLOAT;
						break;
					}
					case InputType_Int2:
					{
						return DXGI_FORMAT_R32G32_SINT;
						break;
					}
					case InputType_Int3:
					{
						return DXGI_FORMAT_R32G32B32_SINT;
						break;
					}
					case InputType_Int4:
					{
						return DXGI_FORMAT_R32G32B32A32_SINT;
						break;
					}
					case InputType_UInt2:
					{
						return DXGI_FORMAT_R32G32_UINT;
						break;
					}
					case InputType_UInt3:
					{
						return DXGI_FORMAT_R32G32B32_UINT;
						break;
					}
					case InputType_UInt4:
					{
						return DXGI_FORMAT_R32G32B32A32_UINT;
						break;
					}
					default:
						throw std::exception("Unknown input element type passed");
						return DXGI_FORMAT_UNKNOWN;
				}
			}

		private:
			Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
	};
}