#include "helpers.hpp"
#include "postprocessing.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <array>


/*
 * For image processing as a result of MPI
 */
struct TexVertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(TexVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(TexVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(TexVertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(TexVertex, texCoord);

        return attributeDescriptions;
    }
};


const std::vector<TexVertex> vertices = {
    {{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
     2, 3, 0, 0, 1, 2
};

/*Helpers*/

 void MPICollect::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,pv2::Context context) {
     VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);
 
   /*  VkBufferImageCopy region{};
     region.bufferOffset = 0;
     region.bufferRowLength = 0;
     region.bufferImageHeight = 0;
     region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
     region.imageSubresource.mipLevel = 0;
     region.imageSubresource.baseArrayLayer = 0;
     region.imageSubresource.layerCount = 1;
     region.imageOffset = {0, 0, 0};
     region.imageExtent = {
         width,
         height,
         1
     };*/
    
VkBufferImageCopy region;
	memset(&region, 0, sizeof(region));
	region.bufferOffset = 0;
	region.bufferRowLength = width;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent.width = width;
	region.imageExtent.height = height;
	region.imageExtent.depth = 1;
 
     vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
 
     endSingleTimeCommands(commandBuffer,context);
 }

VkCommandBuffer MPICollect::beginSingleTimeCommands(pv2::Context context ) {
       VkCommandBufferAllocateInfo allocInfo{};
       allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
       allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
       allocInfo.commandPool = m_commandPool;
       allocInfo.commandBufferCount = 1;

       VkCommandBuffer commandBuffer;
       vkAllocateCommandBuffers(context.m_device, &allocInfo, &commandBuffer);

       VkCommandBufferBeginInfo beginInfo{};
       beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
       beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

       vkBeginCommandBuffer(commandBuffer, &beginInfo);

       return commandBuffer;
   }

void MPICollect::CreateMPICommandBuffers(pv2::Context context, pv2::RenderBase ren)
{
   // if(context.GetInteractive())
       m_commandBuffers.resize(ren.m_swapChainFramebuffers.size());
   // else
   //     m_commandBuffers.resize(1);
   DEBUG_LOG<<"Size of command buffers "<<m_commandBuffers.size()<<std::endl;

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
        
        //Non interactive
        if(!context.GetInteractive())
        {
        VkViewport viewport = {};
                   viewport.height = (float)ren.m_Extent.height;
                   viewport.width = (float)ren.m_Extent.width;
                   viewport.minDepth = (float)0.0f;
                   viewport.maxDepth = (float)1.0f;
                   vkCmdSetViewport(m_commandBuffers[i], 0, 1, &viewport);
                   VkRect2D scissor = {};
                   scissor.extent.width = ren.m_Extent.width;
                   scissor.extent.height = ren.m_Extent.height;
                   vkCmdSetScissor(m_commandBuffers[i], 0, 1, &scissor);
               }
//Non interactive
        vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(m_commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
         
        
        vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);
        //vkCmdDispatch(m_commandBuffers[i], BUFFER_ELEMENTS, 1, 1);
        DEBUG_LOG<<"Descriptors size "<<m_descriptorSets.size()<<std::endl;
        //vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        
        vkCmdDrawIndexed(m_commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
            
        vkCmdEndRenderPass(m_commandBuffers[i]);

        if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
        
        if(!context.GetInteractive())
               {
                   submitBuffers(context, 0);
                   DEBUG_LOG<<"Submitted non interactive"<<std::endl;
               }
    }
}

void MPICollect::endSingleTimeCommands(VkCommandBuffer commandBuffer,pv2::Context context) {
       vkEndCommandBuffer(commandBuffer);

       VkSubmitInfo submitInfo{};
       submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
       submitInfo.commandBufferCount = 1;
       submitInfo.pCommandBuffers = &commandBuffer;

       vkQueueSubmit(context.m_queueGCT, 1, &submitInfo, VK_NULL_HANDLE);
       vkQueueWaitIdle(context.m_queueGCT);

       vkFreeCommandBuffers(context.m_device, m_commandPool, 1, &commandBuffer);
   }  		
    		
 void MPICollect::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, pv2::Context context) {
     VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);
 
     VkImageMemoryBarrier barrier{};
     barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
     barrier.oldLayout = oldLayout;
     barrier.newLayout = newLayout;
     barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
     barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
     barrier.image = image;
     barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
     barrier.subresourceRange.baseMipLevel = 0;
     barrier.subresourceRange.levelCount = 1;
     barrier.subresourceRange.baseArrayLayer = 0;
     barrier.subresourceRange.layerCount = 1;
 
     VkPipelineStageFlags sourceStage;
     VkPipelineStageFlags destinationStage;
 
     if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
         barrier.srcAccessMask = 0;
         barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
 
         sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
         destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
     } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
         barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
         barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
 
         sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
         destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
     } else {
         throw std::invalid_argument("unsupported layout transition!");
     }
 
     vkCmdPipelineBarrier(
         commandBuffer,
         sourceStage, destinationStage,
         0,
         0, nullptr,
         0, nullptr,
         1, &barrier
     );
 
     endSingleTimeCommands(commandBuffer,context);
 }   		
    		//---------------------------



void MPICollect::CreateMPIDescriptorPool(pv2::Context context, int size) {
    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(size);
    

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(size);
    
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(size);
    

    if (vkCreateDescriptorPool(context.m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}


void MPICollect::CreateMPIDescriptorSets(pv2::Context context, int size, SimpCamera cam) {
    std::vector<VkDescriptorSetLayout> layouts(size, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    allocInfo.pSetLayouts = layouts.data();
    
    m_descriptorSets.resize(size);
    CHECK_VK(vkAllocateDescriptorSets(context.m_device, &allocInfo, m_descriptorSets.data()));
    

for (size_t i = 0; i < size; i++) {
   
    
    VkDescriptorImageInfo imageInfo{};
         imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
         imageInfo.imageView = m_textureImageView;
         imageInfo.sampler = m_textureSampler;
         
         
         std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
         
       
         
         descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
         descriptorWrites[0].dstSet = m_descriptorSets[i];
         descriptorWrites[0].dstBinding = 1;
         descriptorWrites[0].dstArrayElement = 0;
         descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
         descriptorWrites[0].descriptorCount = 1;
         descriptorWrites[0].pImageInfo = &imageInfo;

  
vkUpdateDescriptorSets(context.m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

}
    

}

void MPICollect::CreateMPIDescriptorSetLayout(pv2::Context context) {


    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(context.m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void MPICollect::CreateVertexBuffer(pv2::Context context) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    pv::createBuffer(context.m_device, context.m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(context.m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(context.m_device, stagingBufferMemory);

    pv::createBuffer(context.m_device, context.m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    pv::copyBuffer(stagingBuffer, vertexBuffer, bufferSize, m_commandPool, context.m_device, context.m_queueGCT);

    vkDestroyBuffer(context.m_device, stagingBuffer, nullptr);
    vkFreeMemory(context.m_device, stagingBufferMemory, nullptr);
}

void MPICollect::CreateIndexBuffer(pv2::Context context) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    pv::createBuffer(context.m_device, context.m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(context.m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, indices.data(), (size_t) bufferSize);
    vkUnmapMemory(context.m_device, stagingBufferMemory);

    pv::createBuffer(context.m_device, context.m_physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    pv::copyBuffer(stagingBuffer, indexBuffer, bufferSize,m_commandPool, context.m_device, context.m_queueGCT);

    vkDestroyBuffer(context.m_device, stagingBuffer, nullptr);
    vkFreeMemory(context.m_device, stagingBufferMemory, nullptr);
}


void MPICollect::CreateTextureImage(pv2::Context context, char* pixel, int texWidth, int texHeight, int texChannels) {
  
    stbi_uc* pixels=(stbi_uc*)pixel;
    VkDeviceSize imageSize = texWidth * texHeight* texChannels;
  
            // int x, y, comp;
             
             //	stbi_uc*pixels = stbi_load_from_memory((stbi_uc*)pixel, texWidth*texHeight, &x, &y, &comp, STBI_rgb_alpha);
             	        
    //stbi_uc* pixels = stbi_load("text.ppm", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    DEBUG_LOG<<"Image size "<< texWidth<<", "<<texHeight<<", "<<texChannels<<std::endl;
    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }
    
    
  

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    pv::createBuffer(context.m_device,context.m_physicalDevice,imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    char* data;
    vkMapMemory(context.m_device, stagingBufferMemory, 0, imageSize, 0, (void**)&data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
       SaveImage("text.ppm",data,imageSize,texWidth,texHeight);
    vkUnmapMemory(context.m_device, stagingBufferMemory);

    //stbi_image_free(pixels);

    pv::createImage(context.m_device, context.m_physicalDevice,texWidth, texHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

    transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,context);
    copyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),context);
    transitionImageLayout(m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,context);

    vkQueueWaitIdle(context.m_queueGCT);
    vkDestroyBuffer(context.m_device, stagingBuffer, nullptr);
    vkFreeMemory(context.m_device, stagingBufferMemory, nullptr);
}



void MPICollect::CreateTextureImageView(pv2::Context context) {
    m_textureImageView = pv::createImageView(context.m_device, m_textureImage, VK_FORMAT_R8G8B8A8_UNORM);
}

void MPICollect::CreateTextureSampler(pv2::Context context) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(context.m_physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(context.m_device, &samplerInfo, nullptr, &m_textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}


/*
void MPICollect::CreateMPIGraphicsPipeline(pv2::Context context, pv2::RenderBase ren)
{
    auto vertShaderCode = pv::readFile(vertMPIShaderName);
    auto fragShaderCode = pv::readFile(fragMPIShaderName);

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

    auto bindingDescription = TexVertex::getBindingDescription();
    auto attributeDescriptions = TexVertex::getAttributeDescriptions();

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



    VkPipelineRasterizationStateCreateInfo rasterizationState {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;


    VkPipelineColorBlendAttachmentState blendAttachmentState {};
    blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachmentState.blendEnable = VK_FALSE;



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
*/

void MPICollect::CreateCommandBuffers(pv2::Context context) {
       m_commandBuffers.resize(1);

       VkCommandBufferAllocateInfo allocInfo{};
       allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
       allocInfo.commandPool = m_commandPool;
       allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
       allocInfo.commandBufferCount = (uint32_t) m_commandBuffers.size();

       if (vkAllocateCommandBuffers(context.m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
           throw std::runtime_error("failed to allocate command buffers!");
       }
   }
   
void MPICollect::RecordCommandBuffer(pv2::RenderBase ren) {
       VkCommandBufferBeginInfo beginInfo{};
       beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   
       if (vkBeginCommandBuffer(m_commandBuffers[0], &beginInfo) != VK_SUCCESS) {
           throw std::runtime_error("failed to begin recording command buffer!");
       }
   
       VkRenderPassBeginInfo renderPassInfo{};
       renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
       renderPassInfo.renderPass = ren.m_renderPass;
       renderPassInfo.framebuffer = ren.m_swapChainFramebuffers[0];
       renderPassInfo.renderArea.offset = {0, 0};
       renderPassInfo.renderArea.extent = ren.m_Extent;
   
       VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
       renderPassInfo.clearValueCount = 1;
       renderPassInfo.pClearValues = &clearColor;
   
       vkCmdBeginRenderPass(m_commandBuffers[0], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
   
           vkCmdBindPipeline(m_commandBuffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
   
           VkBuffer vertexBuffers[] = {vertexBuffer};
           VkDeviceSize offsets[] = {0};
           vkCmdBindVertexBuffers(m_commandBuffers[0], 0, 1, vertexBuffers, offsets);
   
           vkCmdBindIndexBuffer(m_commandBuffers[0], indexBuffer, 0, VK_INDEX_TYPE_UINT16);
   
           vkCmdBindDescriptorSets(m_commandBuffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[0], 0, nullptr);
   
           vkCmdDrawIndexed(m_commandBuffers[0], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
   
       vkCmdEndRenderPass(m_commandBuffers[0]);
   
       if (vkEndCommandBuffer(m_commandBuffers[0]) != VK_SUCCESS) {
           throw std::runtime_error("failed to record command buffer!");
       }
   }
   
   
void MPICollect::CreateMPIGraphicsPipeline(pv2::Context context, pv2::RenderBase ren)
{
    auto vertShaderCode = pv::readFile(vertMPIShaderName);
    auto fragShaderCode = pv::readFile(fragMPIShaderName);

    VkShaderModule vertShaderModule = pv::createShaderModule(vertShaderCode, context.m_device);
    VkShaderModule fragShaderModule = pv::createShaderModule(fragShaderCode, context.m_device);

     VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
     vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
     vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
     vertShaderStageInfo.module = vertShaderModule;
     vertShaderStageInfo.pName = "main";

     VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
     fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
     fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
     fragShaderStageInfo.module = fragShaderModule;
     fragShaderStageInfo.pName = "main";

     VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

     VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
     vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

     auto bindingDescription = TexVertex::getBindingDescription();
     auto attributeDescriptions = TexVertex::getAttributeDescriptions();

     vertexInputInfo.vertexBindingDescriptionCount = 1;
     vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
     vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
     vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

     VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
     inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
     inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
     inputAssembly.primitiveRestartEnable = VK_FALSE;

     VkViewport viewport{};
     viewport.x = 0.0f;
     viewport.y = 0.0f;
     viewport.width = (float)ren.m_Extent.width;
     viewport.height = (float)ren.m_Extent.height;
     viewport.minDepth = 0.0f;
     viewport.maxDepth = 1.0f;

     VkRect2D scissor{};
     scissor.offset = {0, 0};
     scissor.extent = ren.m_Extent;

     VkPipelineViewportStateCreateInfo viewportState{};
     viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
     viewportState.viewportCount = 1;
     viewportState.pViewports = &viewport;
     viewportState.scissorCount = 1;
     viewportState.pScissors = &scissor;

     VkPipelineRasterizationStateCreateInfo rasterizer{};
     rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
     rasterizer.depthClampEnable = VK_FALSE;
     rasterizer.rasterizerDiscardEnable = VK_FALSE;
     rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
     rasterizer.lineWidth = 1.0f;
     rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
     rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;//VK_FRONT_FACE_COUNTER_CLOCKWISE;
     rasterizer.depthBiasEnable = VK_FALSE;

     VkPipelineMultisampleStateCreateInfo multisampling{};
     multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
     multisampling.sampleShadingEnable = VK_FALSE;
     multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

     VkPipelineColorBlendAttachmentState colorBlendAttachment{};
     colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
     colorBlendAttachment.blendEnable = VK_FALSE;

     VkPipelineColorBlendStateCreateInfo colorBlending{};
     colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
     colorBlending.logicOpEnable = VK_FALSE;
     colorBlending.logicOp = VK_LOGIC_OP_COPY;
     colorBlending.attachmentCount = 1;
     colorBlending.pAttachments = &colorBlendAttachment;
     colorBlending.blendConstants[0] = 0.0f;
     colorBlending.blendConstants[1] = 0.0f;
     colorBlending.blendConstants[2] = 0.0f;
     colorBlending.blendConstants[3] = 0.0f;

     VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
     pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
     pipelineLayoutInfo.setLayoutCount = 1;
     pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

     if (vkCreatePipelineLayout(context.m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
         throw std::runtime_error("failed to create pipeline layout!");
     }

     VkGraphicsPipelineCreateInfo pipelineInfo{};
     pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
     pipelineInfo.stageCount = 2;
     pipelineInfo.pStages = shaderStages;
     pipelineInfo.pVertexInputState = &vertexInputInfo;
     pipelineInfo.pInputAssemblyState = &inputAssembly;
     pipelineInfo.pViewportState = &viewportState;
     pipelineInfo.pRasterizationState = &rasterizer;
     pipelineInfo.pMultisampleState = &multisampling;
     pipelineInfo.pColorBlendState = &colorBlending;
     pipelineInfo.layout = m_pipelineLayout;
     pipelineInfo.renderPass = ren.m_renderPass;
     pipelineInfo.subpass = 0;
     pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

     if (vkCreateGraphicsPipelines(context.m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
         throw std::runtime_error("failed to create graphics pipeline!");
     }

     vkDestroyShaderModule(context.m_device, fragShaderModule, nullptr);
     vkDestroyShaderModule(context.m_device, vertShaderModule, nullptr);
 }
/*
void MPICollect::CreateMPIGraphicsPipeline(pv2::Context context, pv2::RenderBase ren)
{

auto vertShaderCode = pv::readFile(vertMPIShaderName);
 auto fragShaderCode = pv::readFile(fragMPIShaderName);

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

      VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

      VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
      vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

      auto bindingDescription = TexVertex::getBindingDescription();
      auto attributeDescriptions = TexVertex::getAttributeDescriptions();

      vertexInputInfo.vertexBindingDescriptionCount = 1;
      vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
      vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
      vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

      VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
      inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      inputAssembly.primitiveRestartEnable = VK_FALSE;

      VkViewport viewport{};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = (float) ren.m_Extent.width;
      viewport.height = (float) ren.m_Extent.height;
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      VkRect2D scissor{};
      scissor.offset = {0, 0};
      scissor.extent = ren.m_Extent;

      VkPipelineViewportStateCreateInfo viewportState{};
      viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      viewportState.viewportCount = 1;
      viewportState.pViewports = &viewport;
      viewportState.scissorCount = 1;
      viewportState.pScissors = &scissor;

      VkPipelineRasterizationStateCreateInfo rasterizer{};
      rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      rasterizer.depthClampEnable = VK_FALSE;
      rasterizer.rasterizerDiscardEnable = VK_FALSE;
      rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
      rasterizer.lineWidth = 1.0f;
      rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
      rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
      rasterizer.depthBiasEnable = VK_FALSE;

      VkPipelineMultisampleStateCreateInfo multisampling{};
      multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      multisampling.sampleShadingEnable = VK_FALSE;
      multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

      VkPipelineColorBlendAttachmentState colorBlendAttachment{};
      colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      colorBlendAttachment.blendEnable = VK_FALSE;

      VkPipelineColorBlendStateCreateInfo colorBlending{};
      colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      colorBlending.logicOpEnable = VK_FALSE;
      colorBlending.logicOp = VK_LOGIC_OP_COPY;
      colorBlending.attachmentCount = 1;
      colorBlending.pAttachments = &colorBlendAttachment;
      colorBlending.blendConstants[0] = 0.0f;
      colorBlending.blendConstants[1] = 0.0f;
      colorBlending.blendConstants[2] = 0.0f;
      colorBlending.blendConstants[3] = 0.0f;

      VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
      pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipelineLayoutInfo.setLayoutCount = 1;
      pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

      if (vkCreatePipelineLayout(context.m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
          throw std::runtime_error("failed to create pipeline layout!");
      }

      VkGraphicsPipelineCreateInfo pipelineInfo{};
      pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      pipelineInfo.stageCount = 2;
      pipelineInfo.pStages = shaderStages;
      pipelineInfo.pVertexInputState = &vertexInputInfo;
      pipelineInfo.pInputAssemblyState = &inputAssembly;
      pipelineInfo.pViewportState = &viewportState;
      pipelineInfo.pRasterizationState = &rasterizer;
      pipelineInfo.pMultisampleState = &multisampling;
      pipelineInfo.pColorBlendState = &colorBlending;
      pipelineInfo.layout = m_pipelineLayout;
      pipelineInfo.renderPass = ren.m_renderPass;
      pipelineInfo.subpass = 0;
      pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

      if (vkCreateGraphicsPipelines(context.m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
          throw std::runtime_error("failed to create graphics pipeline!");
      }

      vkDestroyShaderModule(context.m_device, fragShaderModule, nullptr);
      vkDestroyShaderModule(context.m_device, vertShaderModule, nullptr);
  }
*/
