#include <precomp.h>

#include <Graphics/Bindable/Bindable.h>
#include <Debug/DXGIInfoManager.h>

namespace KDE
{
	DXGIInfoManager& Bindable::GetInfoManager(KDRenderer& renderer) const
	{
		return renderer.GetInfoManager();
	}
	ID3D11Device& Bindable::GetDevice(KDRenderer& renderer) const
	{
		return renderer.GetDevice();
	}
	ID3D11DeviceContext& Bindable::GetContext(KDRenderer& renderer) const
	{
		return renderer.GetContext();
	}
}