using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;

namespace FreeAndDelete
{
    [ClassInterface(ClassInterfaceType.None)]
    public class Class1
    {
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern bool FreeLibrary(Int64 hModule);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern void ExitThread(uint dwExitCode);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
         private static extern bool DeleteFileW(string lpFileName);


        private static void InteranlDeleteAndExitThread(Int64 hModuleHandle, string module_path)
        {
            bool res1 = FreeLibrary(hModuleHandle);
            if (!res1) { 
            }
            while (true)
            {
                bool res2 = DeleteFileW(module_path);
                if (!res2) {
                    if (Marshal.GetLastWin32Error() == 2)
                    {
                        return;
                    }
                    else {
                        Thread.Sleep(10);
                    }
                }
            }
        }


        public static void DeleteFileAndExitThread(Int64 hModuleHandle, string module_path)
        {
            Thread thread = new Thread(() => InteranlDeleteAndExitThread(hModuleHandle, module_path));
            thread.IsBackground = true;
            thread.Start();
            ExitThread(0);
        }
    }
}
