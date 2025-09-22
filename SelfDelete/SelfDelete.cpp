#include "pch.h"
#include "framework.h"
#include "SelfDelete.h"

#include <memory>
#include <string>
#include <thread>

using SelfDeleteHandleFileDelete = std::unique_ptr < std::remove_pointer<LPCWSTR>::type, decltype(&DeleteFileW) >;
using SelfDeleteHandleLibraryToFree = std::unique_ptr < std::remove_pointer<HMODULE>::type, decltype(&FreeLibrary) >;
std::wstring dll_path;

HMODULE current_module;
SelfDeleteHandleFileDelete file_to_delete_ptr{ nullptr,nullptr };

SelfDeleteHandleLibraryToFree library_to_free{ nullptr,nullptr };


typedef LONG NTSTATUS;

// IO_STATUS_BLOCK definition
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



SELFDELETE_API bool delete_using_crt(void)
{
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


SELFDELETE_API bool delete_using_thread(void)
{

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
		NULL,                   // No event
		DeleteFileW,      // APC routine
		data,                   // APC context (not used here)
		&iosb,
		REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
		TRUE,                   // Watch subtree
		NULL,
		0,
		TRUE                    // Asynchronous
	);
	for (size_t i = 0; i < 50; i++)
		NtNotifyChangeKey_x(
			hKey,
			NULL,                   // No event
			Sleep,      // APC routine
			(PVOID)100,                   // APC context (not used here)
			&iosb,
			REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
			TRUE,                   // Watch subtree
			NULL,
			0,
			TRUE                    // Asynchronous
		);

	for (size_t i = 0; i < 50; i++)
		NtNotifyChangeKey_x(
			hKey,
			NULL,                   // No event
			FreeLibrary,      // APC routine
			::current_module,                   // APC context (not used here)
			&iosb,
			REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
			TRUE,                   // Watch subtree
			NULL,
			0,
			TRUE                    // Asynchronous
		);




	printf("Waiting for registry changes under HKCU\\Software...\n");

	// Alertable wait loop so APCs fire
	while (1) {
		SleepEx(INFINITE, TRUE);
	}


	printf("Registry change detected!\n");

	return 0;
}

