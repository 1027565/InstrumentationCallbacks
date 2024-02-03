#ifndef __INSTRUMENTATION_CALLBACKS__
#define __INSTRUMENTATION_CALLBACKS__

#include <phnt_windows.h>
#include <phnt.h>

#pragma comment(lib, "ntdll.lib")

typedef void(*PKNORMAL_ROUTINE)(
	void* NormalContext,
	void* SystemArgument1,
	void* SystemArgument2
);

typedef enum _INSTRUMENTATION_CALLBACK_TYPE
{
	CallbackTypeRaiseUserException,
	CallbackTypeUserApc,
	CallbackTypeUserCallback,
	CallbackTypeUserException,
	CallbackTypeUserThreadStart,
	CallbackTypeSystemCallReturn,
	MaxInstrumentationCallbackType
} INSTRUMENTATION_CALLBACK_TYPE;

typedef void(*RAISE_USER_EXCEPTION_CALLBACK)(CONTEXT* ContextRecord);

typedef void(*USER_APC_CALLBACK)(
	CONTEXT* ContextRecord,
	void*    NormalContext,
	void*    SystemArgument1,
	void*    SystemArgument2,
	PKNORMAL_ROUTINE NormalRoutine
);

typedef void(*USER_WIN32_CALLBACK)(
	void* Argument,
	ULONG ArgumentSize,
	ULONG CallbackIndex
);

typedef void(*USER_EXCEPTION_CALLBACK)(
	CONTEXT*	  ContextRecord,
	EXCEPTION_RECORD* ExceptionRecord
);

typedef void(*USER_THREAD_START_CALLBACK)(
	CONTEXT* ContextRecord,
	void*    Parameter
);

typedef void(*SYSRET_CALLBACK)(
	CONTEXT* ContextRecord,
	NTSTATUS Status
);

NTSTATUS InitializeInstrumentationCallbacks();
NTSTATUS UninitializeInstrumentationCallbacks();

NTSTATUS SetInstrumentationCallback(
    INSTRUMENTATION_CALLBACK_TYPE CallbackType,
    void*                         CallbackRoutine
);

#endif // __INSTRUMENTATION_CALLBACKS__
