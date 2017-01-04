
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


	DirectX::SimpleMath::Matrix GetWorld() const { return m_world; }
	void SetWorld(DirectX::SimpleMath::Matrix val) { m_world = val; }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetDiffuseTexture() const { return m_diffuseTexture; }
	void SetDiffuseTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> val) { m_diffuseTexture = val; }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetNormalTexture() const { return m_normalTexture; }
	void SetNormalTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> val) { m_normalTexture = val; }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSpecularTexture() const { return m_specularTexture; }
	void SetSpecularTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> val) { m_specularTexture = val; }
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetDisplacementTexture() const { return m_displacementTexture; }
	void SetDisplacementTexture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> val) { m_displacementTexture = val; }
	DirectX::Model* GetModel() const { return m_model.get(); }


	WCHAR												m_szStatus[512];
	WCHAR												m_szError[512];

private:
	ID3D11Device*										device;
	ID3D11DeviceContext*								context;

	std::unique_ptr<DirectX::Model>						m_model;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_diffuseTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_normalTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_specularTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_displacementTexture;

	DirectX::SimpleMath::Matrix							m_world;

	WCHAR												m_szModelName[MAX_PATH];
	WCHAR												m_szDiffuseTextureName[MAX_PATH];
	WCHAR												m_szDisplacementTextureName[MAX_PATH];
	WCHAR												m_szNormalTextureName[MAX_PATH];
	WCHAR												m_szSpecularTextureName[MAX_PATH];
	bool												m_reloadModel;
	bool												m_reloadDiffuseTexture;
	bool												m_reloadDisplacementTexture;
	bool												m_reloadNormalTexture;
	bool												m_reloadSpecularTexture;
};

