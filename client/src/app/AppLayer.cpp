#include "./AppLayer.h"
#include "../utils/env/env.h"
#include "../utils/fonts/Fonts.h"

AppLayer::AppLayer()
    : m_GlslVersion("#version 130")
    , m_WsClient("wss://test.deribit.com/ws/api/v2")
    , m_Login(m_WsClient)
    , m_ControlPanel(m_WsClient)
{
    m_WsClient.SetUserId(USER_ID);
}

AppLayer::~AppLayer() = default;

void AppLayer::GlfwErrorCallback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void AppLayer::GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void AppLayer::SetImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 old_title_bg_active = style.Colors[ImGuiCol_TitleBgActive];
    style.Colors[ImGuiCol_TitleBg] = old_title_bg_active;
    style.Colors[ImGuiCol_TitleBgCollapsed] = old_title_bg_active;
}

bool AppLayer::OnStart() {
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit())
        return false;

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    m_Window = glfwCreateWindow(mode->width, mode->height, "Trade Master", NULL, NULL);
    if (!m_Window)
        return false;

    glfwMakeContextCurrent(m_Window);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    SetImGuiStyle();

    g_calibriFont = io.Fonts->AddFontFromFileTTF("./src/utils/fonts/calibri.ttf", 14.0f);
    g_iconsFont = io.Fonts->AddFontFromFileTTF("./src/utils/fonts/icons.ttf", 14.0f);

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init(m_GlslVersion);

    m_WsClient.Connect();
    return true;
}

void AppLayer::OnUIRender() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (m_WsClient.IsAuthenticated()) {
        m_ControlPanel.Render();
    } else {
        m_Login.Render();
    }

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    GLint last_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glUseProgram(last_program);
    
    glfwSwapBuffers(m_Window);
}

void AppLayer::OnExit() {
    m_WsClient.Close();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

bool AppLayer::IsRunning() const {
    return !glfwWindowShouldClose(m_Window);
}

void AppLayer::Run() {
    while (IsRunning()) {
        glfwPollEvents();
        OnUIRender();
    }
}
