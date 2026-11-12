; Reference SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Static disassembly and original bytes; Capstone 5.0.7 x86 32-bit.

; VA 0x0042EB48, file offset 0x2EB48, length 245, SHA-256 605e7b64017fca65620c71ba4b31b4f23eb291de3303273d6e05868260913f20
0042EB48  55                              push ebp
0042EB49  8b ec                           mov ebp, esp
0042EB4B  6a ff                           push -1
0042EB4D  68 c3 af 4a 00                  push 0x4aafc3
0042EB52  64 a1 00 00 00 00               mov eax, dword ptr fs:[0]
0042EB58  50                              push eax
0042EB59  64 89 25 00 00 00 00            mov dword ptr fs:[0], esp
0042EB60  83 ec 34                        sub esp, 0x34
0042EB63  89 4d c4                        mov dword ptr [ebp - 0x3c], ecx
0042EB66  c7 45 d4 10 6a 4c 00            mov dword ptr [ebp - 0x2c], 0x4c6a10
0042EB6D  8d 45 dc                        lea eax, [ebp - 0x24]
0042EB70  50                              push eax
0042EB71  8b 4d c4                        mov ecx, dword ptr [ebp - 0x3c]
0042EB74  e8 b5 06 00 00                  call 0x42f22e
0042EB79  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
0042EB80  c6 45 d8 01                     mov byte ptr [ebp - 0x28], 1
0042EB84  c7 45 d0 00 00 00 00            mov dword ptr [ebp - 0x30], 0
0042EB8B  eb 09                           jmp 0x42eb96
0042EB8D  8b 4d d0                        mov ecx, dword ptr [ebp - 0x30]
0042EB90  83 c1 01                        add ecx, 1
0042EB93  89 4d d0                        mov dword ptr [ebp - 0x30], ecx
0042EB96  83 7d d0 03                     cmp dword ptr [ebp - 0x30], 3
0042EB9A  7f 7b                           jg 0x42ec17
0042EB9C  8b 55 c4                        mov edx, dword ptr [ebp - 0x3c]
0042EB9F  8b 42 44                        mov eax, dword ptr [edx + 0x44]
0042EBA2  0f af 45 d0                     imul eax, dword ptr [ebp - 0x30]
0042EBA6  89 45 c0                        mov dword ptr [ebp - 0x40], eax
0042EBA9  db 45 c0                        fild dword ptr [ebp - 0x40]
0042EBAC  dc 45 e4                        fadd qword ptr [ebp - 0x1c]
0042EBAF  83 ec 08                        sub esp, 8
0042EBB2  dd 1c 24                        fstp qword ptr [esp]
0042EBB5  8b 4d d4                        mov ecx, dword ptr [ebp - 0x2c]
0042EBB8  e8 93 f3 fd ff                  call 0x40df50
0042EBBD  e8 ea d9 06 00                  call 0x49c5ac
0042EBC2  50                              push eax
0042EBC3  8b 4d c4                        mov ecx, dword ptr [ebp - 0x3c]
0042EBC6  db 41 40                        fild dword ptr [ecx + 0x40]
0042EBC9  dc 45 dc                        fadd qword ptr [ebp - 0x24]
0042EBCC  83 ec 08                        sub esp, 8
0042EBCF  dd 1c 24                        fstp qword ptr [esp]
0042EBD2  8b 4d d4                        mov ecx, dword ptr [ebp - 0x2c]
0042EBD5  e8 e6 f2 fd ff                  call 0x40dec0
0042EBDA  e8 cd d9 06 00                  call 0x49c5ac
0042EBDF  50                              push eax
0042EBE0  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0042EBE5  e8 76 f2 fd ff                  call 0x40de60
0042EBEA  8b 10                           mov edx, dword ptr [eax]
0042EBEC  89 55 cc                        mov dword ptr [ebp - 0x34], edx
0042EBEF  8b 45 cc                        mov eax, dword ptr [ebp - 0x34]
0042EBF2  25 80 00 00 00                  and eax, 0x80
0042EBF7  85 c0                           test eax, eax
0042EBF9  75 17                           jne 0x42ec12
0042EBFB  8b 4d d8                        mov ecx, dword ptr [ebp - 0x28]
0042EBFE  81 e1 ff 00 00 00               and ecx, 0xff
0042EC04  33 d2                           xor edx, edx
0042EC06  83 7d cc 00                     cmp dword ptr [ebp - 0x34], 0
0042EC0A  0f 95 c2                        setne dl
0042EC0D  23 ca                           and ecx, edx
0042EC0F  88 4d d8                        mov byte ptr [ebp - 0x28], cl
0042EC12  e9 76 ff ff ff                  jmp 0x42eb8d
0042EC17  8a 45 d8                        mov al, byte ptr [ebp - 0x28]
0042EC1A  88 45 c8                        mov byte ptr [ebp - 0x38], al
0042EC1D  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0042EC24  8d 4d dc                        lea ecx, [ebp - 0x24]
0042EC27  e8 84 52 fd ff                  call 0x403eb0
0042EC2C  8a 45 c8                        mov al, byte ptr [ebp - 0x38]
0042EC2F  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0042EC32  64 89 0d 00 00 00 00            mov dword ptr fs:[0], ecx
0042EC39  8b e5                           mov esp, ebp
0042EC3B  5d                              pop ebp
0042EC3C  c3                              ret
