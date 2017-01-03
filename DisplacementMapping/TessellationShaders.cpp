#include "pch.h"
#include "TessellationShaders.h"
//
//using namespace DirectX;
//using namespace DirectX::SimpleMath;
//
//#define MAX_TESSELLATION_DISTANCE 1000.0f
//#define MAX_TESS_FACTOR 63.0f
//#define MIN_TESS_FACTOR 1.0f
//#define TESS_STEP 0.1f;
//
//#define MAX_DISP_SCALE 20.0f
//#define MIN_DISP_SCALE -20.0f
//#define DISP_SCALE_STEP 0.01f;
//
//#define MAX_DISP_BIAS 5.0f
//#define MIN_DISP_BIAS -5.0f
//#define DISP_BIAS_STEP 0.01f;
//
//// Vertex data for a colored cube.
//struct VertexPosColor
//{
//	XMFLOAT3 Position;
//	XMFLOAT3 Color;
//};
//
//XMFLOAT4 shaderFactors;
//float oldTessellationFactor = 1.0f;
//float oldDisplacementScale = 0.0f;
//float oldDisplacementBias = 0.0f;
//
//TessellationShaders::TessellationShaders(ID3D11Device* device, ID3D11DeviceContext* context) :
//	m_selectHullShader(1), 
//	m_tessellationFactor(1.0f),
//	m_displacementScale(0.0f),
//	m_displacementBias(0.0f),
//	m_globalDistance(0.0f), 
//	m_wireframe(false),
//	m_wireframeWithMaterial(true),
//	m_tessellation(false),
//	m_displacementMap(false),
//	m_backFaceCulling(false),
//	m_samplerState(nullptr)
//{
//	shaderFactors = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
//	this->device = device;
//	this->context = context;
//}
//
//
//TessellationShaders::~TessellationShaders()
//{
//}
//
//void TessellationShaders::Reset()
//{
//	m_vertexShader.Reset();
//	m_hullShader.Reset();
//	m_domainShader.Reset();
//	m_pixelShader.Reset();
//	m_pixelShaderWireframe.Reset();
//	m_samplerState.Reset();
//	m_states.reset();
//}
//
//void TessellationShaders::LoadAndCompileShaders()
//{
//	m_states = std::make_unique<CommonStates>(device);
//
//	// Create the constant buffers for the variables defined in the vertex shader.
//	D3D11_BUFFER_DESC constantBufferDesc;
//	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));
//
//	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//	constantBufferDesc.ByteWidth = sizeof(XMMATRIX);
//	constantBufferDesc.CPUAccessFlags = 0;
//	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
//
//	device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffers[CB_Appliation]);
//
//	device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffers[CB_Frame]);
//
//	device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffers[CB_Object]);
//
//
//	constantBufferDesc.ByteWidth = sizeof(XMFLOAT4);
//	device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffers[CB_TessellationFactor]);
//
//	// Create the sample state
//	D3D11_SAMPLER_DESC sampDesc;
//	ZeroMemory(&sampDesc, sizeof(sampDesc));
//	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
//	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
//	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
//	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
//	sampDesc.MinLOD = 0;
//	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
//	DX::ThrowIfFailed(device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf()));
//
//	// Load & Complie Shaders
//	auto blob = DX::ReadData(L"VertexShader.cso");
//	DX::ThrowIfFailed(device->CreateVertexShader(blob.data(), blob.size(),
//		nullptr, m_vertexShader.ReleaseAndGetAddressOf()));
//
//	// Create the input layout for the vertex shader.
//	D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
//	{
//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor,Position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
//	};
//
//	DX::ThrowIfFailed(device->CreateInputLayout(vertexLayoutDesc, _countof(vertexLayoutDesc),
//		blob.data(), blob.size(), m_vertexInputLayout.ReleaseAndGetAddressOf()));
//
//	blob = DX::ReadData(L"FractionalOddHS.cso");
//	DX::ThrowIfFailed(device->CreateHullShader(blob.data(), blob.size(),
//		nullptr, m_hullShader.ReleaseAndGetAddressOf()));
//
//	blob = DX::ReadData(L"FractionalEvenHS.cso");
//	DX::ThrowIfFailed(device->CreateHullShader(blob.data(), blob.size(),
//		nullptr, m_hullShaderFE.ReleaseAndGetAddressOf()));
//
//	blob = DX::ReadData(L"IntegerHS.cso");
//	DX::ThrowIfFailed(device->CreateHullShader(blob.data(), blob.size(),
//		nullptr, m_hullShaderInt.ReleaseAndGetAddressOf()));
//
//	blob = DX::ReadData(L"Pow2HS.cso");
//	DX::ThrowIfFailed(device->CreateHullShader(blob.data(), blob.size(),
//		nullptr, m_hullShaderPow.ReleaseAndGetAddressOf()));
//
//	blob = DX::ReadData(L"DomainShader.cso");
//	DX::ThrowIfFailed(device->CreateDomainShader(blob.data(), blob.size(),
//		nullptr, m_domainShader.ReleaseAndGetAddressOf()));
//
//	blob = DX::ReadData(L"PixelShader.cso");
//	DX::ThrowIfFailed(device->CreatePixelShader(blob.data(), blob.size(),
//		nullptr, m_pixelShader.ReleaseAndGetAddressOf()));
//
//
//	blob = DX::ReadData(L"PixelShaderWireframe.cso");
//	DX::ThrowIfFailed(device->CreatePixelShader(blob.data(), blob.size(),
//		nullptr, m_pixelShaderWireframe.ReleaseAndGetAddressOf()));
//}
//
//void TessellationShaders::Update(DirectX::Keyboard::KeyboardStateTracker& m_keyboardTracker)
//{
//	if (m_keyboardTracker.pressed.R)
//		m_wireframe = !m_wireframe;
//
//	if (m_keyboardTracker.pressed.F)
//		m_wireframeWithMaterial = !m_wireframeWithMaterial;
//
//	if (m_keyboardTracker.pressed.F1)
//	{
//		m_tessellation = !m_tessellation;
//		if (m_tessellation)
//		{
//			m_tessellationFactor = oldTessellationFactor;
//		}
//		else
//		{
//			oldTessellationFactor = m_tessellationFactor;
//			m_tessellationFactor = MIN_TESS_FACTOR;
//		}
//	}
//
//	if (m_keyboardTracker.pressed.D1)
//	{
//		m_selectHullShader = 1;
//	}
//	if (m_keyboardTracker.pressed.D2)
//	{
//		m_selectHullShader = 2;
//	}
//	if (m_keyboardTracker.pressed.D3)
//	{
//		m_selectHullShader = 3;
//	}
//	if (m_keyboardTracker.pressed.D4)
//	{
//		m_selectHullShader = 4;
//	}
//
//	if (m_keyboardTracker.pressed.F2)
//	{
//		m_displacementMap = !m_displacementMap;
//		if (m_displacementMap)
//		{
//			m_displacementScale = oldDisplacementScale;
//			m_displacementBias = oldDisplacementBias;
//		}
//		else
//		{
//			oldDisplacementScale = m_displacementScale;
//			m_displacementScale = 0.0f;
//			oldDisplacementBias = m_displacementBias;
//			m_displacementBias = 0.0f;
//		}
//	}
//
//	if (m_keyboardTracker.pressed.F3)
//	{
//		m_backFaceCulling = !m_backFaceCulling;
//	}
//
//	if (m_tessellation)
//	{
//		if (m_keyboardTracker.pressed.H)
//		{
//			if (m_tessellationFactor < MAX_TESS_FACTOR)
//			{
//				m_tessellationFactor += TESS_STEP;
//			}
//			else
//			{
//				m_tessellationFactor = MAX_TESS_FACTOR;
//			}
//		}
//
//		if (m_keyboardTracker.pressed.B)
//		{
//			if (m_tessellationFactor > MIN_TESS_FACTOR)
//			{
//				m_tessellationFactor -= TESS_STEP;
//			}
//			else
//			{
//				m_tessellationFactor = MIN_TESS_FACTOR;
//			}
//		}
//	}
//
//	if (m_displacementMap)
//	{
//		if (m_keyboardTracker.pressed.J)
//		{
//			if (m_displacementScale < MAX_DISP_SCALE)
//			{
//				m_displacementScale += DISP_SCALE_STEP;
//			}
//			else
//			{
//				m_displacementScale = MAX_DISP_SCALE;
//			}
//		}
//
//		if (m_keyboardTracker.pressed.N)
//		{
//			if (m_displacementScale > MIN_DISP_SCALE)
//			{
//				m_displacementScale -= DISP_SCALE_STEP;
//			}
//			else
//			{
//				m_displacementScale = MIN_DISP_SCALE;
//			}
//		}
//
//		if (m_keyboardTracker.pressed.K)
//		{
//			if (m_displacementBias < MAX_DISP_BIAS)
//			{
//				m_displacementBias += DISP_BIAS_STEP;
//			}
//			else
//			{
//				m_displacementBias = MAX_DISP_BIAS;
//			}
//		}
//
//		if (m_keyboardTracker.pressed.M)
//		{
//			if (m_displacementBias > MIN_DISP_BIAS)
//			{
//				m_displacementBias -= DISP_BIAS_STEP;
//			}
//			else
//			{
//				m_displacementBias = MIN_DISP_BIAS;
//			}
//		}
//	}
//}
//
//void TessellationShaders::UpdateGlobalDistance(Camera* camera)
//{
//
//	m_globalDistance = SimpleMath::Vector3::Distance(camera->GetLastCameraPos(), Vector3(0.0f, 0.0f, 0.0f));
//	if (m_tessellation)
//	{
//		float distanceFactorToTess = m_globalDistance / MAX_TESSELLATION_DISTANCE;
//
//		m_tessellationFactor = (1 - distanceFactorToTess) * MAX_TESS_FACTOR + distanceFactorToTess;
//
//		if (m_globalDistance > MAX_TESSELLATION_DISTANCE)
//		{
//			m_tessellationFactor = MIN_TESS_FACTOR;
//		}
//	}
//	else
//	{
//		m_tessellationFactor = MIN_TESS_FACTOR;
//	}
//
//	UpdateTessDisplFactors();
//}
//
//void TessellationShaders::UpdateWorldMatrix(DirectX::SimpleMath::Matrix* m_world)
//{
//	context->UpdateSubresource(m_constantBuffers[CB_Object], 0, nullptr, &m_world, 0, 0);
//}
//
//void TessellationShaders::UpdateTessDisplFactors()
//{	
//	shaderFactors = XMFLOAT4(m_tessellationFactor, m_displacementScale, m_displacementBias, 0.0f);
//	context->UpdateSubresource(m_constantBuffers[CB_TessellationFactor], 0, nullptr, &shaderFactors, 0, 0);
//}
//
//WCHAR TessellationShaders::GetWhichHullShaderIsEnabled()
//{
//
//	const WCHAR* selectHullShaderText;
//	switch (m_selectHullShader)
//	{
//	case 1:
//		selectHullShaderText = L"Partitioning ODD";
//		break;
//	case 2:
//		selectHullShaderText = L"Partitioning EVEN";
//		break;
//	case 3:
//		selectHullShaderText = L"Partitioning Integer";
//		break;
//	case 4:
//		selectHullShaderText = L"Partitioning Pow2";
//		break;
//	}
//
//	return *selectHullShaderText;
//}
//
//void TessellationShaders::Render(Camera& camera, ModelResources& main_model, ViewportRendererData& viewport, DX::DeviceResources* deviceResource)
//{
//	// Set sampler state.
//	//ID3D11SamplerState* samplers[] =
//	//{
//	//	m_states->LinearWrap(),
//	//	m_states->LinearWrap(),
//	//};
//
//	// Set Rasterizer State
//	if (m_wireframe)
//		context->RSSetState(m_states->Wireframe());
//	else
//		context->RSSetState(m_states->CullClockwise());
//
//	auto depthStencil = deviceResource->GetDepthStencilView();
//
//	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
//
//	context->UpdateSubresource(m_constantBuffers[CB_Frame], 0, nullptr, &camera.GetView(), 0, 0);
//	context->UpdateSubresource(m_constantBuffers[CB_Appliation], 0, nullptr, &camera.GetProj(), 0, 0);
//
//	context->VSSetConstantBuffers(0, 4, m_constantBuffers);
//	context->HSSetConstantBuffers(0, 4, m_constantBuffers);
//	context->DSSetConstantBuffers(0, 4, m_constantBuffers);
//	context->PSSetConstantBuffers(0, 4, m_constantBuffers);
//
//	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
//
//	switch (m_selectHullShader)
//	{
//	case 1:
//		context->HSSetShader(m_hullShader.Get(), nullptr, 0);
//		break;
//	case 2:
//		context->HSSetShader(m_hullShaderFE.Get(), nullptr, 0);
//		break;
//	case 3:
//		context->HSSetShader(m_hullShaderInt.Get(), nullptr, 0);
//		break;
//	case 4:
//		context->HSSetShader(m_hullShaderPow.Get(), nullptr, 0);
//		break;
//	default:
//		context->HSSetShader(m_hullShader.Get(), nullptr, 0);
//		break;
//	}
//
//	context->DSSetShader(m_domainShader.Get(), nullptr, 0);
//	context->DSSetSamplers(0, 1, m_samplerState.GetAddressOf());
//	context->DSSetShaderResources(0, 1, main_model.m_displacementTexture.GetAddressOf());
//
//	context->GSSetShader(nullptr, nullptr, 0);
//
//	auto mainViewport = deviceResource->GetScreenViewport();
//	context->RSSetViewports(1, &mainViewport);
//
//	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
//	context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
//	context->PSSetShaderResources(0, 1, main_model.m_diffuseTexture.GetAddressOf());
//	context->PSSetShaderResources(1, 1, main_model.m_normalTexture.GetAddressOf());
//	context->PSSetShaderResources(2, 1, main_model.m_specularTexture.GetAddressOf());
//
//	context->OMSetRenderTargets(1, viewport.m_targetView.GetAddressOf(), depthStencil);
//	context->OMSetDepthStencilState(m_states->DepthDefault(), 1);
//
//	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
//	main_model.DrawModel(m_vertexInputLayout.Get());
//
//	if (m_wireframeWithMaterial)
//	{
//		context->RSSetState(m_states->Wireframe());
//		context->PSSetShader(m_pixelShaderWireframe.Get(), nullptr, 0);
//		context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
//		context->PSSetShaderResources(0, 1, main_model.m_diffuseTexture.GetAddressOf());
//		main_model.DrawModel(m_vertexInputLayout.Get());
//	}
//}
