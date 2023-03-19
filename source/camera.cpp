#include <camera.h>
#include <time.h>
#include <input.h>

#define M_PI 3.14159265358979323846264338327950288

namespace core {

	Camera::Camera(glm::mat4 transform, const float& fov, const float& aspectRatio, const float& n, const float& f) {
		this->view = glm::inverse(transform);
		this->viewInverse = transform;
		this->projection = getPerspective(fov, aspectRatio, n, f);
		this->projectionInverse = glm::inverse(projection);
	}

	void Camera::update() {
		float movementSpeed = 10.0f;
		float delta = movementSpeed * static_cast<float>(Time::getDT());
		glm::vec3 movement = glm::vec3(
			(Input::getKey(INPUT_KEY_A) ? -delta : 0.0) + (Input::getKey(INPUT_KEY_D) ? delta : 0.0),
			(Input::getKey(INPUT_KEY_E) ? -delta : 0.0) + (Input::getKey(INPUT_KEY_Q) ? delta : 0.0),
			(Input::getKey(INPUT_KEY_W) ? -delta : 0.0) + (Input::getKey(INPUT_KEY_S) ? delta : 0.0));

		glm::mat4 transform = glm::translate(viewInverse, movement);
		this->view = glm::inverse(transform);
		this->viewInverse = transform;
	}

	glm::mat4 Camera::getPerspective(float vertical_fov, float aspect_ratio, float n, float f) {
		float fov_rad = vertical_fov * 2.0f * static_cast<float>(M_PI) / 360.0f;
		float focal_length = 1.0f / std::tan(fov_rad / 2.0f);

		float x = focal_length / aspect_ratio;
		float y = -focal_length;
		float A = f / (n - f);
		float B = (n * f) / (n - f);

		glm::mat4 projection({
			x,    0.0f,  0.0f,  0.0f,
			0.0f,    y,  0.0f,  0.0f,
			0.0f, 0.0f,     A, -1.0f,
			0.0f, 0.0f,     B,  0.0f,
			});
		return projection;
	}
}