004437F6  c785a8feffff00000000      mov dword ptr [ebp - 0x158], 0
00443800  eb0f                      jmp 0x443811
00443802  8b8da8feffff              mov ecx, dword ptr [ebp - 0x158]
00443808  83c101                    add ecx, 1
0044380B  898da8feffff              mov dword ptr [ebp - 0x158], ecx
00443811  83bda8feffff04            cmp dword ptr [ebp - 0x158], 4
00443818  0f8d80000000              jge 0x44389e
0044381E  8b95a8feffff              mov edx, dword ptr [ebp - 0x158]
00443824  33c0                      xor eax, eax
00443826  8a841560feffff            mov al, byte ptr [ebp + edx - 0x1a0]
0044382D  85c0                      test eax, eax
0044382F  7457                      je 0x443888
00443831  8b8da8feffff              mov ecx, dword ptr [ebp - 0x158]
00443837  8b948d64feffff            mov edx, dword ptr [ebp + ecx*4 - 0x19c]
0044383E  8b0495d8ff4b00            mov eax, dword ptr [edx*4 + 0x4bffd8]
00443845  50                        push eax
00443846  8b8da8feffff              mov ecx, dword ptr [ebp - 0x158]
0044384C  8b148dc8ff4b00            mov edx, dword ptr [ecx*4 + 0x4bffc8]
00443853  52                        push edx
00443854  6890004c00                push 0x4c0090
00443859  8d8554fdffff              lea eax, [ebp - 0x2ac]
0044385F  50                        push eax
00443860  e805920500                call 0x49ca6a
00443865  83c410                    add esp, 0x10
00443868  8d8d54fdffff              lea ecx, [ebp - 0x2ac]
0044386E  51                        push ecx
0044386F  b93cd25800                mov ecx, 0x58d23c
00443874  e8c7270000                call 0x446040
00443879  8b95a8feffff              mov edx, dword ptr [ebp - 0x158]
0044387F  8904955c334e00            mov dword ptr [edx*4 + 0x4e335c], eax
00443886  eb11                      jmp 0x443899
00443888  8b85a8feffff              mov eax, dword ptr [ebp - 0x158]
0044388E  c704855c334e00ffffffff    mov dword ptr [eax*4 + 0x4e335c], 0xffffffff
00443899  e964ffffff                jmp 0x443802
