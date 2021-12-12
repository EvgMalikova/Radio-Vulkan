#ifndef WINDOW
#define WINDOW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <stdexcept>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class WindowBase {
public:
    WindowBase() { m_framebufferResized = false; };
    ~WindowBase() {};

    void CleanUp();
    void InitWindow();
    GLFWwindow* GetWindow() { return m_window; };
    void SetFameBuffeResized(bool val)
    {
        m_framebufferResized = val;
    }

private:
    GLFWwindow* m_window;
    bool m_framebufferResized;
};

#endif //