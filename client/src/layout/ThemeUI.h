#pragma once

#include <imgui.h>

constexpr auto accentColor = IM_COL32(236, 158, 36, 255);
constexpr auto highlightColor = IM_COL32(39, 185, 242, 255);
constexpr auto niceBlueColor = IM_COL32(83, 232, 254, 255);
constexpr auto complimentColor = IM_COL32(78, 151, 166, 255);
constexpr auto backgroundColor = IM_COL32(36, 36, 36, 255);
constexpr auto backgroundDarkColor = IM_COL32(26, 26, 26, 255);
constexpr auto titlebarColor = IM_COL32(21, 21, 21, 255);
constexpr auto propertyFieldColor = IM_COL32(15, 15, 15, 255);
constexpr auto textColor = IM_COL32(192, 192, 192, 255);
constexpr auto textBrighterColor = IM_COL32(210, 210, 210, 255);
constexpr auto textDarkerColor = IM_COL32(128, 128, 128, 255);
constexpr auto textErrorColor = IM_COL32(230, 51, 51, 255);
constexpr auto mutedColor = IM_COL32(77, 77, 77, 255);
constexpr auto groupHeaderColor = IM_COL32(47, 47, 47, 255);
constexpr auto selectionColor = IM_COL32(237, 192, 119, 255);
constexpr auto selectionMutedColor = IM_COL32(237, 201, 142, 23);
constexpr auto backgroundPopupColor = IM_COL32(50, 50, 50, 255);
constexpr auto errorColor = IM_COL32(222, 43, 43, 255);
constexpr auto validPrefabColor = IM_COL32(82, 179, 222, 255);
constexpr auto invalidPrefabColor = IM_COL32(222, 43, 43, 255);
constexpr auto missingMeshColor = IM_COL32(230, 102, 76, 255);
constexpr auto meshNotSetColor = IM_COL32(250, 101, 23, 255);

ImVec4 GetColorFromImCol32(ImU32 color)
{
    return ImVec4(
        (float)((color & 0x00FF0000) >> 16) / 255.0f,
        (float)((color & 0x0000FF00) >> 8) / 255.0f,
        (float)((color & 0x000000FF)) / 255.0f,
        (float)((color & 0xFF000000) >> 24) / 255.0f);
}
