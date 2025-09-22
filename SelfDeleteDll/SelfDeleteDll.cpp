#include <iostream>
#include <Windows.h>
#include <thread>


using DeleteProcdure =  bool (*)();

int main()
{
     wprintf(L"Self Delete DLL Loader! - Starting...!\n");
     HMODULE self_delete_library = LoadLibraryA("SelfDelete.dll");
     if (self_delete_library == NULL) {
		 std::cerr << "Failed to load SelfDelete.dll" << std::endl;
         return 2;
     }
	DeleteProcdure delete_proc = (DeleteProcdure)GetProcAddress(self_delete_library, MAKEINTRESOURCEA(1));
    if (delete_proc == NULL) {
		std::cerr << "Failed to get DeleteProc address" << std::endl;
    }
    auto ruuner_thread = std::thread([&delete_proc]() {delete_proc(); });
    ruuner_thread.join();
    SleepEx(12000,TRUE);
    wprintf(L"HELLO WORLD FROM MAIN! - Exiting...\n\n");
    return 0;
}

