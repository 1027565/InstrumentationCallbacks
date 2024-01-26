#include "instrumentation.h"

#if _M_X64
    #define INSTRUMENTATION_CALLBACK_VERSION 0
#else
    #define INSTRUMENTATION_CALLBACK_VERSION 1
#endif

RAISE_USER_EXCEPTION_CALLBACK RaiseUserExceptionCallback;
USER_APC_CALLBACK UserApcCallback = NULL;
USER_WIN32_CALLBACK UserWin32Callback = NULL;
USER_EXCEPTION_CALLBACK UserExceptionCallback = NULL;
USER_THREAD_START_CALLBACK UserThreadStartCallback = NULL;
SYSRET_CALLBACK SystemCallReturnCallback = NULL;

RTL_SRWLOCK InstrumentationCallbackLock = RTL_SRWLOCK_INIT;

// External methods

EXTERN_C NTSYSAPI void KiRaiseUserExceptionDispatcher();
EXTERN_C NTSYSAPI void KiUserApcDispatcher();
EXTERN_C NTSYSAPI void KiUserCallbackDispatcher();
EXTERN_C NTSYSAPI void KiUserExceptionDispatcher();

// Private methods

extern "C" void InstrumentationCallbackStub();

inline void KiUserApcDispatcherStub(CONTEXT* ContextRecord)
{
    CONTEXT* pReturnContextRecord = (CONTEXT*)(ContextRecord->Rsp);

    UserApcCallback(
        pReturnContextRecord,
        (void*)pReturnContextRecord->P1Home,
        (void*)pReturnContextRecord->P2Home,
        (void*)pReturnContextRecord->P3Home,
        (PKNORMAL_ROUTINE)pReturnContextRecord->P4Home
    );
}

inline void KiUserCallbackDispatcherStub(CONTEXT* ContextRecord)
{
	UserWin32Callback(
		*(void**)(ContextRecord->Rsp + 0x20),
		*(ULONG*)(ContextRecord->Rsp + 0x28),
		*(ULONG*)(ContextRecord->Rsp + 0x2C)
	);
}

inline void KiUserExceptionDispatcherStub(CONTEXT* ContextRecord)
{
    UserExceptionCallback(
        (CONTEXT*)(ContextRecord->Rsp),
        (EXCEPTION_RECORD*)(ContextRecord->Rsp + sizeof(CONTEXT) + 0x20)
    );
}

extern "C" void InstrumentationCallbackDispatcher(CONTEXT* ContextRecord)
{
    TEB* pTeb = NtCurrentTeb();

    ContextRecord->Rip = pTeb->InstrumentationCallbackPreviousPc;
    ContextRecord->Rsp = pTeb->InstrumentationCallbackPreviousSp;
    ContextRecord->Rcx = ContextRecord->R10;

    if (!pTeb->InstrumentationCallbackDisabled)
    {
        pTeb->InstrumentationCallbackDisabled = TRUE;

        RtlAcquireSRWLockShared(&InstrumentationCallbackLock);

		switch (ContextRecord->Rip)
		{
			case (ULONG_PTR)&KiRaiseUserExceptionDispatcher:
			{
				if (RaiseUserExceptionCallback)
					RaiseUserExceptionCallback(ContextRecord);
				break;
			}
			case (ULONG_PTR)&KiUserApcDispatcher:
			{
				if (UserApcCallback)
					KiUserApcDispatcherStub(ContextRecord);
				break;
			}
			case (ULONG_PTR)&KiUserCallbackDispatcher:
			{
				if (UserWin32Callback)
					KiUserCallbackDispatcherStub(ContextRecord);
				break;
			}
			case (ULONG_PTR)&KiUserExceptionDispatcher:
			{
				if (UserExceptionCallback)
					KiUserExceptionDispatcherStub(ContextRecord);
				break;
			}
			case (ULONG_PTR)&LdrInitializeThunk:
			{
				if (UserThreadStartCallback)
					UserThreadStartCallback((CONTEXT*)ContextRecord->Rcx, (void*)ContextRecord->Rdx);
			}
			default:
			{
				if (SystemCallReturnCallback)
					SystemCallReturnCallback(ContextRecord, (NTSTATUS)ContextRecord->Rax);
			}
		}

        RtlReleaseSRWLockShared(&InstrumentationCallbackLock);

        pTeb->InstrumentationCallbackDisabled = FALSE;
    }
}

NTSTATUS InitializeInstrumentationCallbackInternal(void* CallbackStub)
{
    PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION pici = { 0 };

    pici.Version = INSTRUMENTATION_CALLBACK_VERSION;
    pici.Reserved = 0;
    pici.Callback = CallbackStub;

    return NtSetInformationProcess(
        GetCurrentProcess(),
        ProcessInstrumentationCallback,
        &pici,
        sizeof(PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION)
    );
}

// Public methods

NTSTATUS InitializeInstrumentationCallbacks()
{
    return InitializeInstrumentationCallbackInternal(&InstrumentationCallbackStub);
}

NTSTATUS UninitializeInstrumentationCallbacks()
{
    return InitializeInstrumentationCallbackInternal(NULL);
}

NTSTATUS SetInstrumentationCallback(
    INSTRUMENTATION_CALLBACK_TYPE CallbackType,
    void*                         CallbackRoutine
)
{
    NTSTATUS status = STATUS_SUCCESS;

    RtlAcquireSRWLockExclusive(&InstrumentationCallbackLock);

    switch (CallbackType)
    {
		case CallbackTypeRaiseUserException:
			RaiseUserExceptionCallback = (RAISE_USER_EXCEPTION_CALLBACK)CallbackRoutine;
			break;
        case CallbackTypeUserApc:
            UserApcCallback = (USER_APC_CALLBACK)CallbackRoutine;
            break;
		case CallbackTypeUserCallback:
			UserWin32Callback = (USER_WIN32_CALLBACK)CallbackRoutine;
			break;
        case CallbackTypeUserException:
            UserExceptionCallback = (USER_EXCEPTION_CALLBACK)CallbackRoutine;
            break;
        case CallbackTypeUserThreadStart:
            UserThreadStartCallback = (USER_THREAD_START_CALLBACK)CallbackRoutine;
            break;
        case CallbackTypeSystemCallReturn:
            SystemCallReturnCallback = (SYSRET_CALLBACK)CallbackRoutine;
        default:
            status = STATUS_NOT_IMPLEMENTED;
    }

    RtlReleaseSRWLockExclusive(&InstrumentationCallbackLock);

    return status;
}
