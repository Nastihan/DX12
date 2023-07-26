#include "Window.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <stdexcept>

Window::Window(uint16_t width, uint16_t height, std::string title)
    : width(width), height(height)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    pGfx = std::make_unique<Graphics>(width, height, glfwGetWin32Window(pWindow));
}

Window::~Window()
{
    glfwDestroyWindow(pWindow);
    glfwTerminate();
}

Graphics& Window::Gfx()
{
    if (!pGfx)
        throw std::runtime_error("No Gfx");
    return *pGfx;
}


GLFWwindow& Window::Wnd()
{
    return *pWindow;
}

HWND& Window::Hwnd()
{
    HWND hwnd = glfwGetWin32Window(pWindow);
    return hwnd;
}
