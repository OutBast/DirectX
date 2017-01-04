//
// Game.h
//

#pragma once

#include "StepTimer.h"
#include "Camera.h"
#include "ViewportRendererData.h"
#include "ModelResources.h"
#include "TessellationShaders.h"

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

	// Properites
	void GetDefaultSize( int& width, int& height ) const;

	ModelResources* main_model;
private:

	void Update(DX::StepTimer const& timer);
	void Render();
	void RenderSplitScreen();
	void RenderHUD();

	void Clear();

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	
	void CycleBackgroundColor();

	// Device resources.
	std::unique_ptr<DX::DeviceResources>            m_deviceResources;

	// Rendering loop timer.
	DX::StepTimer                                   m_timer;

	std::unique_ptr<DirectX::SpriteBatch>           m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont>            m_fontConsolas;
	std::unique_ptr<DirectX::SpriteFont>            m_fontComic;
	
	ViewportRendererData							m_viewportLeft;
	ViewportRendererData							m_viewportRight;
	
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			m_samplerState;	
	std::unique_ptr<DirectX::Keyboard>              m_keyboard;
	std::unique_ptr<DirectX::Mouse>                 m_mouse;

	DirectX::Keyboard::KeyboardStateTracker         m_keyboardTracker;
	DirectX::Mouse::ButtonStateTracker              m_mouseButtonTracker;

	Camera*											m_camera;
	Camera											m_cameraLeft;
	Camera											m_cameraRight;

	DirectX::SimpleMath::Color                      m_clearColor;
	DirectX::SimpleMath::Color                      m_uiColor;

	bool                                            m_showHud;
	bool											m_leftCameraEnable;
	
	int                                             m_selectFile;
	int                                             m_firstFile;
	std::vector<std::wstring>                       m_fileNames;
	
	TessellationShaders* m_tessShaders;
};