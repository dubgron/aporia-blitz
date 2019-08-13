#pragma once

#if defined(APORIA_WINDOWS)
#	if defined(APORIA_EXPORT)
#		define APORIA_API __declspec(dllexport)
#	elif defined(APORIA_IMPORT)
#		define APORIA_API __declspec(dllimport)
#	endif
#else
#	define APORIA_API
#endif
