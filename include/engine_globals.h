#pragma once

#define ENGINE_GRAPHICS_API_VERSION VK_API_VERSION_1_3

#define VK_CHECK_MSG(func, msg) if(func != VK_SUCCESS) { throw std::runtime_error(msg); }
#define VK_CHECK(func) VK_CHECK_MSG(func, "Vulkan error detected at line " + std::to_string(__LINE__) + " of " + __FILE__ + ".");

#if(_DEBUG)
	#include <iostream>
	#define DEBUG_ASSERT_MSG(cond, msg) if(cond == 0) { std::cout << msg << std::endl; std::abort(); }
	#define DEBUG_ASSERT(cond) DEBUG_ASSERT_MSG(cond, "Assert error detected at line " + std::to_string(__LINE__) + " of " + __FILE__ + ".");
#else
	#define DEBUG_ASSERT_MSG(cond, msg) ((void)0)
	#define DEBUG_ASSERT(cond) ((void)0)
#endif

namespace core {
	const int MAX_FRAMES_IN_FLIGHT = 2;
}