project "MasterServer"
	kind "ConsoleApp"
	language "C++"
	location "./"
	targetname "MasterServer"
	
	files {
			"%{cfg.location}/main.cpp",
			"%{cfg.location}/networkmanager.h",
			"%{cfg.location}/networkmanager.cpp"
		}
	
	includedirs {
				"../thirdparty/enet/include",
				"../public"
		}
	
	links {
			"enet",
			"helperlib" --Project
		}
	
	filter { "system:Windows" } --Are these compiler specific?
		links {
				"ws2_32",
				"winmm",
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
		targetdir "../../build/x86/Debug"
		libdirs {
				"../lib/thirdparty/x86/debug"
			}
		
	filter { "platforms:Win32", "configurations:Release" }
		targetdir "../../build/x86/Release"
		libdirs {
				"../lib/thirdparty/x86"
			}
	
	filter { "platforms:Win64", "configurations:Debug" }
		targetdir "../../build/x64/Debug"
		libdirs {
				"../lib/thirdparty/x64/debug"
			}
	
	filter { "platforms:Win64", "configurations:Release" }
		targetdir "../../build/x64/Release"
		libdirs {
				"../lib/thirdparty/x64"
			}
	
	filter { "system:Windows" }
		defines { "_CRT_SECURE_NO_WARNINGS" }
	
	filter { "platforms:Win32" }
		libdirs {
				"../lib/public/x86"
			}	
	
	filter { "platforms:Win64" }
		libdirs {
				"../lib/public/x64"
			}