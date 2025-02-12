#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "glad/glad.h"
#include "examples/libs/glfw/include/GLFW/glfw3.h"
#include <stdio.h>
#include <iostream>
#include "network/WebSocketClient.h"

// Forward declare callback functions
void glfw_error_callback(int error, const char *description);
void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

int main(int, char **)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return -1;

    // Create window with graphics context
    const char *glsl_version = "#version 130";
    GLFWwindow *window = glfwCreateWindow(1280, 720, "Trade Master", NULL, NULL);
    if (window == NULL)
        return -1;
    glfwMakeContextCurrent(window);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE); // Start window maximized
    glfwSwapInterval(1);                       // Enable vsync

    // Initialize OpenGL loader
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    WebSocketClient ws_client("wss://test.deribit.com/ws/api/v2");
    ws_client.Connect();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create a simple window
        ImGui::Begin("Trade Master Terminal");
        ImGui::Text("Client ID");
        static char client_id[128] = "";
        ImGui::InputText("##client_id", client_id, IM_ARRAYSIZE(client_id));
        ImGui::Text("Client Secret");
        static char client_secret[128] = "";
        ImGui::InputText("##client_secret", client_secret, IM_ARRAYSIZE(client_secret), ImGuiInputTextFlags_Password);
        ImGui::Spacing();
        if (ImGui::Button("Log In"))
        {
            ws_client.Authenticate(client_id, client_secret);
        }
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
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
