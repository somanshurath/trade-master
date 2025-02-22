#pragma once
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "glad/glad.h"
#include "examples/libs/glfw/include/GLFW/glfw3.h"
#include "../network/WebSocketHandler.h"
#include "../ui/ControlPanel.h"
#include "../ui/Misc/Login.h"

class AppLayer {
private:
    GLFWwindow* m_Window;
    const char* m_GlslVersion;
    WebSocketHandler m_WsClient;
    Login m_Login;
    ControlPanel m_ControlPanel;

    static void GlfwErrorCallback(int error, const char* description);
    static void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void SetImGuiStyle();

public:
    AppLayer();
    ~AppLayer();

    bool OnStart();
    void OnUIRender();
    void OnExit();
    
    bool IsRunning() const;
    void Run();

    bool firstLoad = true;
};
