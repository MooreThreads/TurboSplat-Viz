#pragma once
#if _WIN32
	#ifdef DLLIMPORT
		#define GAME_MODULE_API __declspec(dllimport)
	#else
		#define GAME_MODULE_API __declspec(dllexport)
	#endif
	#else
	#define GAME_MODULE_API extern
#endif