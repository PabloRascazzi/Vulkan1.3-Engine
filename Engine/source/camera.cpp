#include <camera.h>
#include <rtime.h>
#include <input.h>

#define M_PI 3.14159265358979323846264338327950288

namespace core {

	Camera::Camera(glm::mat4 transform, const float& vert_fov, const glm::uvec2& resolution, const float& n, const float& f) :
		m_view(glm::inverse(transform)),
		m_viewInverse(transform),
		m_resolution(resolution),
		m_aspectRatio(static_cast<float>(resolution.x) / static_cast<float>(resolution.y)),                   // aspect = width / height
		m_fov(2 * glm::atan(glm::tan(glm::radians(vert_fov) * 0.5) * m_aspectRatio), glm::radians(vert_fov)), // fov.x = 2 * atan(tan(fov.y * 0.5) * aspect)
		m_focalLength(static_cast<glm::vec2>(m_resolution) / (2.0f * glm::tan(m_fov * 0.5f))),                // focal.xy = resolution.xy / (2.0 * tan(fov.xy * 0.5))
		m_projection(GetPerspective(glm::radians(vert_fov), m_aspectRatio, n, f)),
		m_projectionInverse(glm::inverse(m_projection)),
		m_position(glm::vec3(m_viewInverse[3])) {}

	void Camera::Update() {
		// Calculate translation amount.
		float movementSpeed = 10.0f;
		float delta = movementSpeed * static_cast<float>(Time::getDT());
		glm::vec3 movement = glm::vec3(
			(Input::getKey(INPUT_KEY_A) ? -delta : 0.0) + (Input::getKey(INPUT_KEY_D) ? delta : 0.0),
			(Input::getKey(INPUT_KEY_E) ? -delta : 0.0) + (Input::getKey(INPUT_KEY_Q) ? delta : 0.0),
			(Input::getKey(INPUT_KEY_W) ? -delta : 0.0) + (Input::getKey(INPUT_KEY_S) ? delta : 0.0));

		// Calculate rotation amount.
		float rotationSpeed = 50.0f;
		delta = rotationSpeed * static_cast<float>(Time::getDT());
		glm::vec3 rotation = glm::vec3(
			(Input::getKey(INPUT_KEY_UP)    ? delta : 0.0) + (Input::getKey(INPUT_KEY_DOWN)   ? -delta : 0.0),
			(Input::getKey(INPUT_KEY_LEFT)  ? delta : 0.0) + (Input::getKey(INPUT_KEY_RIGHT)  ? -delta : 0.0),
			(Input::getKey(INPUT_KEY_COMMA) ? delta : 0.0) + (Input::getKey(INPUT_KEY_PERIOD) ? -delta : 0.0));
		
		// Create tranformation matrix.
		glm::mat4 transform = glm::rotate(glm::rotate(glm::rotate(glm::translate(m_viewInverse, movement), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		m_view = glm::inverse(transform);
		m_viewInverse = transform;
		m_position = glm::vec3(m_viewInverse[3]);
	}

	glm::mat4 Camera::GetPerspective(float vert_fov, float aspect_ratio, float n, float f) {
		float focal_length = 1.0f / std::tan(vert_fov / 2.0f);

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