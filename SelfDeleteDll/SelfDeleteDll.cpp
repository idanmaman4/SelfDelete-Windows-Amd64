#include <iostream>
#include <Windows.h>
#include <thread>


using DeleteProcdure =  bool (*)();

/*
Emulated Self-Cleanup Simulation Of File-Backed UserMode Rootkit On Modern Windows Enivroment
Purpose:
    This repository contains a simulation harness that demonstrates a variety of observable behaviors a file-backed user-mode rootkit could show during a cleanup/uninstall phase. 
    The project is strictly for defensive research, education, and detection testing in isolated lab environments. 
    The code intentionally contains patterns that mirror observable signals (APC scheduling, FLS callbacks registration, native registry-change notifications), 
    but does not perform destructive operations in safe-mode builds.
    Important safety note:
    Do not run this code on production or on machines you do not own. 
    he default build is non-destructive; 
What this simulation provides:
    A set of exported functions that emulate lifecycle transitions and produce telemetry.
    Structured logs and example outputs for SIEM and detection rule development.    
    A safe environment to practice forensic analysis and to generate synthetic Indicators-of-Behavior (IOBs) for testing.
*/

int main()
{
     wprintf(L"Self Delete DLL Loader! - Starting...!\n");
     HMODULE self_delete_library = LoadLibraryA("SelfDelete.dll");
     if (self_delete_library == NULL) {
		 std::cerr << "Failed to load SelfDelete.dll" << std::endl;
         return 2;
     }
	DeleteProcdure delete_proc = (DeleteProcdure)GetProcAddress(self_delete_library, MAKEINTRESOURCEA(4));
    if (delete_proc == NULL) {
		std::cerr << "Failed to get DeleteProc address" << std::endl;
        return 3;
    }

    auto ruuner_thread = std::thread([&delete_proc]() {delete_proc(); }); // it makes sense to own our on thread as UM-Rootkit.

    std::cout << "ENTERING TO REGULAR OPERATION" << std::endl;
    for(size_t i = 0 ; i < 10 ;i ++){
        SleepEx(2000, FALSE); // some activity burst on main program...

    }
    std::cout << "FINISHED _ EXIT _ THANK YOU ;-)" << std::endl;
    return 0;
}

