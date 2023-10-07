APP_NAME = Mi
BUILD = ./bin

APP_INCLUDES_OSX := -I./vendor -framework Cocoa -framework IOKit -I/Users/dmitriwamback/Documents/vk2/VulkanSDK/macOS/include -I./vendor/glm
APP_LINKERS_OSX  := -L./vendor/GLFW/lib -lglfw3 -L /Users/dmitriwamback/Documents/vk2/VulkanSDK/macOS/lib -lvulkan

# For MacOS, the MoltenVK version must explicitly and strictly be 1.3.216.0 
buildosx:
	g++ --std=c++17 MiVulkan/main.cpp $(APP_INCLUDES_OSX) $(APP_LINKERS_OSX) -o $(BUILD)/$(APP_NAME) && $(BUILD)/$(APP_NAME)

releaseosx:
	g++ --std=c++17 MiVulkan/main.cpp -O -DNDEBUG $(APP_INCLUDES_OSX) $(APP_LINKERS_OSX) -o $(BUILD)/$(APP_NAME) && $(BUILD)/$(APP_NAME)

mtlkit:
	clang++ -lobjc -framework Foundation mtl/main.mm -framework QuartzCore -framework Metal -framework MetalKit -framework Cocoa $(APP_INCLUDES_OSX) $(APP_LINKERS_OSX) -o $(BUILD)/$(APP_NAME) && $(BUILD)/$(APP_NAME)