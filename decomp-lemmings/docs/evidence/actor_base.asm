004230A2  55                        push ebp
004230A3  8bec                      mov ebp, esp
004230A5  6aff                      push -1
004230A7  687ca94a00                push 0x4aa97c
004230AC  64a100000000              mov eax, dword ptr fs:[0]
004230B2  50                        push eax
004230B3  64892500000000            mov dword ptr fs:[0], esp
004230BA  83ec08                    sub esp, 8
004230BD  56                        push esi
004230BE  57                        push edi
004230BF  894dec                    mov dword ptr [ebp - 0x14], ecx
004230C2  8b4dec                    mov ecx, dword ptr [ebp - 0x14]
004230C5  83c108                    add ecx, 8
004230C8  e8d335feff                call 0x4066a0
004230CD  c745fc00000000            mov dword ptr [ebp - 4], 0
004230D4  8b45ec                    mov eax, dword ptr [ebp - 0x14]
004230D7  c700900e4b00              mov dword ptr [eax], 0x4b0e90
004230DD  8b4dec                    mov ecx, dword ptr [ebp - 0x14]
004230E0  c6413800                  mov byte ptr [ecx + 0x38], 0
004230E4  8b55ec                    mov edx, dword ptr [ebp - 0x14]
004230E7  c6422801                  mov byte ptr [edx + 0x28], 1
004230EB  8b7508                    mov esi, dword ptr [ebp + 8]
004230EE  8b7dec                    mov edi, dword ptr [ebp - 0x14]
004230F1  83c708                    add edi, 8
004230F4  b906000000                mov ecx, 6
004230F9  f3a5                      rep movsd dword ptr es:[edi], dword ptr [esi]
004230FB  8b45ec                    mov eax, dword ptr [ebp - 0x14]
004230FE  c7403000000000            mov dword ptr [eax + 0x30], 0
00423105  c7403400000000            mov dword ptr [eax + 0x34], 0
0042310C  8b4dec                    mov ecx, dword ptr [ebp - 0x14]
0042310F  c7412000000000            mov dword ptr [ecx + 0x20], 0
00423116  c7412400000000            mov dword ptr [ecx + 0x24], 0
0042311D  8b55ec                    mov edx, dword ptr [ebp - 0x14]
00423120  66a1ec554c00              mov ax, word ptr [0x4c55ec]
00423126  6689423a                  mov word ptr [edx + 0x3a], ax
0042312A  668b0dec554c00            mov cx, word ptr [0x4c55ec]
00423131  6683c101                  add cx, 1
00423135  66890dec554c00            mov word ptr [0x4c55ec], cx
0042313C  8b55ec                    mov edx, dword ptr [ebp - 0x14]
0042313F  8955f0                    mov dword ptr [ebp - 0x10], edx
00423142  8d45f0                    lea eax, [ebp - 0x10]
00423145  50                        push eax
00423146  b918bb4c00                mov ecx, 0x4cbb18
0042314B  e850020000                call 0x4233a0
00423150  c745fcffffffff            mov dword ptr [ebp - 4], 0xffffffff
00423157  8b45ec                    mov eax, dword ptr [ebp - 0x14]
0042315A  8b4df4                    mov ecx, dword ptr [ebp - 0xc]
0042315D  64890d00000000            mov dword ptr fs:[0], ecx
00423164  5f                        pop edi
00423165  5e                        pop esi
00423166  8be5                      mov esp, ebp
00423168  5d                        pop ebp
00423169  c20400                    ret 4
