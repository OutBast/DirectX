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
	void Update(DirectX::Keyboard::KeyboardStateTracker& m_keyboardTracker, DirectX::Keyboard::State& m_keyborad);
	void UpdateGlobalDistance(Camera* camera);
	void CalculateTessellationFactor();
	void UpdateWorldMatrix(DirectX::SimpleMath::Matrix& m_world);
	void UpdateTessDisplFactors();
	WCHAR GetWhichHullShaderIsEnabled();
	void Render(Camera& camera, ModelResources& model, ViewportRendererData& viewport, DX::DeviceResources* deviceResource);

	int GetSelectHullShader() const { return m_selectHullShader; }
	void SetSelectHullShader(int val) { m_selectHullShader = val; }
	float GetTessellationFactor() const { return m_tessellationFactor; }
	void SetTessellationFactor(float val) { m_tessellationFactor = val; }
	float GetDisplacementScale() const { return m_displacementScale; }
	void SetDisplacementScale(float val) { m_displacementScale = val; }
	float GetDisplacementBias() const { return m_displacementBias; }
	void SetDisplacementBias(float val) { m_displacementBias = val; }
	float GetGlobalDistance() const { return m_globalDistance; }
	void SetGlobalDistance(float val) { m_globalDistance = val; }
	bool GetWireframe() const { return m_wireframe; }
	void SetWireframe(bool val) { m_wireframe = val; }
	bool GetTessellation() const { return m_tessellation; }
	void SetTessellation(bool val) { m_tessellation = val; }
	bool GetDisplacementMap() const { return m_displacementMap; }
	void SetDisplacementMap(bool val) { m_displacementMap = val; }
private:
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

	// Vertex buffer data
	Microsoft::WRL::ComPtr<ID3D11InputLayout>		m_vertexInputLayout;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>		m_samplerState;
	std::unique_ptr<DirectX::CommonStates>          m_states;


	// Shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader>		m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11HullShader>		m_hullShader;
	Microsoft::WRL::ComPtr<ID3D11HullShader>		m_hullShaderFE;
	Microsoft::WRL::ComPtr<ID3D11HullShader>		m_hullShaderInt;
	Microsoft::WRL::ComPtr<ID3D11HullShader>		m_hullShaderPow;
	Microsoft::WRL::ComPtr<ID3D11DomainShader>		m_domainShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_pixelShaderWireframe;

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
};

