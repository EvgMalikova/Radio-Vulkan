#include "helpers.hpp"
#include "postprocessing.hpp"


#include <algorithm>


template <typename T>
class PerlinNoise
{
private:
	uint32_t permutations[512];
	T fade(T t)
	{
		return t * t * t * (t * (t * (T)6 - (T)15) + (T)10);
	}
	T lerp(T t, T a, T b)
	{
		return a + t * (b - a);
	}
	T grad(int hash, T x, T y, T z)
	{
		// Convert LO 4 bits of hash code into 12 gradient directions
		int h = hash & 15;
		T u = h < 8 ? x : y;
		T v = h < 4 ? y : h == 12 || h == 14 ? x : z;
		return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
	}
public:
	PerlinNoise()
	{
		// Generate random lookup for permutations containing all numbers from 0..255
		std::vector<uint8_t> plookup;
		plookup.resize(256);
		std::iota(plookup.begin(), plookup.end(), 0);
		std::default_random_engine rndEngine(std::random_device{}());
		std::shuffle(plookup.begin(), plookup.end(), rndEngine);

		for (uint32_t i = 0; i < 256; i++)
		{
			permutations[i] = permutations[256 + i] = plookup[i];
		}
	}
	T noise(T x, T y, T z)
	{
		// Find unit cube that contains point
		int32_t X = (int32_t)floor(x) & 255;
		int32_t Y = (int32_t)floor(y) & 255;
		int32_t Z = (int32_t)floor(z) & 255;
		// Find relative x,y,z of point in cube
		x -= floor(x);
		y -= floor(y);
		z -= floor(z);

		// Compute fade curves for each of x,y,z
		T u = fade(x);
		T v = fade(y);
		T w = fade(z);

		// Hash coordinates of the 8 cube corners
		uint32_t A = permutations[X] + Y;
		uint32_t AA = permutations[A] + Z;
		uint32_t AB = permutations[A + 1] + Z;
		uint32_t B = permutations[X + 1] + Y;
		uint32_t BA = permutations[B] + Z;
		uint32_t BB = permutations[B + 1] + Z;

		// And add blended results for 8 corners of the cube;
		T res = lerp(w, lerp(v,
			lerp(u, grad(permutations[AA], x, y, z), grad(permutations[BA], x - 1, y, z)), lerp(u, grad(permutations[AB], x, y - 1, z), grad(permutations[BB], x - 1, y - 1, z))),
			lerp(v, lerp(u, grad(permutations[AA + 1], x, y, z - 1), grad(permutations[BA + 1], x - 1, y, z - 1)), lerp(u, grad(permutations[AB + 1], x, y - 1, z - 1), grad(permutations[BB + 1], x - 1, y - 1, z - 1))));
		return res;
	}
};


void MPICollect::prepareNoiseTexture(pv2::Context context, uint32_t width, uint32_t height, uint32_t depth2,uint8_t** pixel2)
	{
		// A 3D texture is described as width x height x depth
		texture.width     = width;
		texture.height    = height;
		texture.depth     = depth2;
		texture.mipLevels = 1;
		

		CheckImageSupport(context,texture.format,texture.width,texture.height,depth2);

		// Create optimal tiled target image
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType         = VK_IMAGE_TYPE_3D;
		imageCreateInfo.format            = texture.format;
		imageCreateInfo.mipLevels         = texture.mipLevels;
		imageCreateInfo.arrayLayers       = 1;
		imageCreateInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.extent.width      = texture.width;
		imageCreateInfo.extent.height     = texture.height;
		imageCreateInfo.extent.depth      = texture.depth;
		// Set initial layout of the image to undefined
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageCreateInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		CHECK_VK(vkCreateImage(context.m_device, &imageCreateInfo, nullptr, &texture.image));
		
		

		// Device local memory to back up image
		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		VkMemoryRequirements memReqs      = {};
		vkGetImageMemoryRequirements(context.m_device, texture.image, &memReqs);
		memAllocInfo.allocationSize  = memReqs.size;
		memAllocInfo.memoryTypeIndex = pv::getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context.m_physicalDevice);
		CHECK_VK(vkAllocateMemory(context.m_device, &memAllocInfo, nullptr, &texture.deviceMemory));
		CHECK_VK(vkBindImageMemory(context.m_device, texture.image, texture.deviceMemory, 0));

		// Create sampler
		VkSamplerCreateInfo sampler{};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.magFilter           = VK_FILTER_LINEAR;
		sampler.minFilter           = VK_FILTER_LINEAR;
		sampler.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeV        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeW        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.mipLodBias          = 0.0f;
		sampler.compareOp           = VK_COMPARE_OP_NEVER;
		sampler.minLod              = 0.0f;
		sampler.maxLod              = 0.0f;
		sampler.maxAnisotropy       = 1.0;
		sampler.anisotropyEnable    = VK_FALSE;
		sampler.borderColor         = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		CHECK_VK(vkCreateSampler(context.m_device, &sampler, nullptr, &texture.sampler));

		// Create image view
		VkImageViewCreateInfo view{};
		view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view.image                           = texture.image;
		view.viewType                        = VK_IMAGE_VIEW_TYPE_3D;
		view.format                          = texture.format;
		view.components                      = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
		view.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		view.subresourceRange.baseMipLevel   = 0;
		view.subresourceRange.baseArrayLayer = 0;
		view.subresourceRange.layerCount     = 1;
		view.subresourceRange.levelCount     = 1;
		CHECK_VK(vkCreateImageView(context.m_device, &view, nullptr, &texture.view));

		

       
		updateNoiseTexture(context,pixel2);
       // else
       //   GenerateTexture(data, pixel2,texWidth, texHeight, pixel2.size(), numLayers);
	}
	
	void MPICollect::GeneratePerlin (uint8_t *data)
	{// Generate perlin based noise
			std::cout << "Generating " << texture.width << " x " << texture.height << " x " << texture.depth << " noise texture..." << std::endl;
	
			//auto tStart = std::chrono::high_resolution_clock::now();
	
			PerlinNoise<float>  perlinNoise;
			
	
			const float noiseScale = static_cast<float>(rand() % 10) + 4.0f;
	
			for (int32_t z = 0; z < texture.depth; z++)
			{
				for (int32_t y = 0; y < texture.height; y++)
				{
					for (int32_t x = 0; x < texture.width; x++)
					{
						float nx = (float) x / (float) texture.width;
						float ny = (float) y / (float) texture.height;
						float nz = (float) z / (float) texture.depth;
	
						float n = 20.0 * perlinNoise.noise(nx, ny, nz);
	
						n = n - floor(n);
	
						for (int32_t l = 0; l < texture.numChannels ; l++)
										  {
						                   
						                     int numChanel=texture.numChannels;
						
						                 	data[l+x*numChanel + y * texture.width*numChanel + z * texture.width * texture.height*numChanel] =static_cast<uint8_t>(floor(n * 255));
						                 	//static_cast<uint8_t>(img[l+x*numChanel + y * width*numChanel ]);////floor(n * 255));
						                      
						                 }
						//data[x + y * texture.width + z * texture.width * texture.height] = static_cast<uint8_t>(floor(n * 255));
					}
				}
			}
	}

	// Generate randomized noise and upload it to the 3D texture using staging
	void MPICollect::updateNoiseTexture(pv2::Context context,uint8_t** pixel2)
	{
		const uint32_t texMemSize = texture.width * texture.height * texture.depth*texture.numChannels;

		uint8_t *data = new uint8_t[texMemSize];
		memset(data, 0, texMemSize);
		
		if (pixel2==nullptr) m_synthesize=true;
		else m_synthesize=false;
		
		if(m_synthesize)
        GeneratePerlin(data) ;
		else
		  GenerateTexture(data, pixel2);

		
		// Create a host-visible staging buffer that contains the raw image data
		VkBuffer       stagingBuffer;
		VkDeviceMemory stagingMemory;
		
		
		
		pv::createBuffer(context.m_device,context.m_physicalDevice,VkDeviceSize(texMemSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingMemory);
		




	
		// Copy texture data into staging buffer
		uint8_t *mapped;
		CHECK_VK(vkMapMemory(context.m_device, stagingMemory, 0, VkDeviceSize(texMemSize), 0,(void**)&mapped));
		memcpy(mapped, data, texMemSize);
		vkUnmapMemory(context.m_device, stagingMemory);//memReqs.size

		VkCommandBuffer copyCmd = context.CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_commandPool,true);

		// The sub resource range describes the regions of the image we will be transitioned
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask              = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel            = 0;
		subresourceRange.levelCount              = 1;
		subresourceRange.layerCount              = 1;

		// Optimal image will be used as destination for the copy, so we must transfer from our
		// initial undefined image layout to the transfer destination layout
		pv::setImageLayout(
		    copyCmd,
		    texture.image,
		    VK_IMAGE_LAYOUT_UNDEFINED,
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		    subresourceRange);

		// Copy 3D noise data to texture

		// Setup buffer copy regions
		VkBufferImageCopy bufferCopyRegion{};
		bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel       = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount     = 1;
		bufferCopyRegion.imageExtent.width               = texture.width;
		bufferCopyRegion.imageExtent.height              = texture.height;
		bufferCopyRegion.imageExtent.depth               = texture.depth;

		vkCmdCopyBufferToImage(
		    copyCmd,
		    stagingBuffer,
		    texture.image,
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		    1,
		    &bufferCopyRegion);

		// Change texture image layout to shader read after all mip levels have been copied
		texture.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		pv::setImageLayout(
		    copyCmd,
		    texture.image,
		    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		    texture.imageLayout,
		    subresourceRange);

		context.flushCommandBuffer(copyCmd, context.m_queueGCT, m_commandPool, true);

		// Clean up staging resources
		delete[] data;
		vkFreeMemory(context.m_device, stagingMemory, nullptr);
		vkDestroyBuffer(context.m_device, stagingBuffer, nullptr);
	}

/*std::vector<TexVertex> vertices = {
    {{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

std::vector<uint16_t> indices = {
     2, 3, 0, 0, 1, 2
};*/

void MPICollect::GenerateSlices(int world_size)
{
  for (int i=0;i<world_size;i++)
  {
   /* float l;
    if (i==0) l=1.0;
    else l=0.0;*/
    
   /* 
    vertices.push_back({{-1.0f, -1.0f, float(i-1)}, {i, l, 0.0f}, {1.0f, 0.0f, float(i)/(world_size)}});
    vertices.push_back({{1.0f, -1.0f, float(i-1)}, {i, l, 0.0f}, {0.0f, 0.0f, float(i)/(world_size)}});
    vertices.push_back({{1.0f, 1.0f, float(i-1)}, {i, l, 0.0f}, {0.0f, 1.0f, float(i)/(world_size)}});
    vertices.push_back({{-1.0f, 1.0f, float(i-1)}, {i, l,0.0f}, {1.0f, 1.0f, float(i)/(world_size)}});
    vertices.push_back({{-1.0f, -1.0f, float(i-1)},{i, l,0.0f}, {1.0f, 1.0f, float(i)/(world_size)}});
    vertices.push_back({{ 1.0f,  1.0f, float(i-1)},{i, l,0.0f}, {1.0f, 1.0f, float(i)/(world_size)}});
    
    indices.push_back(2+i*6);
    indices.push_back(3+i*6);
    indices.push_back(4+i*6);
    indices.push_back(0+i*6);
    indices.push_back(1+i*6);
    indices.push_back(2+i*6);
    
    indices.push_back(3+i*6);
    indices.push_back(4+i*6);
    indices.push_back(5+i*6);
    
    indices.push_back(4+i*6);
    indices.push_back(5+i*6);
    indices.push_back(0+i*6);*/
    
    vertices.push_back({{-1.0f, -1.0f, float(i)}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, float(i)/(world_size-1)}});
     vertices.push_back({{1.0f, -1.0f, float(i)}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, float(i)/(world_size-1)}});
     vertices.push_back({{1.0f, 1.0f, float(i)}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, float(i)/(world_size-1)}});
     vertices.push_back({{-1.0f, 1.0f, float(i)}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, float(i)/(world_size-1)}});
     
     indices.push_back(2+i*4);
     indices.push_back(3+i*4);
     indices.push_back(0+i*4);
     indices.push_back(0+i*4);
     indices.push_back(1+i*4);
     indices.push_back(2+i*4);
DEBUG_LOG<<vertices[vertices.size()-1].pos.z<<", and cord "<<vertices[vertices.size()-1].texCoord.z<<std::endl;

  }
  
}

/*Helpers*/

 void MPICollect::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,pv2::Context context) {
     VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);
 

    
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
 
     if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
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
         imageInfo.imageView = texture.view;
         imageInfo.sampler = texture.sampler;
         
         
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

void MPICollect::GenerateTexture(uint8_t* data, uint8_t** images)
{
  
  
  		// Generate perlin based noise
  		std::cout << "Forming " << texture.width << " x " << texture.height << " x " << texture.depth << " noise texture with " <<texture.numChannels<<" channels" << std::endl;
  
  
  

  		for (int32_t z = 0; z < texture.depth; z++)
  		{
  		  uint8_t* img=images[z];
  			for (int32_t y = 0; y < texture.height; y++)
  			{
  				for (int32_t x = 0; x < texture.width; x++)
  				{
  				 for (int32_t l = 0; l < texture.numChannels; l++)
  				  {
                     
                     

                   	data[l+x*texture.numChannels + y * texture.width*texture.numChannels + z * texture.width * texture.height*texture.numChannels] = static_cast<uint8_t>(img[l+x*texture.numChannels + y * texture.width*texture.numChannels ]);////floor(n * 255));
                        
                   }
               }
  			}
  		}
  		
  		
}

void MPICollect::CheckImageSupport(pv2::Context context, VkFormat format, int width, int height, int depth)
{
  VkFormatProperties formatProperties;
  		vkGetPhysicalDeviceFormatProperties(context.m_physicalDevice, format, &formatProperties);
  		// Check if format supports transfer
  		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT))
  		{
  			std::cout << "Error: Device does not support flag TRANSFER_DST for selected texture format!" << std::endl;
  			return;
  		}
  		VkPhysicalDeviceProperties deviceProperties;
  		vkGetPhysicalDeviceProperties(context.m_physicalDevice, &deviceProperties);
  		// Check if GPU supports requested 3D texture dimensions
  		uint32_t maxImageDimension3D(deviceProperties.limits.maxImageDimension3D);
  		if (width > maxImageDimension3D || height > maxImageDimension3D || depth > maxImageDimension3D)
  		{
  			std::cout << "Error: Requested texture dimensions is greater than supported 3D texture dimension!" << std::endl;
  			return;
  		}
}
/*void MPICollect::CreateTextureImage(pv2::Context context, std::vector< uint8_t*> pixels, int texWidth, int texHeight, int numLayer) {
  
    
    CheckImageSupport(context,VK_FORMAT_R8_UNORM,texWidth , texHeight,3);
    //stbi_uc* pixels=(stbi_uc*)pixel;
    //int numLayers=4;
    //VkDeviceSize imageSize = texWidth * texHeight* texChannels*numLayers;
    int numLayers=1;
    int texMemSize= texWidth * texHeight*numLayers* 3;//pixels.size();
    uint8_t *data = new uint8_t[texMemSize];
    		memset(data, 0, texMemSize);
//SaveImage("test1.ppm",(char*)pixels[0],texWidth * texHeight*numLayers,texWidth,texHeight);
//SaveImage("test2.ppm",(char*)pixels[1],texWidth * texHeight*numLayers,texWidth,texHeight);
  
            // int x, y, comp;
     
    stbi_uc* pixels1 = stbi_load("0_headless.ppm", &texWidth, &texHeight, &numLayers, STBI_rgb_alpha);
    stbi_uc* pixels2 = stbi_load("1_headless.ppm", &texWidth, &texHeight, &numLayers, STBI_rgb_alpha);
    stbi_uc* pixels3 = stbi_load("1_headless.ppm", &texWidth, &texHeight, &numLayers, STBI_rgb_alpha);
         
   std::vector< uint8_t*> pixel2;
   pixel2.push_back(pixels1);
   pixel2.push_back(pixels2);
   pixel2.push_back(pixels3);
   pixel2.resize(3);
   GenerateTexture(data, pixel2,texWidth, texHeight, pixel2.size(), numLayers);
   

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    pv::createBuffer(context.m_device,context.m_physicalDevice,VkDeviceSize(texMemSize), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    char* data2;
    CHECK_VK(vkMapMemory(context.m_device, stagingBufferMemory, 0, VkDeviceSize(texMemSize), 0, (void**)&data2));
        memcpy(data2, data, static_cast<size_t>(texMemSize));
       //SaveImage("text.ppm",data,imageSize,texWidth,texHeight);
    vkUnmapMemory(context.m_device, stagingBufferMemory);


    pv::createImage3D(context.m_device, context.m_physicalDevice,texWidth, texHeight, pixel2.size(), VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_textureImage, m_textureImageMemory);

    transitionImageLayout(m_textureImage, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,context);
    copyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),context);
    transitionImageLayout(m_textureImage, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,context);

    vkQueueWaitIdle(context.m_queueGCT);
    vkDestroyBuffer(context.m_device, stagingBuffer, nullptr);
    vkFreeMemory(context.m_device, stagingBufferMemory, nullptr);
}

*/




//Blending
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
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;



    VkPipelineRasterizationStateCreateInfo rasterizationState {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = VK_CULL_MODE_NONE;
    rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;//VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
    blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |            // VkColorComponentFlags    colorWriteMask
    VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT |
    VK_COLOR_COMPONENT_A_BIT;
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
   
  //Working non blend
  /*
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

*/
