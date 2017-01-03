#include "pch.h"
#include "ViewportRendererData.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;


ViewportRendererData::ViewportRendererData(ID3D11Device* device, ID3D11DeviceContext* context)
{
	this->device = device;
	this->context = context;
}


ViewportRendererData::ViewportRendererData()
{

}

ViewportRendererData::~ViewportRendererData()
{
}

void ViewportRendererData::LoadThings(D3D11_TEXTURE2D_DESC& textureDesc)
{
	//D3D11_TEXTURE2D_DESC textureDesc;
	//ZeroMemory(&textureDesc, sizeof(textureDesc));

	//m_deviceResources->GetRenderTarget()->GetDesc(&textureDesc);
	//textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	DX::ThrowIfFailed(device->CreateTexture2D(&textureDesc, nullptr, m_tagetTexture.ReleaseAndGetAddressOf()));

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	DX::ThrowIfFailed(device->CreateRenderTargetView(m_tagetTexture.Get(), &renderTargetViewDesc, m_targetView.ReleaseAndGetAddressOf()));

	// Create the shader-resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	ZeroMemory(&srDesc, sizeof(srDesc));

	srDesc.Format = textureDesc.Format;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;

	DX::ThrowIfFailed(device->CreateShaderResourceView(m_tagetTexture.Get(), &srDesc, m_shaderResourceView.ReleaseAndGetAddressOf()));
	
}
