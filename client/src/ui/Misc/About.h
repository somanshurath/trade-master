#pragma once
#include <imgui.h>

class About
{
public:
    About() {}

    void Render()
    {
        ImGui::Begin("Wecome to Trade Master");
        ImGui::Text("Real-time market data and trading platform");
        ImGui::Text("Developed by: somanshurath");
        ImGui::Text("Version: 1.0.0");
        ImGui::End();
    }
};
