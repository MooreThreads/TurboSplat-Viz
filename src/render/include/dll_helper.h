#pragma once
#if _WIN32
	#ifdef DLLIMPORT
		#define RENDER_MODULE_API __declspec(dllimport)
	#else
		#define RENDER_MODULE_API __declspec(dllexport)
	#endif
	#else
	#define RENDER_MODULE_API extern
#endif