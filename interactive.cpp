#include <pipeline.hpp>
#include <window.hpp>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "loaders.hpp"
#include <optional>
#include <set>

#include <vulkan/vulkan.h>

#include "cxxsupport/vec3.h"
#include "kernel/colour.h"
#include "kernel/colourmap.h"

//#include <limits.h>
//#include <fstream>
#include "fitsReader.h"
#include "reader/reader.h"

#include <chrono>

#include <tclap/CmdLine.h>

#ifdef USE_MPI
#include <mpi.h>
#endif
#ifdef USE_NVVK
#include "nvvk/context_vk.hpp"
#include <nvvk/descriptorsets_vk.hpp>
#include <nvvk/error_vk.hpp>
#include <nvvk/resourceallocator_vk.hpp> // For NVVK memory allocators
#include <nvvk/shaders_vk.hpp> // For nvvk::createShaderModule
#include <nvvk/structs_vk.hpp>
#endif

//#ifdef LOG_All
//const bool enableValidationLayers = false;
//#else
//const bool enableValidationLayers = true;
//#endif

static float mouse_x, mouse_y;
static float rotate_x, rotate_y;
static bool Xrotate;
static float mouse_prev_x, mouse_prev_y;
static float hval, vval;
static float model_scale;

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

    if (yoffset > 0)
        DEBUG_LOG << "scrolled up" << std::endl;

    else
        DEBUG_LOG << "scrolled down" << std::endl;
    model_scale += yoffset;
    DEBUG_LOG << model_scale << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void cursor_pos_callback(GLFWwindow* window, double x, double y)
{
    // here your code
    mouse_prev_x = mouse_x;
    mouse_prev_y = mouse_y;
    mouse_x = x;
    mouse_y = y;
    if (Xrotate) {

        //glfwGetWindowSize(&width, &height);

        rotate_x = mouse_x - mouse_prev_x;
        rotate_y = mouse_y - mouse_prev_y;
        hval = 10.0f * (float)(mouse_x - mouse_prev_x) / (float)WIDTH;
        vval = 10.0f * (float)(mouse_y - mouse_prev_y) / (float)HEIGHT;

        DEBUG_LOG << "mouse " << rotate_x << " , " << rotate_y << std::endl;
    } else {
        rotate_x = 0;
        rotate_y = 0;
    }
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    int whichkey = 0; //Left 1 Right 2 Middle 3
    if (action == GLFW_PRESS) {
        switch (button) {
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
    } else {
        switch (button) {
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

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto winB = reinterpret_cast<WindowBase*>(glfwGetWindowUserPointer(window));
    winB->SetFameBuffeResized(true);
}

const int MAX_FRAMES_IN_FLIGHT = 2;

class SampleApp {
public:
    void SetFileName(std::string filename)
    {
        m_fitsFilename = filename;
    }
    void run()
    {
        hval = 0;
        vval = 0;
        initWindowAndCallbacks();
        initRenderingPipe();
        mainLoop();
        cleanup();
    }

    std::string m_fitsFilename;

private:
    enum MouseButton {
        MOUSE_BUTTON_LEFT = 0,
        MOUSE_BUTTON_RIGHT = 1,
        MOUSE_BUTTON_MIDDLE = 2,
        NUM_MOUSE_BUTTONIDX,
    };

    SimpCamera m_cam;
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
    PipelineRasterize pipe;

   

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    int m_vsize;

    bool framebufferResized = false;

    void initWindowAndCallbacks()
    {
        win.InitWindow();
        glm::vec3 eyePos = glm::vec3(1.0f, 1.0f, 0.0f);
        glm::vec3 focusPos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

        m_cam = SimpCamera(eyePos, focusPos, up);

        mouse_x = 0;
        mouse_y = 0;
        rotate_x = 0;
        rotate_y = 0;
        hval = 0;
        vval = 0;

        model_scale = 1.0;
        glfwSetFramebufferSizeCallback(win.GetWindow(), framebufferResizeCallback);
        glfwSetCursorPosCallback(win.GetWindow(), cursor_pos_callback);
        glfwSetScrollCallback(win.GetWindow(), scroll_callback);
        glfwSetMouseButtonCallback(win.GetWindow(), mouse_button_callback);

        glfwSetKeyCallback(win.GetWindow(), key_callback);
    }

    void initRenderingPipe()
    {
        m_vsize = 0;
        context.SetInteractive(true);
        context.Initialize(); //allocates all necessary extensions

        ren.SetWindow(win.GetWindow());
        ren.Initialize(context); //creates a surface and deals with swap chains if necessary

        context.PickPhysicalDevice(ren.m_surface);
        context.CreateLogicalDevice(ren.m_surface);
        //Main particle loader

        pLoader.SetFileName(m_fitsFilename);
        pLoader.LoadData(1, 0);

        ren.CreateSwapChain(context);
        ren.CreateImageViews(context);
        ren.CreateRenderPass(context);

        pipe.SetSize(ren.m_Extent.width, ren.m_Extent.height);

        pipe.CreateDescriptorSetLayout(context);
        pipe.CreateGraphicsPipeline(context, ren);

        ren.CreateFramebuffers(context);
        pipe.CreateCommandPool(context, ren);

        pLoader.CreateVertexBuffer(pipe.m_commandPool, context.m_device, context.m_physicalDevice, context.m_queueGCT); //would be different for ray-tracing
        m_vsize = pLoader.vertices.size();

        m_cam.CreateUniformBuffers(context.m_device, context.m_physicalDevice, ren.m_Images.size());
        pipe.CreateDescriptorPool(context,ren.m_Images.size());
        pipe.CreateDescriptorSets(context, ren.m_Images.size(), m_cam);
        pipe.CreateCommandBuffers(context, ren, pLoader.m_vertexBuffer, m_vsize);
        createSyncObjects();
    }

    void mainLoop()
    {
        while (!glfwWindowShouldClose(win.GetWindow())) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(context.m_device);
    }

    void cleanupSwapChain()
    {
        for (auto framebuffer : ren.m_swapChainFramebuffers) {
            vkDestroyFramebuffer(context.m_device, framebuffer, nullptr);
        }

        
        pipe.CleanUp(context);

        ren.CleanUp(context);

        for (size_t i = 0; i < ren.m_Images.size(); i++) {
            m_cam.CleanUp(context.m_device, i);
        }

        vkDestroyDescriptorPool(context.m_device, pipe.m_descriptorPool, nullptr);
    }

    void cleanup()
    {
        cleanupSwapChain();
        

        vkDestroyDescriptorSetLayout(context.m_device, pipe.m_descriptorSetLayout, nullptr);

        //vkDestroyBuffer(device, indexBuffer, nullptr);
        //vkFreeMemory(device, indexBufferMemory, nullptr);
        pLoader.CleanUp(context.m_device);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(context.m_device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(context.m_device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(context.m_device, inFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(context.m_device, pipe.m_commandPool, nullptr);

        vkDestroyDevice(context.m_device, nullptr);

        context.CleanUp();

        vkDestroySurfaceKHR(context.m_instance, ren.m_surface, nullptr);
        vkDestroyInstance(context.m_instance, nullptr);

        win.CleanUp();
    }

    void recreateSwapChain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(win.GetWindow(), &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(win.GetWindow(), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(context.m_device);

        cleanupSwapChain();

        ren.CreateSwapChain(context);

        ren.CreateImageViews(context);
        ren.CreateRenderPass(context);
        pipe.CreateGraphicsPipeline(context, ren);
        ren.CreateFramebuffers(context);
        m_cam.CreateUniformBuffers(context.m_device, context.m_physicalDevice, ren.m_Images.size());
        pipe.CreateDescriptorPool(context,ren.m_Images.size());
        pipe.CreateDescriptorSets(context, ren.m_Images.size(), m_cam);
        pipe.CreateCommandBuffers(context, ren, pLoader.m_vertexBuffer, m_vsize);

        imagesInFlight.resize(ren.m_Images.size(), VK_NULL_HANDLE);
    }

   

 

    void createSyncObjects()
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(ren.m_Images.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(context.m_device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS || vkCreateSemaphore(context.m_device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS || vkCreateFence(context.m_device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    //Consider moving to renderer

    void drawFrame()
    {
        vkWaitForFences(context.m_device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(context.m_device, ren.m_swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        m_cam.UpdateUniformBuffer(context.m_device, imageIndex, model_scale, ren.m_Extent.width, ren.m_Extent.height, rotate_x, rotate_y);

        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(context.m_device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        VkSubmitInfo submitInfo {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &pipe.m_commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(context.m_device, 1, &inFlightFences[currentFrame]);

        if (vkQueueSubmit(context.m_queueGCT, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { ren.m_swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(context.m_queueP, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
};

int main(int argc, char** argv)
{
    TCLAP::CmdLine cmd("Command description message", ' ', "0.9");

    TCLAP::ValueArg<std::string> nameArg("f", "filename", "Datacube filename", true, "LVHIS027.na.icln.fits", "string");
    cmd.add(nameArg);
    // Parse the argv array.
    cmd.parse(argc, argv);

    // Get the value parsed by each arg.
    std::string FitsFileName = nameArg.getValue();

    SampleApp app;
    app.SetFileName(FitsFileName);

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
