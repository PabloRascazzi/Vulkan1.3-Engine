## Vulkan Engine Project
Description: This project is a continuation of my university Honour's Project as a way to continue learning about the Vulkan API.<br>
Author: Pablo Rascazzi

### Instructions
* **Clone/download** repository
* **Build** solution in either *Debug x64* or *Release x64* configuration
* **Move SDL2.dll** file from `.\libary\` directory to the generated `.\x64\Debug\` or `.\x64\Release\` directories
* **Run** python script `compileShaders.py` in the project's root directory
  * Change \``COMPILER_PATH`\` variable from `compileShader.py` if necessary
* **Run** solution

### Important Information
* The project is tested on an **NVIDIA GeForce RTX 2080ti** on driver version **528.49** and **531.79**. 
* The application will crash with driver version **531.18** and **531.29**.

### Input Controls
* Camera Controls:
	* `W` key to move `forward`
	* `A` key to move `left`
	* `S` key to move `backward`
	* `D` key to move `right`
	* `Q` key to move `down`
	* `E` key to move `up`
	* `UP` and `DOWN` arrow keys to control `pitch` axis
	* `Left` and `RIGHT` arrow keys to control `yaw` axis
	* `COMMA` and `PERIOD` keys to control `roll` axis
* Render Controls:
	* `1` key to switch scene rendering to `Ray-Tracing Pipeline`
	* `2` key to switch scene rendering to `Standard Rasterization Pipeline`
