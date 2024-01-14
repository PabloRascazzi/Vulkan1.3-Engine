#pragma once
#include <string>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace core {
	class ScriptEngine {
	public:
		static void setup();
		static void cleanup();

	private: 
		static MonoDomain* RootDomain;
		static MonoDomain* AppDomain;

		static MonoAssembly* CoreAssembly;
		static MonoImage* CoreAssemblyImage;
		static MonoAssembly* GameAssembly;
		static MonoImage* GameAssemblyImage;

		static MonoAssembly* LoadCSharpAssembly(const std::string& path);
		static void PrintAssemblyTypes(MonoAssembly* assembly);

	};
}