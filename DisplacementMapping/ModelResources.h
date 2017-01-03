
#include "DeviceResourcesPC.h"
#define _USE_MATH_DEFINES
#include <math.h>

#pragma once
class ModelResources
{
public:
	ModelResources(ID3D11Device* device, ID3D11DeviceContext* context);
	~ModelResources();


	void ReloadModelResourcesIfNeeded();
	void Reset();
	void LoadModel();
	void LoadDefaultTextures();
	void LoadDiffuseTexture();
	void LoadDisplacementTexture();
	void LoadNormalTexture();
	void LoadSpecularTexture();
	void OnModelOpen(const WCHAR* filename);
	void OnTextureOpen(const WCHAR* filename, LPARAM lParam);
	void DrawModel(ID3D11InputLayout* input);
	float ModelResources::DegreesToRadians(float degrees);


	std::unique_ptr<DirectX::Model>                 m_model;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_diffuseTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_normalTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_specularTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_displacementTexture;
	WCHAR                                           m_szModelName[MAX_PATH];
	WCHAR                                           m_szDiffuseTextureName[MAX_PATH];
	WCHAR                                           m_szDisplacementTextureName[MAX_PATH];
	WCHAR                                           m_szNormalTextureName[MAX_PATH];
	WCHAR                                           m_szSpecularTextureName[MAX_PATH];
	WCHAR                                           m_szStatus[512];
	WCHAR                                           m_szError[512];
	bool                                            m_reloadModel;
	bool											m_reloadDiffuseTexture;
	bool											m_reloadDisplacementTexture;
	bool											m_reloadNormalTexture;
	bool											m_reloadSpecularTexture;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	DirectX::SimpleMath::Matrix                      m_world;
};

