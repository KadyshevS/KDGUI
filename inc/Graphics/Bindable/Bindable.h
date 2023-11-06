#pragma once

#include <Graphics/KDRenderer.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace KDE
{
	class Bindable
	{
		public:
			Bindable() = default;
			Bindable(const Bindable&) = delete;
			Bindable& operator = (const Bindable&) = delete;
			virtual void Bind(KDRenderer& renderer) const = 0;

		protected:
			class DXGIInfoManager& GetInfoManager(class KDRenderer& renderer) const;
			ID3D11Device& GetDevice(class KDRenderer& renderer) const;
			ID3D11DeviceContext& GetContext(class KDRenderer& renderer) const;
	};
}