#include <postprocessing.hpp>
#include <window.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
*/
#include <memory>

#include "loaders.hpp"
#include <optional>
#include <set>

//#include <vulkan/vulkan.h>
/*
#include "cxxsupport/vec3.h"
#include "kernel/colour.h"
#include "kernel/colourmap.h"
*/
//#include <limits.h>
//#include <fstream>
#include "fitsReader.h"
//#include "reader/reader.h"

#include <chrono>

#include <tclap/CmdLine.h>


#ifdef USE_NVVK
#include "nvvk/context_vk.hpp"
#include <nvvk/descriptorsets_vk.hpp>
#include <nvvk/error_vk.hpp>
#include <nvvk/resourceallocator_vk.hpp> // For NVVK memory allocators
#include <nvvk/shaders_vk.hpp> // For nvvk::createShaderModule
#include <nvvk/structs_vk.hpp>
#endif

#include "server/event.h"
//Contains camera that is undefined and controller.cc
#include "server/controller.h"
//#include "server/command.h"
#include "WSocketMServer.h"

#include "SyncQueue.h"
#define ZRF_int32_size
#include "Serialize.h"

#include "cxxsupport/ls_image.h"

class SplotchServer
    {
        
    public:
        int width;
        int height;
    bool server_active    = false;
    bool server_passive =false;
    bool waiting=true;
    bool image_modified         = false;
    const int wsImageStreamPort = 8881;
    std::string wsocket_image_protocol;
    
    WSocketMServer< EventQueue& >* ims;
    
    std::vector<char>image_buffer;
    
    
    // Framerate
    int   frid                  = 0;
    float mspf                  = 0;
    float ideal_mspf            = 32;
    float max_mspf              = 32;
    
    
    
    LS_Image blank_image;
    
   
    
    // Threading
    std::thread imagesend_thread;
    
    // Controllers 
     EventController     ims_events;
     CameraController    camera;
     ClientController*    clients;
    
    // Events
    EventQueue ims_ev_queue;
    SyncQueue< std::vector<char> > ims_send_queue;
    //std::vector<char> image_buffer;
    
    void send_image(const char* image_buffer,int count);
    void launch_image_services();
    
    void init( int width, int height, SimpCamera cam);
    void init_comms();
    
    void finalize();
    // Waiting/loading
    void set_waiting_image();
    void set_loading_image();
    
    //camera events related
    void handle_events();
    
    int quality                 = 95;
    
    //TODO:
    void update_parameters(SimpCamera& m_cam);
    void launch_event_services();
    
   /* CommandController commands;
    WSocketMServer< CommandQueue& >* ctl;*/
    std::string wsocket_control_protocol;
    const int wsControlStreamPort = 8882;
    
    
    
};
    
    

const int MAX_FRAMES_IN_FLIGHT = 1;
//typedef mpi_image
class SampleApp {
public:
    SplotchServer server;
    
    
    
    int world_size;
    int world_rank;
    
    void updateScene();
 
    
    std::vector<std::tuple<int,uint8_t *>> mpi_images;
    std::vector<uint8_t*> images;
    
    void Finalise();
    void CopyMPIBuffer(int l, uint8_t* img);
    void CopyRGBMPIBuffer(int l, uint8_t* img);
    void WriteMPIBuffer(uint8_t* imagedata, int l, int count);
    void generateResultOfMPI();
    
    
    PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag = VK_NULL_HANDLE;
    	PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName = VK_NULL_HANDLE;
    	PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBegin = VK_NULL_HANDLE;
    	PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEnd = VK_NULL_HANDLE;
    	PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsert = VK_NULL_HANDLE;
    	bool debugMarkerExt = false;
   
    void SetFileName(std::string filename)
    {
        m_fitsFilename = filename;
    }
    
    void SetMode(pv2::InteractionMode mode);
    void InitCamera();
   
    void SetPipelineType(pv2::PipelineTYPE type)
       {
           m_pipeType=type;
           if(type==pv2::PipelineTYPE::Rasterisation)
            pipe=std::shared_ptr<PipelineRasterize>(new PipelineRasterize());
           else
               pipe=std::shared_ptr<PipelineRayTrace>(new PipelineRayTrace());
       }


    std::string m_fitsFilename;
    #ifdef USE_NVVK
    nvvk::RaytracingBuilderKHR                      m_rtBuilder;
    nvvk::ResourceAllocatorDma m_alloc;
    #endif
    void initWindowAndCallbacks();
     
    
      void initRenderingPipe(int width,int height);
    
      void mainLoop();
    
      void cleanup();
     
      
      void SetUpAll(int width,int height);
      

            
      

private:
    enum MouseButton {
        MOUSE_BUTTON_LEFT = 0,
        MOUSE_BUTTON_RIGHT = 1,
        MOUSE_BUTTON_MIDDLE = 2,
        NUM_MOUSE_BUTTONIDX,
    };

    SimpCamera m_cam;
    pv2::PipelineTYPE   m_pipeType;
    pv2::InteractionMode m_mode;
    //glm::mat4 world = glm::mat4(1);
    float x_rot, y_rot;

    void orbit(float dx, float dy, bool invert)
    {
        if (dx == 0 && dy == 0)
            return;
    }

    WindowBase win;
    pv::PaticleLoader pLoader;

    pv2::Context context;
    pv2::RenderBase ren;
    std::shared_ptr<PipelineRasterize> pipe;
    std::shared_ptr<MPICollect> post_process;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    int m_vsize;

    bool framebufferResized = false;

  

    void cleanupSwapChain();
    void recreateSwapChain();
    
    void drawHeadless(int imageIndex);
     

    void createSyncObjects();
   

    //Consider moving to renderer

    void drawFrame();
   
};
