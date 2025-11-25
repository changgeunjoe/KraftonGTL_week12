#include "pch.h"
#include "MiniDump.h"
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>
#include <new.h> // _set_new_handler 용
#include <fstream>

#pragma comment(lib, "dbghelp.lib")

// 빌드 시점의 Git 정보 (PreBuildEvent에서 생성됨)
#if __has_include("GitVersion.h")
#include "GitVersion.h"
#endif

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "unknown"
#endif

#ifndef GIT_REPO_URL
#define GIT_REPO_URL "unknown"
#endif

// ============================================================================
// 호출 스택을 캡처하여 문자열로 반환
// ============================================================================
std::wstring CaptureCallStack(CONTEXT* Context)
{
    std::wstringstream StackTrace;

    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();

    // 심볼 초기화
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    if (!SymInitialize(hProcess, nullptr, TRUE))
    {
        StackTrace << L"Failed to initialize symbols. Error: " << GetLastError() << L"\r\n";
        return StackTrace.str();
    }

    // 스택 프레임 설정
    STACKFRAME64 StackFrame = {};
    DWORD MachineType;

#ifdef _M_X64
    MachineType = IMAGE_FILE_MACHINE_AMD64;
    StackFrame.AddrPC.Offset = Context->Rip;
    StackFrame.AddrPC.Mode = AddrModeFlat;
    StackFrame.AddrFrame.Offset = Context->Rbp;
    StackFrame.AddrFrame.Mode = AddrModeFlat;
    StackFrame.AddrStack.Offset = Context->Rsp;
    StackFrame.AddrStack.Mode = AddrModeFlat;
#else
    MachineType = IMAGE_FILE_MACHINE_I386;
    StackFrame.AddrPC.Offset = Context->Eip;
    StackFrame.AddrPC.Mode = AddrModeFlat;
    StackFrame.AddrFrame.Offset = Context->Ebp;
    StackFrame.AddrFrame.Mode = AddrModeFlat;
    StackFrame.AddrStack.Offset = Context->Esp;
    StackFrame.AddrStack.Mode = AddrModeFlat;
#endif

    StackTrace << L"========== Call Stack ==========\r\n";

    // 심볼 정보를 위한 버퍼
    char SymbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    PSYMBOL_INFO Symbol = (PSYMBOL_INFO)SymbolBuffer;
    Symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    Symbol->MaxNameLen = MAX_SYM_NAME;

    IMAGEHLP_LINE64 Line = {};
    Line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    int FrameIndex = 0;
    const int MaxFrames = 64;

    while (FrameIndex < MaxFrames)
    {
        if (!StackWalk64(MachineType, hProcess, hThread, &StackFrame, Context,
            nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
        {
            break;
        }

        if (StackFrame.AddrPC.Offset == 0)
        {
            break;
        }

        DWORD64 Address = StackFrame.AddrPC.Offset;
        DWORD64 Displacement64 = 0;
        DWORD Displacement = 0;

        StackTrace << L"[" << FrameIndex << L"] 0x" << std::hex << Address << std::dec;

        // 함수명 가져오기
        if (SymFromAddr(hProcess, Address, &Displacement64, Symbol))
        {
            // char를 wchar_t로 변환
            wchar_t WideName[MAX_SYM_NAME];
            size_t Converted;
            mbstowcs_s(&Converted, WideName, Symbol->Name, MAX_SYM_NAME);
            StackTrace << L" " << WideName;
        }

        // 파일명과 라인 번호 가져오기
        if (SymGetLineFromAddr64(hProcess, Address, &Displacement, &Line))
        {
            wchar_t WideFileName[MAX_PATH];
            size_t Converted;
            mbstowcs_s(&Converted, WideFileName, Line.FileName, MAX_PATH);
            StackTrace << L" (" << WideFileName << L":" << Line.LineNumber << L")";
        }

        StackTrace << L"\r\n";
        FrameIndex++;
    }

    StackTrace << L"================================\r\n";

    SymCleanup(hProcess);

    return StackTrace.str();
}

// ============================================================================
// 예외 정보를 문자열로 변환
// ============================================================================
std::wstring GetExceptionDescription(DWORD ExceptionCode)
{
    switch (ExceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:         return L"Access Violation";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return L"Array Bounds Exceeded";
    case EXCEPTION_BREAKPOINT:               return L"Breakpoint";
    case EXCEPTION_DATATYPE_MISALIGNMENT:    return L"Datatype Misalignment";
    case EXCEPTION_FLT_DENORMAL_OPERAND:     return L"Float Denormal Operand";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return L"Float Divide By Zero";
    case EXCEPTION_FLT_INEXACT_RESULT:       return L"Float Inexact Result";
    case EXCEPTION_FLT_INVALID_OPERATION:    return L"Float Invalid Operation";
    case EXCEPTION_FLT_OVERFLOW:             return L"Float Overflow";
    case EXCEPTION_FLT_STACK_CHECK:          return L"Float Stack Check";
    case EXCEPTION_FLT_UNDERFLOW:            return L"Float Underflow";
    case EXCEPTION_ILLEGAL_INSTRUCTION:      return L"Illegal Instruction";
    case EXCEPTION_IN_PAGE_ERROR:            return L"In Page Error";
    case EXCEPTION_INT_DIVIDE_BY_ZERO:       return L"Integer Divide By Zero";
    case EXCEPTION_INT_OVERFLOW:             return L"Integer Overflow";
    case EXCEPTION_INVALID_DISPOSITION:      return L"Invalid Disposition";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION: return L"Noncontinuable Exception";
    case EXCEPTION_PRIV_INSTRUCTION:         return L"Privileged Instruction";
    case EXCEPTION_SINGLE_STEP:              return L"Single Step";
    case EXCEPTION_STACK_OVERFLOW:           return L"Stack Overflow";
    case 0xC0000374:                         return L"Heap Corruption";
    default:                                 return L"Unknown Exception";
    }
}

// ============================================================================
// 1. 덤프 생성 로직을 함수 하나로 분리 (재사용 위해)
// ============================================================================
void CreateMiniDump(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
    // 실행 파일 경로 가져오기
    wchar_t ExePath[MAX_PATH];
    GetModuleFileNameW(nullptr, ExePath, MAX_PATH);

    // 실행 파일명 제거하고 디렉토리 경로만 추출
    std::wstring ExeDir(ExePath);
    size_t LastSlash = ExeDir.find_last_of(L"\\/");
    if (LastSlash != std::wstring::npos)
    {
        ExeDir = ExeDir.substr(0, LastSlash + 1);
    }

    auto Now = std::chrono::system_clock::now();
    auto InTimeT = std::chrono::system_clock::to_time_t(Now);

    std::wstringstream ss;
    struct tm TimeInfo;
    errno_t err = localtime_s(&TimeInfo, &InTimeT);

    // 실행 파일 경로 + 덤프 파일명
    ss << ExeDir;
    if (err == 0)
    {
        ss << L"Mundi_CrashDump_";
        ss << std::put_time(&TimeInfo, L"%Y-%m-%d_%H-%M-%S");
        ss << L".dmp";
    }
    else
    {
        ss << L"CrashDump_UnknownTime.dmp";
    }

    std::wstring DumpFileName = ss.str();
    HANDLE hFile = CreateFileW(DumpFileName.c_str(),
        GENERIC_WRITE, FILE_SHARE_READ, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
    );

    if (hFile != INVALID_HANDLE_VALUE)
    {
        _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
        ExInfo.ThreadId = GetCurrentThreadId();
        ExInfo.ExceptionPointers = ExceptionInfo;
        ExInfo.ClientPointers = FALSE;

        // 힙 상태까지 보고 싶다면 MiniDumpWithFullMemory 권장 (용량 큼)
        // 힙 오염 추적용이므로 MiniDumpWithDataSegs, MiniDumpWithHandleData 등을 섞는게 좋음
        MINIDUMP_TYPE DumpType = (MINIDUMP_TYPE)(MiniDumpNormal | MiniDumpWithDataSegs | MiniDumpWithHandleData);

        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
            hFile, DumpType, ExceptionInfo ? &ExInfo : nullptr, nullptr, nullptr);

        CloseHandle(hFile);

        // 심볼 서버에 타임스탬프 폴더 생성 후 덤프 + PDB + EXE 복사 (서버 접속 실패해도 무시)
        std::wstring SymbolServerBase = L"\\\\172.21.11.119\\symbols\\CrashDumps\\";

        // 서버 접속 가능 여부 확인 (폴더 생성 시도)
        if (CreateDirectoryW(SymbolServerBase.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS)
        {
            // 타임스탬프 폴더명 생성
            std::wstringstream folderSS;
            folderSS << L"Crash_";
            folderSS << std::put_time(&TimeInfo, L"%Y-%m-%d_%H-%M-%S");
            std::wstring CrashFolderName = folderSS.str();

            // 크래시 폴더 경로
            std::wstring CrashFolderPath = SymbolServerBase + CrashFolderName + L"\\";
            if (CreateDirectoryW(CrashFolderPath.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS)
            {
                // 덤프 파일 복사
                std::wstring DumpDestPath = CrashFolderPath + L"Mundi_CrashDump.dmp";
                CopyFileW(DumpFileName.c_str(), DumpDestPath.c_str(), FALSE);

                // PDB 파일 복사 (exe 옆에 있는 PDB 사용)
                std::wstring PdbSourcePath = ExeDir + L"Mundi.pdb";
                std::wstring PdbDestPath = CrashFolderPath + L"Mundi.pdb";
                CopyFileW(PdbSourcePath.c_str(), PdbDestPath.c_str(), FALSE);

                // EXE 파일 복사
                std::wstring ExeSourcePath = ExeDir + L"Mundi.exe";
                std::wstring ExeDestPath = CrashFolderPath + L"Mundi.exe";
                CopyFileW(ExeSourcePath.c_str(), ExeDestPath.c_str(), FALSE);

                // 호출 스택 정보를 텍스트 파일로 저장
                std::wstring StackTraceDestPath = CrashFolderPath + L"CallStack.txt";
                HANDLE hStackFile = CreateFileW(StackTraceDestPath.c_str(),
                    GENERIC_WRITE, 0, nullptr,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

                if (hStackFile != INVALID_HANDLE_VALUE)
                {
                    std::wstringstream CrashReport;

                    // 크래시 시간 정보
                    CrashReport << L"========== Crash Report ==========\r\n";
                    CrashReport << L"Time: " << std::put_time(&TimeInfo, L"%Y-%m-%d %H:%M:%S") << L"\r\n";

                    // Git 정보
                    CrashReport << L"Git Commit: " << GIT_COMMIT_HASH << L"\r\n";
                    CrashReport << L"Git Repo: " << GIT_REPO_URL << L"\r\n";

                    // 예외 정보
                    if (ExceptionInfo && ExceptionInfo->ExceptionRecord)
                    {
                        DWORD ExCode = ExceptionInfo->ExceptionRecord->ExceptionCode;
                        CrashReport << L"Exception Code: 0x" << std::hex << ExCode << std::dec << L"\r\n";
                        CrashReport << L"Exception Type: " << GetExceptionDescription(ExCode) << L"\r\n";
                        CrashReport << L"Exception Address: 0x" << std::hex
                                    << (DWORD64)ExceptionInfo->ExceptionRecord->ExceptionAddress << std::dec << L"\r\n";

                        // Access Violation인 경우 추가 정보
                        if (ExCode == EXCEPTION_ACCESS_VIOLATION && ExceptionInfo->ExceptionRecord->NumberParameters >= 2)
                        {
                            ULONG_PTR AccessType = ExceptionInfo->ExceptionRecord->ExceptionInformation[0];
                            ULONG_PTR AccessAddress = ExceptionInfo->ExceptionRecord->ExceptionInformation[1];
                            CrashReport << L"Access Type: " << (AccessType == 0 ? L"Read" : (AccessType == 1 ? L"Write" : L"Execute")) << L"\r\n";
                            CrashReport << L"Access Address: 0x" << std::hex << AccessAddress << std::dec << L"\r\n";
                        }
                    }
                    else
                    {
                        CrashReport << L"Exception Info: Not available (CRT handler)\r\n";
                    }

                    CrashReport << L"\r\n";

                    // 호출 스택 캡처
                    if (ExceptionInfo && ExceptionInfo->ContextRecord)
                    {
                        CrashReport << CaptureCallStack(ExceptionInfo->ContextRecord);
                    }
                    else
                    {
                        // ExceptionInfo가 없는 경우 현재 컨텍스트 캡처
                        CONTEXT CurrentContext = {};
                        CurrentContext.ContextFlags = CONTEXT_FULL;
                        RtlCaptureContext(&CurrentContext);
                        CrashReport << CaptureCallStack(&CurrentContext);
                    }

                    // 파일에 쓰기 (UTF-8 BOM + UTF-8로 변환)
                    std::wstring Report = CrashReport.str();

                    // UTF-8 BOM 쓰기
                    unsigned char BOM[] = { 0xEF, 0xBB, 0xBF };
                    DWORD BytesWritten;
                    WriteFile(hStackFile, BOM, sizeof(BOM), &BytesWritten, nullptr);

                    // wstring을 UTF-8로 변환하여 쓰기
                    int Utf8Size = WideCharToMultiByte(CP_UTF8, 0, Report.c_str(), -1, nullptr, 0, nullptr, nullptr);
                    if (Utf8Size > 0)
                    {
                        char* Utf8Buffer = new char[Utf8Size];
                        WideCharToMultiByte(CP_UTF8, 0, Report.c_str(), -1, Utf8Buffer, Utf8Size, nullptr, nullptr);
                        WriteFile(hStackFile, Utf8Buffer, Utf8Size - 1, &BytesWritten, nullptr); // -1 to exclude null terminator
                        delete[] Utf8Buffer;
                    }

                    CloseHandle(hStackFile);
                }

                // OpenDump.bat 생성 - 실행하면 자동으로 git clone 받고 덤프 열기
                std::wstring BatFilePath = CrashFolderPath + L"OpenDump.bat";
                HANDLE hBatFile = CreateFileW(BatFilePath.c_str(),
                    GENERIC_WRITE, 0, nullptr,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

                if (hBatFile != INVALID_HANDLE_VALUE)
                {
                    std::stringstream BatContent;
                    BatContent << "@echo off\r\n";
                    BatContent << "chcp 65001 > nul\r\n";
                    BatContent << "setlocal\r\n";
                    BatContent << "\r\n";
                    BatContent << "set \"CRASH_DIR=%~dp0\"\r\n";
                    BatContent << "set \"SOURCE_DIR=%CRASH_DIR%Source\"\r\n";
                    BatContent << "set \"GIT_REPO=" << GIT_REPO_URL << "\"\r\n";
                    BatContent << "set \"GIT_COMMIT=" << GIT_COMMIT_HASH << "\"\r\n";
                    BatContent << "\r\n";
                    BatContent << "echo ========================================\r\n";
                    BatContent << "echo   Mundi Crash Dump Analyzer\r\n";
                    BatContent << "echo ========================================\r\n";
                    BatContent << "echo.\r\n";
                    BatContent << "echo Git Repo: %GIT_REPO%\r\n";
                    BatContent << "echo Git Commit: %GIT_COMMIT%\r\n";
                    BatContent << "echo.\r\n";
                    BatContent << "\r\n";
                    BatContent << "if exist \"%SOURCE_DIR%\" (\r\n";
                    BatContent << "    echo Source folder already exists. Skipping clone.\r\n";
                    BatContent << ") else (\r\n";
                    BatContent << "    echo Cloning repository...\r\n";
                    BatContent << "    git clone \"%GIT_REPO%\" \"%SOURCE_DIR%\"\r\n";
                    BatContent << "    if errorlevel 1 (\r\n";
                    BatContent << "        echo Failed to clone repository!\r\n";
                    BatContent << "        pause\r\n";
                    BatContent << "        exit /b 1\r\n";
                    BatContent << "    )\r\n";
                    BatContent << ")\r\n";
                    BatContent << "\r\n";
                    BatContent << "echo Checking out commit %GIT_COMMIT%...\r\n";
                    BatContent << "cd /d \"%SOURCE_DIR%\"\r\n";
                    BatContent << "git checkout %GIT_COMMIT%\r\n";
                    BatContent << "if errorlevel 1 (\r\n";
                    BatContent << "    echo Failed to checkout commit!\r\n";
                    BatContent << "    pause\r\n";
                    BatContent << "    exit /b 1\r\n";
                    BatContent << ")\r\n";
                    BatContent << "\r\n";
                    BatContent << "echo.\r\n";
                    BatContent << "echo Source ready! Opening dump file...\r\n";
                    BatContent << "echo.\r\n";
                    BatContent << "echo [TIP] In Visual Studio:\r\n";
                    BatContent << "echo   1. Click 'Debug with Native Only'\r\n";
                    BatContent << "echo   2. If source not found, browse to:\r\n";
                    BatContent << "echo      %SOURCE_DIR%\\Mundi\\Source\r\n";
                    BatContent << "echo.\r\n";
                    BatContent << "\r\n";
                    BatContent << "start \"\" \"%CRASH_DIR%Mundi_CrashDump.dmp\"\r\n";
                    BatContent << "\r\n";
                    BatContent << "pause\r\n";

                    std::string BatStr = BatContent.str();
                    DWORD BytesWritten;
                    WriteFile(hBatFile, BatStr.c_str(), (DWORD)BatStr.size(), &BytesWritten, nullptr);
                    CloseHandle(hBatFile);
                }
            }
        }
        // 서버 접속 실패 시 로컬 덤프만 남기고 계속 진행
    }
}

// ============================================================================
// 2. 각종 핸들러 구현
// ============================================================================

// [A] SEH 핸들러 (기존꺼)
LONG WINAPI UnhandledExceptionHandler(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
    CreateMiniDump(ExceptionInfo);
    return EXCEPTION_EXECUTE_HANDLER;
}

// [B] VEH 핸들러 (우선순위 1등) - 여기서 잡힐 확률이 높음
LONG WINAPI VectoredExceptionHandler(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
    // 0xC0000005 (Access Violation) 등 치명적인 것만 캡처
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION ||
        ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ARRAY_BOUNDS_EXCEEDED ||
        ExceptionInfo->ExceptionRecord->ExceptionCode == 0xC0000374 /*STATUS_HEAP_CORRUPTION*/)
    {
        CreateMiniDump(ExceptionInfo);

        // 덤프 떴으면 강제 종료 (계속 실행하면 무한 루프 돔)
        TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);
    }

    // 다른 예외는 통과시켜서 다음 핸들러가 처리하게 함
    return EXCEPTION_CONTINUE_SEARCH;
}

// [C] CRT 잘못된 파라미터 핸들러 (delete에 쓰레기값 넣었을 때 호출)
void InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
    // 여기서 덤프 생성 (ExceptionInfo가 없으므로 nullptr 전달하거나 가짜 생성 가능)
    // 보통은 여기서 강제로 예외를 발생시켜 VEH가 잡게 하거나 직접 덤프를 뜸

    // 간단히 덤프만 남기고 종료
    CreateMiniDump(nullptr);
    TerminateProcess(GetCurrentProcess(), 0xC000000D); // STATUS_INVALID_PARAMETER
}

// [D] Pure Virtual Function Call 핸들러
void PureCallHandler()
{
    CreateMiniDump(nullptr);
    TerminateProcess(GetCurrentProcess(), 0xC0000025); // STATUS_NONCONTINUABLE_EXCEPTION
}


// ============================================================================
// 3. 초기화 함수 (모든 안전장치 해제 및 훅 설치)
// ============================================================================
void InitializeMiniDump()
{
    // 1. CRT 에러 메시지 박스 억제
    _CrtSetReportMode(_CRT_WARN, 0);
    _CrtSetReportMode(_CRT_ERROR, 0);
    _CrtSetReportMode(_CRT_ASSERT, 0);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    // 2. CRT 핸들러 등록 (Heap 관련 에러 가로채기)
    _set_invalid_parameter_handler(InvalidParameterHandler);
    _set_purecall_handler(PureCallHandler);

    // 3. SEH 핸들러 등록
    SetUnhandledExceptionFilter(UnhandledExceptionHandler);

    // 4. VEH 핸들러 등록
    // 첫 번째 인자 1: 제일 먼저 호출해달라
    AddVectoredExceptionHandler(1, VectoredExceptionHandler);
}

static bool bIsCrashInitialized = false;

void CauseCrash()
{
    bIsCrashInitialized = true;
}

#pragma optimize("", off)
void CrashLoop()
{
    if (!bIsCrashInitialized) { return; }

    uint32 ObjCount = GUObjectArray.Num();
    if (ObjCount == 0) return;

    uint32 RandomIdx = std::rand() % ObjCount;
    UObject* Target = GUObjectArray[RandomIdx];

    // Release 모드에서도 이 코드가 실행되어
    // 1. Access Violation이 나면 VEH가 잡고
    // 2. CRT 검사(유효하지 않은 힙 포인터)에 걸리면 InvalidParameterHandler가 잡습니다.
    if (Target)
    {
       Target->DeleteObjectDirty();
    }
}
#pragma optimize("", on)
