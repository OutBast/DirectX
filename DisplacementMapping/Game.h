//
// Game.h
//

#pragma once

#include "StepTimer.h"
#include "ArcBall.h"

#define _USE_MATH_DEFINES
#include <math.h>

#if defined(_XBOX_ONE) && defined(_TITLE)
#include "DeviceResourcesXDK.h"
#else
#include "DeviceResourcesPC.h"
#endif


// A basic game implementation that creates a D3D11 device and
// provides a game loop
class Game
#if !defined(_XBOX_ONE) || !defined(_TITLE)
	: public DX::IDeviceNotify
#endif
{
public:

	Game();

	// Initialization and management
#if defined(_XBOX_ONE) && defined(_TITLE)
	void Initialize(IUnknown* window);
#else
	void Initialize(HWND window, int width, int height);
#endif

	// Basic game loop
	void Tick();

#if !defined(_XBOX_ONE) || !defined(_TITLE)
	// IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;
#endif

	// Messages
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
	void OnWindowSizeChanged(int width, int height);
	void OnFileOpen(const WCHAR* filename);

	// Properites
	void GetDefaultSize( int& width, int& height ) const;

private:

	void Update(DX::StepTimer const& timer);
	void Render();

	void Clear();

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	
	void LoadModel();

	void CameraHome();

	void CycleBackgroundColor();

	void CreateProjection();
	
	void DrawModel();

	float DegreesToRadians(float degrees);

#if defined(_XBOX_ONE) && defined(_TITLE)
	void EnumerateModelFiles();
#endif

	// Device resources.
	std::unique_ptr<DX::DeviceResources>            m_deviceResources;

	// Rendering loop timer.
	DX::StepTimer                                   m_timer;

#if defined(_XBOX_ONE) && defined(_TITLE)
	std::unique_ptr<DirectX::GraphicsMemory>        m_graphicsMemory;
#endif

	std::unique_ptr<DirectX::SpriteBatch>           m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont>            m_fontConsolas;
	std::unique_ptr<DirectX::SpriteFont>            m_fontComic;
	std::unique_ptr<DirectX::Model>                 m_model;
	std::unique_ptr<DirectX::CommonStates>          m_states;

	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_lineBatch;

	D3D11_VIEWPORT			m_viewportAdaptive;
	D3D11_VIEWPORT			m_viewportDetails;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_firstTarget;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_secondTarget;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_firstTargetSh;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_secondTargetSh;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_firstTargetTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_secondTargetTexture;

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

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_diffuseTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_normalTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_specularTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_displacementTexture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>	m_samplerState;
	
	std::unique_ptr<DirectX::Keyboard>              m_keyboard;
	std::unique_ptr<DirectX::Mouse>                 m_mouse;

	DirectX::Keyboard::KeyboardStateTracker         m_keyboardTracker;
	DirectX::Mouse::ButtonStateTracker              m_mouseButtonTracker;

	DirectX::SimpleMath::Matrix                      m_world;
	DirectX::SimpleMath::Matrix*                     m_view;
	DirectX::SimpleMath::Matrix*                     m_proj;



	DirectX::SimpleMath::Vector3*                   m_cameraFocus;
	DirectX::SimpleMath::Vector3*                   m_lastCameraPos;
	DirectX::SimpleMath::Quaternion*                m_cameraRot;
	DirectX::SimpleMath::Quaternion*                m_viewRot;

	DirectX::SimpleMath::Vector3                    m_cameraFocus1;
	DirectX::SimpleMath::Vector3                    m_lastCameraPos1;
	DirectX::SimpleMath::Quaternion                 m_cameraRot1;
	DirectX::SimpleMath::Quaternion                 m_viewRot1;
	DirectX::SimpleMath::Matrix                     m_view1;
	DirectX::SimpleMath::Matrix                     m_proj1;

	DirectX::SimpleMath::Vector3                    m_cameraFocus2;
	DirectX::SimpleMath::Vector3                    m_lastCameraPos2;
	DirectX::SimpleMath::Quaternion                 m_cameraRot2;
	DirectX::SimpleMath::Quaternion                 m_viewRot2;
	DirectX::SimpleMath::Matrix                     m_view2;
	DirectX::SimpleMath::Matrix                     m_proj2;

	DirectX::SimpleMath::Color                      m_clearColor;
	DirectX::SimpleMath::Color                      m_uiColor;


	float                                           m_fov;
	float                                           m_distance;
	float                                           m_farPlane;
	float                                           m_sensitivity;

	bool                                            m_showHud;
	bool                                            m_wireframe;
	bool                                            m_wireframeWithMaterial;
	bool                                            m_ccw;
	bool                                            m_reloadModel;
	bool											m_tessellation;
	bool											m_displacementMap;
	bool											m_backFaceCulling;
	bool											m_leftCameraEnable;

	WCHAR                                           m_szModelName[MAX_PATH];
	WCHAR                                           m_szStatus[ 512 ];
	WCHAR                                           m_szError[ 512 ];
	
	int                                             m_selectFile;
	int                                             m_firstFile;
	int												m_selectHullShader;
	std::vector<std::wstring>                       m_fileNames;

	float											m_tessellationFactor;
	float											m_displacementScale;
	float											m_displacementBias;

	float*											m_pitch;
	float*											m_yaw;
	float											m_pitch1;
	float											m_yaw1;
	float											m_pitch2;
	float											m_yaw2;
	float m_globalDistance;
	float m_speed;
};