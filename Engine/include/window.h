#pragma once

// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace core {

	typedef void(*WindowEventCallback)(SDL_Event);

	class Window {
	public:
		void setup();
		bool update();
		void cleanup();

		void* getHandle() { return window; }
		std::string getName() { return name; }
		long getWidth() { return width; }
		long getHeight() { return height; }
		float getAspectRatio() { return static_cast<float>(width) / static_cast<float>(height); }

		void setKeyCallback(WindowEventCallback callback);
		void setMouseButtonCallback(WindowEventCallback callback);
		void setMouseMotionCallback(WindowEventCallback callback);
		void setMouseWheelCallback(WindowEventCallback callback);

		void quit();

	private:
		SDL_Window* window;
		std::string name = "Vulkan Window";
		long width = 1280, height = 720;

		WindowEventCallback keyCallback;
		WindowEventCallback mouseButtonCallback;
		WindowEventCallback mouseMotionCallback;
		WindowEventCallback mouseWheelCallback;

		void createVulkanWindow();

	};
}