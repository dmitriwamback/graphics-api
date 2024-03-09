APP_NAME = render-tools
BUILD = ./bin

APP_INCLUDES_OSX := -I./vendor -framework Cocoa -framework IOKit -I/Users/dmitriwamback/Documents/vk2/VulkanSDK/macOS/include -I./vendor/glm
APP_LINKERS_OSX  := -L./vendor/GLFW/lib -lglfw3 -L /Users/dmitriwamback/Documents/vk2/VulkanSDK/macOS/lib -lvulkan

buildosx:
	g++ --std=c++17 render-tools/main.cpp $(APP_INCLUDES_OSX) $(APP_LINKERS_OSX) -o $(BUILD)/$(APP_NAME) && $(BUILD)/$(APP_NAME)