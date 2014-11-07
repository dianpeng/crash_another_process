#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* HELP= \
    "1 parameters is need, which is the process id \
    of the target process, you could find it in TaskManager.";

int main( int argc , char** argv ) {
    HANDLE hProcess;
    HANDLE hThread;
    void* pPageAddr;
    DWORD dwWaitRet;
    int PID;

    if( argc != 2 ) {
        fprintf(stderr,"%s",HELP);
        return -1;
    } else {
        // Parsing the parameter
        PID = atoi(argv[1]);
        if( PID == 0 ) {
            fprintf(stderr,"Invalid Process ID");
            return -1;
        }
    }

    // Grab the process 
    hProcess =  ::OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)PID);

    if( hProcess == INVALID_HANDLE_VALUE ) {
        fprintf(stderr,"Cannot get process handle from ID:%d",PID);
        return -1;
    }

    // Allocate a virtual page in process hProcess
    pPageAddr = ::VirtualAllocEx(
        hProcess,
        NULL,
        1024,
        MEM_COMMIT,
        PAGE_READWRITE);

    hThread = ::CreateRemoteThread(
        hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)1, // Invalid entry function to force a memory access violation
        pPageAddr,
        0,
        NULL);

    if( hThread == INVALID_HANDLE_VALUE ) {
        fprintf(stderr,"Cannot create remote thread!");
        return -1;
    }

    // Wait until the crash happened or the process is died
    dwWaitRet = WaitForSingleObject(hThread,INFINITE);

    // Now we can safely release our process

    ::CloseHandle(hThread);

    // We don't need to do any clean up since the target process
    // should have been crashed and all the resource is reclaimed
    printf("The target process should have been crashed!");
    return 0;
}
