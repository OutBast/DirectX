#include "pch.h"
#include "Camera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Camera::Camera()
{
	m_view = Matrix::Identity;
	m_proj = Matrix::Identity;
	m_pitch = 0.0f;
	m_yaw = 0.0f;
	m_speed = 100.0f;
	m_distance = 100.0f;
	m_farPlane = 10000.f;
	SetFov(XM_PI / 4.f);
}

Camera::~Camera()
{
}

void Camera::Update(DX::StepTimer const& timer, DirectX::Keyboard& m_keyboard, DirectX::Mouse& m_mouse)
{

	float elapsedTime = float(timer.GetElapsedSeconds());

	auto kb = m_keyboard.GetState();

	// Camera movement
	Vector3 move = Vector3::Zero;
	float handed = -1.f;//(m_lhcoords) ? 1.f : -1.f;

	float scale = GetSpeed();
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

	//if (kb.Q || kb.E)
	//{
	//	if (kb.Q)
	//	{
	//		m_world *= Matrix::CreateRotationZ(0.5f * timer.GetElapsedSeconds());
	//	}
	//	else
	//	{
	//		m_world *= Matrix::CreateRotationZ(-0.5f * timer.GetElapsedSeconds());
	//	}
	//}

	// Mouse controls
	auto mouse = m_mouse.GetState();

	if (mouse.positionMode == Mouse::MODE_RELATIVE)
	{
		Vector3 delta = Vector3(float(mouse.x), float(mouse.y), 0.f) * 0.5f * timer.GetElapsedSeconds();

		m_pitch += delta.y;
		m_yaw += delta.x;

		// limit pitch to straight up or straight down
		// with a little fudge-factor to avoid gimbal lock
		float limit = XM_PI / 2.0f - 0.01f;
		m_pitch = std::max(-limit, m_pitch);
		m_pitch = std::min(+limit, m_pitch);

		// keep longitude in sane range by wrapping
		if (m_yaw > XM_PI)
		{
			m_yaw -= XM_PI * 2.0f;
		}
		else if (m_yaw < -XM_PI)
		{
			m_yaw += XM_PI * 2.0f;
		}
	}

	m_mouse.SetMode(mouse.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);

	Matrix im;
	(m_view).Invert(im);
	move = Vector3::TransformNormal(move, im);

	Quaternion q = Quaternion::CreateFromYawPitchRoll(m_yaw, m_pitch, 0.f);
	q.Normalize();

	q.Inverse(m_cameraRot);
	m_cameraFocus += move * elapsedTime;

	// Update camera
	Vector3 dir = Vector3::Transform(/*(m_lhcoords) ? Vector3::Forward :*/ Vector3::Backward, m_cameraRot);
	Vector3 up = Vector3::Transform(Vector3::Up, m_cameraRot);

	m_lastCameraPos = m_cameraFocus + GetDistance() * dir;


	m_view = XMMatrixLookAtRH(m_lastCameraPos, m_cameraFocus, up);
}


void Camera::CameraHome(DirectX::Mouse* m_mouse, DirectX::Model* m_model)
{
	m_mouse->ResetScrollWheelValue();
	SetFov(XM_PI / 4.f);
	m_cameraRot = Quaternion::Identity;

	if (!m_model)
	{
		m_cameraFocus = Vector3::Zero;
		m_distance = 100.f;
	}
	else
	{
		BoundingSphere sphere;
		BoundingBox box;

		for (auto it = m_model->meshes.cbegin(); it != m_model->meshes.cend(); ++it)
		{
			if (it == m_model->meshes.cbegin())
			{
				sphere = (*it)->boundingSphere;
				box = (*it)->boundingBox;
			}
			else
			{
				BoundingSphere::CreateMerged(sphere, sphere, (*it)->boundingSphere);
				BoundingBox::CreateMerged(box, box, (*it)->boundingBox);
			}
		}

		if (sphere.Radius < 1.f)
		{
			sphere.Center = box.Center;
			sphere.Radius = std::max(box.Extents.x, std::max(box.Extents.y, box.Extents.z));
		}

		if (sphere.Radius < 1.f)
		{
			sphere.Center = XMFLOAT3(0.f, 0.f, 0.f);
			sphere.Radius = 10.f;
		}

		m_distance = sphere.Radius * 2;

		m_cameraFocus = sphere.Center;
	}

	Vector3 dir = Vector3::Transform(Vector3::Backward, m_cameraRot);
	Vector3 up = Vector3::Transform(Vector3::Up, m_cameraRot);

	m_lastCameraPos = m_cameraFocus + m_distance * dir;
}

void Camera::CreateProjection(RECT* size)
{
	m_proj = Matrix::CreatePerspectiveFieldOfView(m_fov, float(size->right) / float(size->bottom), 0.1f, m_farPlane);
}