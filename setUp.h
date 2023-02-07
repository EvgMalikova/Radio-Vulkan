#ifndef SET_UP
#define SET_UP

//Camera parameters

//Program set UP
class PipelineSetUp {
public:
    PipelineSetUp()
    {
        m_interactive = false;
    }
    PipelineSetUp(bool interact)
    {
        m_interactive = interact;
    };

    ~PipelineSetUp()
    {
    }

    void Initialize() /*First stage: context creation and checking on all available extensions*/
        {};
    void SetPipelineProperties() /*Second stage: get properties necessary for furter construction*/
        {};

private:
    bool m_interactive;
};
}
class RayTrace : public PipelineSetUp {
public:
    enum DataLayout {
        Basic, /**/
        Advanced
    };

    enum Bindings {
        BIND_IMAGEDATA, //0
        BIND_TLAS, //1
        BIND_VERTICES //2
    };

    RayTrace()
    {
        m_Dlayout = Basic;
        m_numBind = 3;
        /*First stage: context creation*/
    }
    ~RayTrace()
    {
    }

    virtual void Initialize()
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
        nvvk::Context context; // Encapsulates device state in a single object
        context.init(deviceInfo); // Initialize the context
        // Device must support acceleration structures and ray tracing pipelines:
        assert(asFeatures.accelerationStructure == VK_TRUE && rtPipelineFeatures.rayTracingPipeline == VK_TRUE);

        // Create the allocator
        nvvk::ResourceAllocatorDedicated allocator;
        allocator.init(context, context.m_physicalDevice);
    };
    virtual void SetPipelineProperties()
    {
        // Get the properties of ray tracing pipelines on this device. We do this by
        // using vkGetPhysicalDeviceProperties2, and extending this by chaining on a
        // VkPhysicalDeviceRayTracingPipelinePropertiesKHR object to get both
        // physical device properties and ray tracing pipeline properties.
        // This gives us information about shader binding tables.
        rtPipelineProperties = nvvk::make<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
        VkPhysicalDeviceProperties2 physicalDeviceProperties = nvvk::make<VkPhysicalDeviceProperties2>();
        physicalDeviceProperties.pNext = &rtPipelineProperties;
        vkGetPhysicalDeviceProperties2(context.m_physicalDevice, &physicalDeviceProperties);
        VkDeviceSize sbtHeaderSize = rtPipelineProperties.shaderGroupHandleSize;
        VkDeviceSize sbtBaseAlignment = rtPipelineProperties.shaderGroupBaseAlignment;
        VkDeviceSize sbtHandleAlignment = rtPipelineProperties.shaderGroupHandleAlignment;

        assert(sbtBaseAlignment % sbtHandleAlignment == 0);
        sbtStride = sbtBaseAlignment * //
            ((sbtHeaderSize + sbtBaseAlignment - 1) / sbtBaseAlignment);
        assert(sbtStride <= rtPipelineProperties.maxShaderGroupStride);
    }

    void CreateImage() /* A set of operations for image to write ray-tracing output to*/
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
        nvvk::Image image = allocator.createImage(imageCreateInfo);
        debugUtil.setObjectName(image.image, "image");

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
        VkImageView imageView;
        NVVK_CHECK(vkCreateImageView(context, &imageViewCreateInfo, nullptr, &imageView));
        debugUtil.setObjectName(imageView, "imageView");

        // Also create an image using linear tiling that can be accessed from the CPU,
        // much like how we created the buffer in the main tutorial. The first image
        // will be entirely local to the GPU for performance, while this image can
        // be mapped to CPU memory. We'll copy data from the first image to this
        // image in order to read the image data back on the CPU.
        // As before, we'll transition the image layout in the same command buffer
        // used to upload the vertex and index buffers.
        imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
        imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        nvvk::Image imageLinear = allocator.createImage(imageCreateInfo, //
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT //
                | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT //
                | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
    }

private:
    DataLayout m_Dlayout;
    int m_numBind;

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtPipelineProperties; /*pipeline properties*/

    /*For construction of STB*/
    VkDeviceSize sbtHeaderSize;
    VkDeviceSize sbtBaseAlignment;
    VkDeviceSize sbtHandleAlignment;
    VkDeviceSize sbtStride;
};

class Rasterisation : public PipelineSetUp {
    enum DataLayout {
        Basic, /**/
        Advanced
    };

    enum Bindings {
        BIND_IMAGEDATA, //0
        BIND_VERTICES //1
    };

    RasterisationSetUp()
    {
        m_Dlayout = Basic;
        m_numBind = 1;
    }
    ~RasterisationSetUp()
    {
    }

private:
    DataLayout m_Dlayout;
    int m_numBind;
};

#endif //