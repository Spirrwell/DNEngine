project "Game"
	kind "ConsoleApp" --Change to WindowedApp at some point
	language "C++"
	location "./"
	targetname "Game"
	
	files {
			"%{cfg.location}/game.cpp",
			"%{cfg.location}/game.h",
			"%{cfg.location}/main.cpp",
			"../public/game/igame.h"
		}
		
	includedirs {
				"../public",
				"../public/game"
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
	
	filter { "configurations:Debug"	}
		symbols "On"
		
	filter { "configurations:Release" }
		optimize "On"
	
	filter { "platforms:Win32", "configurations:Debug" }
		targetdir "../../build/x86/Debug"
	
	filter { "platforms:Win32", "configurations:Release" }
		targetdir "../../build/x86/Release"

	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "../../build/x64/Debug"
	
	filter { "platforms:Win64", "configurations:Release" }
		targetdir "../../build/x64/Release"
	
	filter { "platforms:Win32" }
		system "Windows"
		architecture "x32"
		debugcommand "../../game/bin/x86/Game.exe"
		debugdir "../../game/bin/x86"
	
	filter { "platforms:Win64" }
		system "Windows"
		architecture "x64"
		debugcommand "../../game/bin/x64/Game.exe"
		debugdir "../../game/bin/x64"
	
	filter { "platforms:Win32" }
		libdirs {
				"../lib/public/x86"
			}
		
	filter { "platforms:Win64" }
		libdirs {
				"../lib/public/x64"
			}
	
	filter { "action:vs*", "platforms:Win32" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/bin/x86\" /s /i /y" }
	
	filter { "action:vs*", "platforms:Win64" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../../game/bin/x64\" /s /i /y" }