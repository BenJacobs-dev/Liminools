C:\VulkanSDK\1.3.290.0\Bin\glslc.exe shader.vert -o vert.spv || (echo "Vertex shader compilation failed. Press any key to exit..." && pause && exit /b)
C:\VulkanSDK\1.3.290.0\Bin\glslc.exe shader.frag -o frag.spv || (echo "Fragment shader compilation failed. Press any key to exit..." && pause && exit /b)
echo "Shaders Compiled"