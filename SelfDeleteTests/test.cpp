#include "pch.h"
#include <gtest/gtest.h>
#include <string_view>
#include <string>
#include <iostream>
#include <filesystem>
#include <Windows.h>
#include <thread>
#include <optional>
#include <array>



enum class TestCase : int {
	FlsCallbacks = 1,
	RegistryNotification,
	Apc,
	Thread,
	ProcessLolbin1,
	ProcessLolbin2,
	Timers,
	RtlRegisterWait,
	RtlQueueWorkItem,
	Clr,
	Max
};

std::array<std::string_view , (int)TestCase::Max> TestCaseNames = {
	"FILLER",
	"FlsCallbacks",
	"RegistryNotification",
	"Apc",
	"Thread",
	"ProcessLolbin1",
	"ProcessLolbin2",
	"Timers",
	"RtlRegisterWait",
	"RtlQueueWorkItem",
	"Clr"
};

constexpr std::wstring_view DLL_ORIG_PATH = L".\\SelfDelete.dll";


std::filesystem::path PrepareTest(const std::wstring& sourceFilePath) {
	DWORD pathBufferSize = GetTempPath2W(0, nullptr);
	if (pathBufferSize == 0) {
		return {};
	}
	std::vector<WCHAR> tempPathBuffer(pathBufferSize);
	if (GetTempPath2W(pathBufferSize, tempPathBuffer.data()) == 0) {
		return {};
	}
	std::wstring tempPath(tempPathBuffer.data());
	WCHAR tempFileNameBuffer[MAX_PATH];
	if (GetTempFileNameW(tempPath.c_str(), L"TMP", 0, tempFileNameBuffer) == 0) {
		return {};
	}
	std::wstring wTempFileName(tempFileNameBuffer);
	if (!CopyFileW(sourceFilePath.c_str(), wTempFileName.c_str(), FALSE)) {
		return {};
	};
	return wTempFileName;
}


HMODULE LoadTest(std::filesystem::path test_path) {
	HMODULE self_delete_library = LoadLibraryA(test_path.string().c_str());
	if (self_delete_library == NULL) {
		return NULL;
	}
	return self_delete_library;
}

using DeleteProcdure = bool (*)();
bool runTest(std::filesystem::path test_path,TestCase test_number) {
	HMODULE test_module = LoadTest(test_path);
	DeleteProcdure delete_proc = (DeleteProcdure)GetProcAddress(test_module, MAKEINTRESOURCEA((int)test_number));
	if (delete_proc == NULL) {
		return false;
	}

	HANDLE hThread = CreateThread(
		nullptr,
		0,
		reinterpret_cast<LPTHREAD_START_ROUTINE>(delete_proc),
		(LPVOID)0,
		0,
		nullptr
	);

	if (!hThread) return false;
	WaitForSingleObject(hThread, INFINITE);
	Sleep(10000);
	return true;
}

bool checkTest(std::filesystem::path test_path) {
	return LoadTest(test_path) == NULL;
}

void CleanUpTest(std::filesystem::path test_path) {
	DeleteFileA(test_path.string().c_str());
}
template<TestCase TC>
struct TestCaseTag {
	static constexpr TestCase value = TC;
};

typedef ::testing::Types<
	TestCaseTag<TestCase::FlsCallbacks>,
	//TestCaseTag<TestCase::RegistryNotification>,
	TestCaseTag<TestCase::Apc>,
	TestCaseTag<TestCase::Thread>,
	TestCaseTag<TestCase::ProcessLolbin1>,
	TestCaseTag<TestCase::ProcessLolbin2>,
	TestCaseTag<TestCase::Timers>,
	//TestCaseTag<TestCase::RtlRegisterWait>,
	TestCaseTag<TestCase::RtlQueueWorkItem>,
	TestCaseTag<TestCase::Clr>
> AllCases;

template<typename T>
class SelfDeleteTypedTest : public ::testing::Test {};

TYPED_TEST_CASE(SelfDeleteTypedTest, AllCases);   

TYPED_TEST(SelfDeleteTypedTest, SelfDeleteRoutine)
{

	constexpr TestCase tc = TypeParam::value;
	GTEST_LOG_(INFO) << "Starting test for TC=" << TestCaseNames[(int)tc ] << std::endl;

	std::filesystem::path test_path = PrepareTest(DLL_ORIG_PATH.data());

	ASSERT_FALSE(test_path.empty())
		<< "Failed to prepare test for TC=" << TestCaseNames[(int)tc ];


	ASSERT_TRUE(runTest(test_path, tc))
		<< "Routine failed for TC=" << TestCaseNames[(int)tc];

	ASSERT_TRUE(checkTest(test_path))
		<< "DLL not deleted for TC=" << TestCaseNames[(int)tc];

	CleanUpTest(test_path);
}
