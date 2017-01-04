#pragma once
class ViewportRendererData
{
public:
	ViewportRendererData();
	ViewportRendererData(ID3D11Device* device, ID3D11DeviceContext* context);
	~ViewportRendererData();

	void LoadThings(D3D11_TEXTURE2D_DESC& textureDesc);

	D3D11_VIEWPORT GetViewport() const { return m_viewport; }
	void SetViewport(D3D11_VIEWPORT val) { m_viewport = val; }
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetTargetView() const { return m_targetView; }
	void SetTargetView(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> val) { m_targetView = val; }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetShaderResourceView() const { return m_shaderResourceView; }
	void SetShaderResourceView(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> val) { m_shaderResourceView = val; }
private:
	D3D11_VIEWPORT										m_viewport;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		m_targetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				m_tagetTexture;

	ID3D11DeviceContext* context;
	ID3D11Device* device;
};

