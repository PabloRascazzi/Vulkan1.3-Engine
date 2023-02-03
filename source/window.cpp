#include <window.h>

#include <stdexcept>
#include <iostream>

namespace core {

	void Window::setup() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw std::runtime_error("Could not initialize SDL.");
        }
        createVulkanWindow();
	}

	bool Window::update() {
		// Event handler
	    SDL_Event event;
	    while (SDL_PollEvent(&event)) {
		    switch (event.type) {
                case SDL_QUIT: 
                    return false;
                    break;

                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    if(keyCallback) keyCallback(event);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    if(mouseButtonCallback) mouseButtonCallback(event);
                    break;

                case SDL_MOUSEMOTION:
                    if(mouseMotionCallback) mouseMotionCallback(event);
                    break;

                case SDL_MOUSEWHEEL:
                    if(mouseWheelCallback) mouseWheelCallback(event);
                    break;

                default:
                    // Do nothing.
                    break;
            }
	    }
        return true;
	}

	void Window::cleanup() {
        SDL_DestroyWindow(window);
        SDL_Quit();
	}

    void Window::createVulkanWindow() {
        // Create an SDL window that supports Vulkan rendering.
        window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN);
        if (window == NULL) {
            throw std::runtime_error("Could not create SDL window.");
        }
    }

    void Window::setKeyCallback(WindowEventCallback callback) {
	    keyCallback = callback;
    }

    void Window::setMouseButtonCallback(WindowEventCallback callback) {
        mouseButtonCallback = callback;
    }

    void Window::setMouseMotionCallback(WindowEventCallback callback) {
        mouseMotionCallback = callback;
    }

    void Window::setMouseWheelCallback(WindowEventCallback callback) {
        mouseWheelCallback = callback;
    }

    void Window::quit() {
        SDL_Event quitEvent;
        quitEvent.type = SDL_QUIT;
        if (!SDL_PushEvent(&quitEvent)) {
            std::cout << "Failed to push quit event." << std::endl;
        }
    }
}