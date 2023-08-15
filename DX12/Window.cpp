#include "Window.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <stdexcept>
#include "imgui/imgui_impl_glfw.h"

Window::Window(uint16_t width, uint16_t height, std::string title)
    : width(width), height(height)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    
    glfwGetCursorPos(pWindow, reinterpret_cast<double*>(& lastMouseX),reinterpret_cast<double*>(& lastMouseY));

    // init imgui glfw
    ImGui_ImplGlfw_InitForOther(pWindow, true);
    // graphics object
    pGfx = std::make_unique<Graphics>(width, height, Hwnd());
}

Window::~Window()
{
    ImGui_ImplGlfw_Shutdown();
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
    if (!hwnd)
    {
        hwnd = glfwGetWin32Window(pWindow);
    }
    return hwnd;
}

void Window::EnableCursor()
{
    cursorEnabled = true;
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::DisableCursor()
{
    cursorEnabled = false;
    glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool Window::CursorEnabled()
{
    return cursorEnabled;
}

DirectX::XMFLOAT2 Window::GetMouseDelta(float x, float y)
{
    DirectX::XMFLOAT2 mouseDelta{};
    mouseDelta.x = x - lastMouseX;
    mouseDelta.y = y - lastMouseY;
    lastMouseX = x;
    lastMouseY = y;
    return mouseDelta;
}
