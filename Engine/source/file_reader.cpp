#include <file_reader.h>
#include <debugger.h>
#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define RASC_HEADER (('C'<<24)+('S'<<16)+('A'<<8)+'R')

namespace core {

	typedef enum VertexFlagBits {
		VERTEX_FLAG_POSITION_BIT = 0x00000001,
		VERTEX_FLAG_COLOR_BIT = 0x00000002,
		VERTEX_FLAG_UV_BIT = 0x00000004,
		VERTEX_FLAG_NORMAL_BIT = 0x00000008,
		VERTEX_FLAG_TANGENT_BIT = 0x00000010,
		VERTEX_FLAG_BITANGENT_BIT = 0x00000020,
		VERTEX_FLAG_BONE_WEIGHT_BIT = 0x00000040,
		VERTEX_FLAG_BONE_WEIGHT_INDEX_BIT = 0x00000080,
	} VertexFlagBits;
	typedef uint32_t VertexFlag;

	typedef enum VertexBufferFormat { 
		VERTEX_BUFFER_FORMAT_INTERLEAVED = 0, 
		VERTEX_BUFFER_FORMAT_SEPARATED = 1,
	} VertexBufferFormat;

	typedef enum PrimitiveTopology { 
		PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 0, 
		PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 1,
		PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_PRIMITIVE_RESTART = 2,
	} PrimitiveTopology;

	struct FileHeader {
		int32_t identifier; // Used to determine byte order
		int32_t version[2]; // Major and minor version number
	};

	struct MeshHeader {
		VertexFlag vertexFlags;
		VertexBufferFormat vertexBufferFormat;
		PrimitiveTopology primitiveTopology;
		uint32_t vertexStride;
		uint32_t vertexCount;
		uint32_t submeshCount;
	};

	Mesh* FileReader::readMeshFile(std::string filename) {
		const bool logReader = false;

		// Read binary file
		FILE* file;
		std::string fullpathname = (MESH_FOLDER_PATH + filename + ".mesh");
		fopen_s(&file, fullpathname.c_str(), "rb");
		if (file == NULL) {
			std::cerr << "Error: Object " << filename.c_str() << " could not be loaded: \n - File Not Found... \n - Path: " << fullpathname.c_str() << std::endl;
			return nullptr;
		}

		// Read and validate file header
		FileHeader fileHeader;
		fread(&fileHeader, sizeof(FileHeader), 1, file);
		if (logReader) std::cout << "Identifier = " << fileHeader.identifier << ", ExporterVersion = " << fileHeader.version[0] << "." << fileHeader.version[1] << std::endl;
		if (fileHeader.identifier != RASC_HEADER) {
			std::cerr << "Error: Mesh file's byte order does not match system's byte order." << std::endl;
			return nullptr;
		}

		// Read and validate mesh header
		MeshHeader meshHeader;
		fread(&meshHeader, sizeof(MeshHeader), 1, file);
		if (logReader) {
			std::cout << "Vertex Flags: " << meshHeader.vertexFlags << std::endl;
			std::cout << "Vertex Buffer Format: " << meshHeader.vertexBufferFormat << std::endl;
			std::cout << "Primitive Topology: " << meshHeader.primitiveTopology << std::endl;
			std::cout << "Vertex Stride: " << meshHeader.vertexStride << std::endl;
			std::cout << "Vertex Count: " << meshHeader.vertexCount << std::endl;
			std::cout << "Submesh Count: " << meshHeader.submeshCount << std::endl;
		}
		if (meshHeader.vertexFlags != (VERTEX_FLAG_POSITION_BIT | VERTEX_FLAG_COLOR_BIT | VERTEX_FLAG_UV_BIT)) {
			std::cerr << "Error: Mesh file's vertex flag is invalid." << std::endl;
			return nullptr;
		}
		if (meshHeader.vertexBufferFormat != VERTEX_BUFFER_FORMAT_INTERLEAVED) {
			std::cerr << "Error: Mesh file's vertex buffer format is invalid." << std::endl;
			return nullptr;
		}
		if (meshHeader.primitiveTopology != PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {
			std::cerr << "Error: Mesh file's primitive topology is invalid." << std::endl;
			return nullptr;
		}

		// Read vertex data
		Vertex* vertices = new Vertex[meshHeader.vertexCount];
		fread(vertices, sizeof(Vertex), meshHeader.vertexCount, file);
		if (logReader) {
			std::cout << "Vertices = [";
			for (uint32_t k = 0; k < meshHeader.vertexCount; k++) {
				std::cout << (k == 0 ? "[" : "], [") << vertices[k].position.x << ", " << vertices[k].position.y << ", " << vertices[k].position.z;
				std::cout << ", " << vertices[k].color.r << ", " << vertices[k].color.g << ", " << vertices[k].color.b;
			}
			std::cout << "]]" << std::endl;
		}

		uint32_t* indexCountList = new uint32_t[meshHeader.submeshCount];
		uint32_t** indicesList = new uint32_t*[meshHeader.submeshCount];
		for (uint32_t i = 0; i < meshHeader.submeshCount; i++) {
			// Read index count for each submesh
			fread(&indexCountList[i], sizeof(uint32_t), 1, file);
			if (logReader) std::cout << "Index Count = " << indexCountList[i] << std::endl;

			// Read index data for each submesh
			indicesList[i] = new uint32_t[indexCountList[i]];
			fread(indicesList[i], sizeof(uint32_t), indexCountList[i], file);
			if (logReader) {
				std::cout << "Indices = [";
				for (uint32_t k = 1; k < indexCountList[i]; k++) std::cout << (i == 0 ? " " : ", ") << indicesList[i][k];
				std::cout << "]" << std::endl;
			}
		}

		// Create mesh
		Mesh* mesh = new Mesh((float*)vertices, meshHeader.vertexCount, meshHeader.submeshCount, indicesList, indexCountList);
		
		// Cleanup temporary arrays
		delete[] indicesList;
		delete[] indexCountList;

		// Return mesh
		return mesh;
	}

	Texture* FileReader::readImageFile(std::string filename, const ColorSpace& colorSpace) {
		// Read image file
		std::string fullpathname = (IMAGE_FOLDER_PATH + filename);
		int width, height, colorChannels;
		unsigned char* data = stbi_load(fullpathname.c_str(), &width, &height, &colorChannels, 4);

		// Package data into Texture
		VkExtent2D extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
		VkFormat format;
		switch (4) {
			case 1: format = (colorSpace == COLOR_SPACE_LINEAR ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8_SRGB); break;
			case 2: format = (colorSpace == COLOR_SPACE_LINEAR ? VK_FORMAT_R8G8_UNORM : VK_FORMAT_R8G8_SRGB); break;
			case 3: format = (colorSpace == COLOR_SPACE_LINEAR ? VK_FORMAT_R8G8B8_UNORM : VK_FORMAT_R8G8B8_SRGB); break;
			case 4: format = (colorSpace == COLOR_SPACE_LINEAR ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_SRGB); break;
			default: std::cerr << "Error: Image file's color channels count is invalid." << std::endl; return nullptr;
		}
		Texture* texture = new Texture(extent, data, format, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1, VK_TRUE, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		Debugger::setObjectName(texture->getImage().image, "[Image] " + filename);
		Debugger::setObjectName(texture->getImageView(), "[ImageView] " + filename);
		Debugger::setObjectName(texture->getSampler(), "[Sampler] " + filename);

		// Free stbi memory
		stbi_image_free(data);

		return texture;
	}

	char* FileReader::readBytes(const std::string& filepath, size_t* size) {
		// Open file stream.
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
		if (!stream) {
			std::cerr << "Error: File '" << filepath << "' could not be read." << std::endl;
			return nullptr;
		}

		// Fetch file's byte size.
		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		size_t bufferSize = static_cast<size_t>(end - stream.tellg());

		if (bufferSize == 0) {
			std::cerr << "Error: File '" << filepath << "' is empty." << std::endl;
			return nullptr;
		}

		// Read file into buffer, then close stream.
		char* buffer = new char[bufferSize];
		stream.read((char*)buffer, bufferSize);
		stream.close();

		*size = bufferSize;
		return buffer;
	}
}