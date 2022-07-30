/*=============================================================================*/
// Copyright 2021 Elite Engine 2.0
// Authors: Thomas Goussaert
/*=============================================================================*/
// ECamera.h: Base Camera Implementation with movement
/*=============================================================================*/
#pragma once
#include "EMath.h"

namespace Elite
{
	class Camera
	{
	public:

		Camera(float aspectRatio, const FPoint3& position = { 0.f, 0.f, 0.f }, const FVector3& viewForward = { 0.f, 0.f, -1.f }, float fovAngle = 45.f, float nearPlane = 0.1f, float farPlane = 100.f);
		~Camera() = default;

		Camera(const Camera&) = delete;
		Camera(Camera&&) noexcept = delete;
		Camera& operator=(const Camera&) = delete;
		Camera& operator=(Camera&&) noexcept = delete;

		void Update(float elapsedSec);

		const FMatrix4& GetWorldToView() const { return m_WorldToView; }
		const FMatrix4& GetViewToWorld() const { return m_ViewToWorld; }

		const FMatrix4& GetProjectionMatrix() const;

		const float GetFov() const { return m_Fov; }

		const float GetFar() const { return m_Far; }
		const float GetNear() const { return m_Near; }

		void SetRenderMode() { m_UsingDirectx11 = !m_UsingDirectx11; };

	private:
		void CalculateLookAt();
		void CalculateProjection();

		float m_Fov{};
		float m_AspectRatio{};

		const float m_KeyboardMoveSensitivity{ 10.f };
		const float m_KeyboardMoveMultiplier{ 2.5f };
		const float m_MouseRotationSensitivity{ .1f };
		const float m_MouseMoveSensitivity{ 2.f };

		FPoint2 m_AbsoluteRotation{}; //Pitch(x) & Yaw(y) only
		FPoint3 m_RelativeTranslation{};

		FPoint3 m_Position{};
		const FVector3 m_ViewForward{};

		FMatrix4 m_WorldToView{};
		FMatrix4 m_ViewToWorld{};

		FMatrix4 m_ProjectionSRAS{};
		FMatrix4 m_ProjectionDX11{};

		float m_Near{};
		float m_Far{};

		bool m_UsingDirectx11 = false;
	};
}
