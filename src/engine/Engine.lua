project "Engine"
	kind "SharedLib"
	language "C++"
	location "./"
	targetname "engine"
	
	files { 
			"%{cfg.location}/anim.h",
			"%{cfg.location}/anim.cpp",
			"%{cfg.location}/baseshader.h",
			"%{cfg.location}/baseshader.cpp",
			"%{cfg.location}/engine.h",
			"%{cfg.location}/engine.cpp",
			"%{cfg.location}/engine_handler.h",
			"%{cfg.location}/engine_handler.cpp",
			"%{cfg.location}/enginemain.cpp",
			"%{cfg.location}/imesh.h",
			"%{cfg.location}/ishader.h",
			"%{cfg.location}/map.h",
			"%{cfg.location}/map.cpp",
			"%{cfg.location}/meshgl.h",
			"%{cfg.location}/meshgl.cpp",
			"%{cfg.location}/meshvulkan.h",
			"%{cfg.location}/renderer.h",
			"%{cfg.location}/renderer.cpp",
			"%{cfg.location}/sdlmgr.h",
			"%{cfg.location}/sdlmgr.cpp",
			"%{cfg.location}/shadergl.h",
			"%{cfg.location}/shadergl.cpp",
			"%{cfg.location}/shadersystem.h",
			"%{cfg.location}/shadersystem.cpp",
			"%{cfg.location}/sound.h",
			"%{cfg.location}/sound.cpp",
			"%{cfg.location}/soundsystem.h",
			"%{cfg.location}/soundsystem.cpp",
			"%{cfg.location}/vulkanmgr.h",
			"%{cfg.location}/vulkanmgr.cpp"
		}
	
	includedirs {
				"../thirdparty/FMOD/inc",
				"../thirdparty/glew-2.0.0/include",
				"../thirdparty/glm",
				"../thirdparty/SDL-2.0.5/include",
				"../thirdparty/vulkan/Include",
				"../public",
				"../public/engine"
		}
		
	links {
			"SDL2main",
			"SDL2",
			"opengl32",
			"glew32",
			"vulkan-1",
			"helperlib" --This will link the project as a build dependency, but for some reason won't link the library itself
		}
	
	filter { "system:Windows" }
		links { "helperlib.lib" } --Specify .lib directly so premake actually links the library, not just the project
	
	filter { "configurations:Debug"	}
		symbols "On"
		
	filter { "configurations:Release" }
		optimize "On"
	
	filter { "platforms:Win32" }
		system "Windows"
		architecture "x32"
		debugcommand "../../game/bin/x86/Game.exe"
		debugdir "../../game/bin/x86"
	
	filter { "platforms:Win32", "configurations:Debug" }
		targetdir "../../build/x86/Debug"
		libdirs { "../lib/thirdparty/x86/debug" }
		links { "fmodL_vc" }
	
	filter { "platforms:Win32", "configurations:Release" }
		targetdir "../../build/x86/Release"
		links { "fmod_vc" }
	
	filter { "platforms:Win64" }
		system "Windows"
		architecture "x64"
		debugcommand "../../game/bin/x64/Game.exe"
		debugdir "../../game/bin/x64"
	
	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "../../build/x64/Debug"
		libdirs { "../lib/thirdparty/x64/debug" }
		links { "fmodL64_vc" }
	
	filter { "platforms:Win64", "configurations:Release" }
		targetdir "../../build/x64/Release"
		links { "fmod64_vc" }
		
	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS" }
		
	filter { "platforms:Win32" }
		libdirs {
				"../lib/thirdparty/x86",
				"../lib/public/x86"
			}
		
	filter { "platforms:Win64" }
		libdirs {
				"../lib/thirdparty/x64",
				"../lib/public/x64"
			}
	
	filter { "action:vs*", "platforms:Win32" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/bin/x86\" /s /i /y", "xcopy \"$(TargetDir)$(TargetName).lib\" \"../lib/public/x86\" /s /i /y" }
	
	filter { "action:vs*", "platforms:Win64" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/bin/x64\" /s /i /y", "xcopy \"$(TargetDir)$(TargetName).lib\" \"../lib/public/x64\" /s /i /y" }