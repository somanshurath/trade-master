project "TradeMaster-Client"
kind "ConsoleApp"
language "C++"
cppdialect "C++11"
targetdir "bin/%{cfg.buildcfg}"

files {"src/**.h", "src/**.cpp", "../external/glad/glad.c", "../external/imgui/backends/imgui_impl_glfw.cpp",
       "../external/imgui/backends/imgui_impl_opengl3.cpp"}

includedirs {"../external/imgui", "../external/spdlog/include", "../external/glfw/include", "../external",
             "../external/websocketcpp", "../external/boost", "../external/asio/asio/include",
             "../external/json/include", "../external/openssl/include"}

libdirs {"../external/glfw/build/src/Debug",
         "..\\external\\boost\\build\\boost\\bin.v2\\libs\\system\\build\\msvc-14.3\\debug\\x86_64\\link-static\\runtime-link-static\\threading-multi",
         "..\\external\\boost\\build\\boost\\bin.v2\\libs\\filesystem\\build\\msvc-14.3\\debug\\x86_64\\link-static\\runtime-link-static\\threading-multi",
         "../runtime"}

links {"ImGui", "opengl32", "glfw3", "gdi32", "user32", "shell32", "libboost_system-vc143-mt-sgd-x64-1_88",
       "libboost_filesystem-vc143-mt-sgd-x64-1_88", "libssl", "libcrypto"}

targetdir("../bin/" .. outputdir .. "/%{prj.name}")
objdir("../bin-int/" .. outputdir .. "/%{prj.name}")

filter "system:windows"
systemversion "latest"
defines {"WL_PLATFORM_WINDOWS"}

filter "configurations:Debug"
defines {"WL_DEBUG"}
runtime "Debug"
symbols "On"

filter "configurations:Release"
defines {"WL_RELEASE"}
runtime "Release"
optimize "On"
symbols "On"

filter "configurations:Dist"
kind "WindowedApp"
defines {"WL_DIST"}
runtime "Release"
optimize "On"
symbols "Off"
