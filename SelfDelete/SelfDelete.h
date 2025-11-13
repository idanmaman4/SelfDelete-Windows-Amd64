#ifdef SELFDELETE_EXPORTS
#define SELFDELETE_API __declspec(dllexport)
#else
#define SELFDELETE_API __declspec(dllimport)
#endif

#include <string>
#include "pch.h"


extern HMODULE current_module;
extern std::wstring dll_path;

extern "C" SELFDELETE_API bool delete_using_fls_callbacks(void);

extern "C" SELFDELETE_API bool delete_usig_registry_notification(void);

extern "C"  SELFDELETE_API bool delete_using_apc(void);
extern "C"  SELFDELETE_API bool delete_using_thread(void);
extern "C"  SELFDELETE_API bool delete_using_process_lolbin1(void);

extern "C" SELFDELETE_API bool delete_using_process_lolbin2(void);

extern "C" SELFDELETE_API bool delete_using_timers(void);
extern "C" SELFDELETE_API bool delete_using_rtl_register_wait(void);
extern "C" SELFDELETE_API bool delete_using_rtl_queue_work_item(void);



