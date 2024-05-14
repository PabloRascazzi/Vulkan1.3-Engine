#include <input.h>
#include <iostream>

std::map<InputKey, Input::Button*> Input::actualKeys;
std::map<InputMouseButton, Input::Button*> Input::actualMouseButtons;
#ifdef INPUT_USING_SDL
std::map<SDL_Keycode, Input::Button*> Input::sdlKeys;
std::map<SDL_Keycode, Input::Button*> Input::sdlMouseButtons;
#elif defined INPUT_USING_GLFW
std::map<int, Input::Button*> Input::glfwKeys;
std::map<int, Input::Button*> Input::glfwMouseButtons;
#endif

InputVec2 Input::mousePosition;
InputVec2 Input::mouseDownPosition;
InputVec2 Input::mousePositionDelta;
InputVec2 Input::mouseScrollDelta;

float Input::keyboardAxisSensitivity;
float Input::mouseHorizontalSensitivity;
float Input::mouseVerticalSensitivity;

// Allocate huge chunks of memory to then split into multiple Button pointers.
void* mem_keys;
void* mem_mouseButtons;

bool Input::getKey(InputKey key) {
	auto itr = actualKeys.find(key);
	if (itr != actualKeys.end()) { return itr->second->current; }
	else { std::cout << "INPUT ERROR: Key '" << key << "' could not be found." << std::endl; return false; }
}

bool Input::getKeyDown(InputKey key) {
	auto itr = actualKeys.find(key);
	if (itr != actualKeys.end()) { return itr->second->pressed; }
	else { std::cout << "INPUT ERROR: Key '" << key << "' could not be found." << std::endl; return false; }
}

bool Input::getKeyUp(InputKey key) {
	auto itr = actualKeys.find(key);
	if (itr != actualKeys.end()) { return itr->second->released; }
	else { std::cout << "INPUT ERROR: Key '" << key << "' could not be found." << std::endl; return false; }
}

bool Input::getMouseButton(InputMouseButton mouseButton) {
	auto itr = actualMouseButtons.find(mouseButton);
	if (itr != actualMouseButtons.end()) { return itr->second->current; }
	else { std::cout << "INPUT ERROR: Mouse button '" << mouseButton << "' could not be found." << std::endl; return false; }
}

bool Input::getMouseButtonDown(InputMouseButton mouseButton) {
	auto itr = actualMouseButtons.find(mouseButton);
	if (itr != actualMouseButtons.end()) { return itr->second->pressed; }
	else { std::cout << "INPUT ERROR: Mouse button '" << mouseButton << "' could not be found." << std::endl; return false; }
}

bool Input::getMouseButtonUp(InputMouseButton mouseButton) {
	auto itr = actualMouseButtons.find(mouseButton);
	if (itr != actualMouseButtons.end()) { return itr->second->released; }
	else { std::cout << "INPUT ERROR: Mouse button '" << mouseButton << "' could not be found." << std::endl; return false; }
}

InputVec2 Input::getMousePosition() { return mousePosition; }
InputVec2 Input::getMouseDownPosition() { return mouseDownPosition; }
InputVec2 Input::getMouseScroll() { return mouseScrollDelta; }

float Input::getAxis(InputAxis axisName) {
	float axis = 0;
	if (axisName == INPUT_AXIS_HORIZONTAL) {
		if (getKey(INPUT_KEY_LEFT)) axis -= keyboardAxisSensitivity;
		if (getKey(INPUT_KEY_RIGHT)) axis += keyboardAxisSensitivity;
	}
	else if (axisName == INPUT_AXIS_VERTICAL) {
		if (getKey(INPUT_KEY_UP)) axis += keyboardAxisSensitivity;
		if (getKey(INPUT_KEY_DOWN)) axis -= keyboardAxisSensitivity;
	}
	else { std::cout << "Input: warning invalid axis." << std::endl; }
	return axis;
}

float Input::getAxisMouse(InputAxis axisName, InputMouseButton buttonName) {
	float axis = 0;
	if (axisName == INPUT_AXIS_HORIZONTAL) {
		if(buttonName == INPUT_KEY_NULL || getMouseButton(buttonName)) 
			axis += -(float)(mousePositionDelta.x) * mouseHorizontalSensitivity;
	}
	else if (axisName == INPUT_AXIS_VERTICAL) {
		if(buttonName == INPUT_KEY_NULL || getMouseButton(buttonName)) 
			axis += -(float)(mousePositionDelta.y) * mouseVerticalSensitivity;
	}
	else { std::cout << "Input: warning invalid axis." << std::endl; }
	return axis;
}

void Input::reset() {
	mouseScrollDelta = InputVec2(0, 0);
	mousePositionDelta = InputVec2(0, 0);

	// Reset keys' pressed and released.
	for (auto itr = actualKeys.begin(); itr != actualKeys.end(); itr++) {
		itr->second->pressed = false; itr->second->released = false; 
	}
	// Reset mouse buttons' pressed and released.
	for (auto itr = actualMouseButtons.begin(); itr != actualMouseButtons.end(); itr++) {
		itr->second->pressed = false; itr->second->released = false; 
	}
}

void Input::cleanup() {
	delete[] mem_keys;
	delete[] mem_mouseButtons;
}

#ifdef INPUT_USING_SDL
	// Helper macro definitions using SDL
	#define setupKey(inputKey, sdlKey) actualKeys[inputKey] = &((Button*)mem_keys)[i]; sdlKeys[sdlKey] = &((Button*)mem_keys)[i]; i++;
	#define setupMouseButton(inputMouseButton, sdlMouseButton) actualMouseButtons[inputMouseButton] = &((Button*)mem_mouseButtons)[i]; sdlMouseButtons[sdlMouseButton] = &((Button*)mem_mouseButtons)[i];

void Input::setup() {
	uint64_t i;
	keyboardAxisSensitivity = 60.0f;
	mouseHorizontalSensitivity = 0.2f;
	mouseVerticalSensitivity = 0.2f;

	i = 0;
	mem_keys = new Button[InputKeySize];
	setupKey(INPUT_KEY_0, SDLK_0);
	setupKey(INPUT_KEY_1, SDLK_1);
	setupKey(INPUT_KEY_2, SDLK_2);
	setupKey(INPUT_KEY_3, SDLK_3);
	setupKey(INPUT_KEY_4, SDLK_4);
	setupKey(INPUT_KEY_5, SDLK_5);
	setupKey(INPUT_KEY_6, SDLK_6);
	setupKey(INPUT_KEY_7, SDLK_7);
	setupKey(INPUT_KEY_8, SDLK_8);
	setupKey(INPUT_KEY_9, SDLK_9);
	setupKey(INPUT_KEY_A, SDLK_a);
	setupKey(INPUT_KEY_B, SDLK_b);
	setupKey(INPUT_KEY_C, SDLK_c);
	setupKey(INPUT_KEY_D, SDLK_d);
	setupKey(INPUT_KEY_E, SDLK_e);
	setupKey(INPUT_KEY_F, SDLK_f);
	setupKey(INPUT_KEY_G, SDLK_g);
	setupKey(INPUT_KEY_H, SDLK_h);
	setupKey(INPUT_KEY_I, SDLK_i);
	setupKey(INPUT_KEY_J, SDLK_j);
	setupKey(INPUT_KEY_K, SDLK_k);
	setupKey(INPUT_KEY_L, SDLK_l);
	setupKey(INPUT_KEY_M, SDLK_m);
	setupKey(INPUT_KEY_N, SDLK_n);
	setupKey(INPUT_KEY_O, SDLK_o);
	setupKey(INPUT_KEY_P, SDLK_p);
	setupKey(INPUT_KEY_Q, SDLK_q);
	setupKey(INPUT_KEY_R, SDLK_r);
	setupKey(INPUT_KEY_S, SDLK_s);
	setupKey(INPUT_KEY_T, SDLK_t);
	setupKey(INPUT_KEY_U, SDLK_u);
	setupKey(INPUT_KEY_V, SDLK_v);
	setupKey(INPUT_KEY_W, SDLK_w);
	setupKey(INPUT_KEY_X, SDLK_x);
	setupKey(INPUT_KEY_Y, SDLK_y);
	setupKey(INPUT_KEY_Z, SDLK_z);
	setupKey(INPUT_KEY_UP, SDLK_UP);
	setupKey(INPUT_KEY_DOWN, SDLK_DOWN);
	setupKey(INPUT_KEY_LEFT, SDLK_LEFT);
	setupKey(INPUT_KEY_RIGHT, SDLK_RIGHT);
	setupKey(INPUT_KEY_F1, SDLK_F1);
	setupKey(INPUT_KEY_F2, SDLK_F2);
	setupKey(INPUT_KEY_F3, SDLK_F3);
	setupKey(INPUT_KEY_F4, SDLK_F4);
	setupKey(INPUT_KEY_F5, SDLK_F5);
	setupKey(INPUT_KEY_F6, SDLK_F6);
	setupKey(INPUT_KEY_F7, SDLK_F7);
	setupKey(INPUT_KEY_F8, SDLK_F8);
	setupKey(INPUT_KEY_F9, SDLK_F9);
	setupKey(INPUT_KEY_F10, SDLK_F10);
	setupKey(INPUT_KEY_F11, SDLK_F11);
	setupKey(INPUT_KEY_F12, SDLK_F12);
	setupKey(INPUT_KEY_LEFT_SHIFT, SDLK_LSHIFT);
	setupKey(INPUT_KEY_LEFT_CONTROL, SDLK_LCTRL);
	setupKey(INPUT_KEY_LEFT_ALT, SDLK_LALT);
	setupKey(INPUT_KEY_RIGHT_SHIFT, SDLK_RSHIFT);
	setupKey(INPUT_KEY_RIGHT_CONTROL, SDLK_RCTRL);
	setupKey(INPUT_KEY_RIGHT_ALT, SDLK_RALT);
	setupKey(INPUT_KEY_ESCAPE, SDLK_ESCAPE);
	setupKey(INPUT_KEY_GRAVE_ACCENT, SDLK_BACKQUOTE);
	setupKey(INPUT_KEY_MINUS, SDLK_MINUS);
	setupKey(INPUT_KEY_EQUAL, SDLK_EQUALS);
	setupKey(INPUT_KEY_BACKSPACE, SDLK_BACKSPACE);
	setupKey(INPUT_KEY_TAB, SDLK_TAB);
	setupKey(INPUT_KEY_ENTER, SDLK_RETURN);
	setupKey(INPUT_KEY_SPACE, SDLK_SPACE);
	setupKey(INPUT_KEY_LEFT_BRACKET, SDLK_LEFTBRACKET);
	setupKey(INPUT_KEY_RIGHT_BRACKET, SDLK_RIGHTBRACKET);
	setupKey(INPUT_KEY_BACKSLASH, SDLK_BACKSLASH);
	setupKey(INPUT_KEY_SEMICOLON, SDLK_SEMICOLON);
	setupKey(INPUT_KEY_APOSTROPHE, SDLK_QUOTE);
	setupKey(INPUT_KEY_COMMA, SDLK_COMMA);
	setupKey(INPUT_KEY_PERIOD, SDLK_PERIOD);
	setupKey(INPUT_KEY_SLASH, SDLK_SLASH);
	setupKey(INPUT_KEY_CAPS_LOCK, SDLK_CAPSLOCK);
	setupKey(INPUT_KEY_SCROLL_LOCK, SDLK_SCROLLLOCK);
	setupKey(INPUT_KEY_NUM_LOCK, SDLK_NUMLOCKCLEAR);
	setupKey(INPUT_KEY_PRINT_SCREEN, SDLK_PRINTSCREEN);
	setupKey(INPUT_KEY_PAUSE, SDLK_PAUSE);
	setupKey(INPUT_KEY_INSERT, SDLK_INSERT);
	setupKey(INPUT_KEY_DELETE, SDLK_DELETE);
	setupKey(INPUT_KEY_HOME, SDLK_HOME);
	setupKey(INPUT_KEY_END, SDLK_END);
	setupKey(INPUT_KEY_PAGE_UP, SDLK_PAGEUP);
	setupKey(INPUT_KEY_PAGE_DOWN, SDLK_PAGEDOWN);
	setupKey(INPUT_KEY_KP_0, SDLK_KP_0);
	setupKey(INPUT_KEY_KP_1, SDLK_KP_1);
	setupKey(INPUT_KEY_KP_2, SDLK_KP_2);
	setupKey(INPUT_KEY_KP_3, SDLK_KP_3);
	setupKey(INPUT_KEY_KP_4, SDLK_KP_4);
	setupKey(INPUT_KEY_KP_5, SDLK_KP_5);
	setupKey(INPUT_KEY_KP_6, SDLK_KP_6);
	setupKey(INPUT_KEY_KP_7, SDLK_KP_7);
	setupKey(INPUT_KEY_KP_8, SDLK_KP_8);
	setupKey(INPUT_KEY_KP_9, SDLK_KP_9);
	setupKey(INPUT_KEY_KP_PERIOD, SDLK_KP_PERIOD);
	setupKey(INPUT_KEY_KP_DIVIDE, SDLK_KP_DIVIDE);
	setupKey(INPUT_KEY_KP_MULTIPLY, SDLK_KP_MULTIPLY);
	setupKey(INPUT_KEY_KP_ADD, SDLK_KP_PLUS);
	setupKey(INPUT_KEY_KP_SUBSTRACT, SDLK_KP_MINUS);
	setupKey(INPUT_KEY_KP_ENTER, SDLK_KP_ENTER);
	setupKey(INPUT_KEY_KP_EQUAL, SDLK_KP_EQUALS);

	i = 0;
	mem_mouseButtons = new Button[InputMouseButtonSize];
	setupMouseButton(INPUT_MOUSE_BUTTON_1, SDL_BUTTON_LEFT);
	setupMouseButton(INPUT_MOUSE_BUTTON_2, SDL_BUTTON_RIGHT);
	setupMouseButton(INPUT_MOUSE_BUTTON_3, SDL_BUTTON_MIDDLE);
	setupMouseButton(INPUT_MOUSE_BUTTON_4, SDL_BUTTON_X1);
	setupMouseButton(INPUT_MOUSE_BUTTON_5, SDL_BUTTON_X2);
}

void Input::keyCallback(SDL_Event event) {
	SDL_Keycode keycode = event.key.keysym.sym;
	Button* pButton = nullptr;

	// Look for button pointer using keycode.
	auto itr = sdlKeys.find(keycode);
	if (itr == sdlKeys.end()) {
#if defined(_DEBUG)
		std::cout << "INPUT WARNING: Key '" << keycode << "' not found." << std::endl;
#endif
		return;
	} pButton = itr->second; 

	// Set key's new state.
	if (event.type == SDL_KEYDOWN) {
		if (!pButton->current) pButton->pressed = true; 
		pButton->current = true;
	}
	else if (event.type == SDL_KEYUP) {
		if (!pButton->current) pButton->released = true; 
		pButton->current = false;
	}
}

void Input::mouseButtonCallback(SDL_Event event) {
	uint8_t button = event.button.button;
	Button* pButton = nullptr;

	// Look for button pointer using keycode.
	auto itr = sdlMouseButtons.find(button);
	if (itr == sdlMouseButtons.end()) {
#if defined(_DEBUG)
		std::cout << "INPUT WARNING: Mouse button '" << button << "' not found." << std::endl;
#endif
		return;
	} pButton = itr->second; 

	// Set mouse button's new state.
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		if (!pButton->current) pButton->pressed = true; 
		pButton->current = true;
		
		// Set mouse down position.
		if (button == SDL_BUTTON_LEFT) {
			int xpos, ypos; SDL_GetMouseState(&xpos, &ypos);
			mouseDownPosition = InputVec2(xpos, ypos);
		}
	}
	else if (event.type == SDL_MOUSEBUTTONUP) {
		if (!pButton->current) pButton->released = true; 
		pButton->current = false;
	}
}

void Input::mouseMotionCallback(SDL_Event event) {
	InputVec2 lastMousePosition = mousePosition;
	mousePosition = InputVec2(event.motion.x, event.motion.y);
	mousePositionDelta = lastMousePosition - mousePosition;
}

void Input::mouseScrollCallback(SDL_Event event) {
	mouseScrollDelta = InputVec2(event.wheel.preciseX, event.wheel.preciseY);
}

#elif defined INPUT_USING_GLFW
	// Helper macro definitions using GLFW
	#define setupKey(inputKey, glfwKey) actualKeys[inputKey] = &((Button*)mem_keys)[i]; glfwKeys[glfwKey] = &((Button*)mem_keys)[i]; i++;
	#define setupMouseButton(inputMouseButton, glfwMouseButton) actualMouseButtons[inputMouseButton] = &((Button*)mem_mouseButtons)[i]; glfwMouseButtons[glfwMouseButton] = &((Button*)mem_mouseButtons)[i];

void Input::setup() {
	uint64_t i;
	keyboardAxisSensitivity = 60.0f;
	mouseHorizontalSensitivity = 0.2f;
	mouseVerticalSensitivity = 0.2f;

	i = 0;
	mem_keys = new Button[InputKeySize];
	setupKey(INPUT_KEY_0, GLFW_KEY_0);
	setupKey(INPUT_KEY_1, GLFW_KEY_1);
	setupKey(INPUT_KEY_2, GLFW_KEY_2);
	setupKey(INPUT_KEY_3, GLFW_KEY_3);
	setupKey(INPUT_KEY_4, GLFW_KEY_4);
	setupKey(INPUT_KEY_5, GLFW_KEY_5);
	setupKey(INPUT_KEY_6, GLFW_KEY_6);
	setupKey(INPUT_KEY_7, GLFW_KEY_7);
	setupKey(INPUT_KEY_8, GLFW_KEY_8);
	setupKey(INPUT_KEY_9, GLFW_KEY_9);
	setupKey(INPUT_KEY_A, GLFW_KEY_A);
	setupKey(INPUT_KEY_B, GLFW_KEY_B);
	setupKey(INPUT_KEY_C, GLFW_KEY_C);
	setupKey(INPUT_KEY_D, GLFW_KEY_D);
	setupKey(INPUT_KEY_E, GLFW_KEY_E);
	setupKey(INPUT_KEY_F, GLFW_KEY_F);
	setupKey(INPUT_KEY_G, GLFW_KEY_G);
	setupKey(INPUT_KEY_H, GLFW_KEY_H);
	setupKey(INPUT_KEY_I, GLFW_KEY_I);
	setupKey(INPUT_KEY_J, GLFW_KEY_J);
	setupKey(INPUT_KEY_K, GLFW_KEY_K);
	setupKey(INPUT_KEY_L, GLFW_KEY_L);
	setupKey(INPUT_KEY_M, GLFW_KEY_M);
	setupKey(INPUT_KEY_N, GLFW_KEY_N);
	setupKey(INPUT_KEY_O, GLFW_KEY_O);
	setupKey(INPUT_KEY_P, GLFW_KEY_P);
	setupKey(INPUT_KEY_Q, GLFW_KEY_Q);
	setupKey(INPUT_KEY_R, GLFW_KEY_R);
	setupKey(INPUT_KEY_S, GLFW_KEY_S);
	setupKey(INPUT_KEY_T, GLFW_KEY_T);
	setupKey(INPUT_KEY_U, GLFW_KEY_U);
	setupKey(INPUT_KEY_V, GLFW_KEY_V);
	setupKey(INPUT_KEY_W, GLFW_KEY_W);
	setupKey(INPUT_KEY_X, GLFW_KEY_X);
	setupKey(INPUT_KEY_Y, GLFW_KEY_Y);
	setupKey(INPUT_KEY_Z, GLFW_KEY_Z);
	setupKey(INPUT_KEY_UP, GLFW_KEY_UP);
	setupKey(INPUT_KEY_DOWN, GLFW_KEY_DOWN);
	setupKey(INPUT_KEY_LEFT, GLFW_KEY_LEFT);
	setupKey(INPUT_KEY_RIGHT, GLFW_KEY_RIGHT);
	setupKey(INPUT_KEY_F1, GLFW_KEY_F1);
	setupKey(INPUT_KEY_F2, GLFW_KEY_F2);
	setupKey(INPUT_KEY_F3, GLFW_KEY_F3);
	setupKey(INPUT_KEY_F4, GLFW_KEY_F4);
	setupKey(INPUT_KEY_F5, GLFW_KEY_F5);
	setupKey(INPUT_KEY_F6, GLFW_KEY_F6);
	setupKey(INPUT_KEY_F7, GLFW_KEY_F7);
	setupKey(INPUT_KEY_F8, GLFW_KEY_F8);
	setupKey(INPUT_KEY_F9, GLFW_KEY_F9);
	setupKey(INPUT_KEY_F10, GLFW_KEY_F10);
	setupKey(INPUT_KEY_F11, GLFW_KEY_F11);
	setupKey(INPUT_KEY_F12, GLFW_KEY_F12);
	setupKey(INPUT_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_SHIFT);
	setupKey(INPUT_KEY_LEFT_CONTROL, GLFW_KEY_LEFT_CONTROL);
	setupKey(INPUT_KEY_LEFT_ALT, GLFW_KEY_LEFT_ALT);
	setupKey(INPUT_KEY_RIGHT_SHIFT, GLFW_KEY_RIGHT_SHIFT);
	setupKey(INPUT_KEY_RIGHT_CONTROL, GLFW_KEY_RIGHT_CONTROL);
	setupKey(INPUT_KEY_RIGHT_ALT, GLFW_KEY_RIGHT_ALT);
	setupKey(INPUT_KEY_ESCAPE, GLFW_KEY_ESCAPE);
	setupKey(INPUT_KEY_GRAVE_ACCENT, GLFW_KEY_GRAVE_ACCENT);
	setupKey(INPUT_KEY_MINUS, GLFW_KEY_MINUS);
	setupKey(INPUT_KEY_EQUAL, GLFW_KEY_EQUAL);
	setupKey(INPUT_KEY_BACKSPACE, GLFW_KEY_BACKSPACE);
	setupKey(INPUT_KEY_TAB, GLFW_KEY_TAB);
	setupKey(INPUT_KEY_ENTER, GLFW_KEY_ENTER);
	setupKey(INPUT_KEY_SPACE, GLFW_KEY_SPACE);
	setupKey(INPUT_KEY_LEFT_BRACKET, GLFW_KEY_LEFT_BRACKET);
	setupKey(INPUT_KEY_RIGHT_BRACKET, GLFW_KEY_RIGHT_BRACKET);
	setupKey(INPUT_KEY_BACKSLASH, GLFW_KEY_BACKSLASH);
	setupKey(INPUT_KEY_SEMICOLON, GLFW_KEY_SEMICOLON);
	setupKey(INPUT_KEY_APOSTROPHE, GLFW_KEY_APOSTROPHE);
	setupKey(INPUT_KEY_COMMA, GLFW_KEY_COMMA);
	setupKey(INPUT_KEY_PERIOD, GLFW_KEY_PERIOD);
	setupKey(INPUT_KEY_SLASH, GLFW_KEY_SLASH);
	setupKey(INPUT_KEY_CAPS_LOCK, GLFW_KEY_CAPS_LOCK);
	setupKey(INPUT_KEY_SCROLL_LOCK, GLFW_KEY_SCROLL_LOCK);
	setupKey(INPUT_KEY_NUM_LOCK, GLFW_KEY_NUM_LOCK);
	setupKey(INPUT_KEY_PRINT_SCREEN, GLFW_KEY_PRINT_SCREEN);
	setupKey(INPUT_KEY_PAUSE, GLFW_KEY_PAUSE);
	setupKey(INPUT_KEY_INSERT, GLFW_KEY_INSERT);
	setupKey(INPUT_KEY_DELETE, GLFW_KEY_DELETE);
	setupKey(INPUT_KEY_HOME, GLFW_KEY_HOME);
	setupKey(INPUT_KEY_END, GLFW_KEY_END);
	setupKey(INPUT_KEY_PAGE_UP, GLFW_KEY_PAGE_UP);
	setupKey(INPUT_KEY_PAGE_DOWN, GLFW_KEY_PAGE_DOWN);
	setupKey(INPUT_KEY_KP_0, GLFW_KEY_KP_0);
	setupKey(INPUT_KEY_KP_1, GLFW_KEY_KP_1);
	setupKey(INPUT_KEY_KP_2, GLFW_KEY_KP_2);
	setupKey(INPUT_KEY_KP_3, GLFW_KEY_KP_3);
	setupKey(INPUT_KEY_KP_4, GLFW_KEY_KP_4);
	setupKey(INPUT_KEY_KP_5, GLFW_KEY_KP_5);
	setupKey(INPUT_KEY_KP_6, GLFW_KEY_KP_6);
	setupKey(INPUT_KEY_KP_7, GLFW_KEY_KP_7);
	setupKey(INPUT_KEY_KP_8, GLFW_KEY_KP_8);
	setupKey(INPUT_KEY_KP_9, GLFW_KEY_KP_9);
	setupKey(INPUT_KEY_KP_PERIOD, GLFW_KEY_KP_DECIMAL);
	setupKey(INPUT_KEY_KP_DIVIDE, GLFW_KEY_KP_DIVIDE);
	setupKey(INPUT_KEY_KP_MULTIPLY, GLFW_KEY_KP_MULTIPLY);
	setupKey(INPUT_KEY_KP_ADD, GLFW_KEY_KP_ADD);
	setupKey(INPUT_KEY_KP_SUBSTRACT, GLFW_KEY_KP_SUBTRACT);
	setupKey(INPUT_KEY_KP_ENTER, GLFW_KEY_KP_ENTER);
	setupKey(INPUT_KEY_KP_EQUAL, GLFW_KEY_KP_EQUAL);

	i = 0;
	mem_mouseButtons = new Button[InputMouseButtonSize];
	setupMouseButton(INPUT_MOUSE_BUTTON_1, GLFW_MOUSE_BUTTON_1);
	setupMouseButton(INPUT_MOUSE_BUTTON_2, GLFW_MOUSE_BUTTON_2);
	setupMouseButton(INPUT_MOUSE_BUTTON_3, GLFW_MOUSE_BUTTON_3);
	setupMouseButton(INPUT_MOUSE_BUTTON_4, GLFW_MOUSE_BUTTON_4);
	setupMouseButton(INPUT_MOUSE_BUTTON_5, GLFW_MOUSE_BUTTON_5);
}

void Input::keyCallBack(GLFWwindow* window, int keycode, int scancode, int action, int mods) {
	Button* pButton = nullptr;
	
	// Look for button pointer using keycode.
	auto itr = glfwKeys.find(keycode);
	if (itr == glfwKeys.end()) {
#if defined(_DEBUG)
		std::cout << "INPUT WARNING: Mouse button '" << keycode << "' not found." << std::endl;
#endif
		return;
	} pButton = itr->second; 

	// Set key's new state.
	if (action == GLFW_PRESS) {
		if (!pButton->current) pButton->pressed = true; 
		pButton->current = true;
	}
	else if (action == GLFW_RELEASE) {
		if (!pButton->current) pButton->released = true; 
		pButton->current = false;
	}
}

void Input::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	Button* pButton = nullptr;

	// Look for button pointer using keycode.
	auto itr = glfwMouseButtons.find(button);
	if (itr == glfwMouseButtons.end()) {
#if defined(_DEBUG)
		std::cout << "INPUT WARNING: Mouse button '" << button << "' not found." << std::endl;
#endif
		return;
	} pButton = itr->second; 

	// Set mouse button's new state.
	if (action == GLFW_PRESS) {
		if (!pButton->current) pButton->pressed = true; 
		pButton->current = true;
		
		// Set mouse down position.
		if (button == GLFW_MOUSE_BUTTON_1) {
			double xpos, ypos; glfwGetCursorPos(window, &xpos, &ypos);
			mouseDownPosition = InputVec2(xpos, ypos);
		}
	}
	else if (action == GLFW_RELEASE) {
		if (!pButton->current) pButton->released = true; 
		pButton->current = false;
	}
}

void Input::mouseMotionCallback(GLFWwindow* window, double xpos, double ypos) {
	InputVec2 lastMousePosition = mousePosition;
	mousePosition = InputVec2(xpos, ypos);
	mousePositionDelta = lastMousePosition - mousePosition;
}

void Input::mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
	mouseScrollDelta = InputVec2(xoffset, yoffset);
}

#endif