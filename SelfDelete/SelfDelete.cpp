#include "pch.h"
#include "framework.h"
#include "SelfDelete.h"

#include <memory>
#include <string>
#include <thread>
#include <iostream>
#include <format>
#include <filesystem>


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


/*
SELFDELETE_API bool delete_using_process_lolbin3(void) {
}




*/
typedef NTSTATUS(NTAPI* RtlRegisterWait_t)(
	PHANDLE WaitHandle,
	HANDLE Handle,
	PVOID Callback,
	PVOID Context,
	ULONG Milliseconds,
	ULONG Flags
	);

typedef DWORD (NTAPI* RtlDeregisterWait_t)(
	HANDLE WaitHandle
	);



typedef NTSTATUS (NTAPI* RtlQueueWorkItem_t)(IN WORKERCALLBACKFUNC Function,
	IN PVOID Context  OPTIONAL,
	IN ULONG Flags);

SELFDELETE_API bool delete_using_rtl_queue_work_item(void)
{
	MessageBoxA(NULL, "delete_using_rtl_queue_work_item", "delete_using_rtl_queue_work_item", MB_OK);



	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
	RtlRegisterWait_t RtlRegisterWait = (RtlRegisterWait_t)GetProcAddress(ntdll, "RtlRegisterWait");
	RtlDeregisterWait_t RtlDeregisterWait = (RtlDeregisterWait_t)GetProcAddress(ntdll, "RtlDeregisterWait");

	RtlQueueWorkItem_t RtlQueueWorkItem = (RtlQueueWorkItem_t)GetProcAddress(ntdll, "RtlQueueWorkItem");


	void* data = new wchar_t[256];
	GetModuleFileNameW(::current_module, (LPWSTR)data, 255);

	if (RtlQueueWorkItem((WORKERCALLBACKFUNC)&FreeLibrary, (PVOID)::current_module, WT_EXECUTEDEFAULT) != 0) {
		return false; 
	}

	if (RtlQueueWorkItem((WORKERCALLBACKFUNC)&DeleteFileW, (PVOID)data, WT_EXECUTEDEFAULT) != 0) {
		return false;
	}

	return true;
}

SELFDELETE_API bool delete_using_rtl_register_wait(void)
{
	MessageBoxA(NULL, "delete_using_rtl_register_wait", "delete_using_rtl_register_wait", MB_OK);

	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
	RtlRegisterWait_t RtlRegisterWait = (RtlRegisterWait_t)GetProcAddress(ntdll, "RtlRegisterWait");
	RtlDeregisterWait_t RtlDeregisterWait = (RtlDeregisterWait_t)GetProcAddress(ntdll, "RtlDeregisterWait");
	void* data = new wchar_t[256];
	GetModuleFileNameW(::current_module, (LPWSTR)data, 255);
	HANDLE hEvent1 = CreateEventW(NULL, FALSE, FALSE, NULL);
	if (!hEvent1) {
		return false;
	}
	HANDLE hEvent2 = CreateEventW(NULL, FALSE, FALSE, NULL);
	if (!hEvent2) {
		return false;
	}
	HANDLE hWait1 = NULL;
	HANDLE hWait2 = NULL;

	for (size_t i = 0; i < 50; i++) {
		auto status2 = RtlRegisterWait(
			&hWait1,
			hEvent1,
			DeleteFileW,
			(LPVOID)data,
			INFINITE,
			WT_EXECUTEONLYONCE
		);

	}
	auto status1 = RtlRegisterWait(
		&hWait2,
		hEvent2,
		FreeLibrary,
		::current_module,
		INFINITE,
		WT_EXECUTEONLYONCE
	);

	SetEvent(hEvent2);
	SetEvent(hEvent1);

	return true;
}

SELFDELETE_API bool delete_using_timers(void) {
	MessageBoxA(NULL, "delete_using_timers", "delete_using_timers", MB_OK);

	HANDLE timer_handle1 = CreateTimerQueue();
	HANDLE time_handle2 = CreateTimerQueue(); 
	void* data = new wchar_t[256];
	GetModuleFileNameW(::current_module, (LPWSTR)data, 255);


	BOOL timer2_status = CreateTimerQueueTimer(
		&timer_handle1,
		NULL,
		(WAITORTIMERCALLBACK)DeleteFileW,
		(LPVOID)data,
		3000,
		0,
		WT_EXECUTEINTIMERTHREAD
	);
	if (!timer2_status) {
		return false;
	}

	BOOL timer1_status = CreateTimerQueueTimer(
		&timer_handle1,
		NULL,
		(WAITORTIMERCALLBACK)FreeLibrary,
		(LPVOID)::current_module,
		20,
		0,
		WT_EXECUTEINTIMERTHREAD
	);

	return timer1_status;


} 

SELFDELETE_API bool delete_using_process_lolbin2(void) {
	MessageBoxA(NULL, "delete_using_process_lolbin2", "delete_using_process_lolbin2", MB_OK);

	std::wstring dll_path;
	dll_path.resize(256);
	GetModuleFileNameW(::current_module, const_cast<LPWSTR>(dll_path.c_str()), dll_path.size());
	std::filesystem::path dll_path_fs = dll_path;

	std::wstring command = std::format(L" powershell.exe -c 'Start-Sleep -Milliseconds 500 ; Remove-Item _Path \"{}\" -Force '", dll_path_fs.c_str());

	STARTUPINFO startupinfo;
	PROCESS_INFORMATION processinfo;

	CreateProcessW(
		NULL,
		const_cast<LPWSTR>(command.c_str()),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&startupinfo,
		&processinfo
	);

	FreeLibraryAndExitThread(::current_module, 1);

	return true;
}

SELFDELETE_API bool delete_using_process_lolbin1(void) {
	MessageBoxA(NULL, "delete_using_process_lolbin1", "delete_using_process_lolbin1", MB_OK);

	std::wstring dll_path;
	dll_path.resize(256);
	GetModuleFileNameW(::current_module, const_cast<LPWSTR>(dll_path.c_str()), dll_path.size());
	std::filesystem::path dll_path_fs = dll_path;
	
	std::wstring command = std::format(L"cmd.exe /C ping 1.1.1.1 -n 1 -w 3000 > Nul & Del /f /q \"{}\"", dll_path_fs.c_str());

	STARTUPINFO startupinfo;
	PROCESS_INFORMATION processinfo;

	CreateProcessW(
		NULL,
		const_cast<LPWSTR>(command.c_str()),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&startupinfo,
		&processinfo
	);

	FreeLibraryAndExitThread(::current_module, 1);
	
	return true;
}

SELFDELETE_API bool delete_using_thread(void) {
	// Doing problem with WOW+JIT :(
	MessageBoxA(NULL, "delete_using_thread", "delete_using_thread", MB_OK);
	void* data = new wchar_t[256];
	GetModuleFileNameW(::current_module, (LPWSTR)data, 255);

	
	HANDLE freelib_thread_handle =  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, (LPVOID)::current_module, CREATE_SUSPENDED, NULL);
	HANDLE deletefile_thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DeleteFileW, (LPVOID)data, CREATE_SUSPENDED, NULL);
	HANDLE deletefile2_thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DeleteFileW, (LPVOID)data, CREATE_SUSPENDED, NULL);
	HANDLE sleep_thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Sleep, (LPVOID)3000, CREATE_SUSPENDED, NULL);

	DWORD_PTR mask = 1; // CPU 0

	SetThreadAffinityMask(freelib_thread_handle, mask);
	SetThreadAffinityMask(deletefile_thread_handle, mask);
	SetThreadAffinityMask(sleep_thread_handle, mask);
	SetThreadAffinityMask(deletefile2_thread_handle, mask);

	ResumeThread(freelib_thread_handle);
	ResumeThread(deletefile2_thread_handle);
	ResumeThread(sleep_thread_handle);
	ResumeThread(deletefile_thread_handle);

	return true;
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
	}
	QueueUserAPC((PAPCFUNC)DeleteFileW, new_thread_handle, (ULONG_PTR)data);
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
		printf("FlsAlloc %d - %S\n", fls_index, (LPWSTR)(data));
	}
	FreeLibraryAndExitThread(::current_module, 1);
	return true;
}

// UNSTABLE!
SELFDELETE_API bool delete_usig_registry_notification(void)
{
	// Crashing the process for some reason :(
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
	auto NtNotifyChangeKey_x = reinterpret_cast<NtNotifyChangeKey_t>(GetProcAddress(ntdll_module, "NtNotifyChangeKey"));
	
	for (size_t i = 0; i < 200; i++)
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
	for (size_t i = 0; i < 8; i++)
		NtNotifyChangeKey_x(
			hKey,
			NULL,
			Sleep,
			(LPVOID)200,
			&iosb,
			REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
			TRUE,
			NULL,
			0,
			TRUE
		);
	
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
		SleepEx(1, TRUE);
	}

	return 0;
}

SELFDELETE_API bool delete_using_rop_64bit_arm(void){
}
