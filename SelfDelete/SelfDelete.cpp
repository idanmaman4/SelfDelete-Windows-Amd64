#include "pch.h"
#include "framework.h"
#include "SelfDelete.h"

#include <memory>
#include <string>
#include <thread>
#include <iostream>


std::wstring dll_path;
HMODULE current_module;


typedef LONG NTSTATUS;

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID    Pointer;
	};
	ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

typedef NTSTATUS(NTAPI* NtNotifyChangeKey_t)(
	HANDLE KeyHandle,
	HANDLE Event,
	PVOID ApcRoutine,
	PVOID ApcContext,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG CompletionFilter,
	BOOLEAN WatchSubtree,
	PVOID Buffer,
	ULONG BufferSize,
	BOOLEAN Asynchronous
	);



SELFDELETE_API bool delete_using_process_lolbin3(void) {
}

SELFDELETE_API bool delete_using_process_lolbin2(void) {
}

SELFDELETE_API bool delete_using_process_lolbin1(void) {
}

SELFDELETE_API bool delete_using_thread(void) {

}

SELFDELETE_API bool delete_using_apc(void) {
	MessageBoxA(NULL, "delete_using_apc", "delete_using_apc", MB_OK);
	auto func = [](LPVOID param) {SleepEx((DWORD)param, TRUE); };
	auto new_thread_handle = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)SleepEx, (LPVOID)INFINITE, CREATE_SUSPENDED, nullptr);
	void* data = new wchar_t[256];
	GetModuleFileNameW(::current_module, (LPWSTR)data, 255);
	std::string result_processor_count;
	result_processor_count.reserve(256);
	GetEnvironmentVariableA("NUMBER_OF_PROCESSORS",const_cast<char*> (result_processor_count.c_str()), result_processor_count.capacity());
	size_t processor_count = std::stoi(result_processor_count);
	QueueUserAPC((PAPCFUNC)FreeLibrary, new_thread_handle, (ULONG_PTR)::current_module);
	for (size_t i = 0; i < processor_count; i++) {
		QueueUserAPC((PAPCFUNC)Sleep, new_thread_handle, (ULONG_PTR)100);
		QueueUserAPC((PAPCFUNC)DeleteFileW, new_thread_handle, (ULONG_PTR)data);
	}
	ResumeThread(new_thread_handle);
	return true;
}

SELFDELETE_API bool delete_using_fls_callbacks(void)
{
	MessageBoxA(NULL, "delete_using_fls_callbacks", "delete_using_fls_callbacks", MB_OK);

	ConvertThreadToFiber(nullptr);
	void* data = new wchar_t[256];
	GetModuleFileNameW(::current_module, (LPWSTR)data, 255);
	for (size_t i = 0; i < 2000; i++) {
		DWORD fls_index = FlsAlloc((PFLS_CALLBACK_FUNCTION)&DeleteFileW);
		FlsSetValue(fls_index, data);
		printf("FlsAlloc %d - %S\n", fls_index , data);
	}
	FreeLibraryAndExitThread(::current_module, 1);
	return true;
}

SELFDELETE_API bool delete_usig_registry_notification(void)
{
	MessageBoxA(NULL, "delete_usig_registry_notification", "delete_usig_registry_notification", MB_OK);

	HKEY hKey;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software", 0, KEY_NOTIFY, &hKey) != ERROR_SUCCESS) {
		printf("Failed to open registry key\n");
		return 1;
	}

	IO_STATUS_BLOCK iosb;
	HMODULE ntdll_module = GetModuleHandleA("ntdll.dll");
	if (ntdll_module == nullptr) {
		puts("Error in getting ntdll");
		return false;
	}
	void* data = new wchar_t[256];
	GetModuleFileNameW(::current_module, (LPWSTR)data, 255);
	auto NtNotifyChangeKey_x = reinterpret_cast<NtNotifyChangeKey_t>(GetProcAddress(ntdll_module,"NtNotifyChangeKey"));

	NtNotifyChangeKey_x(
		hKey,
		NULL,           
		DeleteFileW,     
		data,                 
		&iosb,
		REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
		TRUE,           
		NULL,
		0,
		TRUE            
	);
	for (size_t i = 0; i < 50; i++)
		NtNotifyChangeKey_x(
			hKey,
			NULL,           
			Sleep,      
			(PVOID)100,                   
			&iosb,
			REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
			TRUE,           
			NULL,
			0,
			TRUE       
		);

	for (size_t i = 0; i < 50; i++)
		NtNotifyChangeKey_x(
			hKey,
			NULL,                 
			FreeLibrary,    
			::current_module,                
			&iosb,
			REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
			TRUE,                
			NULL,
			0,
			TRUE                   
		);

	while (1) {
		SleepEx(INFINITE, TRUE);
	}

	return 0;
}

