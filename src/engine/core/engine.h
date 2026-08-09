#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP19_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef NDEBUG
const bool debug = false;
#else
const bool debug = true;
#endif

namespace longroad {

class Engine {

public:
  static VKAPI_ATTR vk::Bool32 VKAPI_CALL
  debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                vk::DebugUtilsMessageTypeFlagsEXT type,
                const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData);

  static constexpr std::array<char const *, 1> validationLayers{
      "VK_LAYER_KHRONOS_validation",
  };

  static constexpr std:: array<char const *, 1> deviceExtensions{
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  static constexpr bool enableValidationLayers = debug;

  void run();

private:
  GLFWwindow *window{};

  vk::raii::Context context;
  vk::raii::Instance instance{nullptr};
  vk::raii::DebugUtilsMessengerEXT debugMessenger{nullptr};
  vk::raii::PhysicalDevice physicalDevice{nullptr};

  std::vector<const char *> getRequiredInstanceExtensions();
  void initWindow();
  void init();
  void loop();
  void cleanup();
  void createInstance();
  void setupDebugMessenger();
  void pickPhysicalDevice();
};

} // namespace longroad