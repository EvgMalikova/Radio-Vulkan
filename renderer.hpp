#ifndef Renderer
#define Renderer
#include "helpers.hpp"
#include "context.hpp"
namespace pv2 {
class RenderBase
	{
 public:
   VkSurfaceKHR m_surface{VK_NULL_HANDLE};
   VkSwapchainKHR m_swapChain{VK_NULL_HANDLE};
   
   

   
    
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
   VkRenderPass m_renderPass;
   
   std::vector<VkImage> m_Images;
   VkFormat m_ImageFormat;
   VkExtent2D m_Extent;
   
   void SetWindow(GLFWwindow* win){
   m_window=win;
 };
 
  
   RenderBase(){};
   ~RenderBase(){};
   void Initialize(pv2::Context& m_context)
{
 
 CreateSurface(m_context);
 
 //m_context.PickPhysicalDevice(m_surface);
 //m_context.CreateLogicalDevice(m_surface);
 };
   
   
   
   void SaveImage(int ij, pv2::Context context, VkCommandPool commandPool, VkQueue queueG, FrameBufferAttachment colorAttachment);  
   void   CreateSurface(pv2::Context m_context);
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