// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <thread>

BOOL APIENTRY DllMain(HMODULE, DWORD aReason, LPVOID)
{
  if (aReason != DLL_PROCESS_ATTACH) {
    return TRUE;
  }

  MessageBoxA(NULL, "InjectableDLL is running", "InjectableDLL", MB_OK);
  return TRUE;
}

