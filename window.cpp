#include "window.hpp"

void WindowBase::CleanUp()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}
void WindowBase::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan test app", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}