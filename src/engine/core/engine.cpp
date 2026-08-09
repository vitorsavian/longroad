#include "engine.h"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <iostream>
#include <map>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;

namespace longroad {

void Engine::run() {
  initWindow();
  init();
  loop();
  cleanup();
}

void Engine::initWindow() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WIDTH, HEIGHT, "LongRoad Engine", nullptr, nullptr);
  if (!window) {
    const char *error;
    glfwGetError(&error);
    throw std::runtime_error(error);
  }
}

void Engine::init() {
  createInstance();
  setupDebugMessenger();
  pickPhysicalDevice();
}

bool isDeviceSuitable(vk::raii::PhysicalDevice &device) {
  return true; // TODO: Implement device suitability checks
}

void Engine::pickPhysicalDevice() {
  auto physicalDevices = vk::raii::PhysicalDevices(instance);
  if (physicalDevices.empty()) {
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");
  }
  // Use an ordered map to automatically sort candidates by increasing score
  std::multimap<int, vk::raii::PhysicalDevice> candidates;

  for (const auto &pd : physicalDevices) {
    auto deviceProperties = pd.getProperties();
    auto deviceFeatures = pd.getFeatures();

    auto queueFamilies = pd.getQueueFamilyProperties();
    bool supportGraphics = std::ranges::any_of(queueFamilies, [](auto const &qfp) {
      return bool(qfp.queueFlags & vk::QueueFlagBits::eGraphics); 
    });

    std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

    auto availableDeviceExtensions = pd.enumerateDeviceExtensionProperties();
    bool supportsAllRequiredExtensions =
    std::ranges::all_of( requiredDeviceExtension,
                       [&availableDeviceExtensions]( auto const & requiredDeviceExtension )
                       {
                           return std::ranges::any_of( availableDeviceExtensions,
                                                       [requiredDeviceExtension]( auto const & availableDeviceExtension )
                                                       { return strcmp( availableDeviceExtension.extensionName, requiredDeviceExtension ) == 0; } );
                       } );


    // Hard requirements: a device missing any of these can't run the engine at
    // all, so it never makes it into the ranking.
    if (deviceProperties.apiVersion < vk::ApiVersion13) {
      continue; // The engine is built on Vulkan 1.3 features
    }

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader) {
      continue;
    }

    // Anisotropic filtering is a must-have feature
    if (!deviceFeatures.samplerAnisotropy) {
      continue;
    }

    // Preference: everything below only picks the best of the usable GPUs.
    uint32_t score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
      score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    candidates.insert(std::make_pair(score, pd));
  }

  // Check if the best candidate is suitable at all
  if (!candidates.empty()) {
    physicalDevice = candidates.rbegin()->second;
  } else {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

void Engine::setupDebugMessenger() {
  if (!Engine::enableValidationLayers)
    return;
  vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
  vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
  vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
      .messageSeverity = severityFlags,
      .messageType = messageTypeFlags,
      .pfnUserCallback = &debugCallback};
  debugMessenger =
      instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

void Engine::loop() {
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
}

void Engine::cleanup() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

std::vector<const char *> Engine::getRequiredInstanceExtensions() {
  uint32_t glfwExtensionCount = 0;
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if (Engine::enableValidationLayers) {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
  }

  return extensions;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL Engine::debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  std::cerr << "validation layer: type " << to_string(type)
            << " msg: " << pCallbackData->pMessage << std::endl;

  return vk::False;
}

void Engine::createInstance() {
  constexpr vk::ApplicationInfo engineInfo{
      .pApplicationName = "Vulkan Tutorial TODO: CHANGE LATER",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "LongRoad Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = vk::ApiVersion14};

  std::vector<char const *> requiredLayers;
  if (Engine::enableValidationLayers) {
    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
  }

  auto layerProperties = context.enumerateInstanceLayerProperties();
  auto unsupportedLayerIt = std::ranges::find_if(
      requiredLayers, [&layerProperties](auto const &requiredLayer) {
        return std::ranges::none_of(
            layerProperties, [requiredLayer](auto const &layerProperty) {
              return strcmp(layerProperty.layerName, requiredLayer) == 0;
            });
      });
  if (unsupportedLayerIt != requiredLayers.end()) {
    throw std::runtime_error("Required layer not supported: " +
                             std::string(*unsupportedLayerIt));
  }

  // Get the required extensions.
  auto requiredExtensions = getRequiredInstanceExtensions();

  // Check if the required extensions are supported by the Vulkan
  // implementation.
  auto extensionProperties = context.enumerateInstanceExtensionProperties();
  auto unsupportedPropertyIt = std::ranges::find_if(
      requiredExtensions,
      [&extensionProperties](auto const &requiredExtension) {
        return std::ranges::none_of(
            extensionProperties,
            [requiredExtension](auto const &extensionProperty) {
              return strcmp(extensionProperty.extensionName,
                            requiredExtension) == 0;
            });
      });
  if (unsupportedPropertyIt != requiredExtensions.end()) {
    throw std::runtime_error("Required extension not supported: " +
                             std::string(*unsupportedPropertyIt));
  }

  vk::InstanceCreateInfo createInfo{
      .pApplicationInfo = &engineInfo,
      .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
      .ppEnabledLayerNames = requiredLayers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
      .ppEnabledExtensionNames = requiredExtensions.data()};

  instance = vk::raii::Instance(context, createInfo);
}

} // namespace longroad