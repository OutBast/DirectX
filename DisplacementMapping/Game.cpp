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

#define MAX_TESSELLATION_DISTANCE 1000.0f
#define MAX_TESS_FACTOR 63.0f
#define MIN_TESS_FACTOR 1.0f
#define TESS_STEP 0.1f;

#define MAX_DISP_SCALE 20.0f
#define MIN_DISP_SCALE -20.0f
#define DISP_SCALE_STEP 0.01f;

#define MAX_DISP_BIAS 5.0f
#define MIN_DISP_BIAS -5.0f
#define DISP_BIAS_STEP 0.01f;

// Vertex data for a colored cube.
struct VertexPosColor
{
	XMFLOAT3 Position;
	XMFLOAT3 Color;
};

//Vector3 startCameraPosition = Vector3(0.0f, 0.0f, 500.0f)

XMFLOAT4 shaderFactors;
float oldTessellationFactor = 1.0f;
float oldDisplacementScale = 0.0f;
float oldDisplacementBias = 0.0f;

// Constructor.
Game::Game() :
	m_fov(XM_PI / 4.f),
	m_distance(100.f),
	m_speed(100.0f),
	m_farPlane(10000.f),
	m_sensitivity(1.f),
	m_showHud(true),
	m_wireframe(false),
	m_wireframeWithMaterial(true),
	m_ccw(false),
	m_reloadModel(false),
	m_tessellation(false),
	m_displacementMap(false),
	m_backFaceCulling(false),
	m_leftCameraEnable(true),
	m_selectFile(0),
	m_firstFile(0),
	m_selectHullShader(1),
	m_tessellationFactor(1.0f),
	m_displacementScale(0.0f),
	m_displacementBias(0.0f),
	m_globalDistance(0.0f),
	m_diffuseTexture(nullptr),
	m_normalTexture(nullptr),
	m_specularTexture(nullptr),
	m_displacementTexture(nullptr),
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

	m_clearColor = Colors::Black.v;
	m_uiColor = Colors::Yellow;

	*m_szModelName = 0;
	*m_szStatus = 0;
	*m_szError = 0;

	shaderFactors = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
	m_view1 = Matrix::Identity;
	m_view2 = Matrix::Identity;
	m_proj1 = Matrix::Identity;
	m_proj2 = Matrix::Identity;
	m_view = &m_view1;
	m_proj = &m_proj1;
	m_pitch1 = 0.0f;
	m_yaw1 = 0.0f;
	m_pitch2 = 0.0f;
	m_yaw2 = 0.0f;
	m_pitch = &m_pitch1;
	m_yaw = &m_yaw1;
}

// Initialize the Direct3D resources required to run.
#if defined(_XBOX_ONE) && defined(_TITLE)
void Game::Initialize(IUnknown* window)
#else
void Game::Initialize(HWND window, int width, int height)
#endif
{
	m_keyboard = std::make_unique<Keyboard>();
	m_mouse = std::make_unique<Mouse>();

#if defined(_XBOX_ONE) && defined(_TITLE)
	m_deviceResources->SetWindow(window);
#else
	m_deviceResources->SetWindow(window, width, height);
	m_mouse->SetWindow(window);
#endif

	m_deviceResources->CreateDeviceResources();
	CreateDeviceDependentResources();

	m_deviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();
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

	if (m_leftCameraEnable)
	{
		m_cameraFocus = &m_cameraFocus1;
		m_lastCameraPos = &m_lastCameraPos1;
		m_cameraRot = &m_cameraRot1;
		m_viewRot = &m_viewRot1;
		m_view = &m_view1;
		m_proj = &m_proj1;
		m_pitch = &m_pitch1;
		m_yaw = &m_yaw1;
	} 
	else
	{
		m_cameraFocus = &m_cameraFocus2;
		m_lastCameraPos = &m_lastCameraPos2;
		m_cameraRot = &m_cameraRot2;
		m_viewRot = &m_viewRot2;
		m_view = &m_view2;
		m_proj = &m_proj2;
		m_pitch = &m_pitch2;
		m_yaw = &m_yaw2;
	}


	auto context = m_deviceResources->GetD3DDeviceContext();

	if (m_reloadModel)
		LoadModel();

	float elapsedTime = float(timer.GetElapsedSeconds());

	float handed = -1.f;//(m_lhcoords) ? 1.f : -1.f;

#if !defined(_XBOX_ONE) || !defined(_TITLE)

		RECT size = m_deviceResources->GetOutputSize();
		m_viewportAdaptive = Viewport(0.0f, 0.0f, size.right / 2.0f, size.bottom);
		m_viewportDetails = Viewport(size.right / 2.0f, 0.0f, size.right / 2.0f, size.bottom);
		
		auto kb = m_keyboard->GetState();

		// Camera movement
		Vector3 move = Vector3::Zero;

		float scale = m_speed;
		if (kb.LeftShift || kb.RightShift)
			scale *= 0.5f;

		if (kb.Up)
			move.y += scale;

		if (kb.Down)
			move.y -= scale;

		if (kb.Right || kb.D)
			move.x += scale;

		if (kb.Left || kb.A)
			move.x -= scale;

		if (kb.PageUp || kb.W)
			move.z += scale * handed;

		if (kb.PageDown || kb.S)
			move.z -= scale * handed;

		if (kb.Q || kb.E)
		{
			if (kb.Q)
			{
				m_world *= Matrix::CreateRotationZ(0.5f * timer.GetElapsedSeconds());
			}
			else
			{
				m_world *= Matrix::CreateRotationZ(-0.5f * timer.GetElapsedSeconds());
			}
		}
		
		// Mouse controls
		auto mouse = m_mouse->GetState();

		if (mouse.positionMode == Mouse::MODE_RELATIVE)
		{
			Vector3 delta = Vector3(float(mouse.x), float(mouse.y), 0.f) * 0.5f * timer.GetElapsedSeconds();

			*m_pitch += delta.y;
			*m_yaw += delta.x;

			// limit pitch to straight up or straight down
			// with a little fudge-factor to avoid gimbal lock
			float limit = XM_PI / 2.0f - 0.01f;
			*m_pitch = std::max(-limit, *m_pitch);
			*m_pitch = std::min(+limit, *m_pitch);

			// keep longitude in sane range by wrapping
			if (*m_yaw > XM_PI)
			{
				*m_yaw -= XM_PI * 2.0f;
			}
			else if (*m_yaw < -XM_PI)
			{
				*m_yaw += XM_PI * 2.0f;
			}
		}

		m_mouse->SetMode(mouse.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
		
		Matrix im;
		(*m_view).Invert(im);
		move = Vector3::TransformNormal(move, im);

		Quaternion q = Quaternion::CreateFromYawPitchRoll(*m_yaw, *m_pitch, 0.f);
		q.Normalize();
		
		q.Inverse(*m_cameraRot);
		*m_cameraFocus += move * elapsedTime;

		if (kb.Home)
		{
			CameraHome();
		}
		
		// Update camera
		Vector3 dir = Vector3::Transform(/*(m_lhcoords) ? Vector3::Forward :*/ Vector3::Backward, *m_cameraRot);
		Vector3 up = Vector3::Transform(Vector3::Up, *m_cameraRot);

		*m_lastCameraPos = *m_cameraFocus + m_distance * dir;

		
		*m_view = XMMatrixLookAtRH(*m_lastCameraPos, *m_cameraFocus, up);
		context->UpdateSubresource(m_constantBuffers[CB_Object], 0, nullptr, &m_world, 0, 0);
				
		m_keyboardTracker.Update(kb);		

		if (m_keyboardTracker.pressed.R)
			m_wireframe = !m_wireframe;

		if (m_keyboardTracker.pressed.F)
			m_wireframeWithMaterial = !m_wireframeWithMaterial;

		if (m_keyboardTracker.pressed.T && !m_wireframe)
			m_ccw = !m_ccw;

		if (m_keyboardTracker.pressed.Z)
			m_showHud = !m_showHud;

		if (m_keyboardTracker.pressed.C)
			CycleBackgroundColor();

		if (m_keyboardTracker.pressed.O)
		{
			PostMessage(m_deviceResources->GetWindowHandle(), WM_USER, 0, 0);
		}

		if (m_keyboardTracker.pressed.F1)
		{
			m_tessellation = !m_tessellation;
			if (m_tessellation)
			{
				m_tessellationFactor = oldTessellationFactor;
			}
			else
			{
				oldTessellationFactor = m_tessellationFactor;
				m_tessellationFactor = MIN_TESS_FACTOR;
			}
		}

		if (m_keyboardTracker.pressed.D1)
		{
			m_selectHullShader = 1;
		}
		if (m_keyboardTracker.pressed.D2)
		{
			m_selectHullShader = 2;
		}
		if (m_keyboardTracker.pressed.D3)
		{
			m_selectHullShader = 3;
		}
		if (m_keyboardTracker.pressed.D4)
		{
			m_selectHullShader = 4;
		}

		if (m_keyboardTracker.pressed.Tab)
		{
			m_leftCameraEnable = !m_leftCameraEnable;
		}


		if (m_keyboardTracker.pressed.F2)
		{
			m_displacementMap = !m_displacementMap;
			if (m_displacementMap)
			{
				m_displacementScale = oldDisplacementScale;
				m_displacementBias = oldDisplacementBias;
			}
			else
			{
				oldDisplacementScale = m_displacementScale;
				m_displacementScale = 0.0f;
				oldDisplacementBias = m_displacementBias;
				m_displacementBias = 0.0f;
			}
		}

		if (m_keyboardTracker.pressed.F3)
		{
			m_backFaceCulling = !m_backFaceCulling;
		}

		if (m_tessellation)
		{
			if (kb.H)
			{
				if (m_tessellationFactor < MAX_TESS_FACTOR)
				{
					m_tessellationFactor += TESS_STEP;
				}
				else
				{
					m_tessellationFactor = MAX_TESS_FACTOR;
				}
			}


			if (kb.B)
			{
				if (m_tessellationFactor > MIN_TESS_FACTOR)
				{
					m_tessellationFactor -= TESS_STEP;
				}
				else
				{
					m_tessellationFactor = MIN_TESS_FACTOR;
				}
			}
		}

		if (m_displacementMap)
		{
			if (kb.J)
			{
				if (m_displacementScale < MAX_DISP_SCALE)
				{
					m_displacementScale += DISP_SCALE_STEP;
				}
				else
				{
					m_displacementScale = MAX_DISP_SCALE;
				}
			}

			if (kb.N)
			{
				if (m_displacementScale > MIN_DISP_SCALE)
				{
					m_displacementScale -= DISP_SCALE_STEP;
				}
				else
				{
					m_displacementScale = MIN_DISP_SCALE;
				}
			}

			if (kb.K)
			{
				if (m_displacementBias < MAX_DISP_BIAS)
				{
					m_displacementBias += DISP_BIAS_STEP;
				}
				else
				{
					m_displacementBias = MAX_DISP_BIAS;
				}
			}

			if (kb.M)
			{
				if (m_displacementBias > MIN_DISP_BIAS)
				{
					m_displacementBias -= DISP_BIAS_STEP;
				}
				else
				{
					m_displacementBias = MIN_DISP_BIAS;
				}
			}
		}

		m_globalDistance = SimpleMath::Vector3::Distance(m_lastCameraPos1, Vector3(0.0f, 0.0f, 0.0f));
		if (m_tessellation)
		{
			float distanceFactorToTess = m_globalDistance / MAX_TESSELLATION_DISTANCE;

			m_tessellationFactor = (1 - distanceFactorToTess) * MAX_TESS_FACTOR + distanceFactorToTess;

			if (m_globalDistance > MAX_TESSELLATION_DISTANCE)
			{
				m_tessellationFactor = MIN_TESS_FACTOR;
			}
		}
		else
		{
			m_tessellationFactor = MIN_TESS_FACTOR;
		}

		shaderFactors = XMFLOAT4(m_tessellationFactor, m_displacementScale, m_displacementBias, 0.0f);
		context->UpdateSubresource(m_constantBuffers[CB_TessellationFactor], 0, nullptr, &shaderFactors, 0, 0);
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
		if (!m_model)
		{
			m_spriteBatch->Begin();

			if (*m_szError)
			{
				m_fontComic->DrawString(m_spriteBatch.get(), m_szError, XMFLOAT2(100, 100), Colors::Red);
			}
			else
			{
				m_fontComic->DrawString(m_spriteBatch.get(), L"No model is loaded\nPress 'O' to load a model from disk", XMFLOAT2(100, 100), Colors::Red);
			}

			m_spriteBatch->End();
		}
		else
		{
			auto context = m_deviceResources->GetD3DDeviceContext();

			// Set sampler state.
			ID3D11SamplerState* samplers[] =
			{
				m_states->LinearWrap(),
				m_states->LinearWrap(),
			};

			// Set Rasterizer State
			if (m_wireframe)
				context->RSSetState(m_states->Wireframe());
			else
				context->RSSetState(m_ccw ? m_states->CullCounterClockwise() : m_states->CullClockwise());
			

			context->UpdateSubresource(m_constantBuffers[CB_Frame], 0, nullptr, &m_view1, 0, 0);
			context->UpdateSubresource(m_constantBuffers[CB_Appliation], 0, nullptr, &m_proj1, 0, 0);

			context->VSSetConstantBuffers(0, 4, m_constantBuffers);
			context->HSSetConstantBuffers(0, 4, m_constantBuffers);				
			context->DSSetConstantBuffers(0, 4, m_constantBuffers);
			context->PSSetConstantBuffers(0, 4, m_constantBuffers);

			context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

			switch (m_selectHullShader)
			{
			case 1:
				context->HSSetShader(m_hullShader.Get(), nullptr, 0);
				break;
			case 2:
				context->HSSetShader(m_hullShaderFE.Get(), nullptr, 0);
				break;
			case 3:
				context->HSSetShader(m_hullShaderInt.Get(), nullptr, 0);
				break;
			case 4:
				context->HSSetShader(m_hullShaderPow.Get(), nullptr, 0);
				break;
			default:
				context->HSSetShader(m_hullShader.Get(), nullptr, 0);
				break;
			}

			context->DSSetShader(m_domainShader.Get(), nullptr, 0);
			context->DSSetSamplers(0, 1, m_samplerState.GetAddressOf());
			context->DSSetShaderResources(0, 1, m_displacementTexture.GetAddressOf());
				
			context->GSSetShader(nullptr, nullptr, 0);

			auto viewport = m_deviceResources->GetScreenViewport();
			context->RSSetViewports(1, &viewport);
			//context->RSSetViewports(1, &m_viewportAdaptive);

			context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
			context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
			context->PSSetShaderResources(0, 1, m_diffuseTexture.GetAddressOf());
			context->PSSetShaderResources(1, 1, m_normalTexture.GetAddressOf());
			context->PSSetShaderResources(2, 1, m_specularTexture.GetAddressOf());

			auto renderTarget = m_deviceResources->GetRenderTargetView();
			auto depthStencil = m_deviceResources->GetDepthStencilView();
			//context->OMSetRenderTargets(1, m_firstTarget.GetAddressOf(), depthStencil);
			context->OMSetRenderTargets(1, m_firstTarget.GetAddressOf(), depthStencil);
			context->OMSetDepthStencilState(m_states->DepthDefault(), 1);

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			DrawModel();

			if (m_wireframeWithMaterial)
			{
				context->RSSetState(m_states->Wireframe());
				context->PSSetShader(m_pixelShaderWireframe.Get(), nullptr, 0);
				context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
				context->PSSetShaderResources(0, 1, m_diffuseTexture.GetAddressOf());
				DrawModel();
			}
			
			// Set Rasterizer State
			if (m_wireframe)
				context->RSSetState(m_states->Wireframe());
			else
				context->RSSetState(m_ccw ? m_states->CullCounterClockwise() : m_states->CullClockwise());


			context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			context->UpdateSubresource(m_constantBuffers[CB_Frame], 0, nullptr, &m_view2, 0, 0);
			context->UpdateSubresource(m_constantBuffers[CB_Appliation], 0, nullptr, &m_proj2, 0, 0);
			

			context->VSSetConstantBuffers(0, 4, m_constantBuffers);
			context->HSSetConstantBuffers(0, 4, m_constantBuffers);
			context->DSSetConstantBuffers(0, 4, m_constantBuffers);
			context->PSSetConstantBuffers(0, 4, m_constantBuffers);

			context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

			switch (m_selectHullShader)
			{
			case 1:
				context->HSSetShader(m_hullShader.Get(), nullptr, 0);
				break;
			case 2:
				context->HSSetShader(m_hullShaderFE.Get(), nullptr, 0);
				break;
			case 3:
				context->HSSetShader(m_hullShaderInt.Get(), nullptr, 0);
				break;
			case 4:
				context->HSSetShader(m_hullShaderPow.Get(), nullptr, 0);
				break;
			default:
				context->HSSetShader(m_hullShader.Get(), nullptr, 0);
				break;
			}


			context->DSSetShader(m_domainShader.Get(), nullptr, 0);
			context->DSSetSamplers(0, 1, m_samplerState.GetAddressOf());
			context->DSSetShaderResources(0, 1, m_displacementTexture.GetAddressOf());

			context->GSSetShader(nullptr, nullptr, 0);
			
			context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
			context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
			context->PSSetShaderResources(0, 1, m_diffuseTexture.GetAddressOf());
			context->PSSetShaderResources(1, 1, m_normalTexture.GetAddressOf());
			context->PSSetShaderResources(2, 1, m_specularTexture.GetAddressOf());

			context->OMSetRenderTargets(1, m_secondTarget.GetAddressOf(), depthStencil);
			context->OMSetDepthStencilState(m_states->DepthDefault(), 1);

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
			DrawModel();

			if (m_wireframeWithMaterial)
			{
				context->RSSetState(m_states->Wireframe());
				context->PSSetShader(m_pixelShaderWireframe.Get(), nullptr, 0);
				context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
				context->PSSetShaderResources(0, 1, m_diffuseTexture.GetAddressOf());
				DrawModel();
			}

			context->HSSetShader(nullptr, nullptr, 0);
			context->DSSetShader(nullptr, nullptr, 0);
			
			context->OMSetRenderTargets(1, &renderTarget, nullptr);


			RECT firstRect;
			firstRect.left = static_cast<LONG>(0);
			firstRect.top = static_cast<LONG>(0);
			firstRect.right = static_cast<LONG>(1920);
			firstRect.bottom = static_cast<LONG>(1080);


			m_spriteBatch->Begin();
			context->RSSetViewports(1, &m_viewportAdaptive);
			m_spriteBatch->Draw(m_firstTargetSh.Get(), firstRect);
			m_spriteBatch->End();

			m_spriteBatch->Begin();
			context->RSSetViewports(1, &m_viewportDetails);
			m_spriteBatch->Draw(m_secondTargetSh.Get(), firstRect);
			m_spriteBatch->End();


			context->RSSetViewports(1, &viewport);


			if (*m_szStatus && m_showHud)
			{
				m_spriteBatch->Begin();

				Vector3 up = Vector3::TransformNormal(Vector3::Up, *m_view);

				WCHAR szCamera[256] = { 0 };
				swprintf_s(szCamera, L"Camera: (%8.4f,%8.4f,%8.4f) Look At: (%8.4f,%8.4f,%8.4f) Up: (%8.4f,%8.4f,%8.4f) FOV: %8.4f",
					(*m_lastCameraPos).x, (*m_lastCameraPos).y, (*m_lastCameraPos).z,
					(*m_cameraFocus).x, (*m_cameraFocus).y, (*m_cameraFocus).z,
					up.x, up.y, up.z, XMConvertToDegrees(m_fov));

				const WCHAR* mode = m_ccw ? L"Counter clockwise" : L"Clockwise";
				if (m_wireframe)
					mode = L"Wireframe";

				WCHAR szState[128] = { 0 };
				swprintf_s(szState, L"%ls", mode);
				
				WCHAR szTessellation[100] = { 0 };
				const WCHAR* tessellationMode = m_tessellation ? L"Turn ON" : L"Turn OFF";
				swprintf_s(szTessellation, L"Tessellation: %ls", tessellationMode);

				WCHAR szDisplacement[100] = { 0 };
				const WCHAR* displacementMode = m_displacementMap ? L"Turn ON" : L"Turn OFF";
				swprintf_s(szDisplacement, L"Displacement Mapping: %ls", displacementMode);

				WCHAR szBacFaceCulling[100] = { 0 };
				const WCHAR* backFaceCullingMode = m_backFaceCulling ? L"Turn ON" : L"Turn OFF";
				swprintf_s(szBacFaceCulling, L"Back Face Culling: %ls", backFaceCullingMode);


				WCHAR szTessellationFactor[100] = { 0 };
				swprintf_s(szTessellationFactor, L"Tessellation Factor: %8.4f", m_tessellationFactor);

				WCHAR szDisplacementScale[100] = { 0 };
				swprintf_s(szDisplacementScale, L"Displacement Scale: %8.4f", m_displacementScale);

				WCHAR szDisplacementBias[100] = { 0 };
				swprintf_s(szDisplacementBias, L"Displacement Bias: %8.4f", m_displacementBias);

				WCHAR szDistance[100] = { 0 };
				swprintf_s(szDistance, L"Distance: %8.4f", m_globalDistance);


#if defined(_XBOX_ONE) && defined(_TITLE)
				RECT rct = Viewport::ComputeTitleSafeArea(size.right, size.bottom);

				m_fontConsolas->DrawString(m_spriteBatch.get(), m_szStatus, XMFLOAT2(float(rct.left), float(rct.top)), m_uiColor);
				m_fontConsolas->DrawString(m_spriteBatch.get(), szCamera, XMFLOAT2(float(rct.left), float(rct.top + 20)), m_uiColor);
				m_fontConsolas->DrawString(m_spriteBatch.get(), szState, XMFLOAT2(float(rct.left), float(rct.top + 40)), m_uiColor);
				if (m_usingGamepad)
				{
					m_fontConsolas->DrawString(m_spriteBatch.get(), szMode, XMFLOAT2(float(rct.right) - modeLen.x, float(rct.bottom) - modeLen.y), m_uiColor);
				}
#else
				m_fontConsolas->DrawString(m_spriteBatch.get(), m_szStatus, XMFLOAT2(0, 10), m_uiColor);
				m_fontConsolas->DrawString(m_spriteBatch.get(), szCamera, XMFLOAT2(0, 10 + 20), m_uiColor);
				m_fontConsolas->DrawString(m_spriteBatch.get(), szState, XMFLOAT2(0, 10 + 40), m_uiColor);
				
				m_fontConsolas->DrawString(m_spriteBatch.get(), szTessellation, XMFLOAT2(0, 10 + 60), m_uiColor);
				m_fontConsolas->DrawString(m_spriteBatch.get(), szDisplacement, XMFLOAT2(0, 10 + 80), m_uiColor);
				m_fontConsolas->DrawString(m_spriteBatch.get(), szBacFaceCulling, XMFLOAT2(0, 10 + 100), m_uiColor);
				m_fontConsolas->DrawString(m_spriteBatch.get(), szTessellationFactor, XMFLOAT2(0, 10 + 120), m_uiColor);
				m_fontConsolas->DrawString(m_spriteBatch.get(), szDisplacementScale, XMFLOAT2(0, 10 + 140), m_uiColor);
				m_fontConsolas->DrawString(m_spriteBatch.get(), szDisplacementBias, XMFLOAT2(0, 10 + 160), m_uiColor);
				m_fontConsolas->DrawString(m_spriteBatch.get(), szDistance, XMFLOAT2(0, 10 + 180), m_uiColor);
#endif

				m_spriteBatch->End();
			}
		}
	}

	m_deviceResources->Present();

#if defined(_XBOX_ONE) && defined(_TITLE)
	m_graphicsMemory->Commit();
#endif
}

// Helper method to clear the backbuffers
void Game::Clear()
{
	auto context = m_deviceResources->GetD3DDeviceContext();

	// Clear the views.
	auto renderTarget = m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_deviceResources->GetDepthStencilView();

	context->ClearRenderTargetView(m_secondTarget.Get(), m_clearColor);
	context->ClearRenderTargetView(m_firstTarget.Get(), m_clearColor);
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
#if defined(_XBOX_ONE) && defined(_TITLE)
	auto context = m_deviceResources->GetD3DDeviceContext();
	context->Suspend(0);
#endif
}

void Game::OnResuming()
{
#if defined(_XBOX_ONE) && defined(_TITLE)
	auto context = m_deviceResources->GetD3DDeviceContext();
	context->Resume();
#endif  
	m_timer.ResetElapsedTime();
	m_keyboardTracker.Reset();
	m_mouseButtonTracker.Reset();
}

#if !defined(_XBOX_ONE) || !defined(_TITLE)
void Game::OnWindowSizeChanged(int width, int height)
{
	if (!m_deviceResources->WindowSizeChanged(width, height))
		return;

	CreateWindowSizeDependentResources();
}
#endif

void Game::OnFileOpen(const WCHAR* filename)
{
	if (!filename)
		return;

	wcscpy_s(m_szModelName, filename);
	m_reloadModel = true;
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

#if defined(_XBOX_ONE) && defined(_TITLE)
	m_graphicsMemory = std::make_unique<GraphicsMemory>(device, m_deviceResources->GetBackBufferCount());
#endif

	m_spriteBatch = std::make_unique<SpriteBatch>(context);

	m_fontConsolas = std::make_unique<SpriteFont>(device, L"consolas.spritefont");
	m_fontComic = std::make_unique<SpriteFont>(device, L"comic.spritefont");

	m_states = std::make_unique<CommonStates>(device);
	
	// Create the constant buffers for the variables defined in the vertex shader.
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(XMMATRIX);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffers[CB_Appliation]);
	
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffers[CB_Frame]);
	
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffers[CB_Object]);


	constantBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffers[CB_TessellationFactor]);

	// Load the Textures
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Media\\Tiny\\Tiny_skin_COLOR.dds", nullptr, m_diffuseTexture.GetAddressOf()));
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Media\\Tiny\\Tiny_skin_NRM.dds", nullptr, m_normalTexture.GetAddressOf()));
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Media\\Tiny\\Tiny_skin_SPEC.dds", nullptr, m_specularTexture.GetAddressOf()));
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Media\\Tiny\\Tiny_skin_DISP.dds", nullptr, m_displacementTexture.GetAddressOf()));
	
	// Create the sample state
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	DX::ThrowIfFailed(device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf()));

	// Load & Complie Shaders
	auto blob = DX::ReadData(L"VertexShader.cso");
	DX::ThrowIfFailed(device->CreateVertexShader(blob.data(), blob.size(),
		nullptr, m_vertexShader.ReleaseAndGetAddressOf()));
	
	// Create the input layout for the vertex shader.
	D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor,Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	//device->CreateInputLayout(vertexLayoutDesc, _countof(vertexLayoutDesc), blob.data(), blob.size(), &m_vertexInputLayout);

	DX::ThrowIfFailed(device->CreateInputLayout(vertexLayoutDesc, _countof(vertexLayoutDesc),
		blob.data(), blob.size(), m_vertexInputLayout.ReleaseAndGetAddressOf()));
			
	blob = DX::ReadData(L"FractionalOddHS.cso");
	DX::ThrowIfFailed(device->CreateHullShader(blob.data(), blob.size(),
		nullptr, m_hullShader.ReleaseAndGetAddressOf()));

	blob = DX::ReadData(L"FractionalEvenHS.cso");
	DX::ThrowIfFailed(device->CreateHullShader(blob.data(), blob.size(),
		nullptr, m_hullShaderFE.ReleaseAndGetAddressOf()));

	blob = DX::ReadData(L"IntegerHS.cso");
	DX::ThrowIfFailed(device->CreateHullShader(blob.data(), blob.size(),
		nullptr, m_hullShaderInt.ReleaseAndGetAddressOf()));

	blob = DX::ReadData(L"Pow2HS.cso");
	DX::ThrowIfFailed(device->CreateHullShader(blob.data(), blob.size(),
		nullptr, m_hullShaderPow.ReleaseAndGetAddressOf()));

	blob = DX::ReadData(L"DomainShader.cso");
	DX::ThrowIfFailed(device->CreateDomainShader(blob.data(), blob.size(),
		nullptr, m_domainShader.ReleaseAndGetAddressOf()));

	blob = DX::ReadData(L"PixelShader.cso");
	DX::ThrowIfFailed(device->CreatePixelShader(blob.data(), blob.size(),
		nullptr, m_pixelShader.ReleaseAndGetAddressOf()));


	blob = DX::ReadData(L"PixelShaderWireframe.cso");
	DX::ThrowIfFailed(device->CreatePixelShader(blob.data(), blob.size(),
		nullptr, m_pixelShaderWireframe.ReleaseAndGetAddressOf()));
	
	m_world = Matrix::Identity;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
	auto size = m_deviceResources->GetOutputSize();
	CreateProjection();

	auto device = m_deviceResources->GetD3DDevice();

	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	/*textureDesc.Width = 1920;
	textureDesc.Height = 1080;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;*/

	m_deviceResources->GetRenderTarget()->GetDesc(&textureDesc);
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	DX::ThrowIfFailed(device->CreateTexture2D(&textureDesc, nullptr, m_firstTargetTexture.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateTexture2D(&textureDesc, nullptr, m_secondTargetTexture.ReleaseAndGetAddressOf()));



	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	DX::ThrowIfFailed(device->CreateRenderTargetView(m_firstTargetTexture.Get(), &renderTargetViewDesc, m_firstTarget.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateRenderTargetView(m_secondTargetTexture.Get(), &renderTargetViewDesc, m_secondTarget.ReleaseAndGetAddressOf()));

	// Create the shader-resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	ZeroMemory(&srDesc, sizeof(srDesc));

	srDesc.Format = textureDesc.Format;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;

	DX::ThrowIfFailed(device->CreateShaderResourceView(m_firstTargetTexture.Get(), &srDesc, m_firstTargetSh.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(device->CreateShaderResourceView(m_secondTargetTexture.Get(), &srDesc, m_secondTargetSh.ReleaseAndGetAddressOf()));
}

#if !defined(_XBOX_ONE) || !defined(_TITLE)
void Game::OnDeviceLost()
{
	m_spriteBatch.reset();
	m_fontConsolas.reset();
	m_fontComic.reset();
	m_model.reset();
	m_states.reset();
	m_lineBatch.reset();


	m_vertexShader.Reset();
	m_hullShader.Reset();
	m_domainShader.Reset();
	m_pixelShader.Reset();
	m_pixelShaderWireframe.Reset();

	m_diffuseTexture.Reset();
	m_samplerState.Reset();
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}
#endif

void Game::LoadModel()
{
	m_model.reset();
	*m_szStatus = 0;
	*m_szError = 0;
	m_reloadModel = false;

	if (!*m_szModelName)
		return;

	WCHAR drive[ _MAX_DRIVE ];
	WCHAR path[ MAX_PATH ];
	WCHAR ext[ _MAX_EXT ];
	WCHAR fname[ _MAX_FNAME ];
	_wsplitpath_s( m_szModelName, drive, _MAX_DRIVE, path, MAX_PATH, fname, _MAX_FNAME, ext, _MAX_EXT );

	auto device = m_deviceResources->GetD3DDevice();

	EffectFactory fx(device);

#ifdef GAMMA_CORRECT_RENDERING
	fx.EnableForceSRGB(true);
#endif

	if (*drive || *path)
	{
		WCHAR dir[MAX_PATH] = { 0 };
		_wmakepath_s(dir, drive, path, nullptr, nullptr);
		fx.SetDirectory(dir);
	}

	try
	{
		if (_wcsicmp(ext, L".sdkmesh") == 0)
		{
			m_model = Model::CreateFromSDKMESH(device, m_szModelName, fx);// , m_lhcoords);
		}
		else if (_wcsicmp(ext, L".cmo") == 0)
		{
			m_model = Model::CreateFromCMO(device, m_szModelName, fx);// , !m_lhcoords);
		}
		else if (_wcsicmp(ext, L".vbo") == 0)
		{
			m_model = Model::CreateFromVBO(device, m_szModelName, nullptr);// , m_lhcoords);
		}
		else
		{
			swprintf_s(m_szError, L"Unknown file type %ls", ext);
			m_model.reset();
			*m_szStatus = 0;
		}
	}
	catch(...)
	{
		swprintf_s(m_szError, L"Error loading model %ls%ls\n", fname, ext);
		m_model.reset();
		*m_szStatus = 0;
	}

	m_wireframe = false;

	if (m_model)
	{
		size_t nmeshes = 0;
		size_t nverts = 0;
		size_t nfaces = 0;
		size_t nsubsets = 0;

		std::set<ID3D11Buffer*> vbs;
		for (auto it = m_model->meshes.cbegin(); it != m_model->meshes.cend(); ++it)
		{
			if ( it == m_model->meshes.cbegin() )
			{
				m_ccw = (*it)->ccw;
			}

			for (auto mit = (*it)->meshParts.cbegin(); mit != (*it)->meshParts.cend(); ++mit)
			{
				++nsubsets;

				nfaces += ((*mit)->indexCount / 3);

				ID3D11Buffer* vbptr = (*mit)->vertexBuffer.Get();
				size_t vertexStride = (*mit)->vertexStride;

				if (vbptr && (vertexStride > 0) && vbs.find(vbptr) == vbs.end())
				{
					D3D11_BUFFER_DESC desc;
					vbptr->GetDesc(&desc);

					nverts += (desc.ByteWidth / vertexStride);

					vbs.insert(vbptr);
				}
			}
			++nmeshes;
		}

		if (nmeshes > 1)
		{
			swprintf_s(m_szStatus, L"Meshes: %6Iu   Verts: %6Iu   Faces: %6Iu   Subsets: %6Iu", nmeshes, nverts, nfaces, nsubsets);
		}
		else
		{
			swprintf_s(m_szStatus, L"Verts: %6Iu   Faces: %6Iu   Subsets: %6Iu", nverts, nfaces, nsubsets);
		}

		//m_world = Matrix::Identity;//Matrix::CreateRotationX(DegreesToRadians(-90.0f)) * Matrix::CreateRotationY(DegreesToRadians());
		m_world = Matrix::CreateRotationX(DegreesToRadians(-90.0f));
	}

	CameraHome();
}

void Game::CameraHome()
{
	m_mouse->ResetScrollWheelValue();
	m_fov = XM_PI / 4.f;
	*m_cameraRot = Quaternion::Identity;

	if (!m_model)
	{
		*m_cameraFocus = Vector3::Zero;
		m_distance = 100.f;
	}
	else
	{
		BoundingSphere sphere;
		BoundingBox box;

		for( auto it = m_model->meshes.cbegin(); it != m_model->meshes.cend(); ++it )
		{
			if ( it == m_model->meshes.cbegin() )
			{
				sphere = (*it)->boundingSphere;
				box = (*it)->boundingBox;
			}
			else
			{
				BoundingSphere::CreateMerged( sphere, sphere, (*it)->boundingSphere );
				BoundingBox::CreateMerged( box, box, (*it)->boundingBox );
			}
		}

		if ( sphere.Radius < 1.f )
		{
			sphere.Center = box.Center;
			sphere.Radius = std::max( box.Extents.x, std::max( box.Extents.y, box.Extents.z ) );
		}

		if ( sphere.Radius < 1.f )
		{
			sphere.Center = XMFLOAT3(0.f,0.f,0.f);
			sphere.Radius = 10.f;
		}
		
		m_distance = sphere.Radius * 2;

		*m_cameraFocus = sphere.Center;
	}

	Vector3 dir = Vector3::Transform(Vector3::Backward, *m_cameraRot);
	Vector3 up = Vector3::Transform(Vector3::Up, *m_cameraRot);

	*m_lastCameraPos = *m_cameraFocus + m_distance * dir;
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

void Game::CreateProjection()
{
	auto size = m_deviceResources->GetOutputSize();

	m_proj1 = Matrix::CreatePerspectiveFieldOfView(m_fov, float(size.right) / float(size.bottom), 0.1f, m_farPlane);
	m_proj2 = Matrix::CreatePerspectiveFieldOfView(m_fov, float(size.right) / float(size.bottom), 0.1f, m_farPlane);
}


void Game::DrawModel() 
{
	auto context = m_deviceResources->GetD3DDeviceContext();
	// Draw opaque parts
	for (auto it = m_model->meshes.cbegin(); it != m_model->meshes.cend(); ++it)
	{
		auto mesh = it->get();
		assert(mesh != 0);

		for (auto pit = mesh->meshParts.cbegin(); pit != mesh->meshParts.cend(); ++pit)
		{
			auto part = pit->get();

			auto vb = part->vertexBuffer.Get();
			UINT vbStride = part->vertexStride;
			UINT vbOffset = 0;

			context->IASetInputLayout(m_vertexInputLayout.Get());
			context->IASetVertexBuffers(0, 1, &vb, &vbStride, &vbOffset);
			context->IASetIndexBuffer(part->indexBuffer.Get(), part->indexFormat, 0);

			context->DrawIndexed(part->indexCount, part->startIndex, part->vertexOffset);
		}
	}
}

float Game::DegreesToRadians(float degrees)
{
	return (degrees * M_PI) / 180.0f;
}

#if defined(_XBOX_ONE) && defined(_TITLE)
void Game::EnumerateModelFiles()
{
	m_selectFile = m_firstFile = 0;
	m_fileNames.clear();

	WIN32_FIND_DATA ffdata = { 0 };

	static const WCHAR* exts[] = { L"D:\\*.sdkmesh", L"D:\\*.cmo", L"D:\\*.vbo" };
	
	for (size_t j = 0; j < _countof(exts); ++j)
	{
		HANDLE hFind = FindFirstFileEx( exts[j], FindExInfoStandard, &ffdata, FindExSearchNameMatch, nullptr, 0);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			continue;
		}

		if ( !(ffdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			OutputDebugStringW(ffdata.cFileName);
			std::wstring fname = ffdata.cFileName;

			m_fileNames.emplace_back(fname);
		}

		while (FindNextFile(hFind, &ffdata))
		{
			if (!(ffdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				OutputDebugStringW(ffdata.cFileName);
				std::wstring fname = ffdata.cFileName;

				m_fileNames.emplace_back(fname);
			}
		}

		FindClose(hFind);
	}
}
#endif