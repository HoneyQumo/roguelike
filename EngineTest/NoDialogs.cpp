#include "pch.h"

#include <cstdio>
#include <cstdlib>

#include <windows.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

namespace
{
	void OnInvalidParameter(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t)
	{
		std::fputs("CRT rejected a parameter - aborting\n", stderr);
		std::fflush(stderr);

		std::abort();
	}

	/**
	*	Глушит все окна, которыми падение теста способно остановить прогон.
	*
	*	В Actions нажать «ОК» некому: job висит до потолка в шесть часов вместо
	*	честного красного. Окна тут четыре, и механизмы у них разные, поэтому
	*	гасить приходится каждое отдельно.
	*
	*	Глобальный объект, потому что конструктор обязан отработать до gtest.
	*/
	struct NoDialogs
	{
		NoDialogs()
		{
			SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

			_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
			_set_invalid_parameter_handler(&OnInvalidParameter);

#ifdef _DEBUG
			for (int report : {_CRT_ASSERT, _CRT_ERROR, _CRT_WARN})
			{
				_CrtSetReportMode(report, _CRTDBG_MODE_FILE);
				_CrtSetReportFile(report, _CRTDBG_FILE_STDERR);
			}
#endif
		}
	};

	const NoDialogs noDialogs;
}
