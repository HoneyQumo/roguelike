#include "CrashReport.h"
#include "LoggerRegistry.h"
#include <windows.h>
#include <dbghelp.h>
#include <csignal>
#include <crtdbg.h>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <sstream>
#pragma comment(lib, "dbghelp.lib")

namespace RoguelikeGame
{
    namespace
    {
    void WriteStack(const char* reason, CONTEXT* context)
    {
        std::ostringstream head;
        head << "CRASH " << reason;
        LOG_ERROR(head.str());

        HANDLE process = GetCurrentProcess();
        SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        SymInitialize(process, nullptr, TRUE);

        void* frames[64] = {};
        USHORT taken = 0;

        if (context != nullptr)
        {
            STACKFRAME64 frame = {};
            frame.AddrPC.Offset = context->Rip;
            frame.AddrPC.Mode = AddrModeFlat;
            frame.AddrFrame.Offset = context->Rbp;
            frame.AddrFrame.Mode = AddrModeFlat;
            frame.AddrStack.Offset = context->Rsp;
            frame.AddrStack.Mode = AddrModeFlat;

            CONTEXT walked = *context;
            while (taken < 64 && StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, GetCurrentThread(), &frame,
                &walked, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
            {
                if (frame.AddrPC.Offset == 0) { break; }
                frames[taken++] = reinterpret_cast<void*>(frame.AddrPC.Offset);
            }
        }
        else
        {
            taken = CaptureStackBackTrace(0, 64, frames, nullptr);
        }

        for (USHORT i = 0; i < taken; i++)
        {
            char buffer[sizeof(SYMBOL_INFO) + 512] = {};
            auto symbol = reinterpret_cast<SYMBOL_INFO*>(buffer);
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = 500;

            DWORD64 displacement = 0;
            std::ostringstream line;
            line << "CRASH  #" << i << " ";

            if (SymFromAddr(process, reinterpret_cast<DWORD64>(frames[i]), &displacement, symbol))
            {
                line << symbol->Name;
            }
            else
            {
                line << "0x" << std::hex << reinterpret_cast<DWORD64>(frames[i]);
            }

            IMAGEHLP_LINE64 source = {};
            source.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            DWORD column = 0;
            if (SymGetLineFromAddr64(process, reinterpret_cast<DWORD64>(frames[i]), &column, &source))
            {
                line << "  (" << source.FileName << ":" << source.LineNumber << ")";
            }

            LOG_ERROR(line.str());
        }
    }

    LONG WINAPI OnSeh(EXCEPTION_POINTERS* info)
    {
        std::ostringstream reason;
        reason << "SEH code 0x" << std::hex << info->ExceptionRecord->ExceptionCode
               << " at 0x" << reinterpret_cast<DWORD64>(info->ExceptionRecord->ExceptionAddress);

        if (info->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION
            && info->ExceptionRecord->NumberParameters >= 2)
        {
            reason << (info->ExceptionRecord->ExceptionInformation[0] == 0 ? " reading 0x" : " writing 0x")
                   << info->ExceptionRecord->ExceptionInformation[1];
        }

        WriteStack(reason.str().c_str(), info->ContextRecord);

        return EXCEPTION_EXECUTE_HANDLER;
    }

    void OnTerminate()
    {
        std::string what = "terminate";
        try
        {
            if (std::current_exception()) { std::rethrow_exception(std::current_exception()); }
        }
        catch (const std::exception& exception) { what = std::string("terminate: ") + exception.what(); }
        catch (...) { what = "terminate: unknown exception"; }

        WriteStack(what.c_str(), nullptr);
        _exit(3);
    }

    void OnAbort(int) { WriteStack("abort", nullptr); _exit(3); }

    void OnInvalidParameter(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t)
    {
        WriteStack("CRT invalid parameter (STL bounds/iterator check)", nullptr);
        _exit(3);
    }

    int OnCrtReport(int reportType, char* message, int* returnValue)
    {
        WriteStack((std::string("CRT assert: ") + (message != nullptr ? message : "?")).c_str(), nullptr);
        _exit(3);
    }

    LONG WINAPI OnVectored(EXCEPTION_POINTERS* info)
    {
        DWORD code = info->ExceptionRecord->ExceptionCode;
        if (code == EXCEPTION_ACCESS_VIOLATION || code == EXCEPTION_STACK_OVERFLOW
            || code == EXCEPTION_ILLEGAL_INSTRUCTION || code == EXCEPTION_INT_DIVIDE_BY_ZERO
            || code == EXCEPTION_ARRAY_BOUNDS_EXCEEDED || code == 0xC0000374)
        {
            OnSeh(info);
            _exit(3);
        }

        return EXCEPTION_CONTINUE_SEARCH;
    }
    }

    void InstallCrashReport()
    {
        AddVectoredExceptionHandler(1, OnVectored);
        _set_invalid_parameter_handler(OnInvalidParameter);
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportHook(OnCrtReport);
        SetUnhandledExceptionFilter(OnSeh);
        std::set_terminate(OnTerminate);
        signal(SIGABRT, OnAbort);
        signal(SIGSEGV, OnAbort);
        _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

        // Без этой строки нельзя отличить «не падало» от «сеть безопасности не включилась».
        LOG_INFO("Crash report is installed");
    }
}
