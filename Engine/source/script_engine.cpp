#include <script_engine.h>
#include <engine_globals.h>
#include <file_reader.h>

namespace core {

	MonoDomain* ScriptEngine::RootDomain = nullptr;
	MonoDomain* ScriptEngine::AppDomain = nullptr;

	MonoAssembly* ScriptEngine::CoreAssembly = nullptr;
	MonoImage* ScriptEngine::CoreAssemblyImage = nullptr;
	MonoAssembly* ScriptEngine::GameAssembly = nullptr;
	MonoImage* ScriptEngine::GameAssemblyImage = nullptr;

	void ScriptEngine::setup() {
		mono_set_assemblies_path("../mono/lib/4.5");

		MonoDomain* rootDomain = mono_jit_init("JITRuntime");
		DEBUG_ASSERT_MSG(rootDomain != nullptr, "Failed to initialize mono root domain.");
		ScriptEngine::RootDomain = rootDomain;

		// Create app domain.
		ScriptEngine::AppDomain = mono_domain_create_appdomain("ScriptRuntime", nullptr);
		mono_domain_set(AppDomain, true);

		// Load core assembly.
		std::printf("Core Assembly Types:\n");
		ScriptEngine::CoreAssembly = LoadCSharpAssembly("./resource/Engine-ScriptCore.dll");
		ScriptEngine::CoreAssemblyImage = mono_assembly_get_image(CoreAssembly);
		PrintAssemblyTypes(CoreAssembly);

		// Load game assembly.
		std::printf("Game Assembly Types:\n");
		ScriptEngine::GameAssembly = LoadCSharpAssembly("./resource/ScriptTest.dll");
		ScriptEngine::GameAssemblyImage = mono_assembly_get_image(GameAssembly);
		PrintAssemblyTypes(GameAssembly);
	}

	void ScriptEngine::cleanup() {
		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(AppDomain);
		mono_jit_cleanup(RootDomain);
		ScriptEngine::AppDomain = nullptr;
		ScriptEngine::RootDomain = nullptr;
	}

	MonoAssembly* ScriptEngine::LoadCSharpAssembly(const std::string& path) {
		// Read file as bytes.
		uint32_t size = 0;
		char* data = FileReader::readBytes(path, &size);

		// Load mono assembly from file data.
		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(data, size, true, &status, false);
		DEBUG_ASSERT_MSG(status == MONO_IMAGE_OK, "Failed to load C# assembly '" + path + "' with error: " + mono_image_strerror(status) + ".");

		MonoAssembly* assembly = mono_assembly_load_from_full(image, path.c_str(), &status, false);
		mono_image_close(image);

		// Delete file data.
		delete[] data;

		return assembly;
	}

	void ScriptEngine::PrintAssemblyTypes(MonoAssembly* assembly) { 
		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefsTable);

		for (int32_t i = 0; i < numTypes; i++) {
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

			printf("%s.%s\n", nameSpace, name);
		}
	}
}
