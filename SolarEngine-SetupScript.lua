workspace "SolarEngine"
    architecture "x64"
    configurations { "Release", "Debug" }
    startproject "SolarEngine"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "SolarEngine"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    systemversion "latest"

    targetdir ("Build/" .. outputdir .. "/%{prj.name}")
    objdir ("Build/Intermediates/" .. outputdir .. "/%{prj.name}")

    
    includedirs {
        "Vendors",
        "Vendors/GLFW/include",
        "Vendors/imguii",
        "Vendors/zlib_x64-windows-static/include",
        "Vendors/JoltPhysics",
        "Vendors/Assimp/include",
        "Vendors/ImGui-Nodes/NodeEditor/Include",
    }

    files {
        "Source/**.h",
        "Source/**.cpp",
        "Source/**.hpp",
        "Source/**.c",
        "Vendors/stb_image/**.h",
        "Vendors/stb_image/**.cpp",
        "Vendors/imguii/**.h",
        "Vendors/imguii/**.cpp",
        "Vendors/glad.c",
        "Vendors/JoltPhysics/Jolt/**.h",
        "Vendors/JoltPhysics/Jolt/**.cpp",
    }
    
    -- Organize files under a filter called "ImGui" "stb_image"
    vpaths {
        ["Vendors/ImGui/*"] = { "Vendors/imguii/**.h", "Vendors/imguii/**.cpp" },
        ["Vendors/stb_image/*"] = { "Vendors/stb_image/**.h", "Vendors/stb_image/**.cpp" },
        ["Vendors/glad/*"] = { "Vendors/glad.c"},
        ["Vendors/JoltPhysics/*"] = { "Vendors/JoltPhysics/Jolt/**.h", "Vendors/JoltPhysics/Jolt/**.cpp" },
        ["SolarEngine/*"] = { "Source/**.h", "Source/**.cpp" }

    }
  
    libdirs {
        "Vendors/GLFW/lib-vc2022",
        "Vendors/zlib_x64-windows-static/lib",
        "Vendors/Assimp/lib",
        "Vendors/JoltPhysics/Jolt/lib"
    }

    links {
        "glfw3.lib",
        "opengl32.lib",
        "assimp-vc143-mt.lib",
        "zlib.lib",
    }

    defines {
    }

    filter "system:windows"
        systemversion "latest"
        defines { "PLATFORM_WINDOWS" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        optimize "Off"