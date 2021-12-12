#ifndef Renderer
#define Renderer
#include "context.hpp"
#include "helpers.hpp"
namespace pv2 {
class RenderBase {
public:
    VkSurfaceKHR m_surface { VK_NULL_HANDLE };
    VkSwapchainKHR m_swapChain { VK_NULL_HANDLE };

    struct HeadlessBufferAttachment {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView view;
    };

    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    HeadlessBufferAttachment m_cAttachment, m_dAttachment;

    VkRenderPass m_renderPass;

    std::vector<VkImage> m_Images;
    VkFormat m_ImageFormat;
    VkExtent2D m_Extent;

    void SetWindow(GLFWwindow* win)
    {
        m_window = win;
    };

    void SetExtent(int width, int height)
    {
        m_ImageFormat = VK_FORMAT_R8G8B8A8_UNORM;

        m_Extent.width = width;
        m_Extent.height = height;
    }
    void CreateImage(pv2::Context context);

    RenderBase() {};
    ~RenderBase() {};
    void Initialize(pv2::Context& m_context)
    {
        if (m_context.GetInteractive())
            CreateSurface(m_context);

        //m_context.PickPhysicalDevice(m_surface);
        //m_context.CreateLogicalDevice(m_surface);
    };

    void SaveImage(int ij, pv2::Context context, VkCommandPool commandPool, VkQueue queueG, FrameBufferAttachment colorAttachment);
    void CreateSurface(pv2::Context m_context);
    void CreateSwapChain(pv2::Context m_context);
    void CreateImageViews(pv2::Context m_context);
    void CreateRenderPass(pv2::Context m_context);
    void CreateFramebuffers(pv2::Context m_context);

    // void CleanupSwapChain(pv2::Context context);
    void CleanUp(pv2::Context context);

private:
    GLFWwindow* m_window;
    std::vector<VkImageView> m_ImageViews;

    //created only for interactive
};
}

#endif //