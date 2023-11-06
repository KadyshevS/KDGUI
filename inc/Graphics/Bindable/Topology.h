#pragma once

#include "Bindable.h"

#include <Graphics/KDRenderer.h>

#include <Debug/DXGIInfoManager.h>
#include <Debug/GfxExcept.h>

#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>

#include <exception>

namespace KDE
{
	enum TopologyType
	{
		TopologyType_Unknown = 0,
		TopologyType_PointList,
		TopologyType_LineList,
		TopologyType_LineStrip,
		TopologyType_TriangleList,
		TopologyType_TriangleStrip
	};

	class Topology : public Bindable
	{
		public:
			Topology(KDRenderer& renderer, const TopologyType& type)
				: rowTopology( TopologyTypeToD3DTopology(type) )
			{
			}

			void Bind(KDRenderer& renderer) const override
			{
				GetContext(renderer).IASetPrimitiveTopology(rowTopology);
			}

		private:
			static D3D11_PRIMITIVE_TOPOLOGY TopologyTypeToD3DTopology(const TopologyType& type)
			{
				switch (type)
				{
					case TopologyType_Unknown:
					{
						throw std::exception("Unknown topology type is not supported");
						return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
						break;
					}
					case TopologyType_PointList:
					{
						return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
						break;
					}
					case TopologyType_LineList:
					{
						return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
						break;
					}
					case TopologyType_LineStrip:
					{
						return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
						break;
					}
					case TopologyType_TriangleList:
					{
						return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
						break;
					}
					case TopologyType_TriangleStrip:
					{
						return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
						break;
					}
				default:
					throw std::exception("Unknown topology type has passed");
					return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
				}
			}
		private:
			D3D11_PRIMITIVE_TOPOLOGY rowTopology;
	};
}