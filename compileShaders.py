from subprocess import run
from os import path
from pathlib import Path

# Absolute file path of glslc.exe
COMPILER_PATH = "C:/VulkanSDK/1.3.236.0/Bin/glslc.exe"
# Relative file paths for the GLSL and SPIR-V files.
GLSL_SHADER_PATH  = "/resource/shaders/GLSL/"
SPIRV_SHADER_PATH = "/resource/shaders/SPIR-V/"

# Find all GLSL files.
rootDir = str(Path(__file__).parent.resolve())
filePaths = list(Path(rootDir+GLSL_SHADER_PATH).glob("*.vert")) + list(Path(rootDir+GLSL_SHADER_PATH).glob("*.frag"))

# Compile all GLSL files to SPIR-V files.
for filePath in filePaths:
	fileName = path.basename(filePath)
	run([COMPILER_PATH, rootDir+GLSL_SHADER_PATH+fileName, "-o", rootDir+SPIRV_SHADER_PATH+fileName+".spv"])

