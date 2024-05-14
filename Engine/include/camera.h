#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace core {

	class Camera {
	public:
		Camera(glm::mat4 transform, const float& fov, const float& aspectRatio, const float& n = 0.01f, const float& f = 1000.f);
		void update();

		glm::mat4& getViewMatrix() { return this->view; }
		glm::mat4& getViewInverseMatrix() { return this->viewInverse; }
		glm::mat4& getProjectionMatrix() { return this->projection; }
		glm::mat4& getProjectionInverseMatrix() { return this->projectionInverse; }

	private:
		glm::mat4 view;
		glm::mat4 viewInverse;
		glm::mat4 projection;
		glm::mat4 projectionInverse;

		static glm::mat4 getPerspective(float vertical_fov, float aspect_ratio, float n, float f);

	};
}
