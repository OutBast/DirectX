#pragma once
class ViewportRenderer
{
public:
	ViewportRenderer();
	~ViewportRenderer();
	

	D3D11_VIEWPORT										m_viewport;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_targetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_tagetTexture;

private:
};

