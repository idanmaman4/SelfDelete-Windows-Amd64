#ifdef SELFDELETE_EXPORTS
#define SELFDELETE_API __declspec(dllexport)
#else
#define SELFDELETE_API __declspec(dllimport)
#endif

#include <string>
#include "pch.h"


extern HMODULE current_module;
extern std::wstring dll_path;

extern "C" SELFDELETE_API bool delete_using_crt();

extern "C" SELFDELETE_API bool delete_using_thread();
