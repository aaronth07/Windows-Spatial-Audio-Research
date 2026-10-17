option casemap:none
EXTERN NativeDataHelper:QWORD
.code
CreateHrtfDataHelper PROC
    mov rax, qword ptr [NativeDataHelper]
    test rax, rax
    jz HelperNotReady
    jmp rax
HelperNotReady:
    mov eax, 8000FFFFh
    ret
CreateHrtfDataHelper ENDP
Forward3 PROC
    mov rcx, [rcx+8]
    mov rax, [rcx]
    jmp qword ptr [rax+24]
Forward3 ENDP
Forward4 PROC
    mov rcx, [rcx+8]
    mov rax, [rcx]
    jmp qword ptr [rax+32]
Forward4 ENDP
Forward6 PROC
    mov rcx, [rcx+8]
    mov rax, [rcx]
    jmp qword ptr [rax+48]
Forward6 ENDP
Forward7 PROC
    mov rcx, [rcx+8]
    mov rax, [rcx]
    jmp qword ptr [rax+56]
Forward7 ENDP
Forward8 PROC
    mov rcx, [rcx+8]
    mov rax, [rcx]
    jmp qword ptr [rax+64]
Forward8 ENDP
END
