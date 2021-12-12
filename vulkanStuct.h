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

#include <config.h>
#ifdef LOG_All
#define DEBUG_LOG std::cout

#else
class log_disabled_output {
};
static log_disabled_output log_disabled_output_instance;

template <typename T>
log_disabled_output& operator<<(log_disabled_output& any, T const& thing) { return any; }

// std::endl simple, quick and dirty
log_disabled_output& operator<<(log_disabled_output& any, std::ostream& (*)(std::ostream&)) { return any; }

#define DEBUG_LOG log_disabled_output_instance
#endif

#ifdef LOG_All
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

#endif //