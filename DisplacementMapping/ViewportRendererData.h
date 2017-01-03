#pragma once
class ViewportRendererData
{
public:
	ViewportRendererData();
	ViewportRendererData(ID3D11Device* device, ID3D11DeviceContext* context);
	~ViewportRendererData();

	void LoadThings(D3D11_TEXTURE2D_DESC& textureDesc);
	

	ID3D11DeviceContext* context;
	ID3D11Device* device;

	D3D11_VIEWPORT										m_viewport;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_targetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_tagetTexture;

private:
};

