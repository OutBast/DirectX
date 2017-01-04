//
// Game.cpp -
//

#include "pch.h"
#include "Game.h"

#define GAMMA_CORRECT_RENDERING

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

namespace
{
#ifdef GAMMA_CORRECT_RENDERING
	const XMVECTORF32 c_Gray = { 0.215861f, 0.215861f, 0.215861f, 1.f };
	const XMVECTORF32 c_CornflowerBlue = { 0.127438f, 0.300544f, 0.846873f, 1.f };
#else
	const XMVECTORF32 c_Gray = Colors::Gray;
	const XMVECTORF32 c_CornflowerBlue = Colors::CornflowerBlue;
#endif
}

// Constructor.
Game::Game() :
	m_showHud(true),
	m_leftCameraEnable(true),
	m_selectFile(0),
	m_firstFile(0),
	m_samplerState(nullptr)
{
#ifdef GAMMA_CORRECT_RENDERING
	m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
#else
	m_deviceResources = std::make_unique<DX::DeviceResources>();
#endif

#if !defined(_XBOX_ONE) || !defined(_TITLE)
	m_deviceResources->RegisterDeviceNotify(this);
#endif

	m_clearColor = c_CornflowerBlue.v;
	m_uiColor = Colors::Black;
	m_cameraLeft = Camera();
	m_cameraRight = Camera();
	m_camera = &m_cameraLeft;

}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();

	m_deviceResources->SetWindow(window, width, height);
	m_mouse->SetWindow(window);

	m_deviceResources->CreateDeviceResources();
	CreateDeviceDependentResources();

	m_viewportLeft = ViewportRendererData(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());
	m_viewportRight = ViewportRendererData(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());
	
	m_deviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();	

	m_model = new ModelResources(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext()); 
	m_model->LoadDefaultTextures();

	m_tessShaders = new TessellationShaders(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());
	m_tessShaders->LoadAndCompileShaders();
}

#pragma region Frame Update
// Executes basic game loop.
void Game::Tick()
{
	m_timer.Tick([&]()
	{
		Update(m_timer);
	});

   Render();
}

// Updates the world
void Game::Update(DX::StepTimer const& timer)
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	m_model->ReloadModelResourcesIfNeeded();


#if !defined(_XBOX_ONE) || !defined(_TITLE)

		RECT size = m_deviceResources->GetOutputSize();
		m_viewportLeft.SetViewport(Viewport(0.0f, 0.0f, size.right / 2.0f, size.bottom));
		m_viewportRight.SetViewport(Viewport(size.right / 2.0f, 0.0f, size.right / 2.0f, size.bottom));
		

		m_camera->Update(timer, m_keyboard->Get(), m_mouse->Get());

		m_tessShaders->UpdateWorldMatrix(m_model->GetWorld());

		auto kb = m_keyboard->GetState();
		m_keyboardTracker.Update(kb);		

		if (m_keyboardTracker.pressed.Z)
			m_showHud = !m_showHud;

		if (m_keyboardTracker.pressed.C)
			CycleBackgroundColor();

		if (m_keyboardTracker.pressed.Y)
		{
			PostMessage(m_deviceResources->GetWindowHandle(), WM_USER, 0, 0);
		}

		if (m_keyboardTracker.pressed.U)
		{
			PostMessage(m_deviceResources->GetWindowHandle(), WM_USER, 0, 1);
		}

		if (m_keyboardTracker.pressed.I)
		{
			PostMessage(m_deviceResources->GetWindowHandle(), WM_USER, 0, 2);
		}

		if (m_keyboardTracker.pressed.O)
		{
			PostMessage(m_deviceResources->GetWindowHandle(), WM_USER, 0, 3);
		}

		if (m_keyboardTracker.pressed.P)
		{
			PostMessage(m_deviceResources->GetWindowHandle(), WM_USER, 0, 4);
		}

		if (m_keyboardTracker.pressed.Tab)
		{
			m_leftCameraEnable = !m_leftCameraEnable;
			if (m_leftCameraEnable)
			{
				m_camera = &m_cameraLeft;
			}
			else
			{
				m_camera = &m_cameraRight;
			}
		}

		m_tessShaders->Update(m_keyboardTracker);
		m_tessShaders->UpdateGlobalDistance(&m_cameraLeft);

#endif
}
#pragma endregion

#pragma region Frame Render
void Game::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
		return;

	Clear();

	RECT size = m_deviceResources->GetOutputSize();

	if (!m_fileNames.empty())
	{
		m_spriteBatch->Begin();

		RECT rct = Viewport::ComputeTitleSafeArea(size.right, size.bottom);

		float spacing = m_fontComic->GetLineSpacing();

		int maxl = static_cast<int>(float(rct.bottom - rct.top + spacing) / spacing);

		if (m_selectFile < m_firstFile)
			m_firstFile = m_selectFile;

		if (m_selectFile > (m_firstFile + maxl))
			m_firstFile = m_selectFile;

		float y = float(rct.top);
		for (int j = m_firstFile; j < int(m_fileNames.size()); ++j)
		{
			XMVECTOR hicolor = m_uiColor;
			m_fontComic->DrawString(m_spriteBatch.get(), m_fileNames[j].c_str(), XMFLOAT2( float(rct.left), y), (m_selectFile == j) ? hicolor : c_Gray);
			y += spacing;
		}

		m_spriteBatch->End();
	}
	else
	{
		if (!m_model->GetModel())
		{
			m_spriteBatch->Begin();

			if (*m_model->m_szError)
			{
				m_fontComic->DrawString(m_spriteBatch.get(), m_model->m_szError, XMFLOAT2(100, 100), Colors::Red);
			}
			else
			{
				m_fontComic->DrawString(m_spriteBatch.get(), L"No model is loaded\nPress 'Y' to load a model from disk", XMFLOAT2(100, 100), Colors::Red);
			}

			m_spriteBatch->End();
		}
		else
		{
			m_tessShaders->Render(m_cameraLeft, *m_model, m_viewportLeft, m_deviceResources.get());
			m_tessShaders->Render(m_cameraRight, *m_model, m_viewportRight, m_deviceResources.get());

			RenderSplitScreen();

			RenderHUD();
		}
	}

	m_deviceResources->Present();
}

void Game::RenderSplitScreen()
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	RECT size = m_deviceResources->GetOutputSize();

	context->HSSetShader(nullptr, nullptr, 0);
	context->DSSetShader(nullptr, nullptr, 0);

	auto renderTarget = m_deviceResources->GetRenderTargetView();
	context->OMSetRenderTargets(1, &renderTarget, nullptr);

	m_spriteBatch->Begin();
	context->RSSetViewports(1, &m_viewportLeft.GetViewport());
	m_spriteBatch->Draw(m_viewportLeft.GetShaderResourceView().Get(), size);
	m_spriteBatch->End();

	m_spriteBatch->Begin();
	context->RSSetViewports(1, &m_viewportRight.GetViewport());
	m_spriteBatch->Draw(m_viewportRight.GetShaderResourceView().Get(), size);
	m_spriteBatch->End();

	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);
}

void Game::RenderHUD()
{
	if (*m_model->m_szStatus && m_showHud)
	{
		m_spriteBatch->Begin();

		Vector3 up = Vector3::TransformNormal(Vector3::Up, m_camera->GetView());

		WCHAR szCamera[256] = { 0 };
		swprintf_s(szCamera, L"Camera: (%8.4f,%8.4f,%8.4f) Look At: (%8.4f,%8.4f,%8.4f) Up: (%8.4f,%8.4f,%8.4f) FOV: %8.4f",
			(m_camera->GetLastCameraPos()).x, (m_camera->GetLastCameraPos()).y, (m_camera->GetLastCameraPos()).z,
			(m_camera->GetCameraFocus()).x, (m_camera->GetCameraFocus()).y, (m_camera->GetCameraFocus()).z,
			up.x, up.y, up.z, XMConvertToDegrees(m_camera->GetFov()));

		const WCHAR* mode = L"Clockwise";
		if (m_tessShaders->GetWireframe())
			mode = L"Wireframe";
		else
			mode = L"Clockwise";

		WCHAR szState[128] = { 0 };
		swprintf_s(szState, L"%ls", mode);

		WCHAR szSelectHullShader[100] = { 0 };
		const WCHAR* selectHullShaderText;
		switch (m_tessShaders->GetSelectHullShader())
		{
		case 1:
			selectHullShaderText = L"Partitioning ODD";
			break;
		case 2:
			selectHullShaderText = L"Partitioning EVEN";
			break;
		case 3:
			selectHullShaderText = L"Partitioning Integer";
			break;
		case 4:
			selectHullShaderText = L"Partitioning Pow2";
			break;
		}
		swprintf_s(szSelectHullShader, L"Tessellation Mode: %ls", selectHullShaderText);

		WCHAR szTessellation[100] = { 0 };
		const WCHAR* tessellationMode = m_tessShaders->GetTessellation() ? L"Turn ON" : L"Turn OFF";
		swprintf_s(szTessellation, L"Tessellation: %ls", tessellationMode);

		WCHAR szDisplacement[100] = { 0 };
		const WCHAR* displacementMode = m_tessShaders->GetDisplacementMap() ? L"Turn ON" : L"Turn OFF";
		swprintf_s(szDisplacement, L"Displacement Mapping: %ls", displacementMode);

		WCHAR szBacFaceCulling[100] = { 0 };
		const WCHAR* backFaceCullingMode = m_tessShaders->GetBackFaceCulling() ? L"Turn ON" : L"Turn OFF";
		swprintf_s(szBacFaceCulling, L"Back Face Culling: %ls", backFaceCullingMode);


		WCHAR szTessellationFactor[100] = { 0 };
		swprintf_s(szTessellationFactor, L"Tessellation Factor: %8.4f", m_tessShaders->GetTessellationFactor());

		WCHAR szDisplacementScale[100] = { 0 };
		swprintf_s(szDisplacementScale, L"Displacement Scale: %8.4f", m_tessShaders->GetDisplacementScale());

		WCHAR szDisplacementBias[100] = { 0 };
		swprintf_s(szDisplacementBias, L"Displacement Bias: %8.4f", m_tessShaders->GetDisplacementBias());

		WCHAR szDistance[100] = { 0 };
		swprintf_s(szDistance, L"Distance: %8.4f", m_tessShaders->GetGlobalDistance());

		m_fontConsolas->DrawString(m_spriteBatch.get(), m_model->m_szStatus, XMFLOAT2(0, 10), m_uiColor);
		m_fontConsolas->DrawString(m_spriteBatch.get(), szCamera, XMFLOAT2(0, 10 + 20), m_uiColor);
		m_fontConsolas->DrawString(m_spriteBatch.get(), szState, XMFLOAT2(0, 10 + 40), m_uiColor);

		m_fontConsolas->DrawString(m_spriteBatch.get(), szSelectHullShader, XMFLOAT2(0, 10 + 60), m_uiColor);
		m_fontConsolas->DrawString(m_spriteBatch.get(), szTessellation, XMFLOAT2(0, 10 + 80), m_uiColor);
		m_fontConsolas->DrawString(m_spriteBatch.get(), szDisplacement, XMFLOAT2(0, 10 + 100), m_uiColor);
		m_fontConsolas->DrawString(m_spriteBatch.get(), szBacFaceCulling, XMFLOAT2(0, 10 + 120), m_uiColor);
		m_fontConsolas->DrawString(m_spriteBatch.get(), szTessellationFactor, XMFLOAT2(0, 10 + 140), m_uiColor);
		m_fontConsolas->DrawString(m_spriteBatch.get(), szDisplacementScale, XMFLOAT2(0, 10 + 160), m_uiColor);
		m_fontConsolas->DrawString(m_spriteBatch.get(), szDisplacementBias, XMFLOAT2(0, 10 + 180), m_uiColor);
		m_fontConsolas->DrawString(m_spriteBatch.get(), szDistance, XMFLOAT2(0, 10 + 200), m_uiColor);

		m_spriteBatch->End();
	}
}

// Helper method to clear the backbuffers
void Game::Clear()
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	// Clear the views.
	auto renderTarget = m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_deviceResources->GetDepthStencilView();

	context->ClearRenderTargetView(m_viewportLeft.GetTargetView().Get(), m_clearColor);
	context->ClearRenderTargetView(m_viewportRight.GetTargetView().Get(), m_clearColor);
	context->ClearRenderTargetView(renderTarget, m_clearColor);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->OMSetRenderTargets(1, &renderTarget, depthStencil);
	
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
	m_keyboardTracker.Reset();
	m_mouseButtonTracker.Reset();
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
}

void Game::OnResuming()
{
	m_timer.ResetElapsedTime();
	m_keyboardTracker.Reset();
	m_mouseButtonTracker.Reset();
}

void Game::OnWindowSizeChanged(int width, int height)
{
	if (!m_deviceResources->WindowSizeChanged(width, height))
		return;

	CreateWindowSizeDependentResources();
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
	width = 1280;
	height = 720;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();
	auto context = m_deviceResources->GetD3DDeviceContext();

	m_spriteBatch = std::make_unique<SpriteBatch>(context);

	m_fontConsolas = std::make_unique<SpriteFont>(device, L"consolas.spritefont");
	m_fontComic = std::make_unique<SpriteFont>(device, L"comic.spritefont");
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
	auto size = m_deviceResources->GetOutputSize();
	m_cameraLeft.CreateProjection(&size);
	m_cameraRight.CreateProjection(&size);

	auto device = m_deviceResources->GetD3DDevice();

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	
	m_deviceResources->GetRenderTarget()->GetDesc(&textureDesc);
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	m_viewportLeft.LoadThings(textureDesc);
	m_viewportRight.LoadThings(textureDesc);
}

#if !defined(_XBOX_ONE) || !defined(_TITLE)
void Game::OnDeviceLost()
{
	m_spriteBatch.reset();
	m_fontConsolas.reset();
	m_fontComic.reset();
	m_model->Reset();
	m_samplerState.Reset();
	m_tessShaders->Reset();
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}

void Game::CycleBackgroundColor()
{
	if (m_clearColor == Vector4(c_CornflowerBlue.v))
	{
		m_clearColor = Colors::Black.v;
		m_uiColor = Colors::Yellow;
	}
	else if (m_clearColor == Vector4(Colors::Black.v))
	{
		m_clearColor = Colors::White.v;
		m_uiColor = Colors::Black.v;
	}
	else
	{
		m_clearColor = c_CornflowerBlue.v;
		m_uiColor = Colors::White.v;
	}
}

#endif
