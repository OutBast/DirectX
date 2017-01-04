#include "Camera.h"
#include "ModelResources.h"
#include "ViewportRendererData.h"

#pragma once
class TessellationShaders
{
public:
	TessellationShaders(ID3D11Device* device, ID3D11DeviceContext* context);
	~TessellationShaders();

	void Reset();
	void LoadAndCompileShaders();
	void Update(DirectX::Keyboard::KeyboardStateTracker& m_keyboardTracker);
	void UpdateGlobalDistance(Camera* camera);
	void UpdateWorldMatrix(DirectX::SimpleMath::Matrix& m_world);
	void UpdateTessDisplFactors();
	WCHAR GetWhichHullShaderIsEnabled();
	void Render(Camera& camera, ModelResources& model, ViewportRendererData& viewport, DX::DeviceResources* deviceResource);


	// Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader>		m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11HullShader>		m_hullShader;
	Microsoft::WRL::ComPtr<ID3D11HullShader>		m_hullShaderFE;
	Microsoft::WRL::ComPtr<ID3D11HullShader>		m_hullShaderInt;
	Microsoft::WRL::ComPtr<ID3D11HullShader>		m_hullShaderPow;
	Microsoft::WRL::ComPtr<ID3D11DomainShader>		m_domainShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_pixelShaderWireframe;

	// Vertex buffer data
	Microsoft::WRL::ComPtr<ID3D11InputLayout>		m_vertexInputLayout;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>		m_samplerState;
	std::unique_ptr<DirectX::CommonStates>          m_states;

	// Shader resources
	enum ConstantBuffer
	{
		CB_Appliation,
		CB_Frame,
		CB_Object,
		CB_TessellationFactor,
		NumConstantBuffers
	};
	ID3D11Buffer* m_constantBuffers[NumConstantBuffers];

	ID3D11Device* device;
	ID3D11DeviceContext* context;
	int												m_selectHullShader;
	float											m_tessellationFactor;
	float											m_displacementScale;
	float											m_displacementBias;
	float											m_globalDistance;	
	bool                                            m_wireframe;
	bool                                            m_wireframeWithMaterial;
	bool											m_tessellation;
	bool											m_displacementMap;
	bool											m_backFaceCulling;
};

