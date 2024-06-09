#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace core {

	class Camera {
	public:
		Camera(glm::mat4 transform, const float& fov, const float& aspectRatio, const float& n = 0.01f, const float& f = 1000.f);
		void Update();

		glm::mat4& GetViewMatrix() { return m_view; }
		glm::mat4& GetViewInverseMatrix() { return m_viewInverse; }
		glm::mat4& GetProjectionMatrix() { return m_projection; }
		glm::mat4& GetProjectionInverseMatrix() { return m_projectionInverse; }
		glm::vec3& GetWorldPosition() { return m_position; }
		glm::vec2& GetFOV() { return m_fov; }

	private:
		glm::vec2 m_fov;
		glm::mat4 m_view;
		glm::mat4 m_viewInverse;
		glm::mat4 m_projection;
		glm::mat4 m_projectionInverse;
		glm::vec3 m_position;

		static glm::mat4 GetPerspective(const glm::vec2& fov, float aspect_ratio, float n, float f);

	};
}
