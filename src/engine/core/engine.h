#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP19_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

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

  static constexpr std::array<char const *, 1> deviceExtensions{
      vk::KHRSwapchainExtensionName,
  };

  // Both the instance and the physical device filter are pinned to this: the
  // engine relies on 1.3 core features (dynamic rendering, extended dynamic
  // state), so anything older can't run it.
  static constexpr uint32_t apiVersion = vk::ApiVersion13;

  static constexpr bool enableValidationLayers = debug;

  void run();

private:
  GLFWwindow *window{};

  vk::raii::Context context;
  vk::raii::Instance instance{nullptr};
  vk::raii::DebugUtilsMessengerEXT debugMessenger{nullptr};
  vk::raii::PhysicalDevice physicalDevice{nullptr};
  vk::raii::Device device{nullptr};
  vk::raii::Queue graphicsQueue{nullptr};

  // Properties of the picked GPU, kept around for its limits.
  vk::PhysicalDeviceProperties deviceProperties{};

  // Resolved by pickPhysicalDevice(), which only accepts devices that have one.
  uint32_t graphicsQueueFamilyIndex{};

  std::vector<const char *> getRequiredInstanceExtensions();
  void initWindow();
  void init();
  void loop();
  void cleanup();
  void createInstance();
  void setupDebugMessenger();
  void pickPhysicalDevice();
  void createLogicalDevice();

  // Device suitability helpers, used while ranking candidates.
  static std::optional<uint32_t>
  findGraphicsQueueFamily(vk::raii::PhysicalDevice const &pd);
  static bool supportsRequiredExtensions(vk::raii::PhysicalDevice const &pd);
};

} // namespace longroad