#include "pch.h"
#include "ModelResources.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


ModelResources::ModelResources(ID3D11Device* device, ID3D11DeviceContext* context) :
	m_reloadModel(false),
	m_reloadDiffuseTexture(false),
	m_reloadDisplacementTexture(false),
	m_reloadNormalTexture(false),
	m_reloadSpecularTexture(false),
	m_diffuseTexture(nullptr),
	m_normalTexture(nullptr),
	m_specularTexture(nullptr),
	m_displacementTexture(nullptr)
{
	*m_szModelName = 0;
	*m_szDiffuseTextureName = 0;
	*m_szDisplacementTextureName = 0;
	*m_szNormalTextureName = 0;
	*m_szSpecularTextureName = 0;
	*m_szStatus = 0;
	*m_szError = 0;

	m_world = Matrix::Identity;

	this->device = device;
	this->context = context;
}


ModelResources::~ModelResources()
{
}

void ModelResources::Reset()
{
	m_model.reset();
	m_diffuseTexture.Reset();
	m_displacementTexture.Reset();
	m_normalTexture.Reset();
	m_specularTexture.Reset();
}

void ModelResources::ReloadModelResourcesIfNeeded()
{
	if (m_reloadModel)
		LoadModel();

	if (m_reloadDiffuseTexture)
		LoadDiffuseTexture();
	if (m_reloadDisplacementTexture)
		LoadDisplacementTexture();
	if (m_reloadNormalTexture)
		LoadNormalTexture();
	if (m_reloadSpecularTexture)
		LoadSpecularTexture();
}

void ModelResources::OnModelOpen(const WCHAR* filename)
{
	if (!filename)
		return;

	wcscpy_s(m_szModelName, filename);
	m_reloadModel = true;
}

void ModelResources::OnTextureOpen(const WCHAR* filename, LPARAM lParam)
{
	if (!filename)
		return;

	if (lParam == 1)
	{
		wcscpy_s(m_szDiffuseTextureName, filename);
		m_reloadDiffuseTexture = true;
	}
	else if (lParam == 2)
	{
		wcscpy_s(m_szDisplacementTextureName, filename);
		m_reloadDisplacementTexture = true;
	}
	else if (lParam == 3)
	{
		wcscpy_s(m_szNormalTextureName, filename);
		m_reloadNormalTexture = true;
	}
	else if (lParam == 4)
	{
		wcscpy_s(m_szSpecularTextureName, filename);
		m_reloadSpecularTexture = true;
	}
}

void ModelResources::LoadModel()
{
	m_model.reset();
	*m_szStatus = 0;
	*m_szError = 0;
	m_reloadModel = false;

	if (!*m_szModelName)
		return;

	WCHAR drive[_MAX_DRIVE];
	WCHAR path[MAX_PATH];
	WCHAR ext[_MAX_EXT];
	WCHAR fname[_MAX_FNAME];
	_wsplitpath_s(m_szModelName, drive, _MAX_DRIVE, path, MAX_PATH, fname, _MAX_FNAME, ext, _MAX_EXT);
	
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
	catch (...)
	{
		swprintf_s(m_szError, L"Error loading model %ls%ls\n", fname, ext);
		m_model.reset();
		*m_szStatus = 0;
	}
	
	if (m_model)
	{
		size_t nmeshes = 0;
		size_t nverts = 0;
		size_t nfaces = 0;
		size_t nsubsets = 0;

		std::set<ID3D11Buffer*> vbs;
		for (auto it = m_model->meshes.cbegin(); it != m_model->meshes.cend(); ++it)
		{
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

		m_world = Matrix::CreateRotationX(DegreesToRadians(-90.0f));
	}
}

void ModelResources::LoadDefaultTextures()
{
	// Load the Textures
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Media\\Tiny\\Tiny_skin_COLOR.dds", nullptr, m_diffuseTexture.GetAddressOf()));
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Media\\Tiny\\Tiny_skin_NRM.dds", nullptr, m_normalTexture.GetAddressOf()));
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Media\\Tiny\\Tiny_skin_SPEC.dds", nullptr, m_specularTexture.GetAddressOf()));
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Media\\Tiny\\Tiny_skin_DISP.dds", nullptr, m_displacementTexture.GetAddressOf()));
}

void ModelResources::LoadDiffuseTexture()
{
	m_diffuseTexture.Reset();
	m_reloadDiffuseTexture = false;

	if (!*m_szDiffuseTextureName)
		return;
	
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, m_szDiffuseTextureName, nullptr, m_diffuseTexture.GetAddressOf()));
}

void ModelResources::LoadDisplacementTexture()
{
	m_displacementTexture.Reset();
	m_reloadDisplacementTexture = false;

	if (!*m_szDisplacementTextureName)
		return;
	
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, m_szDisplacementTextureName, nullptr, m_displacementTexture.GetAddressOf()));
}

void ModelResources::LoadNormalTexture()
{
	m_normalTexture.Reset();
	m_reloadNormalTexture = false;

	if (!*m_szNormalTextureName)
		return;
	
	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, m_szNormalTextureName, nullptr, m_normalTexture.GetAddressOf()));
}

void ModelResources::LoadSpecularTexture()
{
	m_specularTexture.Reset();
	m_reloadSpecularTexture = false;

	if (!*m_szSpecularTextureName)
		return;

	DX::ThrowIfFailed(CreateDDSTextureFromFile(device, m_szSpecularTextureName, nullptr, m_specularTexture.GetAddressOf()));
}

void ModelResources::DrawModel(ID3D11InputLayout* input)
{
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

			context->IASetInputLayout(input);
			context->IASetVertexBuffers(0, 1, &vb, &vbStride, &vbOffset);
			context->IASetIndexBuffer(part->indexBuffer.Get(), part->indexFormat, 0);

			context->DrawIndexed(part->indexCount, part->startIndex, part->vertexOffset);
		}
	}
}

float ModelResources::DegreesToRadians(float degrees)
{
	return (degrees * M_PI) / 180.0f;
}