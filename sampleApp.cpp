

#include "sampleApp.h"
#include <dlfcn.h>
#include <renderdoc_app.h>
#ifdef USE_MPIRV
#include <mpi.h>
#endif
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
RENDERDOC_API_1_1_2 *renderDocApi = NULL;

#include "TJCompressor.h"
#include "JPEGImage.h"

//! Specialization for JPEGImage from tjpp library
// May not need WEB_BYTEARRAY, use ZRF_UNSIGNED_CHAR?
struct SerializeJPEGImage
{
    using SS = srz::SerializePOD<size_t>;
    using WEB_BYTEARRAY = std::vector<unsigned char>;
    static srz::ByteArray Pack(const tjpp::JPEGImage &jpi,
                               srz::ByteArray buf = srz::ByteArray())
    {
        buf = srz::PackArgs(jpi.Width(), jpi.Height(), jpi.PixelFormat(), jpi.ChrominanceSubSampling(), jpi.Quality(), jpi.CompressedSize());
        int sz = buf.size();
        buf.resize(sz + jpi.CompressedSize());
        memcpy(&buf[sz], jpi.DataPtr(), jpi.CompressedSize());
        return buf;
    }
    static WEB_BYTEARRAY PackDataOnlyWeb(const tjpp::JPEGImage &jpi,
                                         WEB_BYTEARRAY buf = WEB_BYTEARRAY())
    {
        int sz = 0;
        buf.resize(jpi.CompressedSize());
        memcpy(&buf[sz], jpi.DataPtr(), jpi.CompressedSize());
        return buf;
    }

    static srz::ConstByteIterator UnPack(srz::ConstByteIterator bi,
                                         tjpp::JPEGImage &jpi)
    {
        int w, h, pf, ss, q;
        size_t sz;
        bi = srz::SerializePOD<int>::UnPack(bi, w);
        bi = srz::SerializePOD<int>::UnPack(bi, h);
        bi = srz::SerializePOD<int>::UnPack(bi, pf);
        bi = srz::SerializePOD<int>::UnPack(bi, ss);
        bi = srz::SerializePOD<int>::UnPack(bi, q);
        bi = srz::SerializePOD<size_t>::UnPack(bi, sz);
        TJPF pf1 = (TJPF)pf;
        TJSAMP ss1 = (TJSAMP)ss;

        jpi.Reset(w, h, pf1, ss1, q);
        planck_assert(UncompressedSize(jpi) >= sz, "srz::UnPack: allocated buffer < recieved JPEGSize");
        memmove(jpi.DataPtr(), &(*bi), sz);
        jpi.SetCompressedSize(sz);
        return bi;
    }
};
//! De-serialize data from byte array.
void UnPack(const srz::ByteArray &ba, tjpp::JPEGImage &d)
{
    SerializeJPEGImage::UnPack(ba.begin(), d);
}

//#ifdef LOG_All
//const bool enableValidationLayers = false;
//#else
//const bool enableValidationLayers = true;
//#endif

static float mouse_x, mouse_y;
static float rotate_x, rotate_y;
static bool Xrotate;
static float mouse_prev_x, mouse_prev_y;
static float hval = 0;
static float vval = 0;
static float model_scale;

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{

    if (yoffset > 0)
        DEBUG_LOG << "scrolled up" << std::endl;

    else
        DEBUG_LOG << "scrolled down" << std::endl;
    model_scale += yoffset;
    DEBUG_LOG << model_scale << std::endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    // here your code
    mouse_prev_x = mouse_x;
    mouse_prev_y = mouse_y;
    mouse_x = x;
    mouse_y = y;
    if (Xrotate)
    {

        //glfwGetWindowSize(&width, &height);

        rotate_x = mouse_x - mouse_prev_x;
        rotate_y = mouse_y - mouse_prev_y;
        hval = 10.0f * (float)(mouse_x - mouse_prev_x) / (float)WIDTH;
        vval = 10.0f * (float)(mouse_y - mouse_prev_y) / (float)HEIGHT;

        DEBUG_LOG << "mouse " << rotate_x << " , " << rotate_y << std::endl;
    }
    else
    {
        rotate_x = 0;
        rotate_y = 0;
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    int whichkey = 0; //Left 1 Right 2 Middle 3
    if (action == GLFW_PRESS)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            Xrotate = true;
            //rotate_x=0;
            //rotate_y=0;

            DEBUG_LOG << "left down" << std::endl;

            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:

            DEBUG_LOG << "mid down" << std::endl;

            break;
        case GLFW_MOUSE_BUTTON_RIGHT:

            DEBUG_LOG << "right down" << std::endl;

            break;
        default:
            return;
        }
    }
    else
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            Xrotate = false;
            rotate_x = mouse_prev_x - mouse_x;
            rotate_y = mouse_prev_y - mouse_y;

            DEBUG_LOG << "released left down" << std::endl;

            break;
        case GLFW_MOUSE_BUTTON_MIDDLE:

            DEBUG_LOG << "eleased mid down" << std::endl;

            break;
        case GLFW_MOUSE_BUTTON_RIGHT:

            DEBUG_LOG << "released right down" << std::endl;

            break;
        default:
            return;
        }
    }
    //Par1 above
    //...
    return;
}

static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto winB = reinterpret_cast<WindowBase *>(glfwGetWindowUserPointer(window));
    winB->SetFameBuffeResized(true);
}
void SampleApp::InitCamera()
{
    glm::vec3 eyePos = glm::vec3(1.0f, 1.0f, 0.0f);
    glm::vec3 focusPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

    m_cam = SimpCamera(eyePos, focusPos, up);

    //a duplicate for server
    /*server.camera=CameraController(eyePos, focusPos, up);
        server.camera.move_speed = (0.1);
        server.camera.rotate_speed = (0.1);
        */

    mouse_x = 0;
    mouse_y = 0;
    rotate_x = 0;
    rotate_y = 0;
    hval = 0;
    vval = 0;

    model_scale = 1.0;

    float move_speed = (0.1);
    float rotate_speed = (0.1);

    server.InitCamera(eyePos, focusPos, up, move_speed, rotate_speed);
}

void SampleApp::initWindowAndCallbacks()
{
    win.InitWindow();

    glfwSetFramebufferSizeCallback(win.GetWindow(), framebufferResizeCallback);
    glfwSetCursorPosCallback(win.GetWindow(), cursor_pos_callback);
    glfwSetScrollCallback(win.GetWindow(), scroll_callback);
    glfwSetMouseButtonCallback(win.GetWindow(), mouse_button_callback);

    glfwSetKeyCallback(win.GetWindow(), key_callback);
}

void SampleApp::SetMode(pv2::InteractionMode mode)
{
    m_mode = mode;
    if (m_mode == pv2::InteractionMode::Headless)
    {
        if (void *mod = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
        {
            pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
            int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&renderDocApi);
            assert(ret == 1);
        }
    }
}
void SampleApp::initRenderingPipe(int width, int height)
{
    m_vsize = 0;
    if (m_mode == pv2::InteractionMode::Interactive)
        context.SetInteractive(true);
    else
        context.SetInteractive(false);

    //TODO: make dependent on MPI enabling and rank>1
    post_process = std::shared_ptr<MPICollect>(new MPICollect());

    context.SetPipelineType(m_pipeType);
    context.Initialize(); //allocates all necessary extensions
#ifdef USE_GLFW
    if (m_mode == pv2::InteractionMode::Interactive)
        ren.SetWindow(win.GetWindow());
    else
        ren.SetExtent(width, height);
#else
    m_mode = pv2::InteractionMode::Headless;
    ren.SetExtent(width, height);
#endif
    ren.Initialize(context); //creates a surface

    context.PickPhysicalDevice(ren.m_surface); //checks rasterization and ray-tracing
    context.CreateLogicalDevice(ren.m_surface);

    //TODO:should be first before render pass and etc.
    pipe->CreateCommandPool(context, ren);
    post_process->CreateCommandPool(context, ren);
    if (renderDocApi)
    {
        renderDocApi->SetCaptureOptionU32(RENDERDOC_CaptureOption::eRENDERDOC_Option_RefAllResources, 1);
        renderDocApi->SetCaptureOptionU32(RENDERDOC_CaptureOption::eRENDERDOC_Option_SaveAllInitials, 1);
        renderDocApi->StartFrameCapture(nullptr, nullptr);
        if (renderDocApi->IsFrameCapturing)
        {
            std::cout << "RenderDoc capturing..." << std::endl;
        }
    }
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(context.m_physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(context.m_physicalDevice, nullptr, &extensionCount, extensions.data());
    for (auto extension : extensions)
    {
        if (strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
        {
            debugMarkerExt = true;
            break;
        }
    }
    if (debugMarkerExt)
    {
        vkDebugMarkerSetObjectTag = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(context.m_device, "vkDebugMarkerSetObjectTagEXT");
        vkDebugMarkerSetObjectName = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(context.m_device, "vkDebugMarkerSetObjectNameEXT");
        vkCmdDebugMarkerBegin = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(context.m_device, "vkCmdDebugMarkerBeginEXT");
        vkCmdDebugMarkerEnd = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(context.m_device, "vkCmdDebugMarkerEndEXT");
        vkCmdDebugMarkerInsert = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(context.m_device, "vkCmdDebugMarkerInsertEXT");
        std::cout << VK_EXT_DEBUG_MARKER_EXTENSION_NAME << " detected, using debug markers" << std::endl;
    }
    else
    {
        std::cout << "Warning: " << VK_EXT_DEBUG_MARKER_EXTENSION_NAME << " not present, debug markers are disabled." << std::endl;
        std::cout << "Try running from inside a Vulkan graphics debugger (e.g. RenderDoc)" << std::endl;
    }

    //Main particle loader

    pLoader.SetDataDir("../data/");
    pLoader.SetFileName(m_fitsFilename);
    pLoader.LoadData(world_size, world_rank); //1, 0);

//TODO
//no depth attachement as far, only color
#ifdef USE_GLFW
    if (m_mode == pv2::InteractionMode::Interactive)
    {

        ren.CreateSwapChain(context);
    }

    else
        ren.CreateImage(context);
#else
    m_mode = pv2::InteractionMode::Headless;
    ren.CreateImage(context);
#endif

    ren.CreateImageViews(context);

    ren.CreateRenderPass(context);

    pipe->SetSize(ren.m_Extent.width, ren.m_Extent.height);

    ren.CreateFramebuffers(context);

    pLoader.CreateVertexBuffer(pipe->m_commandPool, context.m_device, context.m_physicalDevice, context.m_queueGCT); //would be different for ray-tracing
    m_vsize = pLoader.vertices.size();

    // m_alloc.init(context.m_instance, context.m_device, context.m_physicalDevice);
    // m_rtBuilder.setup(context.m_device, &m_alloc, context.indices.graphicsFamily.value(),);

    //std::vector<nvvk::RaytracingBuilderKHR::BlasInput> allBlas;
    //allBlas.push_back(pipe->sphereToVkGeometryKHR(context,pLoader.m_vertexBuffer,m_vsize));

    //m_rtBuilder.buildBlas(allBlas, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

    //TODO  check pipeline
    if (m_pipeType == pv2::PipelineTYPE::RayTracing)
    {
        pLoader.CreateAABBsBuffer(pipe->m_commandPool, context.m_device, context.m_physicalDevice, context.m_queueGCT);

        std::dynamic_pointer_cast<PipelineRayTrace>(pipe)->CreateBottomLevelAccelerationStructure(context, pLoader.aabbs.buffer, pLoader.aabbs.count);

        std::dynamic_pointer_cast<PipelineRayTrace>(pipe)->CreateTopLevelAccelerationStructure(context);

        //TODO: createStorageImage(swapChain.colorFormat, { width, height, 1 }); should be ren.CreateImage(context);
    }

    //TODO Here ray-tracing

    pipe->CreateDescriptorSetLayout(context);
    pipe->CreateGraphicsPipeline(context, ren);
    if (this->m_pipeType == pv2::PipelineTYPE::RayTracing)
    {
        //TODO:

        //CreateShadersBindingTable
    }
    //m_cam.CreateUniformBuffers(context.m_device, context.m_physicalDevice, ren.m_Images.size());
    //switch for headless and server rendering
    if (m_mode == pv2::InteractionMode::Interactive)
        m_cam.CreateUniformBuffers(context.m_device, context.m_physicalDevice, ren.m_Images.size());
    else
    {
        //use controller camera
        server.camera.CreateUniformBuffers(context.m_device, context.m_physicalDevice, ren.m_Images.size());
    }

    //TODO: check correct creation of those
    pipe->CreateDescriptorPool(context, ren.m_Images.size());
    //TODO: pass vertex and imageViews
    // DEBUG_LOG << "Successfull as far " << ren.m_Images.size() <<std::endl;
    if (this->m_pipeType == pv2::PipelineTYPE::RayTracing)
    {
        for (int i = 0; i < ren.m_Images.size(); i++)
        {
            std::dynamic_pointer_cast<PipelineRayTrace>(pipe)->CreateImageDescriptor(ren.GetImageView(i));
        }
        std::dynamic_pointer_cast<PipelineRayTrace>(pipe)->CreateBufferDescriptor(pLoader.m_vertexBuffer);
    }
    //DEBUG_LOG << "Successfull as far " << std::endl;
    //pipe->CreateDescriptorSets(context, ren.m_Images.size(), m_cam);
    if (m_mode == pv2::InteractionMode::Interactive)
        pipe->CreateDescriptorSets(context, ren.m_Images.size(), m_cam); // are defined after layout
    else
        pipe->CreateDescriptorSets(context, ren.m_Images.size(), server.camera.GetCamera());

    DEBUG_LOG << "Successfull as far " << std::endl;

    if (this->m_pipeType == pv2::PipelineTYPE::Rasterisation)
        pipe->CreateCommandBuffers(context, ren, pLoader.m_vertexBuffer, m_vsize);

    if (m_mode == pv2::InteractionMode::Interactive)
        createSyncObjects();
    /*else
          {
              pipe->submitBuffers(context,0);
              
          }*/
    DEBUG_LOG << "Successfull as far " << std::endl;
}

void SampleApp::mainLoop()
{

    while (!glfwWindowShouldClose(win.GetWindow()))
    {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(context.m_device);
}

void SampleApp::cleanup()
{
    if (m_mode == pv2::InteractionMode::Interactive)
        cleanupSwapChain();

    vkDestroyDescriptorSetLayout(context.m_device, pipe->m_descriptorSetLayout, nullptr);

    //vkDestroyBuffer(device, indexBuffer, nullptr);
    //vkFreeMemory(device, indexBufferMemory, nullptr);
    pLoader.CleanUp(context.m_device);

    if (m_mode == pv2::InteractionMode::Interactive)
    {
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(context.m_device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(context.m_device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(context.m_device, inFlightFences[i], nullptr);
        }
    }
    vkDestroyCommandPool(context.m_device, pipe->m_commandPool, nullptr);

    vkDestroyDevice(context.m_device, nullptr);

    context.CleanUp();
    if (m_mode == pv2::InteractionMode::Interactive)
    {
        vkDestroySurfaceKHR(context.m_instance, ren.m_surface, nullptr);
    }

    vkDestroyInstance(context.m_instance, nullptr);
    if (m_mode == pv2::InteractionMode::Interactive)
        win.CleanUp();
}

void SampleApp::Finalise()
{
#ifdef USE_MPIRV
    // Finalize the MPI environment.

    MPI_Finalize();
#endif
}

void SampleApp::SetUpAll(int width, int height)
{

    world_size = 1;
    world_rank = 0;

#ifdef USE_MPIRV
    //TODO: further modify with reading
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    DEBUG_LOG << " processor " << processor_name << " rank " << world_rank << "out of " << world_size << std::endl;

    context.SetMPI(world_rank, world_size);

#endif

    InitCamera();
    if (m_mode == pv2::InteractionMode::Interactive)
    {
        initWindowAndCallbacks();
    }

    initRenderingPipe(width, height); //Different implementation for rasterization and ray-tracing pipes

    if (m_mode == pv2::InteractionMode::Interactive)
    {

        mainLoop();
    }

    else
    {
        //Streaming service

        int count = ren.m_Extent.width * ren.m_Extent.height * 4;
        int count2 = ren.m_Extent.width * ren.m_Extent.height * 3;

        uint8_t *imgF2 = (uint8_t *)malloc(count2);
        uint8_t *imgF3 = (uint8_t *)malloc(count2);

        //Write test image for debug purpuses
        drawHeadless(0);
        uint8_t *img = (uint8_t *)malloc(count);
        CopyMPIBuffer(0, img);

        //Sending image data to main process
        //DEBUG_LOG << "Test output of image" << std::endl;
        uint8_t *imgF = (uint8_t *)malloc(count2);
        //CopyRGBMPIBuffer(0, imgF);
        //WriteMPIBuffer(imgF, world_rank, count2);

#ifdef USE_MPIRV

        //Allocate size for images
        if (world_rank == 0)
        {
            images.resize(world_size);
            mpi_images.resize(world_size);

            for (int ii = 0; ii < world_size; ii++)
            {
                uint8_t *imagedata2 = (uint8_t *)malloc(count);
                mpi_images[ii] = std::make_tuple(ii, imagedata2);
            }
        }
        RenderMPI(count, count2, img, imgF2);
        //Postprocessing
        if (world_rank == 0)
        {
            //testing the result
            DEBUG_LOG << "Output of images stack " << mpi_images.size() << std::endl;

            //Postprocessing
            generateResultOfMPI();

            // writing to buffer
            CopyRGBMPIBuffer(0, imgF2);
        }

#else
        DEBUG_LOG << "NO MPI pushing one image for world size " << world_size << std::endl;

        CopyRGBMPIBuffer(0, imgF2);
#endif

        DEBUG_LOG << "Finish writing test images " << std::endl;
        //server.server_active=true;
        //if (server.server_active)
        //#ifdef USE_MPIRV
        // Initiate server for MPI

        if (world_rank == 0)
        {
            server.image_modified = true;
            server.init(ren.m_Extent.width, ren.m_Extent.height, m_cam, context);
            server.camera.active = true;
            DEBUG_LOG << "Server configured, further rendering" << std::endl;
        }

        float m_rotateY = 0;
        float m_rotateX = 0;
        float angleY = 0;
        float angleX = 0;

        bool running = true;
        bool onedirX = false;
        bool onedirY = false;
        int coef = 1;
        while (running)
        {

            //update staff
            //update_state();

            if (world_rank == 0)
            {
                server.handle_events();
                //TODO: how to transmit the values of cam???
                server.update_parameters(m_cam); //TODO:update buffer and queue, get image
            }

            //Introduce
            //rotate_x+=0.5;
            /*angleY += 5.5 * coef;
            if ((angleY >= 250) || (angleY <= -250))
            {
                coef = -1 * coef;
            }
            m_rotateX = 1.5;
            //if (rotate_x>=2*M_PI) rotate_x=0;
            m_rotateY = coef * 5.5;

           m_cam.UpdateUniformBuffer(context.m_device, 0, model_scale, ren.m_Extent.width, ren.m_Extent.height, m_rotateX, m_rotateY);
           */
            pv::submitWork(pipe->m_commandBuffers[0], context.m_queueGCT, context.m_device);

            vkDeviceWaitIdle(context.m_device);

#ifdef USE_MPIRV
            //Copy buffers for all processes
            CopyMPIBuffer(0, img);

            //Render for all processes
            RenderMPI(count, count2, img, imgF3);
            if (world_rank == 0)
            {
                //testing the result
                DEBUG_LOG << "Output of images stack " << mpi_images.size() << std::endl;

                //Postprocessing
                updateResultOfMPI();

                // writing to buffer
                CopyRGBMPIBuffer(0, imgF3);
            }
#else
            CopyRGBMPIBuffer(0, imgF3);
#endif
            //WriteMPIBuffer(imgF3, 11, count2);

            //CopyRGBMPIBuffer(0, imgF3);

            //----------
            if (world_rank == 0)
            {
                //WriteMPIBuffer(imgF3, 11, count2);

                updateScene();

                server.send_image((char *)imgF3, count2);
            }
        }

//Free data
#ifdef USE_MPIRV
        for (int ii = 0; ii < world_size; ii++)
        {

            delete[] std::get<1>(mpi_images[ii]);
        }
#endif

        if (world_rank == 0)
        {
            server.finalize();
        }

        //#endif
        delete[] imgF2;
        delete[] imgF3;

        //           #endif

        if (renderDocApi)
        {
            renderDocApi->EndFrameCapture(nullptr, nullptr);
            renderDocApi->Shutdown();
        }
    }

    cleanup();
    Finalise(); //finish MPI
};
void SampleApp::updateScene()
{
}

#ifdef USE_MPIRV
void SampleApp::RenderMPI(int count, int count2, uint8_t *img, uint8_t *imgF2)
{
    MPI_Barrier(MPI_COMM_WORLD);

    // WriteMPIBuffer(img, world_rank, count2);
    //MPI_Barrier(MPI_COMM_WORLD);

    char *str = "test";
    if (world_rank != 0)
    {
        // DEBUG_LOG << "Send from " << world_rank << " " << str[0] << std::endl;

        MPI_Send(img, count, MPI_CHAR, 0, world_rank, MPI_COMM_WORLD);
        DEBUG_LOG << "Send from " << world_rank << std::endl;
    }
    else if (world_rank == 0)
    {
        //mpi_images.reserve(world_size);
        //for (int i = 0; i < world_size; i++)
        //   std::get<0>(mpi_images[i]) < 10000;                          //se big for further sorting
        //mpi_images.emplace(mpi_images.begin(), std::make_tuple(0, img)); //push first one
        mpi_images[0] = std::make_tuple(0, img);
        for (int ii = 1; ii < world_size; ii++)
        {
            uint8_t *imagedata2 = std::get<1>(mpi_images[ii]); //(uint8_t *)malloc(count);
            MPI_Recv(imagedata2, count, MPI_CHAR, ii, ii, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            //mpi_images.emplace(mpi_images.begin() + ii, std::make_tuple(ii, imagedata2));
            //mpi_images[ii]=std::make_tuple(ii, imagedata2);
            DEBUG_LOG << "Process " << world_rank << " received from process " << ii << std::endl;
        }
        //mpi_images[world_size]=std::make_tuple(world_size, std::get<1>mpi_images[world_size-1]);
    }
    //MPI_Barrier(MPI_COMM_WORLD);

    //MPI_Barrier(MPI_COMM_WORLD);
}
#endif

void SplotchServer::update_parameters(SimpCamera &m_cam)
{
    if (camera.modified)
    {
        DEBUG_LOG << "Cam modified" << std::endl;
        camera.modified = false;
    }
}
void SplotchServer::set_waiting_image()
{
    // Create annotated grey image
    blank_image = LS_Image(width, height);
    blank_image.fill(Colour(0, 0, 0));
    blank_image.annotate_centered(width / 2, height / 3, Colour(1, 1, 1), "Connected to Splotch Server");
    blank_image.annotate_centered(width / 2, 2 * (height / 3), Colour(1, 1, 1), "Waiting for data input...");
    // Write to image buffer
    for (unsigned i = 0; i < height; i++)
        for (unsigned j = 0; j < width; j++)
            *(Colour8 *)(&image_buffer[(i * width + j) * 3]) = blank_image.get_pixel(j, i);
}

void SplotchServer::set_loading_image()
{
    // Create annotated grey image

    blank_image = LS_Image(width, height);
    blank_image.fill(Colour(0, 0, 0));
    blank_image.annotate_centered(width / 2, height / 3, Colour(1, 1, 1), "Connected to Splotch Server");
    blank_image.annotate_centered(width / 2, 2 * (height / 3), Colour(1, 1, 1), "Loading Data");
}

// Check for events and pass to appropriate handlers
void SplotchServer::handle_events()
{
    if (server_active)
    {
        // Enforce update rate limit
        camera.mspf = (mspf < max_mspf) ? max_mspf : mspf;
        // Events recieved from control stream server
        /* while(!cmd_queue.empty())
          { 
            commands.handler(cmd_queue.pop());
          }
          */
        //if (ims_ev_queue.empty())
        //DEBUG_LOG << "Event Queue is empty " << std::endl;
        //else
        {
            // Events recieved from image stream server
            while (!ims_ev_queue.empty())
            {
                ims_events.handler(ims_ev_queue.pop());
                //DEBUG_LOG << "Event Occured " << std::endl;
            }
        }
    }
}

void SplotchServer::init(int width, int height, SimpCamera cam, pv2::Context context)

{
    this->width = width;
    this->height = height;

    bool master = true; //considering one MPI
    if (!master)
    {
        server_passive = true;
        return;
    }
    server_active = true;

    image_buffer.resize(width * height * 3);
    set_waiting_image();
    DEBUG_LOG << "Waiting image set" << std::endl;
    //image_buffer.resize(width*ren.m_Extent.height*3);
    image_modified = true;
    waiting = true;

    float move_speed = (0.1);
    float rotate_speed = (0.1);
    /*glm::vec3 eye = cam.GetEye();
    glm::vec3 look = cam.GetTarget();
    glm::vec3 up = cam.GetUpVector();*/
    cam.SetWindowSize(width, height);
    cam.SetModelScale(model_scale);
    cam.SetDevice(context.m_device);
    //camera.init(eye, look, up, move_speed, rotate_speed, ideal_mspf);
    camera.SetWindowSize(width, height);
    camera.SetModelScale(model_scale);
    camera.SetDevice(context.m_device);

    camera.SetCam(cam);

    // Observers setup
    // Could probably do this in constructors rather than manually?
    ims_events.mouse.attach(camera.mouse_observer);
    ims_events.keyboard.attach(camera.key_observer);

    // Communication setup
    wsocket_image_protocol = "appclient-image-stream-protocol";
    //wsocket_control_protocol = "splotch-control-protocol";

    // Launch servers and services
    init_comms();
    DEBUG_LOG << "Coms" << std::endl;
    launch_image_services();

    // launch_event_services();

    // Initialize command system
    //init_cmds();

    // Client stuff
    // This new is temporary until action queue is introduced

    //TODO: Should be set a splotch server
    clients = new ClientController(this);
    DEBUG_LOG << "Services launched" << std::endl;
    //ctl_events.client.attach(clients->ctl_observer);
    ims_events.client.attach(clients->observer);
    //commands.client.attach(clients->observer);
    clients->load_default_client();
}

// Launch the asynuc services for sending/receiving events
void SplotchServer::launch_event_services()
{
    // The event sending service
    /*  auto eventsender = [this]() {
   while(server_active){
          //std::cout << "ctl connected clients: " <<ctl->ConnectedClients() << std::endl;
          if(ctl->ConnectedClients()>0){ 
             std::tuple<ClientId, WSMSGTYPE, std::vector<unsigned char> > tosend  = commands.sender.Pop();
             ctl->Push(std::get<2>(tosend), false, std::get<1>(tosend), std::get<0>(tosend));
          }
          else{
           std::this_thread::sleep_for(std::chrono::milliseconds(100));
          } 
       }
     };
     eventsend_thread = std::thread(eventsender);
     */
}

// Any setup thats needed for communication
void SplotchServer::init_comms()
{
    // Set websockets logging levels
    lws_set_log_level(LLL_ERR, nullptr);
    lws_set_log_level(LLL_WARN, nullptr);
    lws_set_log_level(LLL_NOTICE, nullptr);
    lws_set_log_level(LLL_INFO, nullptr);
    lws_set_log_level(LLL_DEBUG, nullptr);
    lws_set_log_level(LLL_PARSER, nullptr);
    lws_set_log_level(LLL_HEADER, nullptr);
    lws_set_log_level(LLL_EXT, nullptr);
    lws_set_log_level(LLL_CLIENT, nullptr);
    lws_set_log_level(LLL_LATENCY, nullptr);
    lws_set_log_level(LLL_ERR,
                      [](int /*level*/, const char *msg)
                      {
                          std::cerr << msg << std::endl;
                      });
    // Launch the websockets event and image servers
    /* ctl = new  WSocketMServer< CommandQueue& >(wsocket_control_protocol, //protocol name
                     200, //timeout: will spend this time to process
                           //websocket traffic, the higher the better
                     wsControlStreamPort, //port
                     cmd_queue, //callback
                     false, //recycle memory
                     0x1000, //input buffer size
                     0);  //min interval between sends in ms*/

    ims = new WSocketMServer<EventQueue &>(wsocket_image_protocol, //protocol name
                                           1000,                   //timeout: will spend this time to process
                                                                   //websocket traffic, the higher the better
                                           wsImageStreamPort,      //port
                                           ims_ev_queue,           //callback
                                           false,                  //recycle memory
                                           0x1000,                 //input buffer size
                                           0);                     //min interval between sends in ms
}

// Launch the asynuc services for sending/receiving events
/* void SampleApp::launch_event_services()
 {
   // The event sending service
   auto eventsender = [this]() {
   while(server_active){
          //std::cout << "ctl connected clients: " <<ctl->ConnectedClients() << std::endl;
          if(ctl->ConnectedClients()>0){ 
             std::tuple<ClientId, WSMSGTYPE, std::vector<unsigned char> > tosend  = commands.sender.Pop();
             ctl->Push(std::get<2>(tosend), false, std::get<1>(tosend), std::get<0>(tosend));
          }
          else{
           std::this_thread::sleep_for(std::chrono::milliseconds(100));
          } 
       }
     };
     eventsend_thread = std::thread(eventsender);
 }*/

void SplotchServer::finalize()
{
    if (server_active)
    {
        // Wait for service threads
        //eventsend_thread.join();
        imagesend_thread.join();
        delete (clients);
    }
    // Deactivate server
    server_active = false;
    server_passive = false;
}
// Launch the async services for image sending
void SplotchServer::launch_image_services()
{
    //quality = params->find<int>("tjpp_quality",95);

    // Create image sending service
    auto imagesender = [this]()
    {
        tjpp::JPEGImage jpegImage;
        tjpp::TJCompressor comp;
        jpegImage.Reset(width, height, TJPF_RGB, TJSAMP_420, quality);
        while (server_active)
        {
            jpegImage = comp.Compress(std::move(jpegImage), (const unsigned char *)&(ims_send_queue.Pop())[0], width, height, TJPF_RGB, TJSAMP_420, quality);
            if (ims->ConnectedClients() > 0)
            {
                ims->Push(SerializeJPEGImage::PackDataOnlyWeb(jpegImage), false);
                //std::cout<<"Image services launched "<<std::endl;
            }
        }
    };
    imagesend_thread = std::thread(imagesender);
}
/*void SplotchServer::notify_interface(InterfaceType it)
{
  using namespace rapidjson;
  // Create interface object
  Value v(kObjectType);
  v = make_interface_descriptor(it, rjdoc);
  // Send to all users
  commands.send_client_message(BroadcastId(), commands.client_notify(rjdoc, "cmd_interface_desc", std::move(v)));
}
   */
/* Add the current image to the buffer for sending*/
void SplotchServer::send_image(const char *img, int count)
{
    // Push back image buffer

    image_buffer.resize(count);
    image_buffer.assign(img, img + count);
    if (server_active && image_modified)
    {
        ims_send_queue.Push(image_buffer);
    }
}
void SampleApp::WriteMPIBuffer(uint8_t *imageData, int l, int count)
{

    char str[16];

    sprintf(str, "%d%s", l, "_headless.ppm");
    const char *filename = str;
    pipe->SaveImage(filename, (char *)imageData, count, ren.m_Extent.width, ren.m_Extent.height);

    DEBUG_LOG << "Framebuffer image saved " << str << std::endl;
}

void SampleApp::CopyMPIBuffer(int l, uint8_t *img)
{

    const uint8_t *imagedata;

    // Create the linear tiled destination image to copy to and to read the memory from
    VkImageCreateInfo imgCreateInfo{};
    imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imgCreateInfo.extent.width = ren.m_Extent.width;
    imgCreateInfo.extent.height = ren.m_Extent.height;
    imgCreateInfo.extent.depth = 1;
    imgCreateInfo.arrayLayers = 1;
    imgCreateInfo.mipLevels = 1;
    imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
    imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    // Create the image
    VkImage dstImage;
    CHECK_VK(vkCreateImage(context.m_device, &imgCreateInfo, nullptr, &dstImage));
    // Create memory to back up the image
    VkMemoryRequirements memRequirements;
    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkDeviceMemory dstImageMemory;
    vkGetImageMemoryRequirements(context.m_device, dstImage, &memRequirements);
    memAllocInfo.allocationSize = memRequirements.size;
    // Memory must be host visible to copy from
    memAllocInfo.memoryTypeIndex = pv::getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context.m_physicalDevice);
    CHECK_VK(vkAllocateMemory(context.m_device, &memAllocInfo, nullptr, &dstImageMemory));
    CHECK_VK(vkBindImageMemory(context.m_device, dstImage, dstImageMemory, 0));
    // Do the actual blit from the offscreen image to our host visible destination image
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = pv::commandBufferAllocateInfo(pipe->m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    VkCommandBuffer copyCmd;
    CHECK_VK(vkAllocateCommandBuffers(context.m_device, &cmdBufAllocateInfo, &copyCmd));
    VkCommandBufferBeginInfo cmdBufInfo{};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    CHECK_VK(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));

    // Transition destination image to transfer destination layout
    pv::InsertImageMemoryBarrier(
        copyCmd,
        dstImage,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    // colorAttachment.image is already in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, and does not need to be transitioned

    VkImageCopy imageCopyRegion{};
    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.srcSubresource.layerCount = 1;
    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.dstSubresource.layerCount = 1;
    imageCopyRegion.extent.width = ren.m_Extent.width;
    imageCopyRegion.extent.height = ren.m_Extent.height;
    imageCopyRegion.extent.depth = 1;

    vkCmdCopyImage(
        copyCmd,
        ren.m_Images[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageCopyRegion);

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    pv::InsertImageMemoryBarrier(
        copyCmd,
        dstImage,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    CHECK_VK(vkEndCommandBuffer(copyCmd));

    pv::submitWork(copyCmd, context.m_queueGCT, context.m_device);

    // Get layout of the image (including row pitch)
    VkImageSubresource subResource{};
    subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkSubresourceLayout subResourceLayout;

    vkGetImageSubresourceLayout(context.m_device, dstImage, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    vkMapMemory(context.m_device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void **)&imagedata);

    //loop through data
    uint8_t *outImData = new uint8_t[4 * ren.m_Extent.width * ren.m_Extent.height];
    uint8_t *pOut = &outImData[0];

    imagedata += subResourceLayout.offset;
    //std::cout<<"start copying image buffer"<<ren.m_Extent.height<<", "<<ren.m_Extent.width<<std::endl;
    //ComputeTF

    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    // Check if source is BGR and needs swizzle
    std::vector<VkFormat> formatsBGR = {VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM};
    const bool colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), VK_FORMAT_R8G8B8A8_UNORM) != formatsBGR.end());

    // ppm binary pixel data
    for (int32_t y = 0; y < ren.m_Extent.height; y++)
    {
        unsigned char *row = (unsigned char *)imagedata;
        for (int32_t x = 0; x < ren.m_Extent.width; x++)
        {
            if (colorSwizzle)
            {
                //file.write((char*)row + 2, 1);
                memcpy(pOut, (char *)row + 3, 1);
                memcpy(pOut, (char *)row + 2, 1);
                //file.write((char*)row + 1, 1);
                memcpy(pOut, (char *)row + 1, 1);
                //file.write((char*)row, 1);
                memcpy(pOut, (char *)row, 1);
                //printf("%x\n",(char*)row);
                //std::cout<<(char*)row<<std::endl;
            }
            else
            {
                //file.write((char*)row, 3);
                memcpy(pOut, (char *)row, 4);
                //printf("%d\n",row);
            }
            row += 4;  //for int
            pOut += 4; //for char
        }
        imagedata += subResourceLayout.rowPitch;
    }
    //std::cout<<"Conventionally copied"<<std::endl;
    /**/

    // Clean up resources
    vkUnmapMemory(context.m_device, dstImageMemory);

    vkFreeMemory(context.m_device, dstImageMemory, nullptr);
    vkDestroyImage(context.m_device, dstImage, nullptr);
    vkQueueWaitIdle(context.m_queueGCT);
    memcpy(img, outImData, 4 * ren.m_Extent.width * ren.m_Extent.height);
    // return outImData;
}

void SampleApp::CopyRGBMPIBuffer(int l, uint8_t *outImData)
{

    const uint8_t *imagedata;

    // Create the linear tiled destination image to copy to and to read the memory from
    VkImageCreateInfo imgCreateInfo{};
    imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imgCreateInfo.extent.width = ren.m_Extent.width;
    imgCreateInfo.extent.height = ren.m_Extent.height;
    imgCreateInfo.extent.depth = 1;
    imgCreateInfo.arrayLayers = 1;
    imgCreateInfo.mipLevels = 1;
    imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
    imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    // Create the image
    VkImage dstImage;
    CHECK_VK(vkCreateImage(context.m_device, &imgCreateInfo, nullptr, &dstImage));
    // Create memory to back up the image
    VkMemoryRequirements memRequirements;
    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkDeviceMemory dstImageMemory;
    vkGetImageMemoryRequirements(context.m_device, dstImage, &memRequirements);
    memAllocInfo.allocationSize = memRequirements.size;
    // Memory must be host visible to copy from
    memAllocInfo.memoryTypeIndex = pv::getMemoryTypeIndex(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context.m_physicalDevice);
    CHECK_VK(vkAllocateMemory(context.m_device, &memAllocInfo, nullptr, &dstImageMemory));
    CHECK_VK(vkBindImageMemory(context.m_device, dstImage, dstImageMemory, 0));

    // Do the actual blit from the offscreen image to our host visible destination image
    VkCommandBufferAllocateInfo cmdBufAllocateInfo = pv::commandBufferAllocateInfo(pipe->m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
    VkCommandBuffer copyCmd;
    CHECK_VK(vkAllocateCommandBuffers(context.m_device, &cmdBufAllocateInfo, &copyCmd));
    VkCommandBufferBeginInfo cmdBufInfo{};
    cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    CHECK_VK(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));

    // Transition destination image to transfer destination layout
    pv::InsertImageMemoryBarrier(
        copyCmd,
        dstImage,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    // colorAttachment.image is already in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, and does not need to be transitioned

    VkImageCopy imageCopyRegion{};
    imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.srcSubresource.layerCount = 1;
    imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageCopyRegion.dstSubresource.layerCount = 1;
    imageCopyRegion.extent.width = ren.m_Extent.width;
    imageCopyRegion.extent.height = ren.m_Extent.height;
    imageCopyRegion.extent.depth = 1;

    vkCmdCopyImage(
        copyCmd,
        ren.m_Images[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &imageCopyRegion);

    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    pv::InsertImageMemoryBarrier(
        copyCmd,
        dstImage,
        VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_MEMORY_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    CHECK_VK(vkEndCommandBuffer(copyCmd));

    pv::submitWork(copyCmd, context.m_queueGCT, context.m_device);

    // Get layout of the image (including row pitch)
    VkImageSubresource subResource{};
    subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    VkSubresourceLayout subResourceLayout;

    vkGetImageSubresourceLayout(context.m_device, dstImage, &subResource, &subResourceLayout);

    // Map image memory so we can start copying from it
    vkMapMemory(context.m_device, dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void **)&imagedata);

    //loop through data
    //uint8_t *outImData = new uint8_t[3 * ren.m_Extent.width * ren.m_Extent.height];
    uint8_t *pOut = &outImData[0];

    imagedata += subResourceLayout.offset;

    //ComputeTF

    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    // Check if source is BGR and needs swizzle
    std::vector<VkFormat> formatsBGR = {VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM};
    const bool colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), VK_FORMAT_R8G8B8A8_UNORM) != formatsBGR.end());

    // ppm binary pixel data
    for (int32_t y = 0; y < ren.m_Extent.height; y++)
    {
        unsigned char *row = (unsigned char *)imagedata;
        for (int32_t x = 0; x < ren.m_Extent.width; x++)
        {
            if (colorSwizzle)
            {
                //file.write((char*)row + 2, 1);

                memcpy(pOut, (char *)row + 2, 1);
                //file.write((char*)row + 1, 1);
                memcpy(pOut, (char *)row + 1, 1);
                //file.write((char*)row, 1);
                memcpy(pOut, (char *)row, 1);
                //printf("%x\n",(char*)row);
                //std::cout<<(char*)row<<std::endl;
            }
            else
            {
                //file.write((char*)row, 3);
                memcpy(pOut, (char *)row, 3);
                //printf("%d\n",row);
            }
            row += 4;  //for int
            pOut += 3; //for char
        }
        imagedata += subResourceLayout.rowPitch;
    }
    // std::cout << "Conventionally copied" << std::endl;
    /**/

    // Clean up resources
    vkUnmapMemory(context.m_device, dstImageMemory);

    vkFreeMemory(context.m_device, dstImageMemory, nullptr);
    vkDestroyImage(context.m_device, dstImage, nullptr);
    vkQueueWaitIdle(context.m_queueGCT);
    //memcpy(img, outImData, 3 * ren.m_Extent.width * ren.m_Extent.height);
    // return outImData;
}

void SampleApp::cleanupSwapChain()
{
    for (auto framebuffer : ren.m_swapChainFramebuffers)
    {
        vkDestroyFramebuffer(context.m_device, framebuffer, nullptr);
    }
    DEBUG_LOG << "Framebuffers cleanded" << std::endl;

    pipe->CleanUp(context);
    DEBUG_LOG << "Pipe cleanded" << std::endl;

    ren.CleanUp(context);
    DEBUG_LOG << "Camera cleaned" << std::endl;

    for (size_t i = 0; i < ren.m_Images.size(); i++)
    {
        m_cam.CleanUp(context.m_device, i);
    }

    vkDestroyDescriptorPool(context.m_device, pipe->m_descriptorPool, nullptr);
}

void SampleApp::recreateSwapChain()
{
    DEBUG_LOG << "recreated as far " << std::endl;
    int width = 0, height = 0;
    glfwGetFramebufferSize(win.GetWindow(), &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(win.GetWindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(context.m_device);
    DEBUG_LOG << "recreated as far " << std::endl;
    cleanupSwapChain();
    DEBUG_LOG << "recreated as far " << std::endl;
#ifdef USE_GLFW
    ren.CreateSwapChain(context);
#endif

    ren.CreateImageViews(context);
    ren.CreateRenderPass(context);
    pipe->CreateGraphicsPipeline(context, ren);
    DEBUG_LOG << "recreated as far " << std::endl;
    ren.CreateFramebuffers(context);
    m_cam.CreateUniformBuffers(context.m_device, context.m_physicalDevice, ren.m_Images.size());
    pipe->CreateDescriptorPool(context, ren.m_Images.size());
    pipe->CreateDescriptorSets(context, ren.m_Images.size(), m_cam);
    if (this->m_pipeType == pv2::PipelineTYPE::Rasterisation)
        pipe->CreateCommandBuffers(context, ren, pLoader.m_vertexBuffer, m_vsize);

    imagesInFlight.resize(ren.m_Images.size(), VK_NULL_HANDLE);
}

void SampleApp::drawHeadless(int imageIndex)
{
    //Consider moving to renderer

    //vkWaitForFences(context.m_device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    rotate_x += 10;

    DEBUG_LOG << "Start drawing frame " << std::endl;
    //This is non interactive so updete via controller
    //m_cam.UpdateUniformBuffer(context.m_device, imageIndex, model_scale, ren.m_Extent.width, ren.m_Extent.height, rotate_x, rotate_y);
    server.camera.UpdateUniformBuffer(context.m_device, imageIndex, model_scale, ren.m_Extent.width, ren.m_Extent.height, rotate_x, rotate_y);

    DEBUG_LOG << "Buffer is updated " << std::endl;

    pipe->CreateCommandBuffers(context, ren, pLoader.m_vertexBuffer, m_vsize);

    //imagesInFlight.resize(ren.m_Images.size(), VK_NULL_HANDLE);
    /*if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
                 vkWaitForFences(context.m_device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
             }
             imagesInFlight[imageIndex] = inFlightFences[currentFrame];*/

    /*DEBUG_LOG<<"Submit info start"<<std::endl;
             VkSubmitInfo submitInfo {};
             submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
         
             VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
             VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
             submitInfo.waitSemaphoreCount = 1;
             submitInfo.pWaitSemaphores = waitSemaphores;
             submitInfo.pWaitDstStageMask = waitStages;
         
             submitInfo.commandBufferCount = 1;
             
            
             submitInfo.pCommandBuffers = &pipe->m_commandBuffers[imageIndex];
         
             VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
             submitInfo.signalSemaphoreCount = 1;
             submitInfo.pSignalSemaphores = signalSemaphores;
         
             vkResetFences(context.m_device, 1, &inFlightFences[currentFrame]);
             std::cout<<"Current frame updated "<<currentFrame<<std::endl;
             if (vkQueueSubmit(context.m_queueGCT, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
                 throw std::runtime_error("failed to submit draw command buffer!");
             }*/

    DEBUG_LOG << "Queeu submitted " << std::endl;
    //currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void SampleApp::generateResultOfMPI()
{
    //create an new staff for execution

    //images.resize(0);
    vkDeviceWaitIdle(context.m_device);
    DEBUG_LOG << "Start processing" << std::endl;

    //Initialise other postprocessing pipeline
    //post_process->SetCommandPool(pipe->m_commandPool);
    post_process->SetSize(ren.m_Extent.width, ren.m_Extent.height);

    post_process->CreateMPIDescriptorSetLayout(context);
    post_process->CreateMPIGraphicsPipeline(context, ren);

    //TODO: Create vertex ,buffer, Texture
    // pLoader.CreateVertexBuffer(pipe->m_commandPool, context.m_device, context.m_physicalDevice, context.m_queueGCT); //would be different for ray-tracing
    DEBUG_LOG << "All preliminary commands executed " << std::endl;

    /*                     int ts=(world_size-2)*3+2;
                     for (int i=0;i<ts;i++)
                     {
                         if(i==0)
                         images.push_back(std::get<1>(mpi_images[i]));
                         else {
                           if(i!=ts-1){
                           images.push_back(std::get<1>(mpi_images[jj]));
                           images.push_back(std::get<1>(mpi_images[jj]));
                           images.push_back(std::get<1>(mpi_images[jj]));
                           jj++;
                           } else 
                           images.push_back(std::get<1>(mpi_images[world_size-1]));
                         }
                     }
                     images.resize(ts);


*/

    int ts = world_size;
    for (int jj = 0; jj < world_size; jj++)
    {
        images[jj] = std::get<1>(mpi_images[jj]);
    }
    //images[world_size]=std::get<1>(mpi_images[world_size-1]);
    //ts=ts+3;
    DEBUG_LOG << "Total Images " << ts << std::endl;
    // images.resize(ts);
    DEBUG_LOG << "Images size " << images.size() << std::endl;
    int numSl = world_size;
    post_process->GenerateSlices(numSl); //world_size);

    int texWidth = ren.m_Extent.width;
    int texHeight = ren.m_Extent.height;
    int channel = 4;

    post_process->SetImageFormat(VK_FORMAT_R8G8B8A8_UNORM, channel);
    post_process->prepareNoiseTexture(context, ren.m_Extent.width, ren.m_Extent.height, ts, images.data());
    //updateNoiseTexture(pv2::Context context,uint8_t** pixel2)
    post_process->CreateVertexBuffer(context);
    post_process->CreateIndexBuffer(context);

    //m_cam.CreateUniformBuffers(context.m_device, context.m_physicalDevice, ren.m_Images.size());

    //TODO: check correct creation of those
    post_process->CreateMPIDescriptorPool(context, ren.m_Images.size());
    //TODO: pass vertex and imageViews
    // DEBUG_LOG << "Successfull as far " << ren.m_Images.size() <<std::endl;

    post_process->CreateMPIDescriptorSets(context, ren.m_Images.size(), m_cam); // are defined after layout
    DEBUG_LOG << "Successfull as far " << std::endl;

    //TODO: submit vertex to command buffer
    post_process->CreateCommandBuffers(context);

    vkResetCommandBuffer(post_process->m_commandBuffers[0], 0);
    post_process->RecordCommandBuffer(ren);

    post_process->submitBuffers(context, 0);

    DEBUG_LOG << "PostProcessing Queeu submitted " << std::endl;
    //currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void SampleApp::updateResultOfMPI()
{
    //create an new staff for execution

    //images.resize(0);
    vkDeviceWaitIdle(context.m_device);

    /*                     int ts=(world_size-2)*3+2;
                     for (int i=0;i<ts;i++)
                     {
                         if(i==0)
                         images.push_back(std::get<1>(mpi_images[i]));
                         else {
                           if(i!=ts-1){
                           images.push_back(std::get<1>(mpi_images[jj]));
                           images.push_back(std::get<1>(mpi_images[jj]));
                           images.push_back(std::get<1>(mpi_images[jj]));
                           jj++;
                           } else 
                           images.push_back(std::get<1>(mpi_images[world_size-1]));
                         }
                     }
                     images.resize(ts);


*/

    int ts = world_size;
    for (int jj = 0; jj < world_size; jj++)
    {
        images[jj] = std::get<1>(mpi_images[jj]);
    }
    //images[world_size]=std::get<1>(mpi_images[world_size-1]);
    //ts=ts+3;
    DEBUG_LOG << "Total Images " << ts << std::endl;
    // images.resize(ts);
    DEBUG_LOG << "Images size " << images.size() << std::endl;
    int numSl = world_size;

    //post_process->prepareNoiseTexture(context,ren.m_Extent.width,ren.m_Extent.height,ts,images.data());
    post_process->updateNoiseTexture(context, images.data());

    //TODO: submit vertex to command buffer
    // post_process->CreateCommandBuffers(context);

    //vkResetCommandBuffer(post_process->m_commandBuffers[0],  0);
    //post_process->RecordCommandBuffer(ren);

    post_process->submitBuffers(context, 0);

    // DEBUG_LOG<<"PostProcessing Queeu u "<<std::endl;
    //currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void SampleApp::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(ren.m_Images.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(context.m_device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS || vkCreateSemaphore(context.m_device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS || vkCreateFence(context.m_device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

//Consider moving to renderer

void SampleApp::drawFrame()
{

    vkWaitForFences(context.m_device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(context.m_device, ren.m_swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        DEBUG_LOG << "Swap recriation " << std::endl;
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    DEBUG_LOG << "Start drawing next image " << imageIndex << std::endl;
    m_cam.UpdateUniformBuffer(context.m_device, imageIndex, model_scale, ren.m_Extent.width, ren.m_Extent.height, rotate_x, rotate_y);
    DEBUG_LOG << "Buffer is updated " << std::endl;
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(context.m_device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    DEBUG_LOG << "Submit info start" << std::endl;
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;

    DEBUG_LOG << "Error here as command buffers are not implemented in ray-tracing pipe " << std::endl;
    submitInfo.pCommandBuffers = &pipe->m_commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(context.m_device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(context.m_queueGCT, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    DEBUG_LOG << "Current frame " << currentFrame << std::endl;

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {ren.m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(context.m_queueP, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
