#pragma once

// C4091: 'typedef ': 'long'의 왼쪽에 아무것도 지정하지 않으면 무시됩니다.
// dbghelp.h에서 발생하는 경고를 비활성화합니다.
#pragma warning(push)
#pragma warning(disable: 4091)
#include <dbghelp.h>
#pragma warning(pop)

#include <windows.h>

#pragma comment(lib, "dbghelp.lib")

/**
 * @brief 처리되지 않은 예외가 발생했을 때 호출될 콜백 함수
 * 이 함수 내에서 MiniDumpWriteDump를 호출하여 덤프 파일 생성
 * @param ExceptionInfo 예외 정보 포인터
 * @return 예외 처리 상태
 */
LONG WINAPI UnhandledExceptionHandler(struct _EXCEPTION_POINTERS* ExceptionInfo);

/**
 * @brief 미니덤프 예외 핸들러를 시스템에 등록합니다.
 * 엔진 초기화 시 한 번만 호출
 */
void InitializeMiniDump();

/**
 * @brief "CauseCrash" 콘솔 명령어에 바인딩할 함수.
*/
void CauseCrash();

void CrashLoop();

/**
 * @brief 크래시 다이얼로그를 표시합니다.
 * 사용자에게 크래시 정보를 보여주고 덤프 파일 위치를 안내합니다.
 * @param ExceptionInfo 예외 정보 포인터 (nullptr 가능)
 * @param DumpFilePath 생성된 덤프 파일 경로
 * @param CallStackInfo 호출 스택 정보 문자열
 * @param CrashLocation 크래시 발생 위치 (함수명, 파일, 라인)
 */
void ShowCrashDialog(struct _EXCEPTION_POINTERS* ExceptionInfo, const wchar_t* DumpFilePath, const wchar_t* CallStackInfo, const wchar_t* CrashLocation);
