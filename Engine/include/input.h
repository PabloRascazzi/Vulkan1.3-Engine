#pragma once
#include <map>
#include <stdint.h>

/*
* THis library is an Input Manager that abstracts the SDL2 or GLFW libraries.
* You cannot use both SDL2 and GLFW at the same time so you must uncomment the 
* library that you want to use.
*/
// Select Library Here:
#include <SDL2/SDL.h> 
//#include <GLFW/glfw3.h>

/*
* Optionally, if you have the glm library imported, it will allow casting 
* certain data structures into glm data structures.
*/
// Include glm Here:
#include <glm/vec2.hpp>

//******************************************************************************************//
//                                   Library Macro Definitions                              //
//******************************************************************************************//

#if defined SDL_h_
	#define INPUT_USING_SDL
#elif defined _glfw3_h_
	#define INPUT_USING_GLFW
#endif

#if defined GLM_SETUP_INCLUDED
	#define INPUT_USING_GLM
#endif

//******************************************************************************************//
//                                  Input Types Definition                                  //
//******************************************************************************************//

static const uint64_t InputKeySize = 102;  // Does not include repeats or null.
typedef enum {
	INPUT_KEY_NULL = 0x00000000,
	INPUT_KEY_0 = 0x00000001,
	INPUT_KEY_1 = 0x00000002,
	INPUT_KEY_2 = 0x00000003,
	INPUT_KEY_3 = 0x00000004,
	INPUT_KEY_4 = 0x00000005,
	INPUT_KEY_5 = 0x00000006,
	INPUT_KEY_6 = 0x00000007,
	INPUT_KEY_7 = 0x00000008,
	INPUT_KEY_8 = 0x00000009,
	INPUT_KEY_9 = 0x0000000a,
	INPUT_KEY_A = 0x0000000b,
	INPUT_KEY_B = 0x0000000c,
	INPUT_KEY_C = 0x0000000d,
	INPUT_KEY_D = 0x0000000e,
	INPUT_KEY_E = 0x0000000f,
	INPUT_KEY_F = 0x00000010,
	INPUT_KEY_G = 0x00000011,
	INPUT_KEY_H = 0x00000012,
	INPUT_KEY_I = 0x00000013,
	INPUT_KEY_J = 0x00000014,
	INPUT_KEY_K = 0x00000015,
	INPUT_KEY_L = 0x00000016,
	INPUT_KEY_M = 0x00000017,
	INPUT_KEY_N = 0x00000018,
	INPUT_KEY_O = 0x00000019,
	INPUT_KEY_P = 0x0000001a,
	INPUT_KEY_Q = 0x0000001b,
	INPUT_KEY_R = 0x0000001c,
	INPUT_KEY_S = 0x0000001d,
	INPUT_KEY_T = 0x0000001e,
	INPUT_KEY_U = 0x0000001f,
	INPUT_KEY_V = 0x00000020,
	INPUT_KEY_W = 0x00000021,
	INPUT_KEY_X = 0x00000022,
	INPUT_KEY_Y = 0x00000023,
	INPUT_KEY_Z = 0x00000024,
	INPUT_KEY_UP = 0x00000025,
	INPUT_KEY_DOWN = 0x00000026,
	INPUT_KEY_LEFT = 0x00000027,
	INPUT_KEY_RIGHT = 0x00000028,
	INPUT_KEY_F1 = 0x00000029,
	INPUT_KEY_F2 = 0x0000002a,
	INPUT_KEY_F3 = 0x0000002b,
	INPUT_KEY_F4 = 0x0000002c,
	INPUT_KEY_F5 = 0x0000002d,
	INPUT_KEY_F6 = 0x0000002e,
	INPUT_KEY_F7 = 0x0000002f,
	INPUT_KEY_F8 = 0x00000030,
	INPUT_KEY_F9 = 0x00000031,
	INPUT_KEY_F10 = 0x00000032,
	INPUT_KEY_F11 = 0x00000033,
	INPUT_KEY_F12 = 0x00000034,
	INPUT_KEY_LEFT_SHIFT = 0x00000035,
	INPUT_KEY_LEFT_CONTROL = 0x00000036,
	INPUT_KEY_LEFT_ALT = 0x00000037,
	INPUT_KEY_RIGHT_SHIFT = 0x00000038,
	INPUT_KEY_RIGHT_CONTROL = 0x00000039,
	INPUT_KEY_RIGHT_ALT = 0x0000003a,
	INPUT_KEY_ESCAPE = 0x0000003b,
	INPUT_KEY_GRAVE_ACCENT = 0x0000003c,
	INPUT_KEY_MINUS = 0x0000003d,
	INPUT_KEY_EQUAL = 0x0000003e,
	INPUT_KEY_BACKSPACE = 0x0000003f,
	INPUT_KEY_TAB = 0x00000040,
	INPUT_KEY_ENTER = 0x00000041,
	INPUT_KEY_SPACE = 0x00000042,
	INPUT_KEY_LEFT_BRACKET = 0x00000043,
	INPUT_KEY_RIGHT_BRACKET = 0x00000044,
	INPUT_KEY_BACKSLASH = 0x00000045,
	INPUT_KEY_SEMICOLON = 0x00000046,
	INPUT_KEY_APOSTROPHE = 0x00000047,
	INPUT_KEY_COMMA = 0x00000048,
	INPUT_KEY_PERIOD = 0x00000049,
	INPUT_KEY_SLASH = 0x0000004a,
	INPUT_KEY_CAPS_LOCK = 0x0000004b,
	INPUT_KEY_SCROLL_LOCK = 0x0000004c,
	INPUT_KEY_NUM_LOCK = 0x0000004d,
	INPUT_KEY_PRINT_SCREEN = 0x0000004e,
	INPUT_KEY_PAUSE = 0x0000004f,
	INPUT_KEY_INSERT = 0x00000050,
	INPUT_KEY_DELETE = 0x00000051,
	INPUT_KEY_HOME = 0x00000052,
	INPUT_KEY_END = 0x00000053,
	INPUT_KEY_PAGE_UP = 0x00000054,
	INPUT_KEY_PAGE_DOWN = 0x00000055,
	INPUT_KEY_KP_0 = 0x00000056,
	INPUT_KEY_KP_1 = 0x00000057,
	INPUT_KEY_KP_2 = 0x00000058,
	INPUT_KEY_KP_3 = 0x00000059,
	INPUT_KEY_KP_4 = 0x0000005a,
	INPUT_KEY_KP_5 = 0x0000005b,
	INPUT_KEY_KP_6 = 0x0000005c,
	INPUT_KEY_KP_7 = 0x0000005d,
	INPUT_KEY_KP_8 = 0x0000005e,
	INPUT_KEY_KP_9 = 0x0000005f,
	INPUT_KEY_KP_PERIOD = 0x00000060,
	INPUT_KEY_KP_DIVIDE = 0x00000061,
	INPUT_KEY_KP_MULTIPLY = 0x00000062,
	INPUT_KEY_KP_ADD = 0x00000063,
	INPUT_KEY_KP_SUBSTRACT = 0x00000064,
	INPUT_KEY_KP_ENTER = 0x00000065,
	INPUT_KEY_KP_EQUAL = 0x00000066,
} InputKey;

static const uint64_t InputMouseButtonSize = 5; // Does not include repeats or null.
typedef enum {
	INPUT_MOUSE_BUTTON_NULL = 0x00100000,
	INPUT_MOUSE_BUTTON_1 = 0x00100001, // Left
	INPUT_MOUSE_BUTTON_2 = 0x00100002, // Right
	INPUT_MOUSE_BUTTON_3 = 0x00100003, // Middle
	INPUT_MOUSE_BUTTON_4 = 0x00100004,
	INPUT_MOUSE_BUTTON_5 = 0x00100005,
	INPUT_MOUSE_BUTTON_LEFT = INPUT_MOUSE_BUTTON_1,
	INPUT_MOUSE_BUTTON_RIGHT = INPUT_MOUSE_BUTTON_2,
	INPUT_MOUSE_BUTTON_MIDDLE = INPUT_MOUSE_BUTTON_3,
} InputMouseButton;

static const uint64_t InputAxisSize = 2;  // Does not include repeats or null.
typedef enum {
	INPUT_AXIS_NULL = 0x00200000,
	INPUT_AXIS_HORIZONTAL = 0x00200001,
	INPUT_AXIS_VERTICAL = 0x00200002,
} InputAxis;

//******************************************************************************************//
//                                Input Vector 2D Definition                                //
//******************************************************************************************//

struct InputVec2 {
public:
	double x;
	double y;

	InputVec2() { 
		this->x = 0; 
		this->y = 0; 
	}

	InputVec2(double x, double y) {
		this->x = x;
		this->y = y;
	}

	inline InputVec2 operator- (InputVec2 other) const {
		return InputVec2(this->x - other.x, this->y - other.y);
	}

	inline InputVec2 operator+ (InputVec2 other) const {
		return InputVec2(this->x + other.x, this->y + other.y);
	}

#ifdef INPUT_USING_GLM
	inline operator glm::vec2() const { 
		return glm::vec2(x, y); 
	}
#endif // INPUT_USING_GLM
};

//******************************************************************************************//
//                                  Input Class Definition                                  //
//******************************************************************************************//

class Input {
public:
	static void setup();
	static void reset(); // Resets perFrameKeys and perFrameMouseButtons, should be called before polling window events.
	static void cleanup();

#ifdef INPUT_USING_SDL
	static void keyCallback(SDL_Event e);
	static void mouseButtonCallback(SDL_Event e);
	static void mouseMotionCallback(SDL_Event e);
	static void mouseScrollCallback(SDL_Event event);
#elif defined INPUT_USING_GLFW
	static void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void mouseMotionCallback(GLFWwindow* window, double xpos, double ypos);
	static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
#endif

	static bool getKey(InputKey); // Returns true if key is pressed and not released
	static bool getKeyDown(InputKey); // Returns true if key is pressed, resets every frames
	static bool getKeyUp(InputKey); // Returns true if key is released, resets every frames

	static bool getMouseButton(InputMouseButton); // Returns true if mouse button is pressed and not released
	static bool getMouseButtonDown(InputMouseButton); // Returns true if mouse button is pressed, resets every frames
	static bool getMouseButtonUp(InputMouseButton); // Returns true if mouse button is released, resets every frames

	static InputVec2 getMousePosition(); // Mouse position relative to window
	static InputVec2 getMouseDownPosition(); // Mouse position relative to window when the last left button down event occured
	static InputVec2 getMouseScroll(); // Mouse scroll delta

	static float getAxis(InputAxis axis); // Horizontal or Vertical
	static float getAxisMouse(InputAxis axis, InputMouseButton buttonName = INPUT_MOUSE_BUTTON_NULL); // Horizontal or Vertical

private:
	struct Button { bool current = false, pressed = false, released = false; };

	// Two parrallel hash maps with button pointers to the same memory location.
	// Depending on if the library used is SDL or GLFW, the appropriate hash map will be created.
	static std::map<InputKey, Button*> actualKeys;
#ifdef INPUT_USING_SDL
	static std::map<SDL_Keycode, Button*> sdlKeys;
#elif defined INPUT_USING_GLFW
	static std::map<int, Button*> glfwKeys;
#endif

	// Same as the key maps, but for the mouse buttons.
	static std::map<InputMouseButton, Button*> actualMouseButtons;
#ifdef INPUT_USING_SDL
	static std::map<SDL_Keycode, Button*> sdlMouseButtons;
#elif defined INPUT_USING_GLFW
	static std::map<int, Button*> glfwMouseButtons;
#endif

	static InputVec2 mousePosition;
	static InputVec2 mouseDownPosition;
	static InputVec2 mousePositionDelta;
	static InputVec2 mouseScrollDelta;

	static float keyboardAxisSensitivity;
	static float mouseVerticalSensitivity;
	static float mouseHorizontalSensitivity;
};