EXTERN InstrumentationCallbackDispatcher : NEAR
EXTERNDEF __imp_RtlCaptureContext : QWORD
EXTERNDEF __imp_RtlRestoreContext : QWORD

.code
    InstrumentationCallbackStub proc
        mov     gs:[2D8h], r10
        mov     gs:[2E0h], rsp
        mov     r10, rcx
        sub     rsp, 4D0h
        and     rsp, -10h
        mov     rcx, rsp
        call    __imp_RtlCaptureContext
        sub     rsp, 20h
        call    InstrumentationCallbackDispatcher
		  add		rsp, 20h
		  mov		rcx, rsp
		  xor		rdx, rdx
		  call	__imp_RtlRestoreContext
    InstrumentationCallbackStub endp
end
