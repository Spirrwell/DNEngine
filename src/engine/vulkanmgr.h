#ifndef VULKANMGR_H
#define VULKANMGR_H

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#endif

#include <iostream>
#include <functional>
#include <vector>
#include <cassert>
#include <string>

#include "vulkan/vulkan.h"

#include "SDL.h"
#include "SDL_syswm.h"

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector< VkSurfaceFormatKHR > formats;
	std::vector< VkPresentModeKHR > presentModes;
};

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;
	
	bool IsComplete() { return graphicsFamily >= 0 && presentFamily >= 0; }
};

// Vulkan requires a lot of setup, so and SDL has no default way for creating Vulkan surfaces, so let's not clutter the SDL manager
// Implementation is based on this Vulkan Guide: https://vulkan-tutorial.com/Introduction
class VulkanManager
{
public:
	VulkanManager();
	~VulkanManager();

	bool Init();
	void DrawFrame();

private:

	bool InitializeVulkan();
	bool CreateVulkanInstance();
	bool SetupVulkanDebugCallback();
	bool CreateSurface();
	bool PickPhysicalDevice();
	bool CreateLogicalDevice();
	bool CreateSwapChain();
	bool CreateImageViews();
	bool CreateRenderPass();
	bool CreateGraphicsPipeline();
	bool CreateFramebuffers();
	bool CreateCommandPool();
	bool CreateCommandBuffers();
	bool CreateSemaphores();

	bool CreateShaderModule( const std::vector< char > &code, VkShaderModule &shaderModule );

	bool IsPhysicalDeviceSuitable( VkPhysicalDevice device );

	QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice device );
	SwapChainSupportDetails QuerySwapChainSupport( VkPhysicalDevice device );

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat( const std::vector< VkSurfaceFormatKHR > &availableFormats );
	VkPresentModeKHR ChooseSwapPresentMode( const std::vector< VkPresentModeKHR > &availablePresentModes );
	VkExtent2D ChooseSwapExtent( const VkSurfaceCapabilitiesKHR &capabilities );

	void ListVulkanExtensions();

	bool CheckVulkanExtensionSupport( VkPhysicalDevice device );
	bool CheckVulkanValidationLayerSupport();

	std::vector< const char* > GetRequiredExtensions();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char *layerPrefix, const char *msg, void *userData );

	VkInstance m_vkInstance;
	VkDevice m_vkLogicalDevice;
	VkDebugReportCallbackEXT m_vkDebugCallback;
	VkSurfaceKHR m_vkSurface;
	VkSwapchainKHR m_vkSwapChain;
	std::vector< VkImageView > m_vkSwapChainImageViews;
	VkRenderPass m_vkRenderPass;
	VkPipelineLayout m_vkPipelineLayout;
	VkPipeline m_vkGraphicsPipeline;
	std::vector< VkFramebuffer > m_vkSwapChainFramebuffers;
	VkCommandPool m_vkCommandPool;
	VkSemaphore m_vkImageAvailableSemaphore;
	VkSemaphore m_vkRenderFinishedSemaphore;

	VkPhysicalDevice m_vkPhysicalDevice;

	VkQueue m_vkGraphicsQueue;
	VkQueue m_vkPresentQueue;

	std::vector< VkImage > m_vkSwapChainImages;
	VkFormat m_vkSwapChainImageFormat;
	VkExtent2D m_vkSwapChainExtent;
	
	std::vector< VkCommandBuffer > m_vkCommandBuffers;
};

#endif