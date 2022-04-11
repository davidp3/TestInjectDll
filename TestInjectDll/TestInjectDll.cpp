#include <iostream>
#include "Windows.h"

int main(int argc, char** argv)
{
  if (argc != 3) {
    printf("Usage: %s dll_full_path target_pid \n"
           "       where dll_full_path is the full path to the DLL to inject\n"
           "       and target_pid is the PID of the process to inject into",
           argv[0]);
    exit(-1);
  }

  auto* dll_full_path = argv[1];
  auto pid = atoi(argv[2]);
  printf("Injecting <%s> into process %d\n", dll_full_path, pid);

  // Get HANDLE to process
  HANDLE procH = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (!procH) {
    fprintf(stderr, "Failed to get handle for process %d\n", pid);
    exit(1);
  }

  // Allocate buffer in remote process that will contain the DLL path.
  LPVOID remote_dll_full_path =
      VirtualAllocEx(procH, NULL, strlen(dll_full_path),
                     MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  if (!remote_dll_full_path) {
    fprintf(stderr, "Failed to allocate path in process %d\n", pid);
    exit(2);
  }

  // Copy DLL path into remote buffer
  size_t bytes_written;
  if (!WriteProcessMemory(procH, remote_dll_full_path, dll_full_path,
                          strlen(dll_full_path), &bytes_written) ||
      bytes_written != strlen(dll_full_path)) {
    fprintf(stderr, "Failed to allocate path in process %d\n", pid);
    exit(3);
  }

  // Get address of LoadLibraryA (which is the same in all processes).
  LPVOID loadLibraryA =
      (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
  if (!loadLibraryA) {
    fprintf(stderr, "Failed to find LoadLibraryA in kernel32.dll!\n");
    exit(4);
  }

  // Create a thread that runs LoadLibraryA in the remote process.  Pass it
  // the remote process' DLL full path string.
  if (!CreateRemoteThread(procH, NULL, NULL,
                          (LPTHREAD_START_ROUTINE)loadLibraryA,
                          remote_dll_full_path, NULL, NULL)) {
    fprintf(stderr, "Failed to create remote thread!\n");
    exit(5);
  }

  CloseHandle(procH);
  printf("Successfully hooked %s into process %d\n", dll_full_path, pid);
}
