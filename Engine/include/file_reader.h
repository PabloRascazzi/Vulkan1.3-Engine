#pragma once
#include <mesh.h>
#include <texture.h>
#include <string>

#define MESH_FOLDER_PATH ".\\resource\\meshes\\"
#define IMAGE_FOLDER_PATH ".\\resource\\textures\\"

namespace core {

	typedef enum ColorSpace {
		COLOR_SPACE_LINEAR,
		COLOR_SPACE_SRGB
	} ColorSpace;

	class FileReader {
	public:
		static Mesh* readMeshFile(std::string filename);
		static Texture* readImageFile(std::string filename, const ColorSpace& colorSpace = COLOR_SPACE_SRGB);
		static char* readBytes(const std::string& filepath, size_t* size);

	private:

	};
}