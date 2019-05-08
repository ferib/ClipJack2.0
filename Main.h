#pragma once
#include "stdafx.h"

namespace Main
{
	void InjectHook();
	LONG vehHandler(_EXCEPTION_POINTERS* exceptionInfo);
	DWORD WINAPI Init(void* const(param));
}