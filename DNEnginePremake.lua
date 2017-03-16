workspace "Duke3DEngine"
	configurations { "Debug", "Release" }
	filter { "system:Windows" }
		platforms { "Win32", "Win64" }
		toolset "v140_xp"

include "src/engine/Engine.lua"
include "src/game/Game.lua"
include "src/helperlib/helperlib.lua"
include "src/masterserver/MasterServer.lua"