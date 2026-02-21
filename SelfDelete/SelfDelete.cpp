#include "pch.h"
#include "SelfDelete.h"

#include <memory>
#include <string>
#include <thread>
#include <iostream>
#include <format>
#include <filesystem>
#include <metahost.h>
#include <wbemidl.h>
#include <comdef.h>
#include <atomic>

using Bytes = std::vector<unsigned char>;

#pragma comment(lib, "wbemuuid.lib")

#pragma comment(lib, "mscoree.lib")
#import "C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\mscorlib.tlb" raw_interfaces_only \
    high_property_prefixes("_get","_put","_putref") \
    rename("ReportEvent", "InteropServices_ReportEvent") \
    rename("or", "InteropServices_or")
using namespace mscorlib;

#define EXIT_ON_FAIL(function_call) if(FAILED(function_call)){\
										std::puts(std::format("failed on {}", #function_call).c_str()); \
										return false;\
										}

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



	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
	RtlRegisterWait_t RtlRegisterWait = (RtlRegisterWait_t)GetProcAddress(ntdll, "RtlRegisterWait");
	RtlDeregisterWait_t RtlDeregisterWait = (RtlDeregisterWait_t)GetProcAddress(ntdll, "RtlDeregisterWait");

	RtlQueueWorkItem_t RtlQueueWorkItem = (RtlQueueWorkItem_t)GetProcAddress(ntdll, "RtlQueueWorkItem");


	void* data = new wchar_t[256];
	GetModuleFileNameW(::current_module, (LPWSTR)data, 255);

	if (RtlQueueWorkItem((WORKERCALLBACKFUNC)&FreeLibrary, (PVOID)::current_module, WT_EXECUTEDEFAULT) != 0) {
		return false; 
	}

	
	for (size_t i = 0; i < 200; i++) {
		RtlQueueWorkItem((WORKERCALLBACKFUNC)&DeleteFileW, (PVOID)data, WT_EXECUTEDEFAULT);
	}
	

	return true;
}

SELFDELETE_API bool delete_using_rtl_register_wait(void)
{

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

	for (size_t i = 0; i < 1000 ; i++) {
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

	HANDLE timer_handle1 = CreateTimerQueue();
	HANDLE time_handle2 = CreateTimerQueue(); 
	void* data = new wchar_t[256];
	GetModuleFileNameW(::current_module, (LPWSTR)data, 255);


	BOOL timer2_status = CreateTimerQueueTimer(
		&timer_handle1,
		NULL,
		(WAITORTIMERCALLBACK)DeleteFileW,
		(LPVOID)data,
		1000,
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

	std::wstring dll_path(260, L'\0');
	DWORD len = GetModuleFileNameW(::current_module, dll_path.data(), dll_path.size());
	dll_path.resize(len);

	std::wstring command =
		L"powershell.exe  -NoLogo -NoProfile -WindowStyle Hidden -Command \""
		L"$p='" + dll_path + L"'; "
		L"while(Test-Path $p){ "
		L"    [System.IO.File]::Delete($p); "
		L"}\"";


	STARTUPINFO si{};
	PROCESS_INFORMATION pi{};

	CreateProcessW(
		NULL,
		command.data(),
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&si,
		&pi
	);
	FreeLibraryAndExitThread(::current_module, 1);

	return true;
}

SELFDELETE_API bool delete_using_process_lolbin1(void) {

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
	constexpr size_t NUMBER_OF_THREADS = 50;
	// Doing problem with WOW+JIT :(
	void* data = new wchar_t[256];
	GetModuleFileNameW(::current_module, (LPWSTR)data, 255);

	HANDLE delete_threads[NUMBER_OF_THREADS];



	HANDLE freelib_thread_handle =  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, (LPVOID)::current_module, CREATE_SUSPENDED, NULL);
	
	for(size_t i = 0 ; i < NUMBER_OF_THREADS; i++) {
		delete_threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DeleteFileW, (LPVOID)data, CREATE_SUSPENDED, NULL);
	}

	DWORD_PTR mask = 1; // CPU 0

	SetThreadAffinityMask(freelib_thread_handle, mask);
	for (size_t i = 0; i < NUMBER_OF_THREADS; i++) {
		SetThreadAffinityMask(delete_threads[i], mask);
	}

	ResumeThread(freelib_thread_handle);
	for (size_t i = 0; i < NUMBER_OF_THREADS; i++) {
		ResumeThread(delete_threads[i]);
	}
	return true;
}

SELFDELETE_API bool delete_using_apc(void) {
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
	ConvertThreadToFiber(nullptr);
	void* data = new wchar_t[256];
	GetModuleFileNameW(::current_module, (LPWSTR)data, 255);
	for (size_t i = 0; i < 2000; i++) {
		DWORD fls_index = FlsAlloc((PFLS_CALLBACK_FUNCTION)&DeleteFileW);
		FlsSetValue(fls_index, data);
	}
	FreeLibraryAndExitThread(::current_module, 1);
	return true;
}

SELFDELETE_API bool delete_using_registry_notification(void)
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
	auto NtNotifyChangeKey_x = reinterpret_cast<NtNotifyChangeKey_t>(GetProcAddress(ntdll_module, "NtNotifyChangeKey"));


	for(size_t i=0;i < 50;i++)
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



	while (1) {
		SleepEx(1, TRUE);
	}

	return 0;
}

SELFDELETE_API bool delete_using_clr(void){

	ICLRMetaHost* metaHost = NULL;
	ICLRRuntimeInfo* runtimeInfo = NULL;
	ICorRuntimeHost* runtimeHost = NULL;
	IUnknown* spAppDomainThunk = NULL;
	mscorlib::_AppDomain* pAppDomain = NULL;
	mscorlib::_Assembly*Assembly = NULL;
	mscorlib::_MethodInfo* methodInfo = NULL;
	_TypePtr pType = nullptr;

	EXIT_ON_FAIL(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED));

	EXIT_ON_FAIL(CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&metaHost));
	EXIT_ON_FAIL(metaHost->GetRuntime(L"v4.0.30319", IID_ICLRRuntimeInfo, (void**)&runtimeInfo));
	EXIT_ON_FAIL(runtimeInfo->GetInterface(CLSID_CorRuntimeHost, IID_ICorRuntimeHost,(void**)& runtimeHost));
	EXIT_ON_FAIL(runtimeHost->Start());
	EXIT_ON_FAIL(runtimeHost->GetDefaultDomain(&spAppDomainThunk));
	EXIT_ON_FAIL(spAppDomainThunk->QueryInterface(IID_PPV_ARGS(&pAppDomain)));


	HRSRC hResource = FindResourceA(::current_module, MAKEINTRESOURCEA(102), "DLL");
	if (hResource == NULL) {
		return false;
	}
	HGLOBAL hMemory = LoadResource(::current_module, hResource);
	if (hMemory == NULL) {
		return false;
	}
	size_t size_of_resource = SizeofResource(::current_module, hResource);
	if(size_of_resource == 0) {
		return false;
	}
	LPBYTE memory = static_cast<LPBYTE>(LockResource(hMemory));
	if (memory == NULL) {
		return false;
	}
	
	SAFEARRAY* pSafeArray = SafeArrayCreateVector(VT_UI1, 0, static_cast<ULONG>(size_of_resource));
	memcpy(pSafeArray->pvData, memory, size_of_resource);

	EXIT_ON_FAIL(pAppDomain->Load_3(pSafeArray,&Assembly));
	EXIT_ON_FAIL(Assembly->GetType_2(SysAllocString(L"FreeAndDelete.Class1"), &pType));
	EXIT_ON_FAIL(pType->GetMethod_6(SysAllocString(L"DeleteFileAndExitThread"), &methodInfo));

	VARIANT vtEmpty;
	VARIANT vtResult;
	VARIANT var1, var2;
	SAFEARRAY* pParams = SafeArrayCreateVector(VT_VARIANT, 0, 2);

	VariantInit(&var1);
	VariantInit(&var2);
	VariantInit(&vtEmpty);
	VariantInit(&vtResult);

	var1.vt = VT_I8;
	var1.llVal = (INT64)::current_module;
	LONG index = 0;
	SafeArrayPutElement(pParams, &index, &var1);
	// param 2: string
	index = 1;
	var2.vt = VT_BSTR;
	var2.bstrVal = SysAllocString(::dll_path.c_str());
	SafeArrayPutElement(pParams, &index, &var2);
	EXIT_ON_FAIL(methodInfo->Invoke_3(vtEmpty,pParams,&vtResult));
	return true;
}

mscorlib::_MethodInfoPtr FindMethod(mscorlib::_TypePtr pType, std::wstring methodName, int num_of_params = -1)
{
	if (pType == nullptr)
	{
		return nullptr;
	}


	struct {
		mscorlib::BindingFlags flags;
		const wchar_t* desc;
	} flagSets[] = {
		{ (mscorlib::BindingFlags)(mscorlib::BindingFlags_Public | mscorlib::BindingFlags_Instance),                L"Public Instance" },
		{ (mscorlib::BindingFlags)(mscorlib::BindingFlags_Public | mscorlib::BindingFlags_Static),                 L"Public Static" },
		{ (mscorlib::BindingFlags)(mscorlib::BindingFlags_NonPublic | mscorlib::BindingFlags_Instance),            L"Non-Public Instance" },
		{ (mscorlib::BindingFlags)(mscorlib::BindingFlags_NonPublic | mscorlib::BindingFlags_Static),              L"Non-Public Static" },
		{ (mscorlib::BindingFlags)(mscorlib::BindingFlags_Public | mscorlib::BindingFlags_NonPublic |
									mscorlib::BindingFlags_Instance | mscorlib::BindingFlags_Static),             L"All (Public + Non-Public, Instance + Static)" },
		{ (mscorlib::BindingFlags)(mscorlib::BindingFlags_Public | mscorlib::BindingFlags_Instance |
									mscorlib::BindingFlags_DeclaredOnly),                                          L"Public Instance (DeclaredOnly)" },
	};

	for (const auto& fs : flagSets)
	{
		SAFEARRAY* saMethods = nullptr;
		HRESULT hr = pType->GetMethods(fs.flags, &saMethods);

		if (FAILED(hr) || saMethods == nullptr)
		{
			continue;
		}

		LONG lBound = 0, uBound = -1;
		SafeArrayGetLBound(saMethods, 1, &lBound);
		SafeArrayGetUBound(saMethods, 1, &uBound);

		if (uBound < lBound)
		{
			SafeArrayDestroy(saMethods);
			continue;
		}


		for (LONG i = lBound; i <= uBound; ++i)
		{
			mscorlib::_MethodInfoPtr pMethod = nullptr;
			hr = SafeArrayGetElement(saMethods, &i, &pMethod);

			if (FAILED(hr) || pMethod == nullptr) continue;

			BSTR bstrName = nullptr;
			pMethod->get_name(&bstrName);
			std::wstring name = bstrName;
			SAFEARRAY* saParams = nullptr;
			if (SUCCEEDED(pMethod->GetParameters(&saParams)) && saParams)
			{
				LONG pLB = 0, pUB = -1;
				SafeArrayGetLBound(saParams, 1, &pLB);
				SafeArrayGetUBound(saParams, 1, &pUB);

				int paramCount = (pUB >= pLB) ? (pUB - pLB + 1) : 0;
				if (-1 != num_of_params and paramCount != num_of_params)
					continue;

				SafeArrayDestroy(saParams);
			}
			if (name == methodName) {
				std::wcout << fs.desc << " " << name  << std::endl;
				return pMethod;
			}

		}

		SafeArrayDestroy(saMethods);
	}

	return nullptr;
}

SELFDELETE_API bool delete_using_powershell(void)
{
	ICLRMetaHost* metaHost = nullptr;
	ICLRRuntimeInfo* runtimeInfo = nullptr;
	ICorRuntimeHost* runtimeHost = nullptr;
	IUnknown* spAppDomainThunk = nullptr;
	mscorlib::_AppDomain* pAppDomain = nullptr;
	mscorlib::_Assembly* pAssembly = nullptr;
	_TypePtr pType = nullptr;

	HRESULT hr;

	if (FAILED(hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return false;

	if (FAILED(hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID*)&metaHost))) 
		return false;
	if (FAILED(hr = metaHost->GetRuntime(L"v4.0.30319", IID_ICLRRuntimeInfo, (void**)&runtimeInfo))) 
		return false;
	if (FAILED(hr = runtimeInfo->GetInterface(CLSID_CorRuntimeHost, IID_ICorRuntimeHost, (void**)&runtimeHost)))
		return false;
	if (FAILED(hr = runtimeHost->Start())) 
		return false;
	if (FAILED(hr = runtimeHost->GetDefaultDomain(&spAppDomainThunk))) 
		return false;
	if (FAILED(hr = spAppDomainThunk->QueryInterface(IID_PPV_ARGS(&pAppDomain))))
		return false;


	// Load PowerShell engine assembly
	BSTR bstrAsm = SysAllocString(L"System.Management.Automation, Version=3.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35");
	hr = pAppDomain->Load_2(bstrAsm, &pAssembly);
	SysFreeString(bstrAsm);
	if (FAILED(hr)) 
		return false;

	BSTR bstrType = SysAllocString(L"System.Management.Automation.PowerShell");
	hr = pAssembly->GetType_2(bstrType, &pType);
	SysFreeString(bstrType);
	if (FAILED(hr))
		return false;

	mscorlib::_MethodInfoPtr miCreate = FindMethod(pType, L"Create", 0);
	mscorlib::_MethodInfoPtr miAddScript = FindMethod(pType, L"AddScript", 1);
	mscorlib::_MethodInfoPtr miInvoke = FindMethod(pType, L"Invoke", 0);

	if (!miCreate || !miAddScript || !miInvoke) 
		return false;


	VARIANT vtEmpty = { VT_EMPTY };
	VARIANT vtPSInstance = { VT_EMPTY };
	VARIANT vtTemp = { VT_EMPTY };

	if (FAILED(hr = miCreate->Invoke_3(vtEmpty, nullptr, &vtPSInstance)))
		return false;

	std::wstring script = L"[System.IO.File]::WriteAllText('idan_maman', 'try')";


	SAFEARRAY* psaScript = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	VARIANT vtScript = { VT_BSTR };
	vtScript.bstrVal = SysAllocString(script.c_str());

	LONG idx = 0;
	SafeArrayPutElement(psaScript, &idx, &vtScript);

	if (FAILED(hr = miAddScript->Invoke_3(vtPSInstance, psaScript, &vtTemp))) {
		VariantClear(&vtScript);
		SafeArrayDestroy(psaScript);
		return false;
	}
	VariantClear(&vtScript);
	SafeArrayDestroy(psaScript);
	VariantClear(&vtTemp);

	hr = miInvoke->Invoke_3(vtPSInstance, nullptr, &vtTemp);

	FreeLibraryAndExitThread(::current_module, 0);

	return true; 
}

SELFDELETE_API bool delete_using_file_mark(void) {

	static constexpr std::wstring_view rename_name = L":fuckyou";

	// Step 1: Open handle and rename primary :$DATA stream to ADS
	HANDLE hFile = CreateFileW(
		::dll_path.c_str(),
		DELETE,
		0, 
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	const size_t renameInfoSize = sizeof(FILE_RENAME_INFO) + sizeof(WCHAR) * rename_name.size();
	std::vector<std::byte> rename_data(renameInfoSize, std::byte{ 0 });

	auto* pRename = reinterpret_cast<PFILE_RENAME_INFO>(rename_data.data());
	pRename->FileNameLength = static_cast<DWORD>(rename_name.size() * sizeof(WCHAR));
	// Copy with null terminator like the reference (wcslen + 1)
	std::memcpy(pRename->FileName, rename_name.data(), (rename_name.size() + 1) * sizeof(WCHAR));

	if (!SetFileInformationByHandle(hFile, FileRenameInfo, pRename, static_cast<DWORD>(renameInfoSize))) {
		CloseHandle(hFile);
		return false;
	}

	// Step 2: Close the first handle - this commits the rename
	CloseHandle(hFile);

	// Step 3: Reopen by original path (still works because rename was to ADS, not a real path change)
	hFile = CreateFileW(
		::dll_path.c_str(),
		DELETE,
		0, NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	// Step 4: Mark for deletion with POSIX semantics (works on Windows 11 24H2+)
	FILE_DISPOSITION_INFO_EX fDeleteEx{};
	fDeleteEx.Flags = FILE_DISPOSITION_FLAG_DELETE | FILE_DISPOSITION_FLAG_POSIX_SEMANTICS;
	if (!SetFileInformationByHandle(hFile, FileDispositionInfoEx, &fDeleteEx, sizeof(fDeleteEx))) {
		CloseHandle(hFile);
		return false;
	}

	// Step 5: Close triggers the deletion disposition
	CloseHandle(hFile);

	// Step 6: Unload the DLL - file is already deleted from disk at this point
	FreeLibraryAndExitThread(::current_module, 0);

	return true; // unreachable
}