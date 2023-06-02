#pragma once
#include <mesh.h>
#include <string>

#define MESH_FOLDER_PATH ".\\resource\\meshes\\"

namespace core {

	class FileReader {
	public:
		static Mesh* readMeshFile(std::string filename);

	private:

	};
}