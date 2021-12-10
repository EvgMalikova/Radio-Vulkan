#ifndef Vulk_DATA
#define Vulk_DATA
/*Color attachement
 * Depth attachement
 * */
#include <vulkan/vulkan.h>
struct FrameBufferAttachment {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
};

#endif //