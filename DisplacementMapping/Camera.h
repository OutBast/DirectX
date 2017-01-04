#include "StepTimer.h"

#pragma once
class Camera
{
public:
	Camera();
	~Camera();

	void Update(DX::StepTimer const& timer, DirectX::Keyboard& kb, DirectX::Mouse& mouse);
	void CameraHome(DirectX::Mouse* m_mouse, DirectX::Model* m_model);
	void CreateProjection(RECT* size);

	DirectX::SimpleMath::Vector3 GetCameraFocus() const { return m_cameraFocus; }
	void SetCameraFocus(DirectX::SimpleMath::Vector3 val) { m_cameraFocus = val; }
	DirectX::SimpleMath::Vector3 GetLastCameraPos() const { return m_lastCameraPos; }
	void SetLastCameraPos(DirectX::SimpleMath::Vector3 val) { m_lastCameraPos = val; }
	DirectX::SimpleMath::Quaternion GetCameraRot() const { return m_cameraRot; }
	void SetCameraRot(DirectX::SimpleMath::Quaternion val) { m_cameraRot = val; }
	DirectX::SimpleMath::Quaternion GetViewRot() const { return m_viewRot; }
	void SetViewRot(DirectX::SimpleMath::Quaternion val) { m_viewRot = val; }
	DirectX::SimpleMath::Matrix GetView() const { return m_view; }
	void SetView(DirectX::SimpleMath::Matrix val) { m_view = val; }
	DirectX::SimpleMath::Matrix GetProj() const { return m_proj; }
	void SetProj(DirectX::SimpleMath::Matrix val) { m_proj = val; }

	float GetPitch() const { return m_pitch; }
	void SetPitch(float val) { m_pitch = val; }
	float GetYaw() const { return m_yaw; }
	void SetYaw(float val) { m_yaw = val; }

	float GetSpeed() const { return m_speed; }
	void SetSpeed(float val) { m_speed = val; }
	float GetDistance() const { return m_distance; }
	void SetDistance(float val) { m_distance = val; }

	float GetFov() const { return m_fov; }
	void SetFov(float val) { m_fov = val; }

private:
	DirectX::SimpleMath::Vector3                    m_cameraFocus;
	DirectX::SimpleMath::Vector3                    m_lastCameraPos;
	DirectX::SimpleMath::Quaternion                 m_cameraRot;
	DirectX::SimpleMath::Quaternion                 m_viewRot;
	DirectX::SimpleMath::Matrix                     m_view;
	DirectX::SimpleMath::Matrix                     m_proj;

	float											m_pitch;
	float											m_yaw;
	float											m_speed;
	float											m_distance; 
	float                                           m_fov;
	float                                           m_farPlane;
};

