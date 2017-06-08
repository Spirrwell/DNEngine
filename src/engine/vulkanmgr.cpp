#include <cstring>
#include <set>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <fstream>

#include "vulkanmgr.h"
#include "basehelpers.h"
#include "sdlmgr.h"

static std::vector< char > readFile( const std::string &fileName )
{
	std::ifstream file( fileName, std::ios::ate | std::ios::binary );
	
	if ( !file.is_open() )
	{
		std::string err = "Failed to open " + fileName + "!";
		throw std::runtime_error( err );
	}

	size_t fileSize = ( size_t ) file.tellg();
	std::vector< char > buffer( fileSize );
	
	file.seekg( 0 );
	file.read( buffer.data(), fileSize );
	
	file.close();
	
	return buffer;
}

static const std::vector < const char* > g_vVulkanValidationLayers =
{
	"VK_LAYER_LUNARG_standard_validation"
};

static const std::vector < const char* > g_vVulkanDeviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static const std::vector < const char* > g_vVulkanRequiredExtensions
{
	VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#else
#error "Unsupported operating system!"
#endif
};

#ifndef _DEBUG
static const bool g_bEnalbeVulkanValidationLayers = false;
#else
static const bool g_bEnalbeVulkanValidationLayers = true;
#endif

VkResult CreateDebugReportCallbackEXT( VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback )
{
	PFN_vkCreateDebugReportCallbackEXT func = ( PFN_vkCreateDebugReportCallbackEXT ) vkGetInstanceProcAddr( instance, "vkCreateDebugReportCallbackEXT" );
	
	if ( func != nullptr ) 
	{
		return func( instance, pCreateInfo, pAllocator, pCallback );
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugReportCallbackEXT( VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator )
{
	PFN_vkDestroyDebugReportCallbackEXT func = ( PFN_vkDestroyDebugReportCallbackEXT ) vkGetInstanceProcAddr( instance, "vkDestroyDebugReportCallbackEXT" );
	
	if ( func != nullptr )
	{
		func( instance, callback, pAllocator );
	}
}

VulkanManager::VulkanManager()
{

}

VulkanManager::~VulkanManager()
{
	if ( m_vkLogicalDevice != VK_NULL_HANDLE )
	{
		vkDeviceWaitIdle( m_vkLogicalDevice );

		if ( m_vkRenderFinishedSemaphore != VK_NULL_HANDLE )
			vkDestroySemaphore( m_vkLogicalDevice, m_vkRenderFinishedSemaphore, nullptr );

		if ( m_vkImageAvailableSemaphore != VK_NULL_HANDLE )
			vkDestroySemaphore( m_vkLogicalDevice, m_vkImageAvailableSemaphore, nullptr );

		if ( m_vkCommandPool != VK_NULL_HANDLE )
			vkDestroyCommandPool( m_vkLogicalDevice, m_vkCommandPool, nullptr );

		for ( uint32_t i = 0; i < m_vkSwapChainFramebuffers.size(); i++ )
		{
			if ( m_vkSwapChainFramebuffers[i] != VK_NULL_HANDLE )
				vkDestroyFramebuffer( m_vkLogicalDevice, m_vkSwapChainFramebuffers[i], nullptr );
		}

		if ( m_vkGraphicsPipeline != VK_NULL_HANDLE )
			vkDestroyPipeline( m_vkLogicalDevice, m_vkGraphicsPipeline, nullptr );

		if ( m_vkPipelineLayout != VK_NULL_HANDLE )
			vkDestroyPipelineLayout( m_vkLogicalDevice, m_vkPipelineLayout, nullptr );

		if ( m_vkRenderPass != VK_NULL_HANDLE )
			vkDestroyRenderPass( m_vkLogicalDevice, m_vkRenderPass, nullptr );

		for ( uint32_t i = 0; i < m_vkSwapChainImageViews.size(); i++ )
		{
			if ( m_vkSwapChainImageViews[i] != VK_NULL_HANDLE )
				vkDestroyImageView( m_vkLogicalDevice, m_vkSwapChainImageViews[i], nullptr );
		}

		if ( m_vkSwapChain != VK_NULL_HANDLE )
			vkDestroySwapchainKHR( m_vkLogicalDevice, m_vkSwapChain, nullptr );
	}
	
	if ( m_vkSurface != VK_NULL_HANDLE )
		vkDestroySurfaceKHR( m_vkInstance, m_vkSurface, nullptr );

	if ( m_vkDebugCallback != VK_NULL_HANDLE )
		DestroyDebugReportCallbackEXT( m_vkInstance, m_vkDebugCallback, nullptr );

	if ( m_vkLogicalDevice != VK_NULL_HANDLE )
		vkDestroyDevice( m_vkLogicalDevice, nullptr );

	if ( m_vkInstance != VK_NULL_HANDLE )
		vkDestroyInstance( m_vkInstance, nullptr );
}

bool VulkanManager::Init()
{
	// Initialize Vulkan API
	return InitializeVulkan();
}

void VulkanManager::DrawFrame()
{
	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR( m_vkLogicalDevice, m_vkSwapChain, std::numeric_limits< uint64_t >::max(), m_vkImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	
	VkSemaphore waitSemaphores[] = { m_vkImageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_vkCommandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { m_vkRenderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if ( vkQueueSubmit( m_vkGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS )
	{
		// Should probably exit
		Msg( "[Vulkan]Failed to submit draw command buffer!\n" );
		return;
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_vkSwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional?

	vkQueuePresentKHR( m_vkPresentQueue, &presentInfo );

	// Memory leak if we don't do this.
	// Seems to have some thing to do with the way the shaders are abused to create vertices instead of taking in a vertex buffer
	if ( g_bEnalbeVulkanValidationLayers )
		vkQueueWaitIdle( m_vkGraphicsQueue );
}

bool VulkanManager::InitializeVulkan()
{
	if ( !CreateVulkanInstance() )
		return false;

	if ( !SetupVulkanDebugCallback() )
		return false;

	if ( !CreateSurface() )
		return false;

	if ( !PickPhysicalDevice() )
		return false;

	if ( !CreateLogicalDevice() )
		return false;

	if ( !CreateSwapChain() )
		return false;

	if ( !CreateImageViews() )
		return false;

	if ( !CreateRenderPass() )
		return false;

	if ( !CreateGraphicsPipeline() )
		return false;

	if ( !CreateFramebuffers() )
		return false;

	if ( !CreateCommandPool() )
		return false;

	if ( !CreateCommandBuffers() )
		return false;

	if ( !CreateSemaphores() )
		return false;

	return true;
}

bool VulkanManager::CreateVulkanInstance()
{
	m_vkInstance = VK_NULL_HANDLE;
	m_vkLogicalDevice = VK_NULL_HANDLE;
	m_vkDebugCallback = VK_NULL_HANDLE;
	m_vkSurface = VK_NULL_HANDLE;
	m_vkSwapChain = VK_NULL_HANDLE;
	m_vkRenderPass = VK_NULL_HANDLE;
	m_vkPipelineLayout = VK_NULL_HANDLE;
	m_vkGraphicsPipeline = VK_NULL_HANDLE;
	m_vkCommandPool = VK_NULL_HANDLE;
	m_vkImageAvailableSemaphore = VK_NULL_HANDLE;
	m_vkRenderFinishedSemaphore = VK_NULL_HANDLE;
	m_vkPhysicalDevice = VK_NULL_HANDLE;

	if ( g_bEnalbeVulkanValidationLayers && !CheckVulkanValidationLayerSupport() )
	{
		Msg( "[Vulkan]Validation layers requested, but not available!\n" );
		return false;
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Duke Nukem 3D";
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.pEngineName = "DNEngine";
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0 , 0 );
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	std::vector< const char* > extensions = GetRequiredExtensions();

	createInfo.enabledExtensionCount = ( uint32_t )extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	if ( g_bEnalbeVulkanValidationLayers )
	{
		createInfo.enabledLayerCount = ( uint32_t )g_vVulkanValidationLayers.size();
		createInfo.ppEnabledLayerNames = g_vVulkanValidationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	ListVulkanExtensions();

	VkResult result = vkCreateInstance( &createInfo, nullptr, &m_vkInstance );

	if ( result != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to create Vulkan instance!\n" );
		return false;
	}

	return true;
}

bool VulkanManager::SetupVulkanDebugCallback()
{
	if ( !g_bEnalbeVulkanValidationLayers )
		return true;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallback;

	if ( CreateDebugReportCallbackEXT( m_vkInstance, &createInfo, nullptr, &m_vkDebugCallback ) != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to set up debug callback!\n" );
		return false;
	}

	return true;
}

bool VulkanManager::CreateSurface()
{
	SDL_SysWMinfo info;
	SDL_VERSION( &info.version );

	if ( SDL_GetWindowWMInfo( GetSDLManager()->GetWindow(), &info ) != SDL_TRUE )
	{
		Msg( "[SDL]Failed to get window info!\n" );
		return false;
	}
#ifdef _WIN32
	VkWin32SurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = NULL;
	createInfo.hinstance = GetModuleHandle( nullptr );
	createInfo.hwnd = info.info.win.window;

	if ( vkCreateWin32SurfaceKHR( m_vkInstance, &createInfo, nullptr, &m_vkSurface ) != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to create window surface!\n" );
		return false;
	}
#else
#error "Unsupported operating system!"
#endif
	return true;
}

bool VulkanManager::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices( m_vkInstance, &deviceCount, nullptr );

	if ( deviceCount == 0 )
	{
		Msg( "[Vulkan]Failed to find GPUs with Vulkan support!\n" );
		return false;
	}

	std::vector< VkPhysicalDevice > devices( deviceCount );
	vkEnumeratePhysicalDevices( m_vkInstance, &deviceCount, devices.data() );

	for ( const VkPhysicalDevice &device : devices )
	{
		if ( IsPhysicalDeviceSuitable( device ) )
		{
			m_vkPhysicalDevice = device;
			break;
		}
	}

	if ( m_vkPhysicalDevice == VK_NULL_HANDLE )
	{
		Msg( "[Vulkan]Failed to find a suitable GPU!\n" );
		return false;
	}

	return true;
}

bool VulkanManager::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies( m_vkPhysicalDevice );

	std::vector< VkDeviceQueueCreateInfo > queueCreateInfos;
	std::set< int > uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	float queuePriorty = 1.0f;

	for ( int queueFamily : uniqueQueueFamilies )
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriorty;
		queueCreateInfos.push_back( queueCreateInfo );
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = ( uint32_t )queueCreateInfos.size();

	createInfo.pEnabledFeatures = &deviceFeatures;
	
	createInfo.enabledExtensionCount = ( uint32_t )g_vVulkanDeviceExtensions.size();
	createInfo.ppEnabledExtensionNames = g_vVulkanDeviceExtensions.data();

	if ( g_bEnalbeVulkanValidationLayers )
	{
		createInfo.enabledLayerCount = ( uint32_t )g_vVulkanValidationLayers.size();
		createInfo.ppEnabledLayerNames = g_vVulkanValidationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	if ( vkCreateDevice( m_vkPhysicalDevice, &createInfo, nullptr, &m_vkLogicalDevice ) != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to create logical device!\n" );
		return false;
	}

	vkGetDeviceQueue( m_vkLogicalDevice, indices.graphicsFamily, 0, &m_vkGraphicsQueue );
	vkGetDeviceQueue( m_vkLogicalDevice, indices.presentFamily, 0, &m_vkPresentQueue );

	return true;
}

bool VulkanManager::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport( m_vkPhysicalDevice );
	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat( swapChainSupport.formats );
	VkPresentModeKHR presentMode = ChooseSwapPresentMode( swapChainSupport.presentModes );
	VkExtent2D extent = ChooseSwapExtent( swapChainSupport.capabilities );

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if ( swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount )
		imageCount = swapChainSupport.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_vkSurface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies( m_vkPhysicalDevice );
	uint32_t queueFamilyIndices[] = { ( uint32_t )indices.graphicsFamily, ( uint32_t )indices.presentFamily };

	if ( indices.graphicsFamily != indices.presentFamily )
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional?
		createInfo.pQueueFamilyIndices = nullptr; // Optional?
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	// Window blending
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if ( vkCreateSwapchainKHR( m_vkLogicalDevice, &createInfo, nullptr, &m_vkSwapChain ) != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to create swap chain!\n" );
		return false;
	}

	vkGetSwapchainImagesKHR( m_vkLogicalDevice, m_vkSwapChain, &imageCount, nullptr );
	m_vkSwapChainImages.resize( imageCount );
	vkGetSwapchainImagesKHR( m_vkLogicalDevice, m_vkSwapChain, &imageCount, m_vkSwapChainImages.data() );

	m_vkSwapChainImageFormat = surfaceFormat.format;
	m_vkSwapChainExtent = extent;
	return true;
}

bool VulkanManager::CreateImageViews()
{
	m_vkSwapChainImageViews.resize( m_vkSwapChainImages.size() );

	for ( uint32_t i = 0; i < m_vkSwapChainImageViews.size(); i++ )
		m_vkSwapChainImageViews[i] = VK_NULL_HANDLE;

	for ( uint32_t i = 0; i < m_vkSwapChainImages.size(); i++ )
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_vkSwapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_vkSwapChainImageFormat;

		// Map channels
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if ( vkCreateImageView( m_vkLogicalDevice, &createInfo, nullptr, &m_vkSwapChainImageViews[i] ) != VK_SUCCESS )
		{
			Msg( "[Vulkan]Failed to create image views!\n" );
			return false;
		}
	}

	return true;
}

bool VulkanManager::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};

	colorAttachment.format = m_vkSwapChainImageFormat;

	// No multisampling yet, so use 1 sample
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Subpasses and attachment references
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef; // The index of the attachment correlates to the layout location in the fragment shader

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if ( vkCreateRenderPass( m_vkLogicalDevice, &renderPassInfo, nullptr, &m_vkRenderPass ) != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to create render pass!\n" );
		return false;
	}

	return true;
}

bool VulkanManager::CreateGraphicsPipeline()
{
	std::vector< char > vertShaderCode = readFile( "shaders/vulkan/vert.spv" );
	std::vector< char > fragShaderCode = readFile( "shaders/vulkan/frag.spv" );

	std::cout << "Vertex Shader Size: " << vertShaderCode.size() << std::endl;
	std::cout << "Fragment Shader Size: " << fragShaderCode.size() << std::endl;

	VkShaderModule vertShaderModule = VK_NULL_HANDLE;
	VkShaderModule fragShaderModule = VK_NULL_HANDLE;

	if ( !CreateShaderModule( vertShaderCode, vertShaderModule ) )
		return false;

	if ( !CreateShaderModule( fragShaderCode, fragShaderModule ) )
		return false;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] =
	{
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional?
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional?

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = ( float )m_vkSwapChainExtent.width;
	viewport.height = ( float )m_vkSwapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Defines which regions pixels will actually be stored
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = m_vkSwapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; // Discard pixels outside depth
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional?
	rasterizer.depthBiasClamp = 0.0f; // Optional?
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional?

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional?
	multisampling.pSampleMask = nullptr; // Optional?
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional?
	multisampling.alphaToOneEnable = VK_FALSE; // Optional?

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	// Allow alpha blending
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	/*colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional?
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional?
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional?
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional?
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional?
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional?*/

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional?
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional?
	colorBlending.blendConstants[1] = 0.0f; // Optional?
	colorBlending.blendConstants[2] = 0.0f; // Optional?
	colorBlending.blendConstants[3] = 0.0f; // Optional?

	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional?
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional?
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional?
	pipelineLayoutInfo.pPushConstantRanges = 0; // Optional?

	if ( vkCreatePipelineLayout( m_vkLogicalDevice, &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout ) != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to create pipeline layout!\n" );
		return false;
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional?
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional?
	pipelineInfo.layout = m_vkPipelineLayout;
	pipelineInfo.renderPass = m_vkRenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional?
	pipelineInfo.basePipelineIndex = -1; // Optional?

	if ( vkCreateGraphicsPipelines( m_vkLogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkGraphicsPipeline ) != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to create graphics pipeline!\n" );
		return false;
	}

	if ( vertShaderModule != VK_NULL_HANDLE ) // Check not really necessary since we return false if creation fails
		vkDestroyShaderModule( m_vkLogicalDevice, vertShaderModule, nullptr );

	if ( fragShaderModule != VK_NULL_HANDLE ) // Check not really necessary since we return false if creation fails
		vkDestroyShaderModule( m_vkLogicalDevice, fragShaderModule, nullptr );

	return true;
}

bool VulkanManager::CreateFramebuffers()
{
	m_vkSwapChainFramebuffers.resize( m_vkSwapChainImageViews.size() );

	for ( uint32_t i = 0; i < m_vkSwapChainFramebuffers.size(); i++ )
		m_vkSwapChainFramebuffers[i] = VK_NULL_HANDLE;

	for ( size_t i = 0; i < m_vkSwapChainImageViews.size(); i++ )
	{
		VkImageView attachements[] =
		{
			m_vkSwapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_vkRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachements;
		framebufferInfo.width = m_vkSwapChainExtent.width;
		framebufferInfo.height = m_vkSwapChainExtent.height;
		framebufferInfo.layers = 1;

		if  ( vkCreateFramebuffer( m_vkLogicalDevice, &framebufferInfo, nullptr, &m_vkSwapChainFramebuffers[i] ) != VK_SUCCESS )
		{
			Msg( "[Vulkan]Failed to create framebuffer!\n" );
			return false;
		}
	}

	return true;
}

bool VulkanManager::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies( m_vkPhysicalDevice );

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = 0; // Optional?

	if ( vkCreateCommandPool( m_vkLogicalDevice, &poolInfo, nullptr, &m_vkCommandPool ) != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to create command pool!\n" );
		return false;
	}

	return true;
}

bool VulkanManager::CreateCommandBuffers()
{
	m_vkCommandBuffers.resize( m_vkSwapChainFramebuffers.size() );

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_vkCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = ( uint32_t )m_vkCommandBuffers.size();

	if ( vkAllocateCommandBuffers( m_vkLogicalDevice, &allocInfo, m_vkCommandBuffers.data() ) != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to allocate command buffers!\n" );
		return false;
	}

	for ( size_t i = 0; i < m_vkCommandBuffers.size(); i++ )
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr; // Optional?

		vkBeginCommandBuffer( m_vkCommandBuffers[i], &beginInfo );

		// Begin render pass, should this be a separate function?
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_vkRenderPass;
		renderPassInfo.framebuffer = m_vkSwapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_vkSwapChainExtent;

		VkClearValue clearColor = { 0.0f, 0.15f, 0.3f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		
		vkCmdBeginRenderPass( m_vkCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
		vkCmdBindPipeline( m_vkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_vkGraphicsPipeline );
		vkCmdDraw( m_vkCommandBuffers[i], 3, 1, 0, 0 );
		vkCmdEndRenderPass( m_vkCommandBuffers[i] );

		if ( vkEndCommandBuffer( m_vkCommandBuffers[i] ) != VK_SUCCESS )
		{
			Msg( "[Vulkan]Failed to record command buffer!\n" );
			return false;
		}
	}

	return true;
}

bool VulkanManager::CreateSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if ( vkCreateSemaphore( m_vkLogicalDevice, &semaphoreInfo, nullptr, &m_vkImageAvailableSemaphore ) != VK_SUCCESS || vkCreateSemaphore( m_vkLogicalDevice, &semaphoreInfo, nullptr, &m_vkRenderFinishedSemaphore ) != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to create semaphores!\n" );
		return false;
	}

	return true;
}

bool VulkanManager::CreateShaderModule( const std::vector< char > &code, VkShaderModule &shaderModule )
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();

	std::vector< uint32_t > codeAligned( code.size() / sizeof( uint32_t ) + 1 );
	memcpy( codeAligned.data(), code.data(), code.size() );
	createInfo.pCode = codeAligned.data();

	if ( vkCreateShaderModule( m_vkLogicalDevice, &createInfo, nullptr, &shaderModule ) != VK_SUCCESS )
	{
		Msg( "[Vulkan]Failed to create shader module!\n" );
		return false;
	}

	return true;
}

bool VulkanManager::IsPhysicalDeviceSuitable( VkPhysicalDevice device )
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties( device, &deviceProperties );

	//VkPhysicalDeviceFeatures deviceFeatures;
	//vkGetPhysicalDeviceFeatures( device, &deviceFeatures );
	
	QueueFamilyIndices indices = FindQueueFamilies( device );

	bool bExtensionsSupported = CheckVulkanExtensionSupport( device );
	bool bSwapChainAdequate = false;

	if ( bExtensionsSupported )
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport( device );
		bSwapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}


	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && indices.IsComplete() && bExtensionsSupported && bSwapChainAdequate;
}

QueueFamilyIndices VulkanManager::FindQueueFamilies( VkPhysicalDevice device )
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, nullptr );

	if ( queueFamilyCount == 0 )
	{
		Msg( "[Vulkan]Failed to get Queue Family Count for GPU!\n" );
		return indices;
	}

	std::vector< VkQueueFamilyProperties > queueFamilies( queueFamilyCount );
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, queueFamilies.data() );

	int i = 0;
	for ( const VkQueueFamilyProperties &queueFamily : queueFamilies )
	{
		if ( queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT )
			indices.graphicsFamily = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR( device, i, m_vkSurface, &presentSupport );

		if ( queueFamily.queueCount > 0 && presentSupport )
			indices.presentFamily = i;

		if ( indices.IsComplete() )
			break;

		i++;
	}
	
	return indices;
}

SwapChainSupportDetails VulkanManager::QuerySwapChainSupport( VkPhysicalDevice device )
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, m_vkSurface, &details.capabilities );

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR( device, m_vkSurface, &formatCount, nullptr );

	if ( formatCount != 0 )
	{
		details.formats.resize( formatCount );
		vkGetPhysicalDeviceSurfaceFormatsKHR( device, m_vkSurface, &formatCount, details.formats.data() );
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR( device, m_vkSurface, &presentModeCount, nullptr );

	if ( presentModeCount != 0 )
	{
		details.presentModes.resize( presentModeCount );
		vkGetPhysicalDeviceSurfacePresentModesKHR( device, m_vkSurface, &presentModeCount, details.presentModes.data() );
	}

	return details;
}

VkSurfaceFormatKHR VulkanManager::ChooseSwapSurfaceFormat( const std::vector< VkSurfaceFormatKHR > &availableFormats )
{
	if ( availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED )
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	for ( const VkSurfaceFormatKHR &availableFormat : availableFormats )
	{
		if ( availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
			return availableFormat;
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanManager::ChooseSwapPresentMode( const std::vector< VkPresentModeKHR > &availablePresentModes )
{
	// Start off with VSync ( Double Buffer ) since it's guaranteed, but may not be well supported by the graphics driver
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for ( const VkPresentModeKHR &availablePresentMode : availablePresentModes )
	{
		// Select triple buffering if available, less latency than VSync, but no tearing
		if ( availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
			return availablePresentMode;
		// Set best possible mode to render to screen immediately, may result in tearing
		else if ( availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR )
			bestMode = availablePresentMode;
	}

	return bestMode;
}

VkExtent2D VulkanManager::ChooseSwapExtent( const VkSurfaceCapabilitiesKHR &capabilities )
{
	if ( capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() )
		return capabilities.currentExtent;
	else
	{
		VkExtent2D actualExtent = { ( uint32_t )GetSDLManager()->GetWindowWidth(), ( uint32_t )GetSDLManager()->GetWindowHeight() };

		actualExtent.width = std::max( capabilities.minImageExtent.width, std::min( capabilities.maxImageExtent.width, actualExtent.width ) );
		actualExtent.height = std::max( capabilities.minImageExtent.height, std::min( capabilities.maxImageExtent.height, actualExtent.height ) );

		return actualExtent;
	}
}

void VulkanManager::ListVulkanExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

	std::vector< VkExtensionProperties > extensions( extensionCount );

	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions.data() );

	Msg( "[Vulkan]Available extensions:\n\n" );

	for ( const VkExtensionProperties &extension : extensions )
		Msg( "\t" + std::string( extension.extensionName ) + "\n" );
}

bool VulkanManager::CheckVulkanExtensionSupport( VkPhysicalDevice device )
{
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );

	std::vector< VkExtensionProperties > availableExtensions( extensionCount );
	vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data() );

	std::set< std::string > requiredExtensions( g_vVulkanDeviceExtensions.begin(), g_vVulkanDeviceExtensions.end() );

	for ( const VkExtensionProperties &extension : availableExtensions )
		requiredExtensions.erase( extension.extensionName );

	return requiredExtensions.empty();
}

bool VulkanManager::CheckVulkanValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

	std::vector< VkLayerProperties > availableLayers( layerCount );
	vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data() );

	for ( const char *layerName : g_vVulkanValidationLayers )
	{
		bool layerFound = false;
		
		for ( const VkLayerProperties &layerProperties : availableLayers )
		{
			if ( strcmp( layerName, layerProperties.layerName ) == 0 )
			{
				layerFound = true;
				break;
			}
		}

		if ( !layerFound )
			return false;
	}

	return true;
}

std::vector< const char* > VulkanManager::GetRequiredExtensions()
{
	std::vector< const char* > extensions;

	for ( unsigned int i = 0; i < g_vVulkanRequiredExtensions.size(); i++ )
		extensions.push_back( g_vVulkanRequiredExtensions[i] );

	if ( g_bEnalbeVulkanValidationLayers )
		extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );

	return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanManager::debugCallback( VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char *layerPrefix, const char *msg, void *userData )
{
	Msg( "Vulkan Validation Layer: " + std::string( msg ) + "\n" );
	return VK_FALSE;
}