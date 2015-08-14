// DllTemplate.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#define BWLAPI 4
#define STARCRAFTBUILD 8

/*  STARCRAFTBUILD
   -1   All
   0   1.04
   1   1.08b
   2   1.09b
   3   1.10
   4   1.11b
   5   1.12b
   6   1.13f
   7   1.14
   8   1.15
   9   1.15.1
*/

#ifdef _MANAGED
#pragma managed(push, off)
#endif

struct ExchangeData
{
   int iPluginAPI;
   int iStarCraftBuild;
   bool bConfigDialog;                 //Is Configurable
   bool bNotSCBWmodule;                //Inform user that closing BWL will shut down your plugin
};


extern "C" BOOL MyIsDBCSLeadByte(BYTE TestChar)
{
	return IsDBCSLeadByteEx(20949, TestChar); /* 50225 = iso-2022-kr (ISO 2022 Korean) */
}

extern "C" unsigned int MyGetUserDefaultLangID()
{
	return MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN); /* Korean */
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                )
{
   //Is this DLL also StarCraft module? yes.
   char filename[1024];
   char * s;

   switch (ul_reason_for_call)
   {
      case DLL_PROCESS_ATTACH:
         GetModuleFileNameA(NULL, filename, sizeof(filename));
		 s = strrchr(filename, '\\');
		 s++;

		 MessageBoxA(NULL, s, "Hue", MB_OK);

		 if(_stricmp(s, "starcraft.exe") != 0)
			 break;
		 
         MessageBoxA(NULL, "StarCraft messagebox", "Hue", MB_OK);


		 DWORD oldProtect;
		 /*
		 VirtualProtect((void*)0x4fe14c, 0x100, PAGE_EXECUTE_READWRITE, &oldProtect);
		 *((void **)0x4fe1c4) = MyIsDBCSLeadByte;
		 *((void **)0x4fe14c) = MyGetUserDefaultLangID;
		 VirtualProtect((void*)0x4fe14c, 0x100, oldProtect, NULL);
		 */

		 VirtualProtect((void*)0x4161c0, 0x100, PAGE_READWRITE, &oldProtect);
		 		 *((void **)0x4161f8) = MyIsDBCSLeadByte;
				 *((void **)0x4161c6) = MyGetUserDefaultLangID;
		 VirtualProtect((void*)0x4161c0, 0x100, oldProtect, NULL);

		 MessageBoxA(NULL, "StarCraft messagebox2", "Hue", MB_OK);
         return TRUE;
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
      case DLL_PROCESS_DETACH:
         break;
   }
   
   return TRUE;
}
//
//GET Functions for BWLauncher
//
//
extern "C" __declspec(dllexport) void GetPluginAPI(ExchangeData &Data)
{
   //BWL Gets version from Resource - VersionInfo
   Data.iPluginAPI = BWLAPI;
   Data.iStarCraftBuild = STARCRAFTBUILD;
   Data.bConfigDialog = false;
   Data.bNotSCBWmodule = true;
}

extern "C" __declspec(dllexport) void GetData(char *name, char *description, char *updateurl)
{
   //if necessary you can add Initialize function here
   //possibly check CurrentCulture (CultureInfo) to localize your DLL due to system settings
   strcpy(name,      "Plugin");
   strcpy(description, "Author lala\r\n\r\nDescription of this plugin. This plugind does this and that.");
   strcpy(updateurl,   "http://www.bwprogrammers.com/files/update/bwl4/plugin/");
}


//
//Functions called by BWLauncher
//
//
extern "C" __declspec(dllexport) bool OpenConfig()
{
   //If you set "Data.bConfigDialog = true;" at function GetPluginAPI then
   //BWLauncher will call this function if user clicks Config button

   //Youll need to make your own Window here
   return true; //everything OK

   //return false; //something went wrong
}

char FileName[] = "KoreanMode.bwl"; 

BOOL ErrorMessage (const char* msg)
{
	MessageBoxA (NULL, msg, "Korean Mode", MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
   return FALSE;
} 

extern "C" __declspec(dllexport) BOOL ApplyPatchSuspended(HANDLE hProcess, DWORD dwProcessID)
{
   //This function is called on suspended process
   //Durning the suspended process some modules of starcraft.exe may not yet exist.
   //the dwProcessID is not checked, its the created pi.dwProcessId

   //here is safe place to call starcraft methods to load your DLL as a module
   //hint - process shoudnt be suspended :)
   //hint - WNDPROCCALL

	    FARPROC     lpLoadLibraryA;
    HMODULE     hKernel32 = GetModuleHandleA ("kernel32");
    LPVOID      lpRemoteBlock;
    HANDLE      hRemoteThread;
    DWORD       dwExitCode;

    if (!hKernel32)
        return ErrorMessage ("GetModuleHandle() error!");

    if (!(lpLoadLibraryA = GetProcAddress (hKernel32, "LoadLibraryA")))
        return ErrorMessage ("GetProcAddress() error!");

    if (!(lpRemoteBlock = VirtualAllocEx (hProcess, NULL, sizeof(FileName), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)))
        return ErrorMessage ("VirtaulAllocEx() error!");

    if (!WriteProcessMemory (hProcess, lpRemoteBlock, (LPVOID)FileName, sizeof(FileName), NULL))
    {
        VirtualFreeEx (hProcess, lpRemoteBlock, 0, MEM_RELEASE);
        return ErrorMessage ("WriteProcessMemory() error!");
    }

    hRemoteThread = CreateRemoteThread (
        hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)lpLoadLibraryA,
        lpRemoteBlock,
        0,
        NULL
        );

    if (hRemoteThread)
    {
        const char *error_message;
        BOOL success = WAIT_OBJECT_0 == WaitForSingleObject (hRemoteThread, 5000);
       
        if (success)
        {
            success = success && GetExitCodeThread (hRemoteThread, &dwExitCode);

            if (success)
            {
                success = success && dwExitCode;
                if (!success) error_message = "Remote LoadLibrary() error! Missing ScWnd.hdl?";
            }
            else
                error_message = "GetExitCodeThread() error!";
        }
        else
            error_message = "WaitForSingleObject() error!";

        CloseHandle (hRemoteThread);
        VirtualFreeEx (hProcess, lpRemoteBlock, 0, MEM_RELEASE);
        if (!success) ErrorMessage (error_message);
        return success;
    }
    else
    {
        VirtualFreeEx (hProcess, lpRemoteBlock, 0, MEM_RELEASE);
        MessageBoxA (NULL, "CreateRemoteThread() error!", "Korean Mode", MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
        return FALSE;
    }

   return true; //everything OK

   //return false; //something went wrong
}

extern "C" __declspec(dllexport) BOOL ApplyPatch(HANDLE hProcess, DWORD dwProcessID)
{
   //This fuction is called after
   //ResumeThread(pi.hThread);
   //WaitForInputIdle(pi.hProcess, INFINITE);
   //EnableDebugPriv() -
   //   OpenProcessToken...
   //   LookupPrivilegeValue...
   //   AdjustTokenPrivileges...
   //
   //the dwProcessID is checked by GetWindowThreadProcessId
   //so it is definitly the StarCraft

   return true; //everything OK

   //return false; //something went wrong
}

#ifdef _MANAGED
#pragma managed(pop)
#endif 