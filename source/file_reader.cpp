#include <file_reader.h>
#include <iostream>

#define RASC_HEADER (('C'<<24)+('S'<<16)+('A'<<8)+'R')

namespace core {

	struct FileHeader {
		int identifier; // Used to determine byte order
		int version[2]; // Major and minor version number
	};

	Mesh* FileReader::readMeshFile(std::string filename) {
		const bool logReader = true;
		
		// Read binary file
		FILE* file;
		std::string fullpathname = (MESH_FOLDER_PATH + filename + ".mesh");
		fopen_s(&file, fullpathname.c_str(), "rb");
		if (file == NULL) { 
			std::cerr << "Error: Object " << filename.c_str() << " could not be loaded: \n - File Not Found... \n - Path: " << fullpathname.c_str() << std::endl; 
			return nullptr; 
		}

		// Read and validate file header
		FileHeader header;
		fread(&header, sizeof(FileHeader), 1, file);
		if (logReader) std::cout << "Identifier = " << header.identifier << ", ExporterVersion = " << header.version[0] << "." << header.version[1] << std::endl;
		if (header.identifier != RASC_HEADER) { 
			std::cerr << "Error: Mesh file's byte order does not match." << std::endl; 
			return nullptr; 
		}

		// Read vertex data
		uint32_t vertexCount;
		fread(&vertexCount, sizeof(uint32_t), 1, file);
		if (logReader) std::cout << "Vertex Count = " << vertexCount << std::endl;

		Vertex* vertices = new Vertex[vertexCount];
		fread(&vertices, sizeof(Vertex), vertexCount, file);
		if (logReader) {
			std::cout << "Vertices = [";
			for (uint32_t k = 1; k < vertexCount; k++) {
				std::cout << (k == 0 ? "[" : "], [") << vertices[k].position.x << ", " << vertices[k].position.y << ", " << vertices[k].position.z;
				std::cout << ", " << vertices[k].color.r << ", " << vertices[k].color.g << ", " << vertices[k].color.b;
			}
			std::cout << "]" << std::endl;
		}

		// Read submesh data
		uint32_t submeshCount;
		fread(&submeshCount, sizeof(uint32_t), 1, file);
		if (logReader) std::cout << "Submesh Count = " << submeshCount << std::endl;

		uint32_t* indexCountList = new uint32_t[submeshCount];
		uint32_t** indicesList = new uint32_t*[submeshCount];
		for (uint32_t i = 0; i < submeshCount; i++) {
			// Read index data for each submesh
			fread(&indexCountList[i], sizeof(uint32_t), 1, file);
			if (logReader) std::cout << "Index Count = " << indexCountList[i] << std::endl;

			indicesList[i] = new uint32_t[indexCountList[i]];
			fread(&indicesList[i], sizeof(uint32_t), indexCountList[i], file);
			if (logReader) {
				std::cout << "Indices = [";
				for (uint32_t k = 1; k < indexCountList[i]; k++) std::cout << (i == 0 ? " " : ", ") << indicesList[i][k];
				std::cout << "]" << std::endl;
			}
		}

		// Create mesh
		Mesh* mesh = new Mesh((float*)vertices, vertexCount, submeshCount, indicesList, indexCountList);
		
		// Cleanup temporary arrays
		delete[] indicesList;
		delete[] indexCountList;

		// Return mesh
		return mesh;
	}
}