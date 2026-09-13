#include "engine.h"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>

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
  createLogicalDevice();
}

// Index of the first queue family able to run graphics work, or nullopt when
// the device has none.
std::optional<uint32_t>
Engine::findGraphicsQueueFamily(vk::raii::PhysicalDevice const &pd) {
  auto queueFamilies = pd.getQueueFamilyProperties();
  auto it = std::ranges::find_if(queueFamilies, [](auto const &qfp) {
    return static_cast<bool>(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
  });
  if (it == queueFamilies.end()) {
    return std::nullopt;
  }
  return static_cast<uint32_t>(std::distance(queueFamilies.begin(), it));
}

bool Engine::supportsRequiredExtensions(vk::raii::PhysicalDevice const &pd) {
  auto available = pd.enumerateDeviceExtensionProperties();
  return std::ranges::all_of(
      deviceExtensions, [&available](auto const &required) {
        return std::ranges::any_of(available, [required](auto const &ext) {
          return strcmp(ext.extensionName, required) == 0;
        });
      });
}

void Engine::pickPhysicalDevice() {
  auto physicalDevices = vk::raii::PhysicalDevices(instance);
  if (physicalDevices.empty()) {
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");
  }

  struct Candidate {
    vk::raii::PhysicalDevice physicalDevice;
    uint32_t graphicsFamily;
    uint32_t score;
  };
  std::vector<Candidate> candidates;

  for (const auto &pd : physicalDevices) {
    auto properties = pd.getProperties();
    auto deviceFeatures = pd.getFeatures();

    // Hard requirements: a device missing any of these can't run the engine at
    // all, so it never makes it into the ranking.
    if (pd.getProperties() .apiVersion < apiVersion) {
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

    // Presenting to the window needs the swapchain extension
    if (!supportsRequiredExtensions(pd)) {
      continue;
    }

    // Without a graphics queue there is nothing to submit draw work to
    auto graphicsFamily = findGraphicsQueueFamily(pd);
    if (!graphicsFamily) {
      continue;
    }

    // Preference: everything below only picks the best of the usable GPUs.
    uint32_t score = 0;

    // Discrete GPUs have a significant performance advantage
    if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
      score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += properties.limits.maxImageDimension2D;

    candidates.push_back({pd, *graphicsFamily, score});
  }

  if (candidates.empty()) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  auto const &best =
      *std::ranges::max_element(candidates, {}, &Candidate::score);
  physicalDevice = best.physicalDevice;
  graphicsQueueFamilyIndex = best.graphicsFamily;
  deviceProperties = physicalDevice.getProperties();

  std::cout << "Selected GPU: " << deviceProperties.deviceName.data()
            << " (graphics queue family " << graphicsQueueFamilyIndex << ")"
            << std::endl;
}

void Engine::createLogicalDevice() {
  float queuePriority = 0.5f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
      .queueFamilyIndex = graphicsQueueFamilyIndex,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};

  // Only the features the engine actually relies on. samplerAnisotropy is
  // required in pickPhysicalDevice(), so it has to be enabled here too --
  // filtering on a feature without enabling it would be a no-op.
  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      featureChain = {{.features = {.samplerAnisotropy = true}},
                      {.shaderDrawParameters = true},
                      {.dynamicRendering = true},
                      {.extendedDynamicState = true}};

  vk::DeviceCreateInfo deviceCreateInfo{
      .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &deviceQueueCreateInfo,
      .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
      .ppEnabledExtensionNames = deviceExtensions.data()};

  device = vk::raii::Device(physicalDevice, deviceCreateInfo);
  graphicsQueue = vk::raii::Queue(device, graphicsQueueFamilyIndex, 0);
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