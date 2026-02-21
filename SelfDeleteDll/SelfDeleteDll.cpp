#include <iostream>
#include <Windows.h>
#include <thread>
#include <unordered_map>
#include <algorithm>
#include <ranges>

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

enum SelfDeleteMethod {
    FLS = 1,
	REGISTRY = 2,
    APC = 3,
    THRED=4,
    PROCESS_LOLBIN1 = 5, 
	PROCESS_LOLBIN2 = 6,
	TIMERS = 7,
	REGISTER_WAIT = 8,
	QUEUE_WORK_ITEM = 9,
	CLR = 10,
    POWERSHELL = 11,
    MARK = 12
};


std::unordered_map<std::string, size_t> method_mapping = {
    {"FLS", SelfDeleteMethod::FLS},
    {"REGISTRY", SelfDeleteMethod::REGISTRY},
    {"APC", SelfDeleteMethod::APC},
    {"THRED", SelfDeleteMethod::THRED},
    {"LOLBIN1", SelfDeleteMethod::PROCESS_LOLBIN1},
    {"LOLBIN2", SelfDeleteMethod::PROCESS_LOLBIN2},
    {"TIMERS", SelfDeleteMethod::TIMERS},
    {"REGISTER_WAIT", SelfDeleteMethod::REGISTER_WAIT},
    {"QUEUE_WORK_ITEM", SelfDeleteMethod::QUEUE_WORK_ITEM},
    {"CLR", SelfDeleteMethod::CLR},
    {"POWERSHELL", SelfDeleteMethod::POWERSHELL},
    {"MARK",SelfDeleteMethod::MARK }
};

int main(int argc, char * argv[])
{

    std::string method_name = argv[1];
	transform(method_name.begin(), method_name.end(), method_name.begin(), ::toupper);
    
    auto found_method = method_mapping.find(argv[1]);
    if(found_method == method_mapping.end()){
        std::cerr << "Invalid method name provided. Please use one of the following: FLS_CALLBACK, REGISTRY_NOTIFICATION, APC, THRED, PROCESS_LOLBIN1, PROCESS_LOLBIN2, TIMERS, REGISTER_WAIT, QUEUE_WORK_ITEM, CLR, WMI" << std::endl;
        return 1;
	}
	size_t num = found_method->second; // for simplicity, no error handling for invalid input. just pass the method name as argument. e.g: "FLS_CALLBACK"
     wprintf(L"Self Delete DLL Loader! - Starting...!\n");
     HMODULE self_delete_library = LoadLibraryA("SelfDelete.dll");
     if (self_delete_library == NULL) {
		 std::cerr << "Failed to load SelfDelete.dll" << std::endl;
         return 2;
     }
	DeleteProcdure delete_proc = (DeleteProcdure)GetProcAddress(self_delete_library, MAKEINTRESOURCEA(num));
    if (delete_proc == NULL) {
		std::cerr << "Failed to get DeleteProc address" << std::endl;
        return 3;
    }

    auto ruuner_thread = std::thread([&delete_proc]() {delete_proc(); }); // it makes sense to own our on thread as UM-Rootkit.

    std::cout << "ENTERING TO REGULAR OPERATION" << std::endl;
    for(size_t i = 0 ; i < 10 ;i ++){
        SleepEx(2000, FALSE); // some activity burst on main program...(NON ALERTABLE!)
    }
    std::cout << "FINISHED _ EXIT _ THANK YOU ;-)" << std::endl;
    return 0;
}

