#ifdef SELFDELETE_EXPORTS
#define SELFDELETE_API __declspec(dllexport)
#else
#define SELFDELETE_API __declspec(dllimport)
#endif

#include <string>
#include "pch.h"


extern HMODULE current_module;
extern std::wstring dll_path;

extern "C" SELFDELETE_API bool delete_using_fls_callbacks();

extern "C" SELFDELETE_API bool delete_usig_registry_notification();
