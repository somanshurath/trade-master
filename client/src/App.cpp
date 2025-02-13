#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "glad/glad.h"
#include "examples/libs/glfw/include/GLFW/glfw3.h"
#include <stdio.h>
#include <iostream>
#include <future>

#include "network/WebSocketHandler.h"
#include "utils/env/env.h"
#include "utils/fonts/Fonts.h"

#include "ui/ControlPanel.h"
#include "ui/Landing/Login.h"

void glfw_error_callback(int error, const char *description);
void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

int main(int, char **)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return -1;

    const char *glsl_version = "#version 130";
    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);
    GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "Trade Master", NULL, NULL);
    if (window == NULL)
        return -1;
    glfwMakeContextCurrent(window);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); // Start window maximized
    glfwSwapInterval(1);                       // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    ImVec4 old_title_bg_active = style.Colors[ImGuiCol_TitleBgActive];
    style.Colors[ImGuiCol_TitleBg] = old_title_bg_active;
    style.Colors[ImGuiCol_TitleBgCollapsed] = old_title_bg_active;

    g_calibriFont = io.Fonts->AddFontFromFileTTF("./src/utils/fonts/calibri.ttf", 14.0f);
    g_iconsFont = io.Fonts->AddFontFromFileTTF("./src/utils/fonts/icons.ttf", 14.0f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    WebSocketHandler ws_client("wss://test.deribit.com/ws/api/v2");
    ws_client.SetUserId(USER_ID);
    ws_client.Connect();

    Login login(ws_client);
    ControlPanel control_panel(ws_client);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (ws_client.IsAuthenticated())
        {
            control_panel.Render();
        }
        else
        {
            login.Render();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        GLint last_program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glUseProgram(last_program);
        glfwSwapBuffers(window);
    }

    ws_client.Close();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
