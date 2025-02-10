-- premake5.lua
workspace "client"
architecture "x64"
configurations {"Debug", "Release", "Dist"}
startproject "TradeMaster-Client"

-- Workspace-wide build options for MSVC
filter "system:windows"
buildoptions {"/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/utf-8"}

defines {"IMGUI_DEFINE_MATH_OPERATORS"}

-- Directories
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
   include "external/imgui"
group ""

group "App"
include "client/build-client.lua"
group ""
