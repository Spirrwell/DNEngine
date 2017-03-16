workspace "Duke3DEngine"
	configurations { "Debug", "Release" }
	filter { "system:Windows" }
		platforms { "Win32", "Win64" }
		toolset "v140_xp"
	
project "Engine"
	kind "SharedLib"
	language "C++"
	location "src/engine"
	targetname "engine"
	
	files { 
			"%{cfg.location}/anim.h",
			"%{cfg.location}/anim.cpp",
			"%{cfg.location}/engine.h",
			"%{cfg.location}/engine.cpp",
			"%{cfg.location}/engine_handler.h",
			"%{cfg.location}/engine_handler.cpp",
			"%{cfg.location}/enginemain.cpp",
			"%{cfg.location}/sound.h",
			"%{cfg.location}/sound.cpp",
			"%{cfg.location}/soundsystem.h",
			"%{cfg.location}/soundsystem.cpp"
		}
	
	includedirs {
				"src/thirdparty/FMOD/inc",
				"src/thirdparty/glew-2.0.0/include",
				"src/thirdparty/glm",
				"src/thirdparty/SDL-2.0.5/include",
				"src/public",
				"src/public/engine"
		}
		
	links {
			"SDL2main",
			"SDL2",
			"opengl32",
			"glew32",
			"helperlib" --This will link the project as a build dependency, but for some reason won't link the library itself
		}
	
	filter { "system:Windows" }
		links { "helperlib.lib" } --Specify .lib directly so premake actually links the library, not just the project
		debugcommand "Game.exe"
	
	filter { "configurations:Debug"	}
		symbols "On"
		
	filter { "configurations:Release" }
		optimize "On"
	
	filter { "platforms:Win32" }
		system "Windows"
		architecture "x32"
		debugdir "game/bin/x86"
	
	filter { "platforms:Win32", "configurations:Debug" }
		targetdir "build/x86/Debug"
		libdirs { "src/lib/thirdparty/x86/debug" }
		links { "fmodL_vc" }
	
	filter { "platforms:Win32", "configurations:Release" }
		targetdir "build/x86/Release"
		links { "fmod_vc" }
	
	filter { "platforms:Win64" }
		system "Windows"
		architecture "x64"
		debugdir "game/bin/x64"
	
	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "build/x64/Debug"
		libdirs { "src/lib/thirdparty/x64/debug" }
		links { "fmodL64_vc" }
	
	filter { "platforms:Win64", "configurations:Release" }
		targetdir "build/x64/Release"
		links { "fmod64_vc" }
		
	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS" }
		
	filter { "platforms:Win32" }
		libdirs {
				"src/lib/thirdparty/x86",
				"src/lib/public/x86"
			}
		
	filter { "platforms:Win64" }
		libdirs {
				"src/lib/thirdparty/x64",
				"src/lib/public/x64"
			}
	
	filter { "action:vs*", "platforms:Win32" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/bin/x86\" /s /i /y", "xcopy \"$(TargetDir)$(TargetName).lib\" \"../lib/public/x86\" /s /i /y" }
	
	filter { "action:vs*", "platforms:Win64" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/bin/x64\" /s /i /y", "xcopy \"$(TargetDir)$(TargetName).lib\" \"../lib/public/x64\" /s /i /y" }
		
project "Game"
	kind "ConsoleApp" --Change to WindowedApp at some point
	language "C++"
	location "src/game"
	targetname "Game"
	
	files {
			"%{cfg.location}/game.cpp",
			"%{cfg.location}/game.h",
			"%{cfg.location}/main.cpp",
			"src/public/game/igame.h"
		}
		
	includedirs {
				"src/public",
				"src/public/game"
		}
	
	links {
			"Engine", --Project
			"helperlib" --Project
		}
	
	filter { "system:Windows" } --Are these compiler specific?
		files {
				"%{cfg.location}/resource.h",
				"%{cfg.location}/game.ico",
				"%{cfg.location}/Game.rc"
			}
		links {
				"engine.lib",
				"helperlib.lib"
			}
		debugcommand "Game.exe"
	
	filter { "configurations:Debug"	}
		symbols "On"
		
	filter { "configurations:Release" }
		optimize "On"
	
	filter { "platforms:Win32", "configurations:Debug" }
		targetdir "build/x86/Debug"
	
	filter { "platforms:Win32", "configurations:Release" }
		targetdir "build/x86/Release"

	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "build/x64/Debug"
	
	filter { "platforms:Win64", "configurations:Release" }
		targetdir "build/x64/Release"
	
	filter { "platforms:Win32" }
		system "Windows"
		architecture "x32"
		debugdir "game/bin/x86"
	
	filter { "platforms:Win64" }
		system "Windows"
		architecture "x64"
		debugdir "game/bin/x64"
	
	filter { "platforms:Win32" }
		libdirs {
				"src/lib/public/x86"
			}
		
	filter { "platforms:Win64" }
		libdirs {
				"src/lib/public/x64"
			}
	
	filter { "action:vs*", "platforms:Win32" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/bin/x86\" /s /i /y" }
	
	filter { "action:vs*", "platforms:Win64" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/bin/x64\" /s /i /y" }
		
project "helperlib"
	kind "StaticLib"
	language "C++"
	location "src/helperlib"
	targetname "helperlib"
	
	files { 
			"%{cfg.location}/basehelpers.cpp",
			"%{cfg.location}/grp.cpp",
			"%{cfg.location}/memoryfile.cpp",
			"src/public/basehelpers.h",
			"src/public/grp.h",
			"src/public/memoryfile.h"
		}
		
	includedirs {
				"src/public"
		}
	
	filter { "configurations:Debug"	}
		symbols "On"
		
	filter { "configurations:Release" }
		optimize "On"
	
	filter { "platforms:Win32" }
		system "Windows"
		architecture "x32"
	
	filter { "platforms:Win64" }
		system "Windows"
		architecture "x64"
		
	filter { "platforms:Win32", "configurations:Debug" }
		targetdir "build/x86/Debug"
		
	filter { "platforms:Win32", "configurations:Release" }
		targetdir "build/x86/Release"
	
	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "build/x64/Debug"
	
	filter { "platforms:Win64", "configurations:Release" }
		targetdir "build/x64/Release"
	
	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS" }
	
	filter { "action:vs*", "platforms:Win32" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/public/x86\" /s /i /y" }
	
	filter { "action:vs*", "platforms:Win64" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/public/x64\" /s /i /y" }
		

project "MasterServer"
	kind "ConsoleApp"
	language "C++"
	location "src/masterserver"
	targetname "MasterServer"
	
	files {
			"%{cfg.location}/main.cpp",
			"%{cfg.location}/networkmanager.h",
			"%{cfg.location}/networkmanager.cpp"
		}
	
	includedirs {
				"src/thirdparty/enet/include",
				"src/public"
		}
	
	links {
			"ws2_32",
			"winmm",
			"enet",
			"helperlib" --Project
		}
	
	filter { "system:Windows" } --Are these compiler specific?
		links {
				"helperlib.lib"
			}
		debugcommand "MasterServer.exe"
	
	filter { "configurations:Debug"	}
		symbols "On"
		
	filter { "configurations:Release" }
		optimize "On"
	
	filter { "platforms:Win32" }
		system "Windows"
		architecture "x32"
	
	filter { "platforms:Win64" }
		system "Windows"
		architecture "x64"
	
	filter { "platforms:Win32", "configurations:Debug" }
		targetdir "build/x86/Debug"
		libdirs {
				"src/lib/thirdparty/x86/debug"
			}
		
	filter { "platforms:Win32", "configurations:Release" }
		targetdir "build/x86/Release"
		libdirs {
				"src/lib/thirdparty/x86"
			}
	
	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "build/x64/Debug"
		libdirs {
				"src/lib/thirdparty/x64/debug"
			}
	
	filter { "platforms:Win64", "configurations:Release" }
		targetdir "build/x64/Release"
		libdirs {
				"src/lib/thirdparty/x64"
			}
	
	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS" }
	
	filter { "platforms:Win32" }
		libdirs {
				"src/lib/public/x86"
			}	
	
	filter { "platforms:Win64" }
		libdirs {
				"src/lib/public/x64"
			}