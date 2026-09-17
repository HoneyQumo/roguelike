#include "pch.h"

#include <cstdio>
#include <io.h>
#include <windows.h>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

// Проверки просят режим значением _CRTDBG_REPORT_MODE: это запрос текущего, а не установка нового.

TEST(NoDialogsTest, WindowsDoesNotOfferToReportACrash)
{
	UINT mode = GetErrorMode();

	EXPECT_EQ(mode & SEM_NOGPFAULTERRORBOX, static_cast<UINT>(SEM_NOGPFAULTERRORBOX))
		<< "a crashing test would hang the run on the Windows error window";
	EXPECT_EQ(mode & SEM_FAILCRITICALERRORS, static_cast<UINT>(SEM_FAILCRITICALERRORS));
}

#ifdef _DEBUG

TEST(NoDialogsTest, ADebugAssertIsWrittenNotShown)
{
	EXPECT_EQ(_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_REPORT_MODE), _CRTDBG_MODE_FILE)
		<< "a broken debug check would wait for someone to click OK";
}

TEST(NoDialogsTest, ADebugAssertGoesToStderr)
{
	// Запрос возвращает уже разрешённый дескриптор, а не ту метку, которой его задавали.
	_HFILE where = _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_REPORT_FILE);

	EXPECT_EQ(where, reinterpret_cast<_HFILE>(_get_osfhandle(_fileno(stderr))));
}

TEST(NoDialogsTest, EveryKindOfReportIsSilenced)
{
	for (int report : {_CRT_ERROR, _CRT_WARN})
	{
		EXPECT_EQ(_CrtSetReportMode(report, _CRTDBG_REPORT_MODE), _CRTDBG_MODE_FILE) << "report kind " << report;
	}
}

#endif
