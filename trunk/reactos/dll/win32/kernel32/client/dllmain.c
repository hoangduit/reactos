/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS system libraries
 * FILE:            lib/kernel32/misc/dllmain.c
 * PURPOSE:         Initialization
 * PROGRAMMERS:     Ariadne (ariadne@xs4all.nl)
 *                  Aleksey Bragin (aleksey@reactos.org)
 */

/* INCLUDES ******************************************************************/

#include <k32.h>

#define NDEBUG
#include <debug.h>

/* GLOBALS *******************************************************************/

PBASE_STATIC_SERVER_DATA BaseStaticServerData;

BOOLEAN BaseRunningInServerProcess;

WCHAR BaseDefaultPathBuffer[6140];

HANDLE BaseNamedObjectDirectory;
HMODULE hCurrentModule = NULL;
HMODULE kernel32_handle = NULL;
PPEB Peb;
ULONG SessionId;
static BOOL DllInitialized = FALSE;

/* Critical section for various kernel32 data structures */
RTL_CRITICAL_SECTION BaseDllDirectoryLock;

extern BOOL FASTCALL NlsInit(VOID);
extern VOID FASTCALL NlsUninit(VOID);

#define WIN_OBJ_DIR L"\\Windows"
#define SESSION_DIR L"\\Sessions"

/* FUNCTIONS *****************************************************************/

NTSTATUS
NTAPI
BaseCreateThreadPoolThread(IN PTHREAD_START_ROUTINE Function,
                           IN PVOID Parameter,
                           OUT PHANDLE ThreadHandle)
{
    NTSTATUS Status;

    /* Create a Win32 thread */
    *ThreadHandle = CreateRemoteThread(NtCurrentProcess(),
                                       NULL,
                                       0,
                                       Function,
                                       Parameter,
                                       CREATE_SUSPENDED,
                                       NULL);
    if (!(*ThreadHandle))
    {
        /* Get the status value if we couldn't get a handle */
        Status = NtCurrentTeb()->LastStatusValue;
        if (NT_SUCCESS(Status)) Status = STATUS_UNSUCCESSFUL;
    }
    else
    {
        /* Set success code */
        Status = STATUS_SUCCESS;
    }

    /* All done */
    return Status;
}

NTSTATUS
NTAPI
BaseExitThreadPoolThread(IN NTSTATUS ExitStatus)
{
    /* Exit the thread */
    ExitThread(ExitStatus);
    return STATUS_SUCCESS;
}

BOOL
WINAPI
DllMain(HANDLE hDll,
        DWORD dwReason,
        LPVOID lpReserved)
{
    NTSTATUS Status;
    ULONG Dummy;
    ULONG DummySize = sizeof(Dummy);
    WCHAR SessionDir[256];

    DPRINT("DllMain(hInst %p, dwReason %lu)\n",
           hDll, dwReason);

    Basep8BitStringToUnicodeString = RtlAnsiStringToUnicodeString;

    /* Cache the PEB and Session ID */
    Peb = NtCurrentPeb();
    SessionId = Peb->SessionId;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            /* Set no filter intially */
            GlobalTopLevelExceptionFilter = RtlEncodePointer(NULL);
            
            /* Enable the Rtl thread pool and timer queue to use proper Win32 thread */
            RtlSetThreadPoolStartFunc(BaseCreateThreadPoolThread, BaseExitThreadPoolThread);

            /* Don't bother us for each thread */
            LdrDisableThreadCalloutsForDll((PVOID)hDll);

            /* Initialize default path to NULL */
            RtlInitUnicodeString(&BaseDefaultPath, NULL);

            /* Setup the right Object Directory path */
            if (!SessionId)
            {
                /* Use the raw path */
                wcscpy(SessionDir, WIN_OBJ_DIR);
            }
            else
            {
                /* Use the session path */
                swprintf(SessionDir,
                         L"%ws\\%ld%ws",
                         SESSION_DIR,
                         SessionId,
                         WIN_OBJ_DIR);
            }

            /* Connect to the base server */
            Status = CsrClientConnectToServer(SessionDir,
                                              BASESRV_SERVERDLL_INDEX,
                                              &Dummy,
                                              &DummySize,
                                              &BaseRunningInServerProcess);
            if (!NT_SUCCESS(Status))
            {
                DPRINT1("Failed to connect to CSR (Status %lx)\n", Status);
                NtTerminateProcess(NtCurrentProcess(), Status);
                return FALSE;
            }

            /* Get the server data */
            ASSERT(Peb->ReadOnlyStaticServerData);
            BaseStaticServerData = Peb->ReadOnlyStaticServerData[BASESRV_SERVERDLL_INDEX];
            ASSERT(BaseStaticServerData);

            /* Check if we are running a CSR Server */
            if (!BaseRunningInServerProcess)
            {
                /* Set the termination port for the thread */
                DPRINT("Creating new thread for CSR\n");
                CsrNewThread();
            }

            /* Initialize heap handle table */
            BaseDllInitializeMemoryManager();

            /* Set HMODULE for our DLL */
            kernel32_handle = hCurrentModule = hDll;

            /* Set the directories */
            BaseWindowsDirectory = BaseStaticServerData->WindowsDirectory;
            BaseWindowsSystemDirectory = BaseStaticServerData->WindowsSystemDirectory;

            /* Construct the default path (using the static buffer) */
            _snwprintf(BaseDefaultPathBuffer,
                       sizeof(BaseDefaultPathBuffer) / sizeof(WCHAR),
                       L".;%wZ;%wZ\\system;%wZ;",
                       &BaseWindowsSystemDirectory,
                       &BaseWindowsDirectory,
                       &BaseWindowsDirectory);

            BaseDefaultPath.Buffer = BaseDefaultPathBuffer;
            BaseDefaultPath.Length = wcslen(BaseDefaultPathBuffer) * sizeof(WCHAR);
            BaseDefaultPath.MaximumLength = sizeof(BaseDefaultPathBuffer);

            /* Use remaining part of the default path buffer for the append path */
            BaseDefaultPathAppend.Buffer = (PWSTR)((ULONG_PTR)BaseDefaultPathBuffer + BaseDefaultPath.Length);
            BaseDefaultPathAppend.Length = 0;
            BaseDefaultPathAppend.MaximumLength = BaseDefaultPath.MaximumLength - BaseDefaultPath.Length;

            /* Initialize command line */
            InitCommandLines();

            /* Initialize the DLL critical section */
            RtlInitializeCriticalSection(&BaseDllDirectoryLock);

            /* Initialize the National Language Support routines */
            if (!NlsInit())
            {
                DPRINT1("NLS Init failed\n");
                return FALSE;
            }

            /* Initialize Console Support */
            if (!BasepInitConsole())
            {
                DPRINT1("Failed to set up console\n");
                return FALSE;
            }

            /* Initialize application certification globals */
            InitializeListHead(&BasepAppCertDllsList);
            RtlInitializeCriticalSection(&gcsAppCert);

            /* Insert more dll attach stuff here! */
            DllInitialized = TRUE;
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            if (DllInitialized == TRUE)
            {
                /* Insert more dll detach stuff here! */
                NlsUninit();

                /* Uninitialize console support */
                BasepUninitConsole();

                /* Delete DLL critical section */
                RtlDeleteCriticalSection(&BaseDllDirectoryLock);
            }
            break;
        }

        default:
            break;
    }

    return TRUE;
}

/* EOF */
