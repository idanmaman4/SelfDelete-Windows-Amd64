// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "SelfDelete.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        
        ::current_module = hModule;
        ::dll_path.resize(MAX_PATH + 1, L'\x00');
        GetModuleFileNameW(hModule, const_cast<LPWSTR>(dll_path.c_str()), MAX_PATH);


    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

