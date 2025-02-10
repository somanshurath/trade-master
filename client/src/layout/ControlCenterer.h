#include "imgui.h"

class CenteredControlWrapper {
public:
    explicit CenteredControlWrapper(bool result) : result_(result) {}

    operator bool() const {
        return result_;
    }

private:
    bool result_;
};

class ControlCenterer {
public:
    ControlCenterer(ImVec2 windowSize) : windowSize_(windowSize) {}

    template<typename Func>
    CenteredControlWrapper operator()(Func control) const {
        ImVec2 originalPos = ImGui::GetCursorPos();

        // Draw offscreen to calculate size
        ImGui::SetCursorPos(ImVec2(-10000.0f, -10000.0f));
        control();
        ImVec2 controlSize = ImGui::GetItemRectSize();

        ImGui::SetCursorPos(ImVec2((windowSize_.x - controlSize.x) * 0.5f, originalPos.y));
        control();

        return CenteredControlWrapper(ImGui::IsItemClicked());
    }

private:
    ImVec2 windowSize_;
};

#define CENTERED_CONTROL(control) ControlCenterer{ImGui::GetWindowSize()}([&]() { control; })
