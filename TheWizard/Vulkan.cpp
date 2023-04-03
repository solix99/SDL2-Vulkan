#include "Vulkan.h"


Vulkan::Vulkan(LWindow &window)
{
	WINDOW = &window;

	// Query for required extensions using SDL_Vulkan_GetInstanceExtensions
	uint32_t extensionCount = 0;
	if (!SDL_Vulkan_GetInstanceExtensions(WINDOW->getWindow(), &extensionCount, nullptr))
	{
		// Error handling
	     cout<<endl<<"VK INSTANCE EXTENSION COUNT FAILED";
	}

	std::vector<const char*> extensionNames(extensionCount);
	if (!SDL_Vulkan_GetInstanceExtensions(window.getWindow(), &extensionCount, extensionNames.data()))
	{
		// Error handling
		cout << endl << "VK INSTANCE EXTENSION NAMES FAILED";
	}

	// Create VkInstance
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.enabledExtensionCount = extensionCount;
	createInfo.ppEnabledExtensionNames = extensionNames.data();

	RESULT_VK = vkCreateInstance(&createInfo, nullptr, &INSTANCE_VK);

	if (INSTANCE_VK == VK_NULL_HANDLE)
	{
		cout << endl << "Instance creation failed";
	}

	if (!SDL_Vulkan_CreateSurface(window.getWindow(), INSTANCE_VK, &SURFACE_VK))
	{
		cout << endl << "Surface creation failed";
	}


	initVulkan();

}


Mesh *Vulkan::getMeshByName(string name)
{
	for (Mesh &msh : MESHES)
	{
		if (msh.getName() == name)
		{
			return &msh;
		}
	}
}

Vulkan::~Vulkan()
{
	_mainDeletionQueue.push_function([=]() {
		//other deletions

		vkDestroyPipelineLayout(LOGICAL_DEVICE_VK, meshPipelineLayout, nullptr);
		vkDestroyPipeline(LOGICAL_DEVICE_VK, PIPE.GRAPHICS_PIPELINES_VK[0], nullptr);

		});
}

bool Vulkan::loadMeshes()
{
	MESH_DUMMY.meshInit("assets/cube.obj", PHYSICAL_DEVICE_VK, LOGICAL_DEVICE_VK, INSTANCE_VK, GRAPHICS_QUEUE_VK, ALLOCATOR, ALLOCATOR_INFO);

	//MESHES.emplace_back("assets/bugatti.obj", PHYSICAL_DEVICE_VK, LOGICAL_DEVICE_VK, INSTANCE_VK, GRAPHICS_QUEUE_VK, ALLOCATOR, ALLOCATOR_INFO);
	MESHES.emplace_back("assets/cube.obj", PHYSICAL_DEVICE_VK, LOGICAL_DEVICE_VK, INSTANCE_VK, GRAPHICS_QUEUE_VK, ALLOCATOR, ALLOCATOR_INFO);
	MESHES.emplace_back("assets/Suzanne.obj", PHYSICAL_DEVICE_VK, LOGICAL_DEVICE_VK, INSTANCE_VK, GRAPHICS_QUEUE_VK, ALLOCATOR, ALLOCATOR_INFO);
	
	return true;

}
	
VkPhysicalDevice Vulkan::getPhysicalDevice()
{
	return PHYSICAL_DEVICE_VK;
}

VkCommandPool Vulkan::getCommandPool()
{
	return COMMAND_POOL_VK;
}


VkPipeline Vulkan::switchPipeline()
{
	PIPE.CURRENT++;

	PIPE.CURRENT = (PIPE.CURRENT >= PIPE.NAME.size()) ? 0 : PIPE.CURRENT;

	return PIPE.GRAPHICS_PIPELINES_VK[PIPE.CURRENT];
}

VkPipeline Vulkan::getCurrentPipeline()
{
	return PIPE.GRAPHICS_PIPELINES_VK[PIPE.CURRENT];
}


VkSubmitInfo *Vulkan::getSubmitInfo(){
	return &SUBMIT_INFO_VK;
}

VkCommandBuffer Vulkan::getCommandBuffer() {
	return COMMAND_BUFFER_VK;
}

VkQueue Vulkan::getGraphicsQueue() {
	return GRAPHICS_QUEUE_VK;
}

VkRenderPassBeginInfo* Vulkan::getRenderPassBeginInfo()  {
	return &RENDER_PASS_BEGIN_INFO_VK;
}
VkRenderPassBeginInfo Vulkan::getRenderPassBeginInfoEx() {
	return RENDER_PASS_BEGIN_INFO_VK;
}

VkCommandBufferBeginInfo* Vulkan::getCommandBufferBeginInfo()  {
	return &COMMAND_BUFFER_BEGIN_INFO_VK;
}

VkSemaphore Vulkan::getSemaphoreWait() const {
	return SEMAPHORE_WAIT_VK;
}

VkPresentInfoKHR* Vulkan::getPresentInfo() {
	return &PRESENT_INFO_VK;
}

VkSwapchainKHR Vulkan::getSwapchain() {
	return SWAPCHAIN_VK;
}

VkSemaphore Vulkan::getSemaphoreSignal()  {
	return SEMAPHORE_SIGNAL_VK;
}

uint32_t *Vulkan::getImageIndex() {
	return &IMAGE_INDEX_VK;
}

VkFramebuffer Vulkan::getSwapchainFramebuffer(int i) {
	return SWAPCHAIN_FRAMEBUFFER_VK[i];
}

VkDevice Vulkan::getLogicalDevice()
{
	return LOGICAL_DEVICE_VK;
}

VkFence *Vulkan::getFenceRenderingFinished()
{
	return &FENCE_RENDERING_FINISHED_VK;
}
VkFence Vulkan::getFenceRenderingFinishedEx()
{
	return FENCE_RENDERING_FINISHED_VK;
}
bool Vulkan::cleanup()
{
	return true;
}

bool Vulkan::isDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;
}

VkInstance Vulkan::getInstance()
{
	return INSTANCE_VK;
}

VkPipeline Vulkan::getPipeline(string name)
{
	for (size_t i=0; i < PIPE.NAME.size(); ++i)
	{
		if (PIPE.NAME[i] == name)
		{
			PIPE.CURRENT = i;
			return PIPE.GRAPHICS_PIPELINES_VK[i];
		}
	}

	return VK_NULL_HANDLE;
}

VkImageCreateInfo Vulkan::imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
	VkImageCreateInfo info = { };
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;

	info.imageType = VK_IMAGE_TYPE_2D;

	info.format = format;
	info.extent = extent;

	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = usageFlags;

	return info;
}

VkImageViewCreateInfo Vulkan::imageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
{
	//build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.pNext = nullptr;

	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.image = image;
	info.format = format;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;
	info.subresourceRange.aspectMask = aspectFlags;

	return info;
}

VkPipelineLayout Vulkan::getPipelineLayout(Mesh & MESH)
{
	// Create a descriptor set layout for the uniform values
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding = 1;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutCreateInfo.bindingCount = 1;
	layoutCreateInfo.pBindings = &layoutBinding;

	VkDescriptorSetLayout descriptorSetLayout;
	result = vkCreateDescriptorSetLayout(LOGICAL_DEVICE_VK, &layoutCreateInfo, nullptr, &descriptorSetLayout);
	if (result != VK_SUCCESS)
	{
		// Handle descriptor set layout creation error
		vkDestroySwapchainKHR(LOGICAL_DEVICE_VK, SWAPCHAIN_VK, nullptr);
		cout << endl << "Descriptor set layout creation failed";
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &GLOBAL_SET_LAYOUT;

	VkPushConstantRange pushConstant = {};
	int pushConstantOffset = 0;
	pushConstant.size = sizeof(MESH.pushConstants);
	pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstant;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

	VkPipelineLayout pipelineLayout;

	result = vkCreatePipelineLayout(LOGICAL_DEVICE_VK, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS)
	{
		// Handle pipeline layout creation error
		vkDestroyDescriptorSetLayout(LOGICAL_DEVICE_VK, descriptorSetLayout, nullptr);
		vkDestroySwapchainKHR(LOGICAL_DEVICE_VK, SWAPCHAIN_VK, nullptr);
		cout << endl << "Pipeline layout creation failed";
	}

	return pipelineLayout;
}

Vulkan::AllocatedBuffer Vulkan::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	//allocate vertex buffer
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;

	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memoryUsage;

	AllocatedBuffer newBuffer;

	cout << endl << ALLOCATOR << " " << newBuffer.BUFFER << " " << newBuffer.ALLOCATION << " " << &bufferInfo << " " << &vmaallocInfo;

	vmaCreateBuffer(ALLOCATOR, &bufferInfo, &vmaallocInfo,&newBuffer.BUFFER,&newBuffer.ALLOCATION,nullptr);


	return newBuffer;
}
void Vulkan::initDescriptors()
{

	//information about the binding.
	VkDescriptorSetLayoutBinding camBufferBinding = {};
	camBufferBinding.binding = 0;
	camBufferBinding.descriptorCount = 1;
	// it's a uniform buffer binding
	camBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	// we use it from the vertex shader
	camBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


	VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pNext = nullptr;

	//we are going to have 1 binding
	setinfo.bindingCount = 1;
	//no flags
	setinfo.flags = 0;
	//point to the camera buffer binding
	setinfo.pBindings = &camBufferBinding;

	vkCreateDescriptorSetLayout(LOGICAL_DEVICE_VK, &setinfo, nullptr, &GLOBAL_SET_LAYOUT);

	//create a descriptor pool that will hold 10 uniform buffers
	std::vector<VkDescriptorPoolSize> sizes =
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = 0;
	pool_info.maxSets = 10;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();

	vkCreateDescriptorPool(LOGICAL_DEVICE_VK, &pool_info, nullptr, &DESCRIPTOR_POOL);

	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		_frames[i].cameraBuffer = createBuffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		//allocate one descriptor set for each frame
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		//using the pool we just set
		allocInfo.descriptorPool = DESCRIPTOR_POOL;
		//only 1 descriptor
		allocInfo.descriptorSetCount = 1;
		//using the global data layout
		allocInfo.pSetLayouts = &GLOBAL_SET_LAYOUT;

		vkAllocateDescriptorSets(LOGICAL_DEVICE_VK, &allocInfo, &_frames[i].globalDescriptor);

		VkDescriptorBufferInfo bufferInfo;
		//it will be the camera buffer
		bufferInfo.buffer = _frames[i].cameraBuffer.BUFFER;
		//at 0 offset
		bufferInfo.offset = 0;
		//of the size of a camera data struct
		bufferInfo.range = sizeof(GPUCameraData);

		VkWriteDescriptorSet setWrite = {};
		setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrite.pNext = nullptr;

		//we are going to write into binding number 0
		setWrite.dstBinding = 0;
		//of the global descriptor
		setWrite.dstSet = _frames[i].globalDescriptor;

		setWrite.descriptorCount = 1;
		//and the type is uniform buffer
		setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setWrite.pBufferInfo = &bufferInfo;

		cout << endl << _frames[i].cameraBuffer.BUFFER;

		vkUpdateDescriptorSets(LOGICAL_DEVICE_VK, 1, &setWrite, 0, nullptr);

	}


	// add buffers to deletion queues
	for (int i = 0; i < FRAME_OVERLAP; i++)
	{
		_mainDeletionQueue.push_function([&]() 
			{
				vmaDestroyBuffer(ALLOCATOR, _frames[i].cameraBuffer.BUFFER, _frames[i].cameraBuffer.ALLOCATION);
			});
	}

	_mainDeletionQueue.push_function([&]() {
		vkDestroyDescriptorSetLayout(LOGICAL_DEVICE_VK, GLOBAL_SET_LAYOUT, nullptr);
		vkDestroyDescriptorPool(LOGICAL_DEVICE_VK, DESCRIPTOR_POOL, nullptr);
		});
}


VkPipelineDepthStencilStateCreateInfo Vulkan::depthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp)
{
	VkPipelineDepthStencilStateCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	info.pNext = nullptr;

	info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
	info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
	info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
	info.depthBoundsTestEnable = VK_FALSE;
	info.minDepthBounds = 0.0f; // Optional
	info.maxDepthBounds = 1.0f; // Optional
	info.stencilTestEnable = VK_FALSE;

	return info;
}
size_t Vulkan::getMeshesSize()
{
	return MESHES.size();
}


VkDescriptorSet *Vulkan::getDescriptorSet()
{
	return &depthDescriptorSet;
}
void Vulkan::initPipeline(string name,string sShaderVertex,string sShaderFragment,Mesh & MESH)
{
	PIPE.NAME.push_back(name);

	VkPipelineLayout pipelineLayout = getPipelineLayout(MESH);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = MESH.description.BINDINGS.size();
	vertexInputInfo.pVertexBindingDescriptions = MESH.description.BINDINGS.data();
	vertexInputInfo.vertexAttributeDescriptionCount = MESH.description.ATTRIBUTES.size();
	vertexInputInfo.pVertexAttributeDescriptions = MESH.description.ATTRIBUTES.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)WINDOW->getWidth();
	viewport.height = (float)WINDOW->getHeight();
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = surfaceCapabilities.currentExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// Multisampling configuration
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Depth-stencil configuration
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};

	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.pNext = nullptr;

	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;

	// Color blending configuration
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VERT_SHADER_MODULE = Shader::createShaderModule(LOGICAL_DEVICE_VK, Shader::readFile(sShaderVertex));
	FRAG_SHADER_MODULE = Shader::createShaderModule(LOGICAL_DEVICE_VK, Shader::readFile(sShaderFragment));

	// Define shader stage create info structures
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = VERT_SHADER_MODULE;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = FRAG_SHADER_MODULE;
	fragShaderStageInfo.pName = "main";

	// Create array of shader stage create info structures

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};

	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = &depthStencil;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = RENDER_PASS_VK;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

	//Create pipeline

	VkPipeline PIPELINE_TEMP = VK_NULL_HANDLE;

	RESULT_VK = vkCreateGraphicsPipelines(LOGICAL_DEVICE_VK, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &PIPELINE_TEMP);
	if (RESULT_VK != VK_SUCCESS) { cout << endl << "Failed to create graphics pipeline";}

	PIPE.GRAPHICS_PIPELINES_VK.push_back(PIPELINE_TEMP);
}

Vulkan::FrameData& Vulkan::getCurrentFrame()
{
	return _frames[_frameNumber % FRAME_OVERLAP];
}

void Vulkan::newFrameRendered()
{
	_frameNumber++;
}
bool Vulkan::initVulkan()
{
	uint32_t deviceCount = 0;

	vkEnumeratePhysicalDevices(INSTANCE_VK, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(INSTANCE_VK, &deviceCount, devices.data());

	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			PHYSICAL_DEVICE_VK = device;
			break;
		}
	}

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(PHYSICAL_DEVICE_VK, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(PHYSICAL_DEVICE_VK, &queueFamilyCount, queueFamilies.data());

	// Find a queue family that supports the required capabilities

	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			GRAPHICS_QUEUE_FAMILY_INDEX_VK = i;
			break;
		}
	}

	if (GRAPHICS_QUEUE_FAMILY_INDEX_VK == UINT32_MAX) {
		// Error: no queue family found that supports the required capabilities
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = GRAPHICS_QUEUE_FAMILY_INDEX_VK;
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;


	std::vector<const char*> deviceExtensionNames = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };


	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.fragmentStoresAndAtomics = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = 1;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNames.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;


	vkGetPhysicalDeviceProperties(PHYSICAL_DEVICE_VK, &DEVICE_PROPERTIES_VK);
	vkGetPhysicalDeviceFeatures(PHYSICAL_DEVICE_VK, &DEVICE_FEATURES_VK);


	//create a logical device using parameters : deviceFeatures,PHYSICAL_DEVICE_VK

	RESULT_VK = vkCreateDevice(PHYSICAL_DEVICE_VK, &deviceCreateInfo, nullptr, &LOGICAL_DEVICE_VK);

	if (RESULT_VK != VK_SUCCESS) {
		// Handle error - device creation failed
		switch (RESULT_VK)
		{
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			// Handle out-of-host-memory error
			cout << endl << "VK_ERROR_OUT_OF_HOST_MEMORY";
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			// Handle out-of-device-memory error
			cout << endl << "VK_ERROR_OUT_OF_DEVICE_MEMORY";
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			// Handle initialization failed error
			cout << endl << "VK_ERROR_INITIALIZATION_FAILED";
			break;
		default:
			// Handle unknown error code
			cout << endl << "VK_ERROR_UNKNOWN";
			break;
		}
	}

	ALLOCATOR_INFO.physicalDevice = PHYSICAL_DEVICE_VK;
	ALLOCATOR_INFO.device = LOGICAL_DEVICE_VK;
	ALLOCATOR_INFO.instance = INSTANCE_VK;
	vmaCreateAllocator(&ALLOCATOR_INFO, &ALLOCATOR);

	initDescriptors();

	// Query available surface formats
	uint32_t formatCount;

	vkGetPhysicalDeviceSurfaceFormatsKHR(PHYSICAL_DEVICE_VK, SURFACE_VK, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(PHYSICAL_DEVICE_VK, SURFACE_VK, &formatCount, formats.data());

	// Choose a surface format
	VkSurfaceFormatKHR surfaceFormat;

	surfaceFormat = { VK_FORMAT_D32_SFLOAT };

	// Query available present modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(PHYSICAL_DEVICE_VK, SURFACE_VK, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(PHYSICAL_DEVICE_VK, SURFACE_VK, &presentModeCount, presentModes.data());

	// Choose a present mode
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // Default to FIFO mode
	for (const auto& mode : presentModes)
	{
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentMode = mode;
			break;
		}
		else if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			presentMode = mode;
		}
	}

	// Create a VkSwapchainKHR 
	// Choose the extent of the swapchain images
	surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PHYSICAL_DEVICE_VK, SURFACE_VK, &surfaceCapabilities);

	if (surfaceCapabilities.currentExtent.width == UINT32_MAX)
	{
		// If the surface size is undefined, set the size to the requested width and height
		SWAPCHAIN_EXTENT.width = WINDOW->getWidth();
		SWAPCHAIN_EXTENT.height = WINDOW->getHeight();
	}
	else {
		// Otherwise, use the surface size as the swapchain size
		SWAPCHAIN_EXTENT = surfaceCapabilities.currentExtent;
	}

	// Determine the number of swapchain images
	uint32_t desiredSwapchainImages = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && desiredSwapchainImages > surfaceCapabilities.maxImageCount) {
		desiredSwapchainImages = surfaceCapabilities.maxImageCount;
	}


	uint32_t surfaceFormatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(PHYSICAL_DEVICE_VK, SURFACE_VK, &surfaceFormatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(PHYSICAL_DEVICE_VK, SURFACE_VK, &surfaceFormatCount, surfaceFormats.data());

	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		surfaceFormat.format = VK_FORMAT_D32_SFLOAT;
		surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	}
	else {
		for (const auto& availableFormat : surfaceFormats) {
			if (availableFormat.format == VK_FORMAT_D32_SFLOAT &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				surfaceFormat = availableFormat;
				break;
			}
		}
		// If the desired format is not available, choose the first available format
		if (surfaceFormats.size() > 0) {
			surfaceFormat = surfaceFormats[0];
		}
	}

	// Fill in the swapchain creation info
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = SURFACE_VK;
	swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = SWAPCHAIN_EXTENT;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create the swapchain

	if (LOGICAL_DEVICE_VK == NULL || &swapchainCreateInfo == NULL || &SWAPCHAIN_VK == NULL)
	{
		cout << endl << "FAILED";
	}

	VkResult result = VK_SUCCESS;
	result = vkCreateSwapchainKHR(LOGICAL_DEVICE_VK, &swapchainCreateInfo, nullptr, &SWAPCHAIN_VK);

	if (result != VK_SUCCESS)
	{
		// Error handling
		cout << "SWAP CHAIN FAILED";
		return false;
	}
	// Get the swapchain images

	uint32_t swapchainImageCount;
	result = vkGetSwapchainImagesKHR(LOGICAL_DEVICE_VK, SWAPCHAIN_VK, &swapchainImageCount, nullptr);
	if (result != VK_SUCCESS) {
		// Handle swapchain image retrieval error
		vkDestroySwapchainKHR(LOGICAL_DEVICE_VK, SWAPCHAIN_VK, nullptr);
		return result;
	}
	// Allocate memory for the swapchain images

	std::vector<VkImage> swapchainImages(swapchainImageCount);
	result = vkGetSwapchainImagesKHR(LOGICAL_DEVICE_VK, SWAPCHAIN_VK, &swapchainImageCount, swapchainImages.data());
	if (result != VK_SUCCESS) {
		// Handle swapchain image retrieval error
		vkDestroySwapchainKHR(LOGICAL_DEVICE_VK, SWAPCHAIN_VK, nullptr);
		return result;
	}

	vector<VkImageView> swapchainImageViews(swapchainImageCount);

	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = swapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = surfaceFormat.format;
		imageViewCreateInfo.components =
		{
			VK_COMPONENT_SWIZZLE_IDENTITY, // R
			VK_COMPONENT_SWIZZLE_IDENTITY, // G
			VK_COMPONENT_SWIZZLE_IDENTITY, // B
			VK_COMPONENT_SWIZZLE_IDENTITY  // A
		};
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		VkResult result = vkCreateImageView(LOGICAL_DEVICE_VK, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]);
		if (result != VK_SUCCESS) {
			// Handle image view creation error
			vkDestroySwapchainKHR(LOGICAL_DEVICE_VK, SWAPCHAIN_VK, nullptr);
			return result;
		}
	}

	//depth image size will match the window
	VkExtent3D depthImageExtent = {
		WINDOW->getWidth(),
		WINDOW->getHeight(),
		1
	};

	_depthFormat = VK_FORMAT_D32_SFLOAT;

	VkImageCreateInfo dimg_info = imageCreateInfo(_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	cout << endl << ALLOCATOR << " " << &dimg_info << " " << &dimg_allocinfo << " " << &_depthImage._image << " " << &_depthImage._allocation << " ";

	vmaCreateImage(ALLOCATOR, &dimg_info, &dimg_allocinfo, &_depthImage._image, &_depthImage._allocation, nullptr);

	VkImageViewCreateInfo dview_info = imageViewCreateInfo(_depthFormat, _depthImage._image, VK_IMAGE_ASPECT_DEPTH_BIT);

	vkCreateImageView(LOGICAL_DEVICE_VK, &dview_info, nullptr, &_depthImageView);
	_mainDeletionQueue.push_function([=]() {
		vkDestroyImageView(LOGICAL_DEVICE_VK, _depthImageView, nullptr);
		vmaDestroyImage(ALLOCATOR, _depthImage._image, _depthImage._allocation);
		});


	VkAttachmentDescription depth_attachment = {};
	// Depth attachment
	depth_attachment.flags = 0;
	depth_attachment.format = _depthFormat;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	//hook the depth attachment into the subpass
	subpass.pDepthStencilAttachment = &depth_attachment_ref;


	//array of 2 attachments, one for the color, and other for depth
	VkAttachmentDescription attachments[2] = { colorAttachment,depth_attachment };

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency depth_dependency = {};
	depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depth_dependency.dstSubpass = 0;
	depth_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	depth_dependency.srcAccessMask = 0;
	depth_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	depth_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency dependencies[2] = { dependency, depth_dependency };

	RENDER_PASS_INFO_VK.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	RENDER_PASS_INFO_VK.attachmentCount = 2;
	RENDER_PASS_INFO_VK.pAttachments = &attachments[0];
	RENDER_PASS_INFO_VK.subpassCount = 1;
	RENDER_PASS_INFO_VK.pSubpasses = &subpass;
	RENDER_PASS_INFO_VK.dependencyCount = 2;
	RENDER_PASS_INFO_VK.pDependencies = &dependencies[0];

	result = vkCreateRenderPass(LOGICAL_DEVICE_VK, &RENDER_PASS_INFO_VK, nullptr, &RENDER_PASS_VK);

	if (result != VK_SUCCESS)
	{
		// Handle render pass creation error
		vkDestroySwapchainKHR(LOGICAL_DEVICE_VK, SWAPCHAIN_VK, nullptr);
		cout << "RENDER PASS FAILED";
		return result;
	}
	//cout << endl << swapchainImageCount;
	SWAPCHAIN_FRAMEBUFFER_VK.assign(swapchainImageCount, VK_NULL_HANDLE);

	for (uint32_t i = 0; i < swapchainImageCount; i++)
	{

		VkImageView attachments[2] = { swapchainImageViews[i],_depthImageView };

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = RENDER_PASS_VK;
		framebufferCreateInfo.attachmentCount = 2;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = WINDOW->getWidth();
		framebufferCreateInfo.height = WINDOW->getHeight();
		framebufferCreateInfo.layers = 1;

		result = vkCreateFramebuffer(LOGICAL_DEVICE_VK, &framebufferCreateInfo, nullptr, &SWAPCHAIN_FRAMEBUFFER_VK[i]);
	}

	// Create array of shader stage create info structures

	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCreateInfo.queueFamilyIndex = queueCreateInfo.queueFamilyIndex;

	vkCreateCommandPool(LOGICAL_DEVICE_VK, &poolCreateInfo, nullptr, &COMMAND_POOL_VK);

	for (int i = 0; i < FRAME_OVERLAP; i++) {


		vkCreateCommandPool(LOGICAL_DEVICE_VK, &poolCreateInfo, nullptr, &_frames[i]._commandPool);

		//allocate the default command buffer that we will use for rendering

		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = _frames[i]._commandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		vkAllocateCommandBuffers(LOGICAL_DEVICE_VK, &allocateInfo, &_frames[i]._mainCommandBuffer);

		_mainDeletionQueue.push_function([=]() {
			vkDestroyCommandPool(LOGICAL_DEVICE_VK, _frames[i]._commandPool, nullptr);
			});
	}

	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = COMMAND_POOL_VK;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(LOGICAL_DEVICE_VK, &allocateInfo, &COMMAND_BUFFER_VK) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffer");
	}

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	RESULT_VK = vkCreateSemaphore(LOGICAL_DEVICE_VK, &semaphoreCreateInfo, nullptr, &SEMAPHORE_WAIT_VK);
	if (RESULT_VK != VK_SUCCESS) {
		// Handle semaphore creation error
		cout << "Failed to create semaphore for rendering" << endl;
	}
	RESULT_VK = vkCreateSemaphore(LOGICAL_DEVICE_VK, &semaphoreCreateInfo, nullptr, &SEMAPHORE_SIGNAL_VK);
	if (RESULT_VK != VK_SUCCESS) {
		// Handle semaphore creation error
		cout << "Failed to create semaphore for rendering" << endl;
	}

	vkGetDeviceQueue(LOGICAL_DEVICE_VK, GRAPHICS_QUEUE_FAMILY_INDEX_VK, 0, &GRAPHICS_QUEUE_VK);

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


	RESULT_VK = vkCreateFence(LOGICAL_DEVICE_VK, &fenceCreateInfo, nullptr, &FENCE_RENDERING_FINISHED_VK);
	if (RESULT_VK != VK_SUCCESS) {
		// Handle fence creation error
		cout << "Failed to create fence for rendering" << endl;
	}

	vkResetFences(LOGICAL_DEVICE_VK, 1, &FENCE_RENDERING_FINISHED_VK);


	for (int i = 0; i < FRAME_OVERLAP; i++) 
	{

		vkCreateFence(LOGICAL_DEVICE_VK, &fenceCreateInfo, nullptr, &_frames[i]._renderFence);

		//enqueue the destruction of the fence
		_mainDeletionQueue.push_function([=]() {
			vkDestroyFence(LOGICAL_DEVICE_VK, _frames[i]._renderFence, nullptr);
			});


		vkCreateSemaphore(LOGICAL_DEVICE_VK, &semaphoreCreateInfo, nullptr, &_frames[i]._presentSemaphore);
		vkCreateSemaphore(LOGICAL_DEVICE_VK, &semaphoreCreateInfo, nullptr, &_frames[i]._renderSemaphore);

		//enqueue the destruction of semaphores
		_mainDeletionQueue.push_function([=]() {
			vkDestroySemaphore(LOGICAL_DEVICE_VK, _frames[i]._presentSemaphore, nullptr);
			vkDestroySemaphore(LOGICAL_DEVICE_VK, _frames[i]._renderSemaphore, nullptr);
			});
	}


	PRESENT_INFO_VK.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PRESENT_INFO_VK.waitSemaphoreCount = 1;
	PRESENT_INFO_VK.pWaitSemaphores = &SEMAPHORE_WAIT_VK;
	PRESENT_INFO_VK.swapchainCount = 1;
	PRESENT_INFO_VK.pSwapchains = &SWAPCHAIN_VK;
	PRESENT_INFO_VK.pImageIndices = &IMAGE_INDEX_VK;

	// Record commands to the command buffer

	COMMAND_BUFFER_BEGIN_INFO_VK.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	COMMAND_BUFFER_BEGIN_INFO_VK.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f }; // Clear to black with full alpha

	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.0f;

	clearValues[0] = clearColor;
	clearValues[1] = depthClear;

	RENDER_PASS_BEGIN_INFO_VK.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RENDER_PASS_BEGIN_INFO_VK.renderPass = RENDER_PASS_VK;
	RENDER_PASS_BEGIN_INFO_VK.renderArea.offset = { 0,0 };
	RENDER_PASS_BEGIN_INFO_VK.renderArea.extent = SWAPCHAIN_EXTENT;
	RENDER_PASS_BEGIN_INFO_VK.clearValueCount = 2;
	RENDER_PASS_BEGIN_INFO_VK.pClearValues = &clearValues[0];
	RENDER_PASS_BEGIN_INFO_VK.framebuffer = SWAPCHAIN_FRAMEBUFFER_VK[0];

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	SUBMIT_INFO_VK.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SUBMIT_INFO_VK.commandBufferCount = 1;
	SUBMIT_INFO_VK.pWaitDstStageMask = waitStages;
	SUBMIT_INFO_VK.pCommandBuffers = &COMMAND_BUFFER_VK;
	//SUBMIT_INFO_VK.waitSemaphoreCount = 1;
	//SUBMIT_INFO_VK.pWaitSemaphores = &SEMAPHORE_WAIT_VK;
	//SUBMIT_INFO_VK.signalSemaphoreCount = 1;
	//SUBMIT_INFO_VK.pSignalSemaphores = &SEMAPHORE_SIGNAL_VK;



}

