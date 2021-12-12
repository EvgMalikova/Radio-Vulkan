#include "pipeline.hpp"
#include <nvvk/error_vk.hpp>
#include <nvvk/images_vk.hpp>
#include <nvvk/resourceallocator_vk.hpp> // For NVVK memory allocators
#include <nvvk/shaders_vk.hpp>
#include <nvvk/structs_vk.hpp>

#include "VulkanBuffer.h"
#include "VulkanTools.h"
#include "helpers.hpp"

#include "loaders.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

void PipelineRasterize::CreateDescriptorSets(pv2::Context context, int size, SimpCamera cam)
{
    std::vector<VkDescriptorSetLayout> layouts(size, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(size);
    if (vkAllocateDescriptorSets(context.m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < size; i++) {
        VkDescriptorBufferInfo bufferInfo {};
        bufferInfo.buffer = cam.m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(SimpCamera::UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(context.m_device, 1, &descriptorWrite, 0, nullptr);
    }
}

void PipelineRasterize::CreateDescriptorPool(pv2::Context context, int size)
{
    VkDescriptorPoolSize poolSize {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(size);

    VkDescriptorPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(size);

    if (vkCreateDescriptorPool(context.m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void PipelineRasterize::CreateDescriptorSetLayout(pv2::Context context)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(context.m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void PipelineRasterize::CreateGraphicsPipeline(pv2::Context context, pv2::RenderBase ren)
{
    auto vertShaderCode = pv::readFile(vertShaderName);
    auto fragShaderCode = pv::readFile(fragShaderName);

    VkShaderModule vertShaderModule = pv::createShaderModule(vertShaderCode, context.m_device);
    VkShaderModule fragShaderModule = pv::createShaderModule(fragShaderCode, context.m_device);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkViewport viewport {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)ren.m_Extent.width;
    viewport.height = (float)ren.m_Extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.offset = { 0, 0 };
    scissor.extent = ren.m_Extent;

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Create pipeline
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    /* VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
             vks::initializers::pipelineInputAssemblyStateCreateInfo(
                 VK_PRIMITIVE_TOPOLOGY_POINT_LIST, 0, VK_FALSE);*/

    VkPipelineRasterizationStateCreateInfo rasterizationState {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;

    /*VkPipelineRasterizationStateCreateInfo rasterizationState =
             vks::initializers::pipelineRasterizationStateCreateInfo(
                 VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT,
                 VK_FRONT_FACE_CLOCKWISE);*/
    VkPipelineColorBlendAttachmentState blendAttachmentState {};
    blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachmentState.blendEnable = VK_FALSE;

    /*VkPipelineColorBlendAttachmentState blendAttachmentState =
             vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);*/

    VkPipelineColorBlendStateCreateInfo colorBlendState {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &blendAttachmentState;
    colorBlendState.blendConstants[0] = 0.0f;
    colorBlendState.blendConstants[1] = 0.0f;
    colorBlendState.blendConstants[2] = 0.0f;
    colorBlendState.blendConstants[3] = 0.0f;

    /*
         VkPipelineColorBlendStateCreateInfo colorBlendState =
             vks::initializers::pipelineColorBlendStateCreateInfo(
                 1, &blendAttachmentState);*/
    VkPipelineDepthStencilStateCreateInfo depthStencilState {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_FALSE;
    depthStencilState.depthWriteEnable = VK_FALSE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;

    //VkPipelineViewportStateCreateInfo viewportState =
    //     vks::initializers::pipelineViewportStateCreateInfo(1, 1);
    VkPipelineMultisampleStateCreateInfo multisampleState {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkDynamicState> dynamicStateEnables = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    dynamicState.flags = 0; //flags
    /* VkPipelineDynamicStateCreateInfo dynamicState =
             vks::initializers::pipelineDynamicStateCreateInfo(
                 dynamicStateEnables);*/

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

    if (vkCreatePipelineLayout(context.m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizationState;
    pipelineInfo.pMultisampleState = &multisampleState;
    pipelineInfo.pColorBlendState = &colorBlendState;
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = ren.m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    // Additive blending
    blendAttachmentState.colorWriteMask = 0xF;
    blendAttachmentState.blendEnable = VK_TRUE;
    blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
    blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;

    if (vkCreateGraphicsPipelines(context.m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(context.m_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(context.m_device, vertShaderModule, nullptr);
}

void PipelineBase::CreateCommandPool(pv2::Context context, pv2::RenderBase ren)
{

    pv::QueueFamilyIndices queueFamilyIndices;
    pv::QueueGraphicFamilyIndices queueGrFamilyIndices;

    if (context.GetInteractive())
        queueFamilyIndices = pv::findQueueFamilies(context.m_physicalDevice, ren.m_surface);
    else
        queueGrFamilyIndices = pv::findGraphicsQueueFamilies(context.m_physicalDevice);

    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    if (context.GetInteractive())
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    else
        poolInfo.queueFamilyIndex = queueGrFamilyIndices.graphicsFamily.value();
    if (vkCreateCommandPool(context.m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void PipelineRasterize::CreateCommandBuffers(pv2::Context context, pv2::RenderBase ren, VkBuffer vertexBuffer, int m_vsize)
{
    m_commandBuffers.resize(ren.m_swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

    if (vkAllocateCommandBuffers(context.m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < m_commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = ren.m_renderPass;
        renderPassInfo.framebuffer = ren.m_swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = ren.m_Extent;

        VkClearValue clearColor = { { { 0.0f, 0.0f, 0.0f, 1.0f } } };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);

        //vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

        vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);

        //vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        vkCmdDraw(m_commandBuffers[i], static_cast<uint32_t>(m_vsize), 1, 0, 0);
        vkCmdEndRenderPass(m_commandBuffers[i]);

        if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

struct PushConstants {
    uint sample_batch;
};

PipelineSetUp::PipelineSetUp()
{
    m_interactive = false;
    render_width = 800;
    render_height = 600;
    world_size = 1;
    world_rank = 0;
#ifdef USE_MPI
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    std::cout << " processor " << processor_name << " rank " << world_rank
              << "out of " << world_size << std::endl;

#endif

    /*
     camera.type = Camera::CameraType::lookat;
     camera.setPerspective(60.0f, (float)render_width / (float)render_height, 0.1f, 512.0f);
     camera.setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
     camera.setTranslation(glm::vec3(0.0f, 0.0f, -4.0f));
     
     camera.movementSpeed = 2.5f;
     camera.updated = true;
     */
};

void PipelineSetUp::CreatePool()
{ // Create the command pool
    VkCommandPoolCreateInfo cmdPoolInfo;
#ifdef USE_NVVK
    cmdPoolInfo = nvvk::make<VkCommandPoolCreateInfo>();
    cmdPoolInfo.queueFamilyIndex = context.m_queueGCT;
    NVVK_CHECK(vkCreateCommandPool(context, &cmdPoolInfo, nullptr, &cmdPool));
#else

    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = m_context.queueFamilyIndex;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK_RESULT(
        vkCreateCommandPool(context.m_device, &cmdPoolInfo, nullptr, &commandPool));

#endif
}

void PipelineSetUp::CopyImage(VkCommandBuffer cmdBuffer)
{
    // Transition `image` from GENERAL to TRANSFER_SRC_OPTIMAL layout. See the
    // code for uploadCmdBuffer above to see a description of what this does:
    const VkAccessFlags srcAccesses = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    const VkAccessFlags dstAccesses = VK_ACCESS_TRANSFER_READ_BIT;
    const VkPipelineStageFlags srcStages = nvvk::makeAccessMaskPipelineStageFlags(srcAccesses);
    const VkPipelineStageFlags dstStages = nvvk::makeAccessMaskPipelineStageFlags(dstAccesses);
    const VkImageMemoryBarrier barrier = nvvk::makeImageMemoryBarrier(image.image, // The VkImage
        srcAccesses, dstAccesses, // Src and dst access masks
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, // Src and dst layouts
        VK_IMAGE_ASPECT_COLOR_BIT);
    vkCmdPipelineBarrier(cmdBuffer, // Command buffer
        srcStages, dstStages, // Src and dst pipeline stages
        0, // Dependency flags
        0, nullptr, // Global memory barriers
        0, nullptr, // Buffer memory barriers
        1, &barrier); // Image memory barriers

    // Now, copy the image (which has layout TRANSFER_SRC_OPTIMAL) to imageLinear
    // (which has layout TRANSFER_DST_OPTIMAL).
    {
        VkImageCopy region;
        // We copy the image aspect, layer 0, mip 0:
        region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.srcSubresource.baseArrayLayer = 0;
        region.srcSubresource.layerCount = 1;
        region.srcSubresource.mipLevel = 0;
        // (0, 0, 0) in the first image corresponds to (0, 0, 0) in the second image:
        region.srcOffset = { 0, 0, 0 };
        region.dstSubresource = region.srcSubresource;
        region.dstOffset = { 0, 0, 0 };
        // Copy the entire image:
        region.extent = { render_width, render_height, 1 };
        vkCmdCopyImage(cmdBuffer, // Command buffer
            image.image, // Source image
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, // Source image layout
            imageLinear.image, // Destination image
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // Destination image layout
            1, &region); // Regions
    }

    // Add a command that says "Make it so that memory writes by transfers
    // are available to read from the CPU." (In other words, "Flush the GPU caches
    // so the CPU can read the data.") To do this, we use a memory barrier.
    VkMemoryBarrier memoryBarrier = nvvk::make<VkMemoryBarrier>();
    memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // Make transfer writes
    memoryBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT; // Readable by the CPU
    vkCmdPipelineBarrier(cmdBuffer, // The command buffer
        VK_PIPELINE_STAGE_TRANSFER_BIT, // From transfers
        VK_PIPELINE_STAGE_HOST_BIT, // To the CPU
        0, // No special flags
        1, &memoryBarrier, // An array of memory barriers
        0, nullptr, 0, nullptr); // No other barriers

    std::cout << "Rendered sample " << std::endl;
}

void PipelineSetUp::CreateImageBarriers(VkCommandBuffer uploadCmdBuffer)
{

    const VkAccessFlags srcAccesses = 0; // (since image and imageLinear aren't initially accessible)
    // finish and can be read correctly by
    const VkAccessFlags dstImageAccesses = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT; // for image
    const VkAccessFlags dstImageLinearAccesses = VK_ACCESS_TRANSFER_WRITE_BIT; // for imageLinear
    // "

    // Here's how to do that:
    const VkPipelineStageFlags srcStages = nvvk::makeAccessMaskPipelineStageFlags(srcAccesses);
    const VkPipelineStageFlags dstStages = nvvk::makeAccessMaskPipelineStageFlags(dstImageAccesses | dstImageLinearAccesses);
    VkImageMemoryBarrier imageBarriers[2];
    // Image memory barrier for `image` from UNDEFINED to GENERAL layout:
    imageBarriers[0] = nvvk::makeImageMemoryBarrier(image.image, // The VkImage
        srcAccesses, dstImageAccesses, // Source and destination access masks
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, // Source and destination layouts
        VK_IMAGE_ASPECT_COLOR_BIT); // Aspects of an image (color, depth, etc.)
    // Image memory barrier for `imageLinear` from UNDEFINED to TRANSFER_DST_OPTIMAL layout:
    imageBarriers[1] = nvvk::makeImageMemoryBarrier(imageLinear.image, // The VkImage
        srcAccesses, dstImageLinearAccesses, // Source and destination access masks
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // Source and dst layouts
        VK_IMAGE_ASPECT_COLOR_BIT); // Aspects of an image (color, depth, etc.)
    // Include the two image barriers in the pipeline barrier:
    vkCmdPipelineBarrier(uploadCmdBuffer, // The command buffer
        srcStages, dstStages, // Src and dst pipeline stages
        0, // Flags for memory dependencies
        0, nullptr, // Global memory barrier objects
        0, nullptr, // Buffer memory barrier objects
        2, imageBarriers); // Image barrier objects
}

void PipelineSetUp::SaveImage()
{
    void* data = allocator.map(imageLinear);
    stbi_write_hdr("out.hdr", render_width, render_height, 4, reinterpret_cast<float*>(data));
}

void PipelineSetUp::Initialize()
{
#ifdef USE_NVVK
    nvvk::ContextCreateInfo deviceInfo; // One can modify this to load different extensions or pick the Vulkan core version
    deviceInfo.apiMajor = 1; // Specify the version of Vulkan we'll use
    deviceInfo.apiMinor = 2;
    // Required by KHR_acceleration_structure; allows work to be offloaded onto background threads and parallelized
    deviceInfo.addDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    VkPhysicalDeviceAccelerationStructureFeaturesKHR asFeatures = nvvk::make<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    deviceInfo.addDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, false, &asFeatures);
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures = nvvk::make<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    deviceInfo.addDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, false, &rtPipelineFeatures);
    deviceInfo.addDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    VkValidationFeaturesEXT validationInfo = nvvk::make<VkValidationFeaturesEXT>();
    VkValidationFeatureEnableEXT validationFeatureToEnable = VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT;
    validationInfo.enabledValidationFeatureCount = 1;
    validationInfo.pEnabledValidationFeatures = &validationFeatureToEnable;
    deviceInfo.instanceCreateInfoExt = &validationInfo;

    putenv("DEBUG_PRINTF_TO_STDOUT=1");

    //VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT
    // Encapsulates device state in a single object
    context.init(deviceInfo); // Initialize the context
    // Device must support acceleration structures and ray tracing pipelines:
    assert(asFeatures.accelerationStructure == VK_TRUE && rtPipelineFeatures.rayTracingPipeline == VK_TRUE);

    // Create the allocator

    allocator.init(context, context.m_physicalDevice);

#else

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan headless example";
    appInfo.pEngineName = "VulkanExample";
    appInfo.apiVersion = VK_API_VERSION_1_0;

    /*
            Vulkan instance creation (without surface extensions)
    */
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    uint32_t layerCount = 0;

    const char* validationLayers[] = { "VK_LAYER_LUNARG_standard_validation" };
    layerCount = 1;

    // Check if layers are available
    uint32_t instanceLayerCount;
    vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
    std::vector<VkLayerProperties> instanceLayers(instanceLayerCount);
    vkEnumerateInstanceLayerProperties(&instanceLayerCount,
        instanceLayers.data());

    bool layersAvailable = true;
    for (auto layerName : validationLayers) {
        bool layerAvailable = false;
        for (auto instanceLayer : instanceLayers) {
            if (strcmp(instanceLayer.layerName, layerName) == 0) {
                layerAvailable = true;
                break;
            }
        }
        if (!layerAvailable) {
            layersAvailable = false;
            break;
        }
    }

    if (layersAvailable) {
        instanceCreateInfo.ppEnabledLayerNames = validationLayers;
        const char* validationExt = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
        instanceCreateInfo.enabledLayerCount = layerCount;
        instanceCreateInfo.enabledExtensionCount = 1;
        instanceCreateInfo.ppEnabledExtensionNames = &validationExt;
    }

    //  VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &pvcontext.m_instance));
    VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &context.m_instance));
    //if(res!=VK_SUCCESS)
    std::cout << "Pass" << std::endl;
    /*
            Vulkan device creation
    */
    uint32_t deviceCount = 0;
    //VkInstance         m_instance{VK_NULL_HANDLE}
    // VK_CHECK_RESULT(vkEnumeratePhysicalDevices(pvcontext.m_instance, &deviceCount, nullptr));
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(context.m_instance, &deviceCount, nullptr));
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);

    //VK_CHECK_RESULT(vkEnumeratePhysicalDevices(pvcontext.m_instance, &deviceCount, physicalDevices.data()));
    VK_CHECK_RESULT(vkEnumeratePhysicalDevices(context.m_instance, &deviceCount, physicalDevices.data()));
    // physicalDevice = physicalDevices[0];

    std::cout << "There are " << deviceCount << " devices found" << std::endl;

    for (int i = 0; i < deviceCount; i++) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

        /*
       * Provided by VK_VERSION_1_0
       typedef enum VkPhysicalDeviceType {
           VK_PHYSICAL_DEVICE_TYPE_OTHER = 0,
           VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU = 1,
           VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU = 2,
           VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU = 3,
           VK_PHYSICAL_DEVICE_TYPE_CPU = 4,
       } VkPhysicalDeviceType;
       * */
        if ((deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) || (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) || (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)) {
            //gpuPhysicalDevices.push_back(physicalDevices[i]);
            //pvcontext.m_physicalDevice=physicalDevices[i];
            context.m_physicalDevice = physicalDevices[i];
        }
    }

    /*
     * There are 3 devices found
     GPUs: Intel(R) UHD Graphics 630 (CFL GT2)
     GPUs: llvmpipe (LLVM 12.0.0, 256 bits)
     GPUs: GeForce RTX 2080

     */

    // Run task per device
    //deviceCount = gpuPhysicalDevices.size();
    //std::cout << "There are " << deviceCount << " devices found" << std::endl;

    // Create the allocator
    //pvcontext.SetUpQueue() ;

    //allocator.init(pvcontext.m_device, pvcontext.m_physicalDevice);
    context.SetUpQueue();

    allocator.init(context.m_device, context.m_physicalDevice);

    /*TODO: Create command pool*/

#endif;
}

void RayTrace::Initialize()
{
    /*Creation of context*/

    nvvk::ContextCreateInfo deviceInfo; // One can modify this to load different extensions or pick the Vulkan core version
    deviceInfo.apiMajor = 1; // Specify the version of Vulkan we'll use
    deviceInfo.apiMinor = 2;
    // Required by KHR_acceleration_structure; allows work to be offloaded onto background threads and parallelized
    deviceInfo.addDeviceExtension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    VkPhysicalDeviceAccelerationStructureFeaturesKHR asFeatures = nvvk::make<VkPhysicalDeviceAccelerationStructureFeaturesKHR>();
    deviceInfo.addDeviceExtension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, false, &asFeatures);
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures = nvvk::make<VkPhysicalDeviceRayTracingPipelineFeaturesKHR>();
    deviceInfo.addDeviceExtension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, false, &rtPipelineFeatures);
    deviceInfo.addDeviceExtension(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    VkValidationFeaturesEXT validationInfo = nvvk::make<VkValidationFeaturesEXT>();
    VkValidationFeatureEnableEXT validationFeatureToEnable = VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT;
    validationInfo.enabledValidationFeatureCount = 1;
    validationInfo.pEnabledValidationFeatures = &validationFeatureToEnable;
    deviceInfo.instanceCreateInfoExt = &validationInfo;

    putenv("DEBUG_PRINTF_TO_STDOUT=1");

    //VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT
    // Encapsulates device state in a single object
    context.init(deviceInfo); // Initialize the context
    // Device must support acceleration structures and ray tracing pipelines:
    assert(asFeatures.accelerationStructure == VK_TRUE && rtPipelineFeatures.rayTracingPipeline == VK_TRUE);

    // Create the allocator

    allocator.init(context, context.m_physicalDevice);
}

void RayTrace::SetPipelineProperties()
{
    // Get the properties of ray tracing pipelines on this device. We do this by
    // using vkGetPhysicalDeviceProperties2, and extending this by chaining on a
    // VkPhysicalDeviceRayTracingPipelinePropertiesKHR object to get both
    // physical device properties and ray tracing pipeline properties.
    // This gives us information about shader binding tables.
    /*rtPipelineProperties =
        nvvk::make<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    VkPhysicalDeviceProperties2 physicalDeviceProperties = nvvk::make<VkPhysicalDeviceProperties2>();
    physicalDeviceProperties.pNext                       = &rtPipelineProperties;
    vkGetPhysicalDeviceProperties2(context.m_physicalDevice, &physicalDeviceProperties);
    VkDeviceSize sbtHeaderSize      = rtPipelineProperties.shaderGroupHandleSize;
    VkDeviceSize sbtBaseAlignment   = rtPipelineProperties.shaderGroupBaseAlignment;
    VkDeviceSize sbtHandleAlignment = rtPipelineProperties.shaderGroupHandleAlignment;
    
    assert(sbtBaseAlignment % sbtHandleAlignment == 0);
     sbtStride = sbtBaseAlignment *  //
                                    ((sbtHeaderSize + sbtBaseAlignment - 1) / sbtBaseAlignment);
     assert(sbtStride <= rtPipelineProperties.maxShaderGroupStride);*/
}

void PipelineSetUp::CreateImage() /* A set of operations for image to write ray-tracing output to*/
{
    // Create an image. Images are more complex than buffers - they can have
    // multiple dimensions, different color+depth formats, be arrays of mips,
    // have multisampling, be tiled in memory in e.g. row-linear order or in an
    // implementation-dependent way (and this layout of memory can depend on
    // what the image is being used for), and be shared across multiple queues.
    // Here's how we specify the image we'll use:
    VkImageCreateInfo imageCreateInfo = nvvk::make<VkImageCreateInfo>();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    // RGB32 images aren't usually supported, so we change this to a RGBA32 image.
    imageCreateInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    // Defines the size of the image:
    imageCreateInfo.extent = { render_width, render_height, 1 };
    // The image is an array of length 1, and each element contains only 1 mip:
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    // We aren't using MSAA (i.e. the image only contains 1 sample per pixel -
    // note that this isn't the same use of the word "sample" as in ray tracing):
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    // The driver controls the tiling of the image for performance:
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    // This image is read and written on the GPU, and data can be transferred
    // from it:
    imageCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    // Image is only used by one queue:
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // The image must be in either VK_IMAGE_LAYOUT_UNDEFINED or VK_IMAGE_LAYOUT_PREINITIALIZED
    // according to the specification; we'll transition the layout shortly,
    // in the same command buffer used to upload the vertex and index buffers:
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    std::cout << "Image" << std::endl;
    image = allocator.createImage(imageCreateInfo);
    std::cout << "Image" << std::endl;
    //debugUtil.setObjectName(image.image, "image");

    // Create an image view for the entire image
    // When we create a descriptor for the image, we'll also need an image view
    // that the descriptor will point to. This specifies what part of the image
    // the descriptor views, and how the descriptor views it.
    VkImageViewCreateInfo imageViewCreateInfo = nvvk::make<VkImageViewCreateInfo>();
    imageViewCreateInfo.image = image.image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = imageCreateInfo.format;
    // We could use imageViewCreateInfo.components to make the components of the
    // image appear to be "swizzled", but we don't want to do that. Luckily,
    // all values are set to VK_COMPONENT_SWIZZLE_IDENTITY, which means
    // "don't change anything", by nvvk::make or zero initialization.
    // This says that the ImageView views the color part of the image (since
    // images can contain depth or stencil aspects):
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // This says that we only look at array layer 0 and mip level 0:
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;

    NVVK_CHECK(vkCreateImageView(context, &imageViewCreateInfo, nullptr, &imageView));

    //debugUtil.setObjectName(imageView, "imageView");

    // Also create an image using linear tiling that can be accessed from the CPU,
    // much like how we created the buffer in the main tutorial. The first image
    // will be entirely local to the GPU for performance, while this image can
    // be mapped to CPU memory. We'll copy data from the first image to this
    // image in order to read the image data back on the CPU.
    // As before, we'll transition the image layout in the same command buffer
    // used to upload the vertex and index buffers.
    imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageLinear = allocator.createImage(imageCreateInfo, //
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT //
            | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT //
            | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
}

void RayTrace::STBCreate()
{
    rtPipelineProperties = nvvk::make<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    VkPhysicalDeviceProperties2 physicalDeviceProperties = nvvk::make<VkPhysicalDeviceProperties2>();
    physicalDeviceProperties.pNext = &rtPipelineProperties;
    vkGetPhysicalDeviceProperties2(context.m_physicalDevice, &physicalDeviceProperties);
    const VkDeviceSize sbtHeaderSize = rtPipelineProperties.shaderGroupHandleSize;
    const VkDeviceSize sbtBaseAlignment = rtPipelineProperties.shaderGroupBaseAlignment;
    const VkDeviceSize sbtHandleAlignment = rtPipelineProperties.shaderGroupHandleAlignment;

    assert(sbtBaseAlignment % sbtHandleAlignment == 0);
    const VkDeviceSize sbtStride = sbtBaseAlignment * //
        ((sbtHeaderSize + sbtBaseAlignment - 1) / sbtBaseAlignment);
    assert(sbtStride <= rtPipelineProperties.maxShaderGroupStride);
    std::cout << "Binding table allocated " << groups.size() << std::endl;
    std::vector<uint8_t> cpuShaderHandleStorage(sbtHeaderSize * groups.size());
    NVVK_CHECK(vkGetRayTracingShaderGroupHandlesKHR(context, // Device
        rtPipeline, // Pipeline
        0, // First group
        static_cast<uint32_t>(groups.size()), // Number of groups
        cpuShaderHandleStorage.size(), // Size of buffer
        cpuShaderHandleStorage.data())); // Data buffer
    // Allocate the shader binding table. We get its device address, and
    // use it as a shader binding table. As before, we set its memory property
    // flags so that it can be read and written from the CPU.

    const uint32_t sbtSize = static_cast<uint32_t>(sbtStride * groups.size());
    rtSBTBuffer = allocator.createBuffer(
        sbtSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //debugUtil.setObjectName(rtSBTBuffer.buffer, "rtSBTBuffer");
    // Copy the shader group handles to the SBT:
    uint8_t* mappedSBT = reinterpret_cast<uint8_t*>(allocator.map(rtSBTBuffer));
    for (size_t groupIndex = 0; groupIndex < groups.size(); groupIndex++) {
        memcpy(&mappedSBT[groupIndex * sbtStride], &cpuShaderHandleStorage[groupIndex * sbtHeaderSize], sbtHeaderSize);
    }
    allocator.unmap(rtSBTBuffer);
    // Clean up:
    allocator.finalizeAndReleaseStaging();

    // vkCmdTraceRaysKHR uses VkStridedDeviceAddressregionKHR objects to say
    // where each block of shaders is held in memory. These could change per
    // draw call, but let's create them up front since they're the same
    // every time here:

    const VkDeviceAddress sbtStartAddress = pv::GetBufferDeviceAddress(context, rtSBTBuffer.buffer);

    // The ray generation shader region:
    sbtRayGenRegion.deviceAddress = sbtStartAddress; // Starts here
    sbtRayGenRegion.stride = sbtStride; // Uses this stride
    sbtRayGenRegion.size = sbtStride; // Is this number of bytes long (1 group)

    sbtMissRegion = sbtRayGenRegion; // The miss shader region:
    sbtMissRegion.deviceAddress = sbtStartAddress + sbtStride; // Starts sbtStride bytes (1 group) in
    sbtMissRegion.size = sbtStride; // Is this number of bytes long (1 group)

    sbtHitRegion = sbtRayGenRegion; // The hit group region:
    sbtHitRegion.deviceAddress = sbtStartAddress + 2 * sbtStride; // Starts 2 * sbtStride bytes (2 groups) in
    sbtHitRegion.size = sbtStride * (NUM_C_HIT_SHADERS + 1); // Is this number of bytes long

    sbtCallableRegion = sbtRayGenRegion; // The callable shader region:
    sbtCallableRegion.size = 0; // Is empty
}

void RayTrace::CreateShaders()
{
    modules[0] = vks::tools::loadShader("shaders/raytrace.rgen.spv", context.m_device);
    //  debugUtil.setObjectName(modules[0], "Ray generation module (raytrace.rgen.glsl.spv)");
    modules[1] = vks::tools::loadShader("shaders/raytrace.rmiss.glsl.spv", context.m_device);
    //  debugUtil.setObjectName(modules[1], "Miss module (raytrace.rmiss.glsl.spv)");
    modules[2] = vks::tools::loadShader("shaders/material0.rchit.spv", context.m_device);
    //  debugUtil.setObjectName(modules[2], "Material 0 shader module");
    modules[3] = vks::tools::loadShader("shaders/raytrace.rint.spv", context.m_device);
    //   debugUtil.setObjectName(modules[3], "Material 0 intesection shader module");
    // First, we create objects that point to each of our shaders.
    // These are called "shader stages" in this context.
    // These are shader module + entry point + stage combinations, because each
    // shader module can contain multiple entry points (e.g. main1, main2...)

    // Stage 0 will be the raygen shader.
    stages[0] = nvvk::make<VkPipelineShaderStageCreateInfo>();
    stages[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR; // Kind of shader
    stages[0].module = modules[0]; // Contains the shader
    stages[0].pName = "main"; // Name of the entry point
    // Stage 1 will be the miss shader.
    stages[1] = stages[0];
    stages[1].stage = VK_SHADER_STAGE_MISS_BIT_KHR; // Kind of shader
    stages[1].module = modules[1]; // Contains the shader
    // Stage 2 will be the closest-hit shader.
    stages[2] = stages[0];
    stages[2].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR; // Kind of shader
    stages[2].module = modules[2]; // Contains the shader

    stages[3] = stages[0];
    stages[3].stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    stages[3].module = modules[3];

    // Then we make groups point to the shader stages. Each group can point to
    // 1-3 shader stages depending on the type, by specifying the index in the
    // stages array. These groups of handles then become the most important
    // part of the entries in the shader binding table.
    // Stores the indices of stages in each group:

    // The vkCmdTraceRays call will eventually refer to ray gen, miss, hit, and
    // callable shader binding tables and ranges.
    // A VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR group type is for a group
    // of one shader (a ray gen shader in a ray gen SBT region, a miss shader in
    // a miss SBT region, and so on.)
    // A VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR group type
    // is for an instance containing triangles. It can point to closest hit and
    // any hit shaders.
    // A VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR group type
    // is for a procedural instance, and can point to an intersection, any hit,
    // and closest hit shader.

    // We lay out our shader binding table like this:
    // RAY GEN REGION
    // Group 0 - points to Stage 0
    groups[0] = nvvk::make<VkRayTracingShaderGroupCreateInfoKHR>();
    groups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    groups[0].generalShader = 0; // Index of ray gen, miss, or callable in `stages`
    // MISS SHADER REGION
    // Group 1 - points to Stage 1
    groups[1] = nvvk::make<VkRayTracingShaderGroupCreateInfoKHR>();
    groups[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    groups[1].generalShader = 1; // Index of ray gen, miss, or callable in `stages`
    // CLOSEST-HIT REGION
    // Group 2 - uses Stage 2
    // closest hit shader + Intersection
    groups[2] = nvvk::make<VkRayTracingShaderGroupCreateInfoKHR>();
    groups[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
    groups[2].closestHitShader = 2; // Index of closest hit in `stages`
    groups[2].intersectionShader = 3; // Index of intersection in `stages`
    /*
    // closest hit shader + Intersection
    group.type               = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
    group.closestHitShader   = eClosestHit2;
    group.intersectionShader = eIntersection;
    m_rtShaderGroups.push_back(group);
    */

    // Now, describe the ray tracing pipeline, ike creating a compute pipeline:
    VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo = nvvk::make<VkRayTracingPipelineCreateInfoKHR>();
    pipelineCreateInfo.flags = 0; // No flags to set
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(stages.size());
    pipelineCreateInfo.pStages = stages.data();
    pipelineCreateInfo.groupCount = static_cast<uint32_t>(groups.size());
    pipelineCreateInfo.pGroups = groups.data();
    pipelineCreateInfo.maxPipelineRayRecursionDepth = m_maxDepth; // Depth of call tree
    pipelineCreateInfo.layout = descriptorSetContainer.getPipeLayout();
    NVVK_CHECK(vkCreateRayTracingPipelinesKHR(context, // Device
        VK_NULL_HANDLE, // Deferred operation or VK_NULL_HANDLE
        VK_NULL_HANDLE, // Pipeline cache or VK_NULL_HANDLE
        1, &pipelineCreateInfo, // Array of create infos
        nullptr, // Allocator
        &rtPipeline));
}

void RayTrace::CreateDescriptorSet(nvvk::Buffer vertexBuffer) //, nvvk::Buffer indexBuffer)
{
    switch (m_Dlayout) {

    case Basic: {
        // Here's the list of bindings for the descriptor set layout, from raytrace.comp.glsl:
        // 0 - a storage image (the image `image`)
        // 1 - an acceleration structure (the TLAS)
        // 2 - a storage buffer (the vertex buffer)
        // 3 - a storage buffer (the index buffer)
        descriptorSetContainer.init(context);
        descriptorSetContainer.addBinding(BIND_IMAGEDATA, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        descriptorSetContainer.addBinding(BIND_TLAS, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        descriptorSetContainer.addBinding(BIND_VERTICES, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR);
        //descriptorSetContainer.addBinding(BIND_INDICES, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        // Create a layout from the list of bindings
        descriptorSetContainer.initLayout();
        // Create a descriptor pool from the list of bindings with space for 1 set, and allocate that set
        descriptorSetContainer.initPool(1);
        // Create a push constant range describing the amount of data for the push constants.
        static_assert(sizeof(PushConstants) % 4 == 0, "Push constant size must be a multiple of 4 per the Vulkan spec!");
        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstants);
        // Create a pipeline layout from the descriptor set layout and push constant range:
        descriptorSetContainer.initPipeLayout(1, // Number of push constant ranges
            &pushConstantRange); // Pointer to push constant ranges

        // Write values into the descriptor set.
        std::array<VkWriteDescriptorSet, 3> writeDescriptorSets;
        // Color image
        VkDescriptorImageInfo descriptorImageInfo {};
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // The image's layout
        descriptorImageInfo.imageView = imageView; // How the image should be accessed
        writeDescriptorSets[0] = descriptorSetContainer.makeWrite(0 /*set index*/, BIND_IMAGEDATA /*binding*/, &descriptorImageInfo);
        // Top-level acceleration structure (TLAS)
        VkWriteDescriptorSetAccelerationStructureKHR descriptorAS = nvvk::make<VkWriteDescriptorSetAccelerationStructureKHR>();
        VkAccelerationStructureKHR tlasCopy = raytracingBuilder.getAccelerationStructure(); // So that we can take its address
        descriptorAS.accelerationStructureCount = 1;
        descriptorAS.pAccelerationStructures = &tlasCopy;
        writeDescriptorSets[1] = descriptorSetContainer.makeWrite(0, BIND_TLAS, &descriptorAS);
        // Vertex buffer
        VkDescriptorBufferInfo vertexDescriptorBufferInfo {};
        vertexDescriptorBufferInfo.buffer = vertexBuffer.buffer;
        vertexDescriptorBufferInfo.range = VK_WHOLE_SIZE;
        writeDescriptorSets[2] = descriptorSetContainer.makeWrite(0, BIND_VERTICES, &vertexDescriptorBufferInfo);
        // Index buffer
        //VkDescriptorBufferInfo indexDescriptorBufferInfo{};
        //indexDescriptorBufferInfo.buffer = indexBuffer.buffer;
        //indexDescriptorBufferInfo.range  = VK_WHOLE_SIZE;
        // writeDescriptorSets[3]           = descriptorSetContainer.makeWrite(0, BIND_INDICES, &indexDescriptorBufferInfo);
        vkUpdateDescriptorSets(context, // The context
            static_cast<uint32_t>(writeDescriptorSets.size()), // Number of VkWriteDescriptorSet objects
            writeDescriptorSets.data(), // Pointer to VkWriteDescriptorSet objects

            0, nullptr); // An array of VkCopyDescriptorSet objects (unused)
    }

    break;
    default: {
        // Here's the list of bindings for the descriptor set layout, from raytrace.comp.glsl:
        // 0 - a storage image (the image `image`)
        // 1 - an acceleration structure (the TLAS)
        // 2 - a storage buffer (the vertex buffer)
        // 3 - a storage buffer (the index buffer)
        descriptorSetContainer.init(context);
        descriptorSetContainer.addBinding(BIND_IMAGEDATA, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        descriptorSetContainer.addBinding(BIND_TLAS, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        descriptorSetContainer.addBinding(BIND_VERTICES, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_INTERSECTION_BIT_KHR);
        //descriptorSetContainer.addBinding(BIND_INDICES, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        // Create a layout from the list of bindings
        descriptorSetContainer.initLayout();
        // Create a descriptor pool from the list of bindings with space for 1 set, and allocate that set
        descriptorSetContainer.initPool(1);
        // Create a push constant range describing the amount of data for the push constants.
        static_assert(sizeof(PushConstants) % 4 == 0, "Push constant size must be a multiple of 4 per the Vulkan spec!");
        VkPushConstantRange pushConstantRange;
        pushConstantRange.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstants);
        // Create a pipeline layout from the descriptor set layout and push constant range:
        descriptorSetContainer.initPipeLayout(1, // Number of push constant ranges
            &pushConstantRange); // Pointer to push constant ranges

        // Write values into the descriptor set.
        std::array<VkWriteDescriptorSet, 3> writeDescriptorSets;
        // Color image
        VkDescriptorImageInfo descriptorImageInfo {};
        descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // The image's layout
        descriptorImageInfo.imageView = imageView; // How the image should be accessed
        writeDescriptorSets[0] = descriptorSetContainer.makeWrite(0 /*set index*/, BIND_IMAGEDATA /*binding*/, &descriptorImageInfo);
        // Top-level acceleration structure (TLAS)
        VkWriteDescriptorSetAccelerationStructureKHR descriptorAS = nvvk::make<VkWriteDescriptorSetAccelerationStructureKHR>();
        VkAccelerationStructureKHR tlasCopy = raytracingBuilder.getAccelerationStructure(); // So that we can take its address
        descriptorAS.accelerationStructureCount = 1;
        descriptorAS.pAccelerationStructures = &tlasCopy;
        writeDescriptorSets[1] = descriptorSetContainer.makeWrite(0, BIND_TLAS, &descriptorAS);
        // Vertex buffer
        VkDescriptorBufferInfo vertexDescriptorBufferInfo {};
        vertexDescriptorBufferInfo.buffer = vertexBuffer.buffer;
        vertexDescriptorBufferInfo.range = VK_WHOLE_SIZE;
        writeDescriptorSets[2] = descriptorSetContainer.makeWrite(0, BIND_VERTICES, &vertexDescriptorBufferInfo);
        // Index buffer
        VkDescriptorBufferInfo indexDescriptorBufferInfo {};
        //indexDescriptorBufferInfo.buffer = indexBuffer.buffer;
        //indexDescriptorBufferInfo.range  = VK_WHOLE_SIZE;
        // writeDescriptorSets[3]           = descriptorSetContainer.makeWrite(0, BIND_INDICES, &indexDescriptorBufferInfo);
        vkUpdateDescriptorSets(context, // The context
            static_cast<uint32_t>(writeDescriptorSets.size()), // Number of VkWriteDescriptorSet objects
            writeDescriptorSets.data(), // Pointer to VkWriteDescriptorSet objects

            0, nullptr); // An array of VkCopyDescriptorSet objects (unused)
    }
    }
}
