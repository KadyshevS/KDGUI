#pragma once

#include "Bindable.h"

#include <Graphics/KDRenderer.h>

#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>

#include <Debug/DXGIInfoManager.h>
#include <Debug/GfxExcept.h>

#include <vector>
#include <wrl.h>

struct ID3D11Buffer;

namespace KDE
{
	template <typename D>
	class Buffer : public Bindable
	{
		public:
			virtual void Update(KDRenderer& renderer, const D& data) const = 0;
			using Bindable::Bind;

		protected:
			Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer;
	};

	template <typename V>
	class VertexBuffer : public Buffer<std::vector<V>>
	{
		using Bindable::GetDevice;
		using Bindable::GetContext;
		using Bindable::GetInfoManager;
		using Buffer<std::vector<V>>::pBuffer;

		public:
			VertexBuffer(KDRenderer& renderer, const std::vector<V>& vertices)
			{
				INFOMAN(renderer);

				D3D11_BUFFER_DESC vbd{};
				vbd.ByteWidth = (UINT)vertices.size() * sizeof(V);
				vbd.Usage = D3D11_USAGE_DYNAMIC;
				vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				vbd.MiscFlags = 0;
				vbd.StructureByteStride = sizeof(V);
				D3D11_SUBRESOURCE_DATA vsd{};
				vsd.pSysMem = vertices.data();
				KDGFX_THROW_INFO( GetDevice(renderer).CreateBuffer(&vbd, &vsd, &pBuffer) );
			}

			void Bind(KDRenderer& renderer) const override
			{
				const UINT strides = sizeof(V), offsets = 0;
				GetContext(renderer).IASetVertexBuffers(0, 1, pBuffer.GetAddressOf(), &strides, &offsets);
			}
			void Update(KDRenderer& renderer, const std::vector<V>& vertices) const override
			{
				INFOMAN(renderer);

				D3D11_MAPPED_SUBRESOURCE msr{};
				KDGFX_THROW_INFO(
					GetContext(renderer).Map(
						pBuffer.Get(), 0,
						D3D11_MAP_WRITE_DISCARD, 0, 
						&msr)
				);
				memcpy(msr.pData, vertices.data(), vertices.size() * sizeof(V));
				GetContext(renderer).Unmap(pBuffer.Get(), 0);
			}
	};
	class IndexBuffer : public Buffer<std::vector<uint32_t>>
	{
		using Bindable::GetDevice;
		using Bindable::GetContext;
		using Bindable::GetInfoManager;
		using Buffer<std::vector<uint32_t>>::pBuffer;

		public:
			IndexBuffer(KDRenderer& renderer, const std::vector<uint32_t>& indices)
				: count((UINT)indices.size())
			{
				INFOMAN(renderer);

				D3D11_BUFFER_DESC vbd{};
				vbd.ByteWidth = (UINT)indices.size() * sizeof(uint32_t);
				vbd.Usage = D3D11_USAGE_DYNAMIC;
				vbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
				vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				vbd.MiscFlags = 0;
				vbd.StructureByteStride = sizeof(uint32_t);
				D3D11_SUBRESOURCE_DATA vsd{};
				vsd.pSysMem = indices.data();
				KDGFX_THROW_INFO( GetDevice(renderer).CreateBuffer(&vbd, &vsd, &pBuffer) );
			}

			void Bind(KDRenderer& renderer) const override
			{
				GetContext(renderer).IASetIndexBuffer(pBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			}
			void Update(KDRenderer& renderer, const std::vector<uint32_t>& indices) const override
			{
				INFOMAN(renderer);

				D3D11_MAPPED_SUBRESOURCE msr{};
				KDGFX_THROW_INFO(
					GetContext(renderer).Map(
						pBuffer.Get(), 0,
						D3D11_MAP_WRITE_DISCARD, 0,
						&msr)
				);
				memcpy(msr.pData, indices.data(), indices.size() * sizeof(uint32_t));
				GetContext(renderer).Unmap(pBuffer.Get(), 0);
			}

			UINT GetCount() const
			{
				return count;
			}
		private:
			UINT count;
	};

	template <typename D>
	class ConstantBuffer : public Buffer<D>
	{
		using Bindable::GetDevice;
		using Bindable::GetContext;
		using Bindable::GetInfoManager;
		using Buffer<D>::pBuffer;

		public:
			ConstantBuffer(KDRenderer& renderer, UINT slot = 0)
				: slot(slot)
			{
				INFOMAN(renderer);

				D3D11_BUFFER_DESC vbd{};
				vbd.ByteWidth = sizeof(D);
				vbd.Usage = D3D11_USAGE_DYNAMIC;
				vbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				vbd.MiscFlags = 0;
				vbd.StructureByteStride = 0;
				KDGFX_THROW_INFO( GetDevice(renderer).CreateBuffer(&vbd, nullptr, &pBuffer) );
			}
			ConstantBuffer(KDRenderer& renderer, const D& consts, UINT slot = 0)
				: slot(slot)
			{
				INFOMAN(renderer);

				D3D11_BUFFER_DESC vbd{};
				vbd.ByteWidth = sizeof(D);
				vbd.Usage = D3D11_USAGE_DYNAMIC;
				vbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				vbd.MiscFlags = 0;
				vbd.StructureByteStride = 0;
				D3D11_SUBRESOURCE_DATA vsd{};
				vsd.pSysMem = &consts;
				KDGFX_THROW_INFO( GetDevice(renderer).CreateBuffer(&vbd, &vsd, &pBuffer) );
			}

			void Update(KDRenderer& renderer, const D& consts) const override
			{
				INFOMAN(renderer);

				D3D11_MAPPED_SUBRESOURCE msr{};
				KDGFX_THROW_INFO(
					GetContext(renderer).Map(
						pBuffer.Get(), 0,
						D3D11_MAP_WRITE_DISCARD, 0,
						&msr)
				);
				memcpy(msr.pData, &consts, sizeof(consts));
				GetContext(renderer).Unmap(pBuffer.Get(), 0);
			}

		protected:
			UINT slot;
	};

	template <typename D>
	class VertexConstantBuffer : public ConstantBuffer<D>
	{
		public:
			using ConstantBuffer<D>::ConstantBuffer;
			using ConstantBuffer<D>::Update;
			using ConstantBuffer<D>::slot;
			using Bindable::GetContext;
			using Buffer<D>::pBuffer;

			void Bind(KDRenderer& renderer) const override
			{
				GetContext(renderer).VSSetConstantBuffers(slot, 1, pBuffer.GetAddressOf());
			}
	};
	template <typename D>
	class PixelConstantBuffer : public ConstantBuffer<D>
	{
		public:
			using ConstantBuffer<D>::ConstantBuffer;
			using ConstantBuffer<D>::Update;
			using ConstantBuffer<D>::slot;
			using Bindable::GetContext;
			using Buffer<D>::pBuffer;

			void Bind(KDRenderer& renderer) const override
			{
				GetContext(renderer).PSSetConstantBuffers(slot, 1, pBuffer.GetAddressOf());
			}
	};
}