; 0x43ce88
0043CE88  55                        push ebp
0043CE89  8bec                      mov ebp, esp
0043CE8B  6aff                      push -1
0043CE8D  68ccbc4a00                push 0x4abccc
0043CE92  64a100000000              mov eax, dword ptr fs:[0]
0043CE98  50                        push eax
0043CE99  64892500000000            mov dword ptr fs:[0], esp
0043CEA0  51                        push ecx
0043CEA1  894df0                    mov dword ptr [ebp - 0x10], ecx
0043CEA4  8b45f0                    mov eax, dword ptr [ebp - 0x10]
0043CEA7  c700a8144b00              mov dword ptr [eax], 0x4b14a8
0043CEAD  c745fc00000000            mov dword ptr [ebp - 4], 0
0043CEB4  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
0043CEB7  81c1b0000000              add ecx, 0xb0
0043CEBD  e84bb30200                call 0x46820d
0043CEC2  c745fcffffffff            mov dword ptr [ebp - 4], 0xffffffff
0043CEC9  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
0043CECC  e8e6890200                call 0x4658b7
0043CED1  8b4df4                    mov ecx, dword ptr [ebp - 0xc]
0043CED4  64890d00000000            mov dword ptr fs:[0], ecx
0043CEDB  8be5                      mov esp, ebp
0043CEDD  5d                        pop ebp
0043CEDE  c3                        ret

; 0x46820d
0046820D  55                        push ebp
0046820E  8bec                      mov ebp, esp
00468210  6aff                      push -1
00468212  68ecda4a00                push 0x4adaec
00468217  64a100000000              mov eax, dword ptr fs:[0]
0046821D  50                        push eax
0046821E  64892500000000            mov dword ptr fs:[0], esp
00468225  51                        push ecx
00468226  894df0                    mov dword ptr [ebp - 0x10], ecx
00468229  c745fc00000000            mov dword ptr [ebp - 4], 0
00468230  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
00468233  e81d000000                call 0x468255
00468238  c745fcffffffff            mov dword ptr [ebp - 4], 0xffffffff
0046823F  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
00468242  e8a989fdff                call 0x440bf0
00468247  8b4df4                    mov ecx, dword ptr [ebp - 0xc]
0046824A  64890d00000000            mov dword ptr fs:[0], ecx
00468251  8be5                      mov esp, ebp
00468253  5d                        pop ebp
00468254  c3                        ret

; 0x4658b7
004658B7  55                        push ebp
004658B8  8bec                      mov ebp, esp
004658BA  51                        push ecx
004658BB  894dfc                    mov dword ptr [ebp - 4], ecx
004658BE  8b45fc                    mov eax, dword ptr [ebp - 4]
004658C1  c70058184b00              mov dword ptr [eax], 0x4b1858
004658C7  8b4dfc                    mov ecx, dword ptr [ebp - 4]
004658CA  83c108                    add ecx, 8
004658CD  e8de4bfbff                call 0x41a4b0
004658D2  8be5                      mov esp, ebp
004658D4  5d                        pop ebp
004658D5  c3                        ret

; 0x468255
00468255  55                        push ebp
00468256  8bec                      mov ebp, esp
00468258  51                        push ecx
00468259  894dfc                    mov dword ptr [ebp - 4], ecx
0046825C  8b45fc                    mov eax, dword ptr [ebp - 4]
0046825F  83b8fc010000ff            cmp dword ptr [eax + 0x1fc], -1
00468266  743a                      je 0x4682a2
00468268  8b4dfc                    mov ecx, dword ptr [ebp - 4]
0046826B  83b9f801000000            cmp dword ptr [ecx + 0x1f8], 0
00468272  7414                      je 0x468288
00468274  8b55fc                    mov edx, dword ptr [ebp - 4]
00468277  8b82fc010000              mov eax, dword ptr [edx + 0x1fc]
0046827D  50                        push eax
0046827E  b93cd25800                mov ecx, 0x58d23c
00468283  e8c87dfcff                call 0x430050
00468288  8b4dfc                    mov ecx, dword ptr [ebp - 4]
0046828B  c781fc010000ffffffff      mov dword ptr [ecx + 0x1fc], 0xffffffff
00468295  8b55fc                    mov edx, dword ptr [ebp - 4]
00468298  c782f801000000000000      mov dword ptr [edx + 0x1f8], 0
004682A2  8be5                      mov esp, ebp
004682A4  5d                        pop ebp
004682A5  c3                        ret

; 0x440bf0
00440BF0  55                        push ebp
00440BF1  8bec                      mov ebp, esp
00440BF3  51                        push ecx
00440BF4  894dfc                    mov dword ptr [ebp - 4], ecx
00440BF7  8b4dfc                    mov ecx, dword ptr [ebp - 4]
00440BFA  e811000000                call 0x440c10
00440BFF  8be5                      mov esp, ebp
00440C01  5d                        pop ebp
00440C02  c3                        ret

; 0x440c10
00440C10  55                        push ebp
00440C11  8bec                      mov ebp, esp
00440C13  51                        push ecx
00440C14  894dfc                    mov dword ptr [ebp - 4], ecx
00440C17  68a00d4200                push 0x420da0
00440C1C  6a04                      push 4
00440C1E  6a68                      push 0x68
00440C20  8b45fc                    mov eax, dword ptr [ebp - 4]
00440C23  83c008                    add eax, 8
00440C26  50                        push eax
00440C27  e829ba0500                call 0x49c655
00440C2C  8be5                      mov esp, ebp
00440C2E  5d                        pop ebp
00440C2F  c3                        ret

; 0x49c655
0049C655  55                        push ebp
0049C656  8bec                      mov ebp, esp
0049C658  6aff                      push -1
0049C65A  68c83f4b00                push 0x4b3fc8
0049C65F  6800f24900                push 0x49f200
0049C664  64a100000000              mov eax, dword ptr fs:[0]
0049C66A  50                        push eax
0049C66B  64892500000000            mov dword ptr fs:[0], esp
0049C672  83ec0c                    sub esp, 0xc
0049C675  53                        push ebx
0049C676  56                        push esi
0049C677  57                        push edi
0049C678  8365e400                  and dword ptr [ebp - 0x1c], 0
0049C67C  8b750c                    mov esi, dword ptr [ebp + 0xc]
0049C67F  8bc6                      mov eax, esi
0049C681  0faf4510                  imul eax, dword ptr [ebp + 0x10]
0049C685  014508                    add dword ptr [ebp + 8], eax
0049C688  8365fc00                  and dword ptr [ebp - 4], 0
0049C68C  ff4d10                    dec dword ptr [ebp + 0x10]
0049C68F  780b                      js 0x49c69c
0049C691  297508                    sub dword ptr [ebp + 8], esi
0049C694  8b4d08                    mov ecx, dword ptr [ebp + 8]
0049C697  ff5514                    call dword ptr [ebp + 0x14]
0049C69A  ebf0                      jmp 0x49c68c
0049C69C  c745e401000000            mov dword ptr [ebp - 0x1c], 1
0049C6A3  834dfcff                  or dword ptr [ebp - 4], 0xffffffff
0049C6A7  e811000000                call 0x49c6bd
0049C6AC  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
0049C6AF  64890d00000000            mov dword ptr fs:[0], ecx
0049C6B6  5f                        pop edi
0049C6B7  5e                        pop esi
0049C6B8  5b                        pop ebx
0049C6B9  c9                        leave
0049C6BA  c21000                    ret 0x10

; 0x49c6bd
0049C6BD  837de400                  cmp dword ptr [ebp - 0x1c], 0
0049C6C1  7511                      jne 0x49c6d4
0049C6C3  ff7514                    push dword ptr [ebp + 0x14]
0049C6C6  ff7510                    push dword ptr [ebp + 0x10]
0049C6C9  ff750c                    push dword ptr [ebp + 0xc]
0049C6CC  ff7508                    push dword ptr [ebp + 8]
0049C6CF  e801000000                call 0x49c6d5
0049C6D4  c3                        ret

; 0x41a4b0
0041A4B0  55                        push ebp
0041A4B1  8bec                      mov ebp, esp
0041A4B3  51                        push ecx
0041A4B4  894dfc                    mov dword ptr [ebp - 4], ecx
0041A4B7  8be5                      mov esp, ebp
0041A4B9  5d                        pop ebp
0041A4BA  c3                        ret
