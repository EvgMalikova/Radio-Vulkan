#include "renderer.hpp"
#include <algorithm>

namespace pv2 {
void RenderBase::CreateSurface(pv2::Context m_context)
{
    //if (glfwCreateWindowSurface(m_context.m_instance, win.GetWindow(), nullptr, &m_surface) != VK_SUCCESS) {
    if (glfwCreateWindowSurface(m_context.m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void RenderBase::CreateSwapChain(pv2::Context m_context)
{
    pv::SwapChainSupportDetails swapChainSupport = pv::querySwapChainSupport(m_context.m_physicalDevice, m_surface);

    VkSurfaceFormatKHR surfaceFormat = pv::chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = pv::chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = pv::chooseSwapExtent(swapChainSupport.capabilities, m_window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    pv::QueueFamilyIndices indices = pv::findQueueFamilies(m_context.m_physicalDevice, m_surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(m_context.m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_context.m_device, m_swapChain, &imageCount, nullptr);
    m_Images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_context.m_device, m_swapChain, &imageCount, m_Images.data());

    // suitable depth format for further Depth attachment introduction
    VkBool32 validDepthFormat = pv::getSupportedDepthFormat(m_context.m_physicalDevice, &m_depthFormat);
    DEBUG_LOG<<validDepthFormat<<std::endl;
    
    m_ImageFormat = surfaceFormat.format;
    m_Extent = extent;
}

void RenderBase::CreateRenderPass(pv2::Context m_context)
{
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format = m_ImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_context.m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderBase::CreateImageViews(pv2::Context m_context)
{
    m_ImageViews.resize(m_Images.size());

    for (size_t i = 0; i < m_Images.size(); i++) {
        VkImageViewCreateInfo createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_Images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_ImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; //VK_IMAGE_ASPECT_DEPTH_BIT;//
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        
        /*if (m_ImageFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
        		createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        	}
        */
        
        if (vkCreateImageView(m_context.m_device, &createInfo, nullptr, &m_ImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
        DEBUG_LOG<<"ImageView is created for "<<i<<std::endl;
    }
}

void RenderBase::CreateImage(pv2::Context context)
{
    
    	
    VkImageCreateInfo image {};
    image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image.imageType = VK_IMAGE_TYPE_2D;
    image.format = m_ImageFormat;
    image.extent.width = m_Extent.width;
    image.extent.height = m_Extent.height;
    image.extent.depth = 1;
    image.mipLevels = 1;
    image.arrayLayers = 1;
    image.samples = VK_SAMPLE_COUNT_1_BIT;
    image.tiling = VK_IMAGE_TILING_OPTIMAL;
    image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    //image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    
    VkImage inputIm;

    vkCreateImage(context.m_device, &image, nullptr, &inputIm);

    VkMemoryAllocateInfo memAlloc {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs;

    VkDeviceMemory imMem;

    vkGetImageMemoryRequirements(context.m_device, inputIm, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = pv::getMemoryTypeIndex(
        memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context.m_physicalDevice);
    vkAllocateMemory(context.m_device, &memAlloc, nullptr,
        &imMem);
    vkBindImageMemory(context.m_device, inputIm,
        imMem, 0);

    m_Images.push_back(inputIm);
    m_Images.resize(1);
    
    
    DEBUG_LOG << "Image object is created" << std::endl;
}
void RenderBase::CreateFramebuffers(pv2::Context m_context)
{
    m_swapChainFramebuffers.resize(m_ImageViews.size());

    for (size_t i = 0; i < m_ImageViews.size(); i++) {
        VkImageView attachments[] = {
            m_ImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_Extent.width;
        framebufferInfo.height = m_Extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_context.m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
        DEBUG_LOG<<"Created FrameBuffer "<<i<<std::endl;
    }
}

void RenderBase::CleanUp(pv2::Context context)
{
    vkDestroyRenderPass(context.m_device, m_renderPass, nullptr);
    for (auto imageView : m_ImageViews) {
        vkDestroyImageView(context.m_device, imageView, nullptr);
    }
    m_Images.resize(0);
    m_ImageViews.resize(0);
    if (context.GetInteractive())
    vkDestroySwapchainKHR(context.m_device, m_swapChain, nullptr);
}

/*
 void RenderBase::CleanupSwapChain(pv2::Context context) {
      for (auto framebuffer : m_swapChainFramebuffers) {
          vkDestroyFramebuffer(context.m_device, framebuffer, nullptr);
      }
 
      vkFreeCommandBuffers(context.m_device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
 
      vkDestroyPipeline(context.m_device, graphicsPipeline, nullptr);
      vkDestroyPipelineLayout(context.m_device, pipelineLayout, nullptr);
      
 
      ren.CleanUp(context);
      
 
      for (size_t i = 0; i < ren.m_Images.size(); i++) {
          vkDestroyBuffer(context.m_device, uniformBuffers[i], nullptr);
          vkFreeMemory(context.m_device, uniformBuffersMemory[i], nullptr);
      }
 
      vkDestroyDescriptorPool(context.m_device, descriptorPool, nullptr);
  }
  */

void RenderBase::SaveImage(int ij, pv2::Context context, VkCommandPool commandPool, VkQueue queueG, FrameBufferAttachment colorAttachment)
{
    char str[16];
    if (ij < 10)
        sprintf(str, "%s%d%s", "00", ij, "_headless.ppm");
    else {
        if (ij < 100)
            sprintf(str, "%s%d%s", "0", ij, "_headless.ppm");
        else
            sprintf(str, "%d%s", ij, "_headless.ppm");
    }

    sprintf(str, "%d%s", ij, "_headless.ppm");
    const char* filename = str;

    /*
                                   Copy framebuffer image to host visible image
                           */
    const char* imagedata;

    // Create the linear tiled destination image to copy to and to read the memory from
    VkImageCreateInfo imgCreateInfo {};
    imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imgCreateInfo.extent.width = m_Extent.width;
    imgCreateInfo.extent.height = m_Extent.height;
    imgCreateInfo.extent.depth = 1;
    imgCreateInfo.arrayLayers = 1;
    imgCreateInfo.mipLevels = 1;
    imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
    imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    // Create the image
    VkImage dstImage;
    vkCreateImage(context.m_device, &imgCreateInfo, nullptr, &dstImage);
    // Create memory to back up the image
    VkMemoryRequirements memRequirements;

    VkMemoryAllocateInfo memAllocInfo {};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkDeviceMemory dstImageMemory;
    vkGetImageMemoryRequirements(context.m_device, dstImage, &memRequirements);
    memAllocInfo.allocationSize = memRequirements.size;
    // Memory must be host visible to copy from
    memAllocInfo.memoryTypeIndex = pv::getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context.m_physicalDevice);
    vkAllocateMemory(context.m_device, &memAllocInfo, nullptr, &dstImageMemory);
    vkBindImageMemory(context.m_device, dstImage, dstImageMemory, 0);

    // Do the actual blit from the offscreen image to our host visible destination image
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = pv::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    VkCommandBuffer copyCmd;
    vkAllocateCommandBuffers(context.m_device, &cmdBufAllocateInfo, &copyCmd);
    VkCommandBufferBeginInfo cmdBufInfo = pv::commandBufferBeginInfo();
    vkBeginCommandBuffer(copyCmd, &cmdBufInfo);

    // Transition destination image to transfer destination layout
    pv::insertImageMemoryBarrier(
        copyCmd,
        dstImage,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    // colorAttachment.image is already in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, and does not need to be transitioned

    VkImageCopy imageCopyRegion {};
    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.srcSubresource.layerCount = 1;
    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.dstSubresource.layerCount = 1;
    imageCopyRegion.extent.width = m_Extent.width;
    imageCopyRegion.extent.height = m_Extent.height;
    imageCopyRegion.extent.depth = 1;

    vkCmdCopyImage(
        copyCmd,
        colorAttachment.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageCopyRegion);

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    pv::insertImageMemoryBarrier(
        copyCmd,
        dstImage,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

    vkEndCommandBuffer(copyCmd);

    pv::submitWork(copyCmd, queueG, context.m_device);

    // Get layout of the image (including row pitch)
    VkImageSubresource subResource {};
    subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkSubresourceLayout subResourceLayout;

    vkGetImageSubresourceLayout(context.m_device, dstImage, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    vkMapMemory(context.m_device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&imagedata);
    imagedata += subResourceLayout.offset;
    /*
                                   Save host visible framebuffer image to disk (ppm format)
                           */

    // const char* filename = "headless.ppm";

    std::ofstream file(filename, std::ios::out | std::ios::binary);

    // ppm header
    file << "P6\n"
         << m_Extent.width << "\n"
         << m_Extent.height << "\n"
         << 255 << "\n";

    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    // Check if source is BGR and needs swizzle
    std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
    const bool colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), VK_FORMAT_R8G8B8A8_UNORM) != formatsBGR.end());

    // ppm binary pixel data
    for (int32_t y = 0; y < m_Extent.height; y++) {
        unsigned int* row = (unsigned int*)imagedata;
        for (int32_t x = 0; x < m_Extent.width; x++) {
            if (colorSwizzle) {
                file.write((char*)row + 2, 1);
                file.write((char*)row + 1, 1);
                file.write((char*)row, 1);
            } else {
                file.write((char*)row, 3);
            }
            row++;
        }
        imagedata += subResourceLayout.rowPitch;
    }
    file.close();

    // Clean up resources
    vkUnmapMemory(context.m_device, dstImageMemory);
    vkFreeMemory(context.m_device, dstImageMemory, nullptr);
    vkDestroyImage(context.m_device, dstImage, nullptr);
}

}