#pragma once
#include "Bindable.h"

#include <wrl.h>

namespace KDE
{
	class Texture : public Bindable
	{
		public:
			Texture(KDRenderer& renderer, const char* filePath);

			void Bind(KDRenderer& renderer) const override;

		private:
			Microsoft::WRL::ComPtr<struct ID3D11ShaderResourceView> pSRView;
	};
}