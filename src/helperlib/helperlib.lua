project "helperlib"
	kind "StaticLib"
	language "C++"
	location "./"
	targetname "helperlib"
	
	files { 
			"%{cfg.location}/basehelpers.cpp",
			"%{cfg.location}/grp.cpp",
			"%{cfg.location}/memoryfile.cpp",
			"../public/basehelpers.h",
			"../public/grp.h",
			"../public/memoryfile.h"
		}
		
	includedirs {
				"../public"
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
		targetdir "../../build/x86/Debug"
		
	filter { "platforms:Win32", "configurations:Release" }
		targetdir "../../build/x86/Release"
	
	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "../../build/x64/Debug"
	
	filter { "platforms:Win64", "configurations:Release" }
		targetdir "../../build/x64/Release"
	
	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS" }
	
	filter { "action:vs*", "platforms:Win32" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/public/x86\" /s /i /y" }
	
	filter { "action:vs*", "platforms:Win64" }
		postbuildcommands { "xcopy \"$(TargetDir)$(TargetFileName)\" \"../lib/public/x64\" /s /i /y" }