#include "pipeline.hpp"
#include <nvvk/error_vk.hpp>
#include <nvvk/images_vk.hpp>
#include <nvvk/resourceallocator_vk.hpp> // For NVVK memory allocators
#include <nvvk/shaders_vk.hpp>
#include <nvvk/structs_vk.hpp>



#include "helpers.hpp"

#include "loaders.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>


void PipelineRayTrace::CreateAccelerationStructureBuffer(pv2::Context context,AccelerationStructure &accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
	{
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		
        vkCreateBuffer(context.m_device, &bufferCreateInfo, nullptr, &accelerationStructure.buffer);
		VkMemoryRequirements memoryRequirements{};
		vkGetBufferMemoryRequirements(context.m_device, accelerationStructure.buffer, &memoryRequirements);
		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = pv::getMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,context.m_physicalDevice);
		vkAllocateMemory(context.m_device, &memoryAllocateInfo, nullptr, &accelerationStructure.memory);
		vkBindBufferMemory(context.m_device, accelerationStructure.buffer, accelerationStructure.memory, 0);
	}
#ifdef USE_NVVK	
	nvvk::RaytracingBuilderKHR::BlasInput PipelineRayTrace::sphereToVkGeometryKHR(pv2::Context context, VkBuffer vertexBuffer,int num_vertices)
	{
	  VkDeviceAddress dataAddress = pv::GetBufferDeviceAddress(context.m_device, vertexBuffer);
	
	  VkAccelerationStructureGeometryAabbsDataKHR aabbs{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR};
	  aabbs.data.deviceAddress = dataAddress;
	  aabbs.stride             = sizeof(Aabb);
	
	  // Setting up the build info of the acceleration (C version, c++ gives wrong type)
	  VkAccelerationStructureGeometryKHR asGeom{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
	  asGeom.geometryType   = VK_GEOMETRY_TYPE_AABBS_KHR;
	  asGeom.flags          = VK_GEOMETRY_OPAQUE_BIT_KHR;
	  asGeom.geometry.aabbs = aabbs;
	
	  VkAccelerationStructureBuildRangeInfoKHR offset{};
	  offset.firstVertex     = 0;
	  offset.primitiveCount  = num_vertices;  // Nb aabb
	  offset.primitiveOffset = 0;
	  offset.transformOffset = 0;
	
	  nvvk::RaytracingBuilderKHR::BlasInput input;
	  input.asGeometry.emplace_back(asGeom);
	  input.asBuildOffsetInfo.emplace_back(offset);
	  return input;
	}
#endif
	
void PipelineRayTrace::CreateBottomLevelAccelerationStructure(pv2::Context context, VkBuffer aabbsBuffer,int num_vertices) {
		
		
				 
		
		PFN_vkGetAccelerationStructureBuildSizesKHR pvkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(context.m_device, "vkGetAccelerationStructureBuildSizesKHR");
		 PFN_vkCreateAccelerationStructureKHR pvkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(context.m_device, "vkCreateAccelerationStructureKHR");
		 PFN_vkGetBufferDeviceAddressKHR pvkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)vkGetDeviceProcAddr(context.m_device, "vkGetBufferDeviceAddressKHR");
		 PFN_vkCmdBuildAccelerationStructuresKHR pvkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(context.m_device, "vkCmdBuildAccelerationStructuresKHR");
		
	
			   
	VkDeviceAddress dataAddress = pv::GetBufferDeviceAddress(context.m_device, aabbsBuffer);
		
	//  VkDeviceOrHostAddressConstKHR vertexDeviceOrHostAddressConst = {};
	//  vertexDeviceOrHostAddressConst.deviceAddress = dataAddress;
	
	
	 VkAccelerationStructureGeometryAabbsDataKHR aabbs_d{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR};
		  aabbs_d.data.deviceAddress = dataAddress;
		  aabbs_d.stride             = sizeof(Aabb);
		
		  // Setting up the build info of the acceleration (C version, c++ gives wrong type)
		  VkAccelerationStructureGeometryKHR accelerationStructureGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
		  accelerationStructureGeometry.geometryType   = VK_GEOMETRY_TYPE_AABBS_KHR;
		  accelerationStructureGeometry.flags          = VK_GEOMETRY_OPAQUE_BIT_KHR;
		  accelerationStructureGeometry.geometry.aabbs = aabbs_d;
		  
		  
		  /*
		
		  VkAccelerationStructureBuildRangeInfoKHR offset{};
		  offset.firstVertex     = 0;
		  offset.primitiveCount  = num_vertices;  // Nb aabb
		  offset.primitiveOffset = 0;
		  offset.transformOffset = 0;
	
	 std::cout<<num_vertices<<std::endl;
	
	  VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo = {
	    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
	    .pNext = NULL,
	    .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
	    .flags = 0,
	    .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
	    .srcAccelerationStructure = VK_NULL_HANDLE,
	    .dstAccelerationStructure = VK_NULL_HANDLE,
	    .geometryCount = 1,
	    .pGeometries = &asGeom,
	    .ppGeometries = NULL,
	    .scratchData = {}
	  };
	
	  */
	  
	  // Get size info
	  		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
	  					accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	  					
	  		accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	  		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	  		accelerationStructureBuildGeometryInfo.geometryCount = 1;
	  		accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
	  		
	  VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo = {
	  	    .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
	  	    .pNext = NULL,
	  	    .accelerationStructureSize = 0,
	  	    .updateScratchSize = 0,
	  	    .buildScratchSize = 0
	  	  };
	  	  
	  
	  uint32_t maxAabbs = num_vertices;
	 pvkGetAccelerationStructureBuildSizesKHR(
	  			context.m_device,
	  			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
	  			&accelerationStructureBuildGeometryInfo,
	  			&maxAabbs, //TODO: maxAabbs
	  			&accelerationStructureBuildSizesInfo);
	  
	  
	  
	  pv::createBuffer(context.m_device,context.m_physicalDevice, accelerationStructureBuildSizesInfo.accelerationStructureSize,  VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bottomLevelAS.buffer,bottomLevelAS.memory);
	 VkBuffer scratchBuffer;
	 VkDeviceMemory scratchBufferMemory;
	 pv::createBuffer(context.m_device,context.m_physicalDevice,
	              accelerationStructureBuildSizesInfo.buildScratchSize,
	              VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 
	              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                  scratchBuffer,
	              scratchBufferMemory);
	 
	 VkDeviceAddress scratchBufferAddress = pv::GetBufferDeviceAddress(context.m_device, scratchBuffer);
	 accelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBufferAddress;
	 
	 
	 VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo = {
	   .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
	   .pNext = NULL,
	   .createFlags = 0,
	   .buffer = bottomLevelAS.buffer,
	   .offset = 0,
	   .size = accelerationStructureBuildSizesInfo.accelerationStructureSize,
	   .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
	   .deviceAddress = 0
	 };
	 
	 pvkCreateAccelerationStructureKHR(context.m_device, &accelerationStructureCreateInfo, NULL, &bottomLevelAS.handle);
	 
	 accelerationStructureBuildGeometryInfo.dstAccelerationStructure = this->bottomLevelAS.handle;
	 
	  VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
	  		accelerationStructureBuildRangeInfo.primitiveCount = maxAabbs; //Todo: maxAabbs
	  		accelerationStructureBuildRangeInfo.primitiveOffset = 0;
	  		accelerationStructureBuildRangeInfo.firstVertex = 0;
	  		accelerationStructureBuildRangeInfo.transformOffset = 0;
	  		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };
	  
	  	
	  VkCommandBufferAllocateInfo bufferAllocateInfo = {};
	  bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	  bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	  bufferAllocateInfo.commandPool = m_commandPool;
	  bufferAllocateInfo.commandBufferCount = 1;
	 
	  VkCommandBuffer commandBuffer;
	  vkAllocateCommandBuffers(context.m_device, &bufferAllocateInfo, &commandBuffer);
	  
	  VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	  commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	  commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	  
	  vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	  pvkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &accelerationStructureBuildGeometryInfo,accelerationBuildStructureRangeInfos.data());
	  vkEndCommandBuffer(commandBuffer);
	 
	  VkSubmitInfo submitInfo = {};
	  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	  submitInfo.commandBufferCount = 1;
	  submitInfo.pCommandBuffers = &commandBuffer;
	 
	  vkQueueSubmit(context.m_queueGCT, 1, &submitInfo, VK_NULL_HANDLE);
	  vkQueueWaitIdle(context.m_queueGCT);
	 
	  vkFreeCommandBuffers(context.m_device, m_commandPool, 1, &commandBuffer);
	 
	  vkDestroyBuffer(context.m_device, scratchBuffer, NULL);
	  vkFreeMemory(context.m_device, scratchBufferMemory, NULL);			
	 
	 /**/
	
	}
	
		/*
			The top level acceleration structure contains the scene's object instances
		*/
		
void PipelineRayTrace::CreateAccelerationStructure(pv2::Context context,AccelerationStructure& accelerationStructure, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo)
		{
			// Buffer and memory
			VkBufferCreateInfo bufferCreateInfo{};
			bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
			bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
			vkCreateBuffer(context.m_device, &bufferCreateInfo, nullptr, &accelerationStructure.buffer);
			
			
			VkMemoryRequirements memoryRequirements{};
			vkGetBufferMemoryRequirements(context.m_device, accelerationStructure.buffer, &memoryRequirements);
			
			VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
			memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
			memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			
			VkMemoryAllocateInfo memoryAllocateInfo{};
			memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
			memoryAllocateInfo.allocationSize = memoryRequirements.size;
			
			memoryAllocateInfo.memoryTypeIndex = pv::getMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context.m_physicalDevice);
			
			CHECK_VK(vkAllocateMemory(context.m_device, &memoryAllocateInfo, nullptr, &accelerationStructure.memory));
			
			CHECK_VK(vkBindBufferMemory(context.m_device, accelerationStructure.buffer, accelerationStructure.memory, 0));
			std::cout<<"Error on bind"<<std::endl;
			// Acceleration structure
			VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info{};
			accelerationStructureCreate_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
			accelerationStructureCreate_info.buffer = accelerationStructure.buffer;
			accelerationStructureCreate_info.size = buildSizeInfo.accelerationStructureSize;
			accelerationStructureCreate_info.type = type;
			vkCreateAccelerationStructureKHR(context.m_device, &accelerationStructureCreate_info, nullptr, &accelerationStructure.handle);
			// AS device address
			std::cout<<"C "<<std::endl;
			VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
			accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
			accelerationDeviceAddressInfo.accelerationStructure = accelerationStructure.handle;
			accelerationStructure.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(context.m_device, &accelerationDeviceAddressInfo);
		}
		
		void PipelineRayTrace::DeleteAccelerationStructure(pv2::Context context,AccelerationStructure& accelerationStructure)
		{
			vkFreeMemory(context.m_device, accelerationStructure.memory, nullptr);
			vkDestroyBuffer(context.m_device, accelerationStructure.buffer, nullptr);
			vkDestroyAccelerationStructureKHR(context.m_device, accelerationStructure.handle, nullptr);
		}

void PipelineRayTrace::CreateTopLevelAccelerationStructure(pv2::Context context)
		{
		    
		    PFN_vkGetAccelerationStructureBuildSizesKHR pvkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(context.m_device, "vkGetAccelerationStructureBuildSizesKHR");
		    PFN_vkCreateAccelerationStructureKHR pvkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(context.m_device, "vkCreateAccelerationStructureKHR");
		    PFN_vkGetBufferDeviceAddressKHR pvkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)vkGetDeviceProcAddr(context.m_device, "vkGetBufferDeviceAddressKHR");
		    PFN_vkCmdBuildAccelerationStructuresKHR pvkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(context.m_device, "vkCmdBuildAccelerationStructuresKHR");
		    PFN_vkGetAccelerationStructureDeviceAddressKHR pvkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(context.m_device, "vkGetAccelerationStructureDeviceAddressKHR");
		    
			VkTransformMatrixKHR transformMatrix = {
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f };
	
			VkAccelerationStructureInstanceKHR instance{};
			instance.transform = transformMatrix;
			instance.instanceCustomIndex = 0;
			instance.mask = 0xFF;
			instance.instanceShaderBindingTableRecordOffset = 0;
			instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			instance.accelerationStructureReference = bottomLevelAS.deviceAddress;
	
			// Buffer for instance data
			VkBuffer stagingBuffer;
			VkDeviceMemory staging_bufferMemory;
			VkDeviceSize instanceBufferSize = sizeof(VkAccelerationStructureInstanceKHR);
			pv::createBuffer(context.m_device,context.m_physicalDevice, instanceBufferSize,
						                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						                 stagingBuffer,staging_bufferMemory);
			
			void* geometryInstanceData;
			vkMapMemory(context.m_device, staging_bufferMemory, 0, instanceBufferSize, 0, &geometryInstanceData);
			memcpy(geometryInstanceData, &instance, instanceBufferSize);
			vkUnmapMemory(context.m_device, staging_bufferMemory);
			
			VkBuffer geometryInstanceBuffer;
			VkDeviceMemory geometryInstanceBufferMemory;
			//pv::createBuffer(app, geometryInstanceBufferSize,
			//VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &geometryInstanceBuffer, &geometryInstanceBufferMemory);  
			pv::createBuffer(context.m_device,context.m_physicalDevice, sizeof(VkAccelerationStructureInstanceKHR),
						                 VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
						                 				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
						                 geometryInstanceBuffer, geometryInstanceBufferMemory);
			
	       
	       pv::copyBuffer(stagingBuffer, geometryInstanceBuffer, instanceBufferSize, m_commandPool, context.m_device, context.m_queueGCT);
	       
	          vkDestroyBuffer(context.m_device, stagingBuffer, nullptr);
	          vkFreeMemory(context.m_device, staging_bufferMemory, nullptr);
	       
	       //TODO: mapping to instance
	       
	        		
          
	
			VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
			instanceDataDeviceAddress.deviceAddress = pv::GetBufferDeviceAddress(context.m_device,geometryInstanceBuffer);
	
			VkAccelerationStructureGeometryInstancesDataKHR accelerationStructureGeometryInstancesData = {
			   .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
			   .pNext = NULL,
			   .arrayOfPointers = VK_FALSE,
			   .data = instanceDataDeviceAddress
			 };
			
			 VkAccelerationStructureGeometryDataKHR accelerationStructureGeometryData = {
			   .instances = accelerationStructureGeometryInstancesData
			 };
VkAccelerationStructureGeometryKHR accelerationStructureGeometry = {
  .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
  .pNext = NULL,
  .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
  .geometry = accelerationStructureGeometryData,
  .flags = VK_GEOMETRY_OPAQUE_BIT_KHR
};

VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo = {
  .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
  .pNext = NULL,
  .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
  .flags = VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR,
  .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
  .srcAccelerationStructure = VK_NULL_HANDLE,
  .dstAccelerationStructure = VK_NULL_HANDLE,
  .geometryCount = 1,
  .pGeometries = &accelerationStructureGeometry,
  .ppGeometries = NULL,
  .scratchData = {}
};

VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo = {
  .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
  .pNext = NULL,
  .accelerationStructureSize = 0,
  .updateScratchSize = 0,
  .buildScratchSize = 0
};

	
			pvkGetAccelerationStructureBuildSizesKHR(context.m_device, 
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR, 
			&accelerationStructureBuildGeometryInfo, 
			&accelerationStructureBuildGeometryInfo.geometryCount, 
			&accelerationStructureBuildSizesInfo);
			
	std::cout<<"OK as far"<<std::endl;
	//CreateAccelerationStructureBuffer(context, topLevelAS, accelerationStructureBuildSizesInfo);
	
			pv::createBuffer(context.m_device,context.m_physicalDevice, accelerationStructureBuildSizesInfo.accelerationStructureSize,  VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->topLevelAS.buffer,this->topLevelAS.memory);
		std::cout<<"buffer created"<<std::endl;	
			VkBuffer scratchBuffer;
			VkDeviceMemory scratchBufferMemory;
			pv::createBuffer(context.m_device,context.m_physicalDevice, 
			             accelerationStructureBuildSizesInfo.buildScratchSize, 
			             VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 
			             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			             scratchBuffer, 
			             scratchBufferMemory);
			
			
			VkBufferDeviceAddressInfo scratchBufferDeviceAddressInfo = {};
			scratchBufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			scratchBufferDeviceAddressInfo.buffer = scratchBuffer;
			
			VkDeviceAddress scratchBufferAddress = pvkGetBufferDeviceAddressKHR(context.m_device, &scratchBufferDeviceAddressInfo);
			
			VkDeviceOrHostAddressKHR scratchDeviceOrHostAddress = {};
			scratchDeviceOrHostAddress.deviceAddress = scratchBufferAddress;
			
			accelerationStructureBuildGeometryInfo.scratchData = scratchDeviceOrHostAddress;
			
			
			VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo = {
			  .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
			  .pNext = NULL,
			  .createFlags = 0,
			  .buffer = topLevelAS.buffer,
			  .offset = 0,
			  .size = accelerationStructureBuildSizesInfo.accelerationStructureSize,
			  .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
			  .deviceAddress = 0
			};
			
			pvkCreateAccelerationStructureKHR(context.m_device, &accelerationStructureCreateInfo,  NULL, &topLevelAS.handle);
			
			accelerationStructureBuildGeometryInfo.dstAccelerationStructure = topLevelAS.handle;
			
			
			VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
			accelerationStructureBuildRangeInfo.primitiveCount = 1;
			accelerationStructureBuildRangeInfo.primitiveOffset = 0;
			accelerationStructureBuildRangeInfo.firstVertex = 0;
			accelerationStructureBuildRangeInfo.transformOffset = 0;
			std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };
	        
			// Build the acceleration structure on the device via a one-time command buffer submission
			// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
			
			VkCommandBufferAllocateInfo bufferAllocateInfo = {};
			 bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			 bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			 bufferAllocateInfo.commandPool = m_commandPool;
			 bufferAllocateInfo.commandBufferCount = 1;
			
			 VkCommandBuffer commandBuffer;
			 CHECK_VK(vkAllocateCommandBuffers(context.m_device, &bufferAllocateInfo, &commandBuffer));
			 
			 VkCommandBufferBeginInfo commandBufferBeginInfo = {};
			 commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			 commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			 
			 CHECK_VK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
			 
			 pvkCmdBuildAccelerationStructuresKHR(
			 				commandBuffer,
			 				1,
			 				&accelerationStructureBuildGeometryInfo,
			 				accelerationBuildStructureRangeInfos.data());
			 CHECK_VK(vkEndCommandBuffer(commandBuffer));
			
			 VkSubmitInfo submitInfo = {};
			 submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			 submitInfo.commandBufferCount = 1;
			 submitInfo.pCommandBuffers = &commandBuffer;
			
			 vkQueueSubmit(context.m_queueGCT, 1, &submitInfo, VK_NULL_HANDLE);
			 vkQueueWaitIdle(context.m_queueGCT);
			 
			
			 vkFreeCommandBuffers(context.m_device, m_commandPool, 1, &commandBuffer);
			
			 vkDestroyBuffer(context.m_device, scratchBuffer, NULL);
			 vkFreeMemory(context.m_device, scratchBufferMemory, NULL);
			 
		
		}
		
		PipelineRayTrace::RayTracingScratchBuffer PipelineRayTrace::CreateScratchBuffer(pv2::Context context, VkDeviceSize size)
			{
				RayTracingScratchBuffer scratchBuffer{};
		
				VkBufferCreateInfo bufferCreateInfo{};
				bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
				bufferCreateInfo.size = size;
				bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
				vkCreateBuffer(context.m_device, &bufferCreateInfo, nullptr, &scratchBuffer.handle);
		
				VkMemoryRequirements memoryRequirements{};
				vkGetBufferMemoryRequirements(context.m_device, scratchBuffer.handle, &memoryRequirements);
		
				VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
				memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
				memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		
				VkMemoryAllocateInfo memoryAllocateInfo = {};
				memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
				memoryAllocateInfo.allocationSize = memoryRequirements.size;
				memoryAllocateInfo.memoryTypeIndex = pv::getMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,context.m_physicalDevice );
				vkAllocateMemory(context.m_device, &memoryAllocateInfo, nullptr, &scratchBuffer.memory);
				vkBindBufferMemory(context.m_device, scratchBuffer.handle, scratchBuffer.memory, 0);
		
				VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
				bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
				bufferDeviceAddressInfo.buffer = scratchBuffer.handle;
				scratchBuffer.deviceAddress = vkGetBufferDeviceAddressKHR(context.m_device, &bufferDeviceAddressInfo);
		
				return scratchBuffer;
			}

void PipelineRasterize::CreateDescriptorSets(pv2::Context context, int size, SimpCamera cam)
{
    std::vector<VkDescriptorSetLayout> layouts(size, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(size);
    CHECK_VK(vkAllocateDescriptorSets(context.m_device, &allocInfo, m_descriptorSets.data()));

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

void PipelineRayTrace::CreateDescriptorPool(pv2::Context context, int size) {
/*

VkDescriptorPoolSize descriptorPoolSizes[4];
descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
descriptorPoolSizes[0].descriptorCount = 1;
descriptorPoolSizes[0].descriptorCount = static_cast<uint32_t>(size);

descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
descriptorPoolSizes[1].descriptorCount = 1;
descriptorPoolSizes[1].descriptorCount = static_cast<uint32_t>(size);
descriptorPoolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
descriptorPoolSizes[2].descriptorCount = 1;
descriptorPoolSizes[2].descriptorCount = static_cast<uint32_t>(size);

descriptorPoolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
descriptorPoolSizes[3].descriptorCount = 1; //1 for vertices
descriptorPoolSizes[3].descriptorCount = static_cast<uint32_t>(size);


VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
descriptorPoolCreateInfo.poolSizeCount = 4;
descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;
descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(size);;

if (vkCreateDescriptorPool(context.m_device, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
}
std::cout<<"Ray-tracing pool"<<std::endl;
*/

std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }
		};
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = pv::DescriptorPoolCreateInfo(poolSizes, static_cast<uint32_t>(size));
		CHECK_VK(vkCreateDescriptorPool(context.m_device, &descriptorPoolCreateInfo, nullptr, &m_descriptorPool));
	std::cout<<"Ray-tracing pool"<<std::endl;	
}


void PipelineRayTrace::CreateDescriptorSets(pv2::Context context, int size, SimpCamera cam)
{
std::vector<VkDescriptorSetLayout> layouts(size, m_descriptorSetLayout);
   VkDescriptorSetAllocateInfo allocInfo {};
   allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocInfo.descriptorPool = m_descriptorPool;
   allocInfo.descriptorSetCount = static_cast<uint32_t>(size);
   allocInfo.pSetLayouts = layouts.data();

   m_descriptorSets.resize(size);
   CHECK_VK(vkAllocateDescriptorSets(context.m_device, &allocInfo, m_descriptorSets.data()));
   

   for (size_t i = 0; i < size; i++) {
       
       VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
       					descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
       					descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
       		descriptorAccelerationStructureInfo.pAccelerationStructures = &topLevelAS.handle;
       
       		VkWriteDescriptorSet accelerationStructureWrite{};
       		accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
       		// The specialized acceleration structure descriptor has to be chained
       		accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
       		accelerationStructureWrite.dstSet = m_descriptorSets[i];
       		accelerationStructureWrite.dstBinding = 0;
       		accelerationStructureWrite.descriptorCount = 1;
       		accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
       
       		//TODO: Make creation with camera and pLoader call
       		//VkDescriptorImageInfo m_storageImageDescriptor[i];
       		//VkDescriptorBufferInfo m_vertexBufferDescriptor[0];
       		//VkDescriptorBufferInfo indexBufferDescriptor{ scene.indices.buffer, 0, VK_WHOLE_SIZE };
       		
       VkDescriptorBufferInfo buffInfo {};
       buffInfo.buffer = cam.m_uniformBuffers[i];
       buffInfo.offset = 0;
       buffInfo.range = sizeof(SimpCamera::UniformBufferObject);

      /* VkWriteDescriptorSet buffdescriptorWrite {};
       buffdescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
       buffdescriptorWrite.dstSet = m_descriptorSets[i];
       buffdescriptorWrite.dstBinding = 0;
       buffdescriptorWrite.dstArrayElement = 0;
       buffdescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
       buffdescriptorWrite.descriptorCount = 1;
       buffdescriptorWrite.pBufferInfo = &buffInfo;*/

std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
			// Binding 0: Top level acceleration structure
			accelerationStructureWrite,
			// Binding 1: Ray tracing result image
			pv::WriteDescriptorSet(m_descriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, &m_storageImageDescriptor[i]),
			// Binding 2: Uniform data
			//TODO:check buffer
			//buffdescriptorWrite,
			pv::WriteDescriptorSet(m_descriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, &buffInfo),
			// Binding 3: Scene vertex buffer
			pv::WriteDescriptorSet(m_descriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, &m_vertexBufferDescriptor[0]),
			// Binding 4: Scene index buffer
			//vks::initializers::writeDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4, &indexBufferDescriptor),
		};
		vkUpdateDescriptorSets(context.m_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, VK_NULL_HANDLE);

      
   }

   /* VkDescriptorSetAllocateInfo descriptorSetAllocateInfo {};
    			descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    			descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
    			descriptorSetAllocateInfo.pSetLayouts = &m_descriptorSetLayout;
    			descriptorSetAllocateInfo.descriptorSetCount = 1;
    			
  		CHECK_VK(vkAllocateDescriptorSets(context.m_device, &descriptorSetAllocateInfo, &m_descriptorSet));
  */
  		
  
  		
}

VkStridedDeviceAddressRegionKHR PipelineRayTrace::getSbtEntryStridedDeviceAddressRegion(pv2::Context context,VkBuffer buffer, uint32_t handleCount)
       {
       	const uint32_t handleSizeAligned = alignedSize(context.rayTracingPipelineProperties.shaderGroupHandleSize, context.rayTracingPipelineProperties.shaderGroupHandleAlignment);
       	VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegionKHR{};
       	stridedDeviceAddressRegionKHR.deviceAddress = pv::GetBufferDeviceAddress(context.m_device, buffer);
       	stridedDeviceAddressRegionKHR.stride = handleSizeAligned;
       	stridedDeviceAddressRegionKHR.size = handleCount * handleSizeAligned;
       	return stridedDeviceAddressRegionKHR;
       }
//For ray-tracing
		uint32_t PipelineRayTrace::alignedSize(uint32_t value, uint32_t alignment)
        {
	        return (value + alignment - 1) & ~(alignment - 1);
        }
void PipelineRayTrace::CreateShaderBindingTable(pv2::Context context, ShaderBindingTable& shaderBindingTable, uint32_t handleCount)
{
       
	// Create buffer to hold all shader handles for the SBT
	pv::createBuffer(context.m_device,context.m_physicalDevice,context.rayTracingPipelineProperties.shaderGroupHandleSize * handleCount,
		VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
		shaderBindingTable.buffer, shaderBindingTable.memory);
		
	// Get the strided address to be used when dispatching the rays
	shaderBindingTable.stridedDeviceAddressRegion = getSbtEntryStridedDeviceAddressRegion(context,shaderBindingTable.buffer, handleCount);
	// Map persistent 
	shaderBindingTable.map(context.m_device);
}

void PipelineRayTrace::CreateShaderBindingTables(pv2::Context context) {
		const uint32_t handleSize = context.rayTracingPipelineProperties.shaderGroupHandleSize;
		const uint32_t handleSizeAligned = alignedSize(context.rayTracingPipelineProperties.shaderGroupHandleSize, context.rayTracingPipelineProperties.shaderGroupHandleAlignment);
		const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
		const uint32_t sbtSize = groupCount * handleSizeAligned;

		std::vector<uint8_t> shaderHandleStorage(sbtSize);
		CHECK_VK(vkGetRayTracingShaderGroupHandlesKHR(context.m_device, m_graphicsPipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()));

		CreateShaderBindingTable(context,shaderBindingTables.raygen, 1);
		// We are using one miss shaders. With shaws there would be 2
		CreateShaderBindingTable(context,shaderBindingTables.miss, 1);// TODO: 2 was used for shadows
		CreateShaderBindingTable(context,shaderBindingTables.hit, 1);

		// Copy handles
		memcpy(shaderBindingTables.raygen.mapped, shaderHandleStorage.data(), handleSize);
		// We are using two miss shaders, so we need to get two handles for the miss shader binding table
		memcpy(shaderBindingTables.miss.mapped, shaderHandleStorage.data() + handleSizeAligned, handleSize );
		memcpy(shaderBindingTables.hit.mapped, shaderHandleStorage.data() + handleSizeAligned * 2, handleSize);
		
		std::cout<<"Shader binding table created"<<std::endl;
	}

void PipelineRayTrace::CreateDescriptorSetLayout(pv2::Context context)
{
   
    
   std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
   			// Binding 0: Acceleration structure
   			pv::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 0),
   			// Binding 1: Storage image
   			pv::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1),
   			// Binding 2: Uniform buffer
   			pv::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR, 2),
   			// Binding 3: Vertex buffer 
   			pv::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR| VK_SHADER_STAGE_INTERSECTION_BIT_KHR, 3),
   				};
   
   		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
   					descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   					descriptorSetLayoutCreateInfo.pBindings = setLayoutBindings.data();
   					descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
   					
   		CHECK_VK(vkCreateDescriptorSetLayout(context.m_device, &descriptorSetLayoutCreateInfo, nullptr, &m_descriptorSetLayout));
   		std::cout<<"Layout descriptor set created"<<std::endl;
   	
}

void PipelineRayTrace::CreateGraphicsPipeline(pv2::Context context, pv2::RenderBase ren)
{
   
      PFN_vkCreateRayTracingPipelinesKHR pvkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(context.m_device, "vkCreateRayTracingPipelinesKHR");
      
    
      		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo {};
      					pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      					pipelineLayoutCreateInfo.setLayoutCount = 1;
      					pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;
      					
      		
      		CHECK_VK(vkCreatePipelineLayout(context.m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));
      		DEBUG_LOG<<"Pipe layout is created "<<std::endl;
      		/*
      					Setup ray tracing shader groups
      				*/
      				std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
      				
      				
      				
      		
      				// Ray generation group
      				{
      				    auto rayGenCode = pv::readFile(raygenShaderName);
          				VkShaderModule rayGenShaderModule = pv::createShaderModule(rayGenCode, context.m_device);
          				
          				VkPipelineShaderStageCreateInfo rayGenShaderStageInfo {};
          				rayGenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
          				rayGenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
          				rayGenShaderStageInfo.module = rayGenShaderModule;
          				rayGenShaderStageInfo.pName = "main";
          				shaderStages.push_back(rayGenShaderStageInfo);
      					
      					VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
      					shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
      					shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
      					shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
      					shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
      					shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
      					shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
      					shaderGroups.push_back(shaderGroup);
      				}
      		
      				// Miss group
      				{
      				    
      				    auto missCode = pv::readFile(missShaderName);
          				VkShaderModule missShaderModule = pv::createShaderModule(missCode, context.m_device);
          				
          				VkPipelineShaderStageCreateInfo missShaderStageInfo {};
          				missShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
          				missShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
          				missShaderStageInfo.module = missShaderModule;
          				missShaderStageInfo.pName = "main";
          				shaderStages.push_back(missShaderStageInfo);
          				
          				VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
      					shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
      					shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
      					shaderGroup.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
      					shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
      					shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
      					shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
      					shaderGroups.push_back(shaderGroup);
      					
      					
      				}
      				
      				// Intersection
      					{
      					    auto intCode = pv::readFile(rintShaderName);
      					    				VkShaderModule intShaderModule = pv::createShaderModule(intCode, context.m_device);
      					    				
      					    				VkPipelineShaderStageCreateInfo intShaderStageInfo {};
      					    				intShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      					    				intShaderStageInfo.stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
      					    				intShaderStageInfo.module = intShaderModule;
      					    				intShaderStageInfo.pName = "main";
      					    				
      						shaderStages.push_back(intShaderStageInfo);
      						
      					    auto rchitCode = pv::readFile(rchitShaderName);
      									VkShaderModule rchitShaderModule = pv::createShaderModule(rchitCode, context.m_device);
      									
      									VkPipelineShaderStageCreateInfo rchitShaderStageInfo {};
      									rchitShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      									rchitShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
      									rchitShaderStageInfo.module = rchitShaderModule;
      									rchitShaderStageInfo.pName = "main";
      						
      						shaderStages.push_back(rchitShaderStageInfo);
      								
      								
      						VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
      						shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
      						shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;// TRIANGLES_HIT_GROUP_KHR;
      						shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
      						shaderGroup.closestHitShader = static_cast<uint32_t>(shaderStages.size()-1); //next one
      						shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
      						shaderGroup.intersectionShader = static_cast<uint32_t>(shaderStages.size()) -2;
      						shaderGroups.push_back(shaderGroup);
      					}
      		
      				
      		
      				VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
      				rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
      							
      				rayTracingPipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
      				rayTracingPipelineCI.pStages = shaderStages.data();
      				rayTracingPipelineCI.groupCount = static_cast<uint32_t>(shaderGroups.size());
      				rayTracingPipelineCI.pGroups = shaderGroups.data();
      				rayTracingPipelineCI.maxPipelineRayRecursionDepth = 2;
      				rayTracingPipelineCI.layout = m_pipelineLayout;
      				CHECK_VK(pvkCreateRayTracingPipelinesKHR(context.m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &m_graphicsPipeline));
      		
      		
      		std::cout<<"Ray-tracing pipe is created"<<std::endl;
     
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
void PipelineBase::SaveImage(const char* filename, char*pixels, int imageSize, int texWidth, int texHeight)
{
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    std::cout<<"start writing to image "<<texHeight<<", "<<texWidth<<std::endl;
                                   
        // ppm header
        file << "P6\n" << texWidth << "\n" << texHeight << "\n" << 255 << "\n";       
               
      file.write(pixels, imageSize);
        file.close();
}
void PipelineBase::submitBuffers(pv2::Context context, int i)
{
    pv::submitWork(m_commandBuffers[i], context.m_queueGCT,context.m_device);
    
    vkDeviceWaitIdle(context.m_device);
    
}
void PipelineRasterize::CreateCommandBuffers(pv2::Context context, pv2::RenderBase ren, VkBuffer vertexBuffer, int m_vsize)
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
        
        vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);
        //vkCmdDispatch(m_commandBuffers[i], BUFFER_ELEMENTS, 1, 1);
        DEBUG_LOG<<"Descriptors size "<<m_descriptorSets.size()<<std::endl;
        //vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        vkCmdDraw(m_commandBuffers[i], static_cast<uint32_t>(m_vsize), 1, 0, 0);
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

struct PushConstants {
    uint sample_batch;
};


