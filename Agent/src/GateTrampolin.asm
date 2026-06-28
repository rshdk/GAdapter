.code

Preparecall PROC

  xor r11, r11
  xor r10, r10
  mov r11, rcx
  mov r10, rdx
  ret


Preparecall ENDP

Docall Proc

  push r10
  xor rax, rax
  mov r10, rcx
  mov eax, r11d
  ret
	
Docall ENDP

end
