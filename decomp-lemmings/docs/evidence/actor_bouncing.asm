; Original x86 instruction bytes; static disassembly, not runtime evidence.
; Reference SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Decoder: Capstone 5.0.7, x86 32-bit; addresses are virtual addresses.

; VA 0x0040EC2A, file offset 0xEC2A, length 25, SHA-256 dfc6af5ce6c9a8e03afeab202ec312f8e5cb6f984bd2d3d341cd795bbc1ffea7
0040EC2A  55                              push ebp
0040EC2B  8b ec                           mov ebp, esp
0040EC2D  dd 05 08 08 4b 00               fld qword ptr [0x4b0808]
0040EC33  d9 e0                           fchs
0040EC35  dc 35 00 08 4b 00               fdiv qword ptr [0x4b0800]
0040EC3B  dd 1d a8 55 4c 00               fstp qword ptr [0x4c55a8]
0040EC41  5d                              pop ebp
0040EC42  c3                              ret

; VA 0x0040EC7E, file offset 0xEC7E, length 48, SHA-256 6eecedeb1303e6e4de0ba7303e979661fb59d1939bb6125cbef3627b5f8f4cc4
0040EC7E  55                              push ebp
0040EC7F  8b ec                           mov ebp, esp
0040EC81  51                              push ecx
0040EC82  89 4d fc                        mov dword ptr [ebp - 4], ecx
0040EC85  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
0040EC88  e8 03 97 00 00                  call 0x418390
0040EC8D  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0040EC90  c7 00 10 08 4b 00               mov dword ptr [eax], 0x4b0810
0040EC96  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
0040EC99  c7 41 14 21 00 00 00            mov dword ptr [ecx + 0x14], 0x21
0040ECA0  8b 55 fc                        mov edx, dword ptr [ebp - 4]
0040ECA3  c6 42 11 00                     mov byte ptr [edx + 0x11], 0
0040ECA7  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0040ECAA  8b e5                           mov esp, ebp
0040ECAC  5d                              pop ebp
0040ECAD  c3                              ret

; VA 0x0040ECCA, file offset 0xECCA, length 464, SHA-256 5d7f1cdd55279cba0c7c587006761c1e63c68623f47cb5de4d3085691c31b35d
0040ECCA  55                              push ebp
0040ECCB  8b ec                           mov ebp, esp
0040ECCD  6a ff                           push -1
0040ECCF  68 39 9c 4a 00                  push 0x4a9c39
0040ECD4  64 a1 00 00 00 00               mov eax, dword ptr fs:[0]
0040ECDA  50                              push eax
0040ECDB  64 89 25 00 00 00 00            mov dword ptr fs:[0], esp
0040ECE2  83 ec 4c                        sub esp, 0x4c
0040ECE5  89 4d b0                        mov dword ptr [ebp - 0x50], ecx
0040ECE8  8d 45 d0                        lea eax, [ebp - 0x30]
0040ECEB  50                              push eax
0040ECEC  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040ECEF  e8 3a 05 02 00                  call 0x42f22e
0040ECF4  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
0040ECFB  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040ECFE  db 41 44                        fild dword ptr [ecx + 0x44]
0040ED01  dc 6d d8                        fsubr qword ptr [ebp - 0x28]
0040ED04  dd 5d d8                        fstp qword ptr [ebp - 0x28]
0040ED07  dd 45 d8                        fld qword ptr [ebp - 0x28]
0040ED0A  e8 9d d8 08 00                  call 0x49c5ac
0040ED0F  89 45 ac                        mov dword ptr [ebp - 0x54], eax
0040ED12  db 45 ac                        fild dword ptr [ebp - 0x54]
0040ED15  8b 55 b0                        mov edx, dword ptr [ebp - 0x50]
0040ED18  dd 5a 18                        fstp qword ptr [edx + 0x18]
0040ED1B  8b 45 b0                        mov eax, dword ptr [ebp - 0x50]
0040ED1E  8b 4d d0                        mov ecx, dword ptr [ebp - 0x30]
0040ED21  89 48 20                        mov dword ptr [eax + 0x20], ecx
0040ED24  8b 55 d4                        mov edx, dword ptr [ebp - 0x2c]
0040ED27  89 50 24                        mov dword ptr [eax + 0x24], edx
0040ED2A  8b 45 dc                        mov eax, dword ptr [ebp - 0x24]
0040ED2D  50                              push eax
0040ED2E  8b 4d d8                        mov ecx, dword ptr [ebp - 0x28]
0040ED31  51                              push ecx
0040ED32  8b 55 d4                        mov edx, dword ptr [ebp - 0x2c]
0040ED35  52                              push edx
0040ED36  8b 45 d0                        mov eax, dword ptr [ebp - 0x30]
0040ED39  50                              push eax
0040ED3A  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040ED3D  e8 be 7c ff ff                  call 0x406a00
0040ED42  0f bf c8                        movsx ecx, ax
0040ED45  51                              push ecx
0040ED46  68 50 f1 4b 00                  push 0x4bf150
0040ED4B  e8 10 0b 00 00                  call 0x40f860
0040ED50  83 c4 18                        add esp, 0x18
0040ED53  68 84 f1 4b 00                  push 0x4bf184
0040ED58  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040ED5B  e8 c5 0f 02 00                  call 0x42fd25
0040ED60  8b 55 08                        mov edx, dword ptr [ebp + 8]
0040ED63  db 42 44                        fild dword ptr [edx + 0x44]
0040ED66  dc 6d d8                        fsubr qword ptr [ebp - 0x28]
0040ED69  e8 3e d8 08 00                  call 0x49c5ac
0040ED6E  89 45 a8                        mov dword ptr [ebp - 0x58], eax
0040ED71  db 45 a8                        fild dword ptr [ebp - 0x58]
0040ED74  83 ec 08                        sub esp, 8
0040ED77  dd 1c 24                        fstp qword ptr [esp]
0040ED7A  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040ED7F  e8 cc f1 ff ff                  call 0x40df50
0040ED84  e8 23 d8 08 00                  call 0x49c5ac
0040ED89  89 45 c8                        mov dword ptr [ebp - 0x38], eax
0040ED8C  dd 45 d0                        fld qword ptr [ebp - 0x30]
0040ED8F  e8 18 d8 08 00                  call 0x49c5ac
0040ED94  89 45 cc                        mov dword ptr [ebp - 0x34], eax
0040ED97  db 45 cc                        fild dword ptr [ebp - 0x34]
0040ED9A  83 ec 08                        sub esp, 8
0040ED9D  dd 1c 24                        fstp qword ptr [esp]
0040EDA0  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040EDA5  e8 16 f1 ff ff                  call 0x40dec0
0040EDAA  e8 fd d7 08 00                  call 0x49c5ac
0040EDAF  89 45 cc                        mov dword ptr [ebp - 0x34], eax
0040EDB2  c7 45 f0 00 00 00 00            mov dword ptr [ebp - 0x10], 0
0040EDB9  c7 45 e8 00 00 02 00            mov dword ptr [ebp - 0x18], 0x20000
0040EDC0  c7 45 ec 00 00 00 00            mov dword ptr [ebp - 0x14], 0
0040EDC7  eb 09                           jmp 0x40edd2
0040EDC9  8b 45 ec                        mov eax, dword ptr [ebp - 0x14]
0040EDCC  83 c0 01                        add eax, 1
0040EDCF  89 45 ec                        mov dword ptr [ebp - 0x14], eax
0040EDD2  b9 e8 89 4c 00                  mov ecx, 0x4c89e8
0040EDD7  e8 54 5f ff ff                  call 0x404d30
0040EDDC  39 45 ec                        cmp dword ptr [ebp - 0x14], eax
0040EDDF  7d 72                           jge 0x40ee53
0040EDE1  8b 4d ec                        mov ecx, dword ptr [ebp - 0x14]
0040EDE4  51                              push ecx
0040EDE5  b9 e8 89 4c 00                  mov ecx, 0x4c89e8
0040EDEA  e8 21 5f ff ff                  call 0x404d10
0040EDEF  8b 10                           mov edx, dword ptr [eax]
0040EDF1  52                              push edx
0040EDF2  b9 08 6a 4c 00                  mov ecx, 0x4c6a08
0040EDF7  e8 a5 5d 03 00                  call 0x444ba1
0040EDFC  89 45 c4                        mov dword ptr [ebp - 0x3c], eax
0040EDFF  83 7d c4 00                     cmp dword ptr [ebp - 0x3c], 0
0040EE03  74 49                           je 0x40ee4e
0040EE05  8b 45 c4                        mov eax, dword ptr [ebp - 0x3c]
0040EE08  dd 40 08                        fld qword ptr [eax + 8]
0040EE0B  e8 9c d7 08 00                  call 0x49c5ac
0040EE10  2b 45 cc                        sub eax, dword ptr [ebp - 0x34]
0040EE13  89 45 b8                        mov dword ptr [ebp - 0x48], eax
0040EE16  8b 4d c4                        mov ecx, dword ptr [ebp - 0x3c]
0040EE19  dd 41 10                        fld qword ptr [ecx + 0x10]
0040EE1C  e8 8b d7 08 00                  call 0x49c5ac
0040EE21  2b 45 c8                        sub eax, dword ptr [ebp - 0x38]
0040EE24  89 45 c0                        mov dword ptr [ebp - 0x40], eax
0040EE27  8b 55 b8                        mov edx, dword ptr [ebp - 0x48]
0040EE2A  0f af 55 b8                     imul edx, dword ptr [ebp - 0x48]
0040EE2E  8b 45 c0                        mov eax, dword ptr [ebp - 0x40]
0040EE31  0f af 45 c0                     imul eax, dword ptr [ebp - 0x40]
0040EE35  03 d0                           add edx, eax
0040EE37  89 55 bc                        mov dword ptr [ebp - 0x44], edx
0040EE3A  8b 4d bc                        mov ecx, dword ptr [ebp - 0x44]
0040EE3D  3b 4d e8                        cmp ecx, dword ptr [ebp - 0x18]
0040EE40  7d 0c                           jge 0x40ee4e
0040EE42  8b 55 c4                        mov edx, dword ptr [ebp - 0x3c]
0040EE45  89 55 f0                        mov dword ptr [ebp - 0x10], edx
0040EE48  8b 45 bc                        mov eax, dword ptr [ebp - 0x44]
0040EE4B  89 45 e8                        mov dword ptr [ebp - 0x18], eax
0040EE4E  e9 76 ff ff ff                  jmp 0x40edc9
0040EE53  83 7d f0 00                     cmp dword ptr [ebp - 0x10], 0
0040EE57  74 22                           je 0x40ee7b
0040EE59  8b 4d f0                        mov ecx, dword ptr [ebp - 0x10]
0040EE5C  8b 91 40 03 00 00               mov edx, dword ptr [ecx + 0x340]
0040EE62  83 7a 14 0e                     cmp dword ptr [edx + 0x14], 0xe
0040EE66  75 13                           jne 0x40ee7b
0040EE68  8b 45 f0                        mov eax, dword ptr [ebp - 0x10]
0040EE6B  8b 88 40 03 00 00               mov ecx, dword ptr [eax + 0x340]
0040EE71  89 4d b4                        mov dword ptr [ebp - 0x4c], ecx
0040EE74  8b 55 b4                        mov edx, dword ptr [ebp - 0x4c]
0040EE77  c6 42 18 01                     mov byte ptr [edx + 0x18], 1
0040EE7B  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0040EE82  8d 4d d0                        lea ecx, [ebp - 0x30]
0040EE85  e8 26 50 ff ff                  call 0x403eb0
0040EE8A  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0040EE8D  64 89 0d 00 00 00 00            mov dword ptr fs:[0], ecx
0040EE94  8b e5                           mov esp, ebp
0040EE96  5d                              pop ebp
0040EE97  c2 04 00                        ret 4

; VA 0x0040EE9A, file offset 0xEE9A, length 42, SHA-256 c52aef7d08c81ee7f1335959a70a1b7e434d658d7d95ef45b9390fbb462a00a2
0040EE9A  55                              push ebp
0040EE9B  8b ec                           mov ebp, esp
0040EE9D  51                              push ecx
0040EE9E  89 4d fc                        mov dword ptr [ebp - 4], ecx
0040EEA1  6a 2d                           push 0x2d
0040EEA3  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040EEA6  e8 1e fe 01 00                  call 0x42ecc9
0040EEAB  25 ff 00 00 00                  and eax, 0xff
0040EEB0  85 c0                           test eax, eax
0040EEB2  74 0a                           je 0x40eebe
0040EEB4  6a 2d                           push 0x2d
0040EEB6  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040EEB9  e8 35 f9 01 00                  call 0x42e7f3
0040EEBE  8b e5                           mov esp, ebp
0040EEC0  5d                              pop ebp
0040EEC1  c2 04 00                        ret 4

; VA 0x0040EEC4, file offset 0xEEC4, length 805, SHA-256 dcf476b9e2156235f9441036d143424b379f049e42f9f5a1e4a8e4680dc5be89
0040EEC4  55                              push ebp
0040EEC5  8b ec                           mov ebp, esp
0040EEC7  6a ff                           push -1
0040EEC9  68 4e 9c 4a 00                  push 0x4a9c4e
0040EECE  64 a1 00 00 00 00               mov eax, dword ptr fs:[0]
0040EED4  50                              push eax
0040EED5  64 89 25 00 00 00 00            mov dword ptr fs:[0], esp
0040EEDC  83 ec 54                        sub esp, 0x54
0040EEDF  89 4d a4                        mov dword ptr [ebp - 0x5c], ecx
0040EEE2  8b 45 0c                        mov eax, dword ptr [ebp + 0xc]
0040EEE5  8b 48 04                        mov ecx, dword ptr [eax + 4]
0040EEE8  51                              push ecx
0040EEE9  8b 10                           mov edx, dword ptr [eax]
0040EEEB  52                              push edx
0040EEEC  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040EEEF  83 c1 78                        add ecx, 0x78
0040EEF2  8b 45 08                        mov eax, dword ptr [ebp + 8]
0040EEF5  8b 50 78                        mov edx, dword ptr [eax + 0x78]
0040EEF8  ff 52 2c                        call dword ptr [edx + 0x2c]
0040EEFB  8b 45 08                        mov eax, dword ptr [ebp + 8]
0040EEFE  33 c9                           xor ecx, ecx
0040EF00  8a 48 60                        mov cl, byte ptr [eax + 0x60]
0040EF03  85 c9                           test ecx, ecx
0040EF05  75 50                           jne 0x40ef57
0040EF07  68 10 01 00 00                  push 0x110
0040EF0C  e8 97 d2 08 00                  call 0x49c1a8
0040EF11  83 c4 04                        add esp, 4
0040EF14  89 45 a8                        mov dword ptr [ebp - 0x58], eax
0040EF17  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
0040EF1E  83 7d a8 00                     cmp dword ptr [ebp - 0x58], 0
0040EF22  74 0d                           je 0x40ef31
0040EF24  8b 4d a8                        mov ecx, dword ptr [ebp - 0x58]
0040EF27  e8 94 f4 ff ff                  call 0x40e3c0
0040EF2C  89 45 a0                        mov dword ptr [ebp - 0x60], eax
0040EF2F  eb 07                           jmp 0x40ef38
0040EF31  c7 45 a0 00 00 00 00            mov dword ptr [ebp - 0x60], 0
0040EF38  8b 55 a0                        mov edx, dword ptr [ebp - 0x60]
0040EF3B  89 55 ac                        mov dword ptr [ebp - 0x54], edx
0040EF3E  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0040EF45  8b 45 ac                        mov eax, dword ptr [ebp - 0x54]
0040EF48  89 45 ec                        mov dword ptr [ebp - 0x14], eax
0040EF4B  8b 4d ec                        mov ecx, dword ptr [ebp - 0x14]
0040EF4E  51                              push ecx
0040EF4F  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040EF52  e8 3f f7 01 00                  call 0x42e696
0040EF57  8b 55 a4                        mov edx, dword ptr [ebp - 0x5c]
0040EF5A  c6 42 28 00                     mov byte ptr [edx + 0x28], 0
0040EF5E  c7 45 f0 00 00 00 00            mov dword ptr [ebp - 0x10], 0
0040EF65  8b 45 0c                        mov eax, dword ptr [ebp + 0xc]
0040EF68  dd 00                           fld qword ptr [eax]
0040EF6A  dc 1d 38 03 4b 00               fcomp qword ptr [0x4b0338]
0040EF70  df e0                           fnstsw ax
0040EF72  f6 c4 41                        test ah, 0x41
0040EF75  0f 85 45 02 00 00               jne 0x40f1c0
0040EF7B  83 7d f0 00                     cmp dword ptr [ebp - 0x10], 0
0040EF7F  0f 85 3b 02 00 00               jne 0x40f1c0
0040EF85  c6 45 c8 00                     mov byte ptr [ebp - 0x38], 0
0040EF89  c6 45 d4 00                     mov byte ptr [ebp - 0x2c], 0
0040EF8D  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040EF90  51                              push ecx
0040EF91  8b 4d a4                        mov ecx, dword ptr [ebp - 0x5c]
0040EF94  e8 c4 05 00 00                  call 0x40f55d
0040EF99  dd 5d cc                        fstp qword ptr [ebp - 0x34]
0040EF9C  8d 55 e8                        lea edx, [ebp - 0x18]
0040EF9F  52                              push edx
0040EFA0  8b 4d a4                        mov ecx, dword ptr [ebp - 0x5c]
0040EFA3  e8 50 06 00 00                  call 0x40f5f8
0040EFA8  dd 5d d8                        fstp qword ptr [ebp - 0x28]
0040EFAB  8b 45 d8                        mov eax, dword ptr [ebp - 0x28]
0040EFAE  89 45 c0                        mov dword ptr [ebp - 0x40], eax
0040EFB1  8b 4d dc                        mov ecx, dword ptr [ebp - 0x24]
0040EFB4  89 4d c4                        mov dword ptr [ebp - 0x3c], ecx
0040EFB7  dd 45 cc                        fld qword ptr [ebp - 0x34]
0040EFBA  dc 5d d8                        fcomp qword ptr [ebp - 0x28]
0040EFBD  df e0                           fnstsw ax
0040EFBF  f6 c4 01                        test ah, 1
0040EFC2  74 10                           je 0x40efd4
0040EFC4  8b 55 cc                        mov edx, dword ptr [ebp - 0x34]
0040EFC7  89 55 c0                        mov dword ptr [ebp - 0x40], edx
0040EFCA  8b 45 d0                        mov eax, dword ptr [ebp - 0x30]
0040EFCD  89 45 c4                        mov dword ptr [ebp - 0x3c], eax
0040EFD0  c6 45 c8 01                     mov byte ptr [ebp - 0x38], 1
0040EFD4  8b 4d 0c                        mov ecx, dword ptr [ebp + 0xc]
0040EFD7  dd 45 c0                        fld qword ptr [ebp - 0x40]
0040EFDA  dc 19                           fcomp qword ptr [ecx]
0040EFDC  df e0                           fnstsw ax
0040EFDE  f6 c4 41                        test ah, 0x41
0040EFE1  75 10                           jne 0x40eff3
0040EFE3  8b 55 0c                        mov edx, dword ptr [ebp + 0xc]
0040EFE6  8b 02                           mov eax, dword ptr [edx]
0040EFE8  89 45 c0                        mov dword ptr [ebp - 0x40], eax
0040EFEB  8b 4a 04                        mov ecx, dword ptr [edx + 4]
0040EFEE  89 4d c4                        mov dword ptr [ebp - 0x3c], ecx
0040EFF1  eb 11                           jmp 0x40f004
0040EFF3  c6 45 d4 01                     mov byte ptr [ebp - 0x2c], 1
0040EFF7  68 8c f1 4b 00                  push 0x4bf18c
0040EFFC  e8 5f 08 00 00                  call 0x40f860
0040F001  83 c4 04                        add esp, 4
0040F004  8b 55 a4                        mov edx, dword ptr [ebp - 0x5c]
0040F007  dd 45 c0                        fld qword ptr [ebp - 0x40]
0040F00A  dc 42 08                        fadd qword ptr [edx + 8]
0040F00D  8b 45 a4                        mov eax, dword ptr [ebp - 0x5c]
0040F010  dd 58 08                        fstp qword ptr [eax + 8]
0040F013  8b 4d c8                        mov ecx, dword ptr [ebp - 0x38]
0040F016  81 e1 ff 00 00 00               and ecx, 0xff
0040F01C  85 c9                           test ecx, ecx
0040F01E  75 5b                           jne 0x40f07b
0040F020  8b 55 d4                        mov edx, dword ptr [ebp - 0x2c]
0040F023  81 e2 ff 00 00 00               and edx, 0xff
0040F029  85 d2                           test edx, edx
0040F02B  74 4e                           je 0x40f07b
0040F02D  8b 45 a4                        mov eax, dword ptr [ebp - 0x5c]
0040F030  dd 40 08                        fld qword ptr [eax + 8]
0040F033  dc 1d a8 55 4c 00               fcomp qword ptr [0x4c55a8]
0040F039  df e0                           fnstsw ax
0040F03B  f6 c4 41                        test ah, 0x41
0040F03E  75 21                           jne 0x40f061
0040F040  b9 31 00 00 00                  mov ecx, 0x31
0040F045  2b 4d e8                        sub ecx, dword ptr [ebp - 0x18]
0040F048  8b 55 a4                        mov edx, dword ptr [ebp - 0x5c]
0040F04B  8b 04 cd 10 54 4c 00            mov eax, dword ptr [ecx*8 + 0x4c5410]
0040F052  89 42 08                        mov dword ptr [edx + 8], eax
0040F055  8b 0c cd 14 54 4c 00            mov ecx, dword ptr [ecx*8 + 0x4c5414]
0040F05C  89 4a 0c                        mov dword ptr [edx + 0xc], ecx
0040F05F  eb 1a                           jmp 0x40f07b
0040F061  8b 55 e8                        mov edx, dword ptr [ebp - 0x18]
0040F064  8b 45 a4                        mov eax, dword ptr [ebp - 0x5c]
0040F067  8b 0c d5 10 54 4c 00            mov ecx, dword ptr [edx*8 + 0x4c5410]
0040F06E  89 48 08                        mov dword ptr [eax + 8], ecx
0040F071  8b 14 d5 14 54 4c 00            mov edx, dword ptr [edx*8 + 0x4c5414]
0040F078  89 50 0c                        mov dword ptr [eax + 0xc], edx
0040F07B  8b 45 0c                        mov eax, dword ptr [ebp + 0xc]
0040F07E  dd 00                           fld qword ptr [eax]
0040F080  dc 65 c0                        fsub qword ptr [ebp - 0x40]
0040F083  8b 4d 0c                        mov ecx, dword ptr [ebp + 0xc]
0040F086  dd 19                           fstp qword ptr [ecx]
0040F088  8b 55 c4                        mov edx, dword ptr [ebp - 0x3c]
0040F08B  52                              push edx
0040F08C  8b 45 c0                        mov eax, dword ptr [ebp - 0x40]
0040F08F  50                              push eax
0040F090  68 a0 f1 4b 00                  push 0x4bf1a0
0040F095  e8 c6 07 00 00                  call 0x40f860
0040F09A  83 c4 0c                        add esp, 0xc
0040F09D  8b 4d d4                        mov ecx, dword ptr [ebp - 0x2c]
0040F0A0  81 e1 ff 00 00 00               and ecx, 0xff
0040F0A6  85 c9                           test ecx, ecx
0040F0A8  74 15                           je 0x40f0bf
0040F0AA  8b 55 c8                        mov edx, dword ptr [ebp - 0x38]
0040F0AD  81 e2 ff 00 00 00               and edx, 0xff
0040F0B3  85 d2                           test edx, edx
0040F0B5  75 08                           jne 0x40f0bf
0040F0B7  db 45 e8                        fild dword ptr [ebp - 0x18]
0040F0BA  dd 5d e0                        fstp qword ptr [ebp - 0x20]
0040F0BD  eb 29                           jmp 0x40f0e8
0040F0BF  8b 45 a4                        mov eax, dword ptr [ebp - 0x5c]
0040F0C2  dd 05 08 08 4b 00               fld qword ptr [0x4b0808]
0040F0C8  dc 48 08                        fmul qword ptr [eax + 8]
0040F0CB  dd 05 a8 03 4b 00               fld qword ptr [0x4b03a8]
0040F0D1  dc 0d 00 08 4b 00               fmul qword ptr [0x4b0800]
0040F0D7  8b 4d a4                        mov ecx, dword ptr [ebp - 0x5c]
0040F0DA  dc 49 08                        fmul qword ptr [ecx + 8]
0040F0DD  8b 55 a4                        mov edx, dword ptr [ebp - 0x5c]
0040F0E0  dc 4a 08                        fmul qword ptr [edx + 8]
0040F0E3  de c1                           faddp st(1)
0040F0E5  dd 5d e0                        fstp qword ptr [ebp - 0x20]
0040F0E8  8b 45 08                        mov eax, dword ptr [ebp + 8]
0040F0EB  db 40 44                        fild dword ptr [eax + 0x44]
0040F0EE  dc 4d e0                        fmul qword ptr [ebp - 0x20]
0040F0F1  dd 5d e0                        fstp qword ptr [ebp - 0x20]
0040F0F4  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F0F7  8b 55 e8                        mov edx, dword ptr [ebp - 0x18]
0040F0FA  0f af 51 44                     imul edx, dword ptr [ecx + 0x44]
0040F0FE  89 55 e8                        mov dword ptr [ebp - 0x18], edx
0040F101  8b 45 a4                        mov eax, dword ptr [ebp - 0x5c]
0040F104  dd 45 e0                        fld qword ptr [ebp - 0x20]
0040F107  dc 40 18                        fadd qword ptr [eax + 0x18]
0040F10A  dd 5d b0                        fstp qword ptr [ebp - 0x50]
0040F10D  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F110  8b 55 08                        mov edx, dword ptr [ebp + 8]
0040F113  db 42 40                        fild dword ptr [edx + 0x40]
0040F116  dc 0d b0 10 4b 00               fmul qword ptr [0x4b10b0]
0040F11C  dc 4d c0                        fmul qword ptr [ebp - 0x40]
0040F11F  dc 41 08                        fadd qword ptr [ecx + 8]
0040F122  dd 5d b8                        fstp qword ptr [ebp - 0x48]
0040F125  8b 45 bc                        mov eax, dword ptr [ebp - 0x44]
0040F128  50                              push eax
0040F129  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
0040F12C  51                              push ecx
0040F12D  8b 55 b4                        mov edx, dword ptr [ebp - 0x4c]
0040F130  52                              push edx
0040F131  8b 45 b0                        mov eax, dword ptr [ebp - 0x50]
0040F134  50                              push eax
0040F135  68 bc f1 4b 00                  push 0x4bf1bc
0040F13A  e8 21 07 00 00                  call 0x40f860
0040F13F  83 c4 14                        add esp, 0x14
0040F142  8b 4d bc                        mov ecx, dword ptr [ebp - 0x44]
0040F145  51                              push ecx
0040F146  8b 55 b8                        mov edx, dword ptr [ebp - 0x48]
0040F149  52                              push edx
0040F14A  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040F14F  e8 6c ed ff ff                  call 0x40dec0
0040F154  dd 5d b8                        fstp qword ptr [ebp - 0x48]
0040F157  8b 45 b4                        mov eax, dword ptr [ebp - 0x4c]
0040F15A  50                              push eax
0040F15B  8b 4d b0                        mov ecx, dword ptr [ebp - 0x50]
0040F15E  51                              push ecx
0040F15F  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040F164  e8 e7 ed ff ff                  call 0x40df50
0040F169  dd 5d b0                        fstp qword ptr [ebp - 0x50]
0040F16C  8b 55 bc                        mov edx, dword ptr [ebp - 0x44]
0040F16F  52                              push edx
0040F170  8b 45 b8                        mov eax, dword ptr [ebp - 0x48]
0040F173  50                              push eax
0040F174  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F177  e8 27 fd 01 00                  call 0x42eea3
0040F17C  8b 4d b4                        mov ecx, dword ptr [ebp - 0x4c]
0040F17F  51                              push ecx
0040F180  8b 55 b0                        mov edx, dword ptr [ebp - 0x50]
0040F183  52                              push edx
0040F184  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F187  e8 e8 fc 01 00                  call 0x42ee74
0040F18C  8b 45 d4                        mov eax, dword ptr [ebp - 0x2c]
0040F18F  25 ff 00 00 00                  and eax, 0xff
0040F194  85 c0                           test eax, eax
0040F196  74 1c                           je 0x40f1b4
0040F198  8a 4d c8                        mov cl, byte ptr [ebp - 0x38]
0040F19B  51                              push ecx
0040F19C  8b 55 08                        mov edx, dword ptr [ebp + 8]
0040F19F  52                              push edx
0040F1A0  8b 4d a4                        mov ecx, dword ptr [ebp - 0x5c]
0040F1A3  e8 41 00 00 00                  call 0x40f1e9
0040F1A8  89 45 f0                        mov dword ptr [ebp - 0x10], eax
0040F1AB  8b 45 a4                        mov eax, dword ptr [ebp - 0x5c]
0040F1AE  c6 40 28 01                     mov byte ptr [eax + 0x28], 1
0040F1B2  eb 07                           jmp 0x40f1bb
0040F1B4  8b 4d a4                        mov ecx, dword ptr [ebp - 0x5c]
0040F1B7  c6 41 28 00                     mov byte ptr [ecx + 0x28], 0
0040F1BB  e9 a5 fd ff ff                  jmp 0x40ef65
0040F1C0  83 7d f0 00                     cmp dword ptr [ebp - 0x10], 0
0040F1C4  75 10                           jne 0x40f1d6
0040F1C6  8b 55 0c                        mov edx, dword ptr [ebp + 0xc]
0040F1C9  c7 02 00 00 00 00               mov dword ptr [edx], 0
0040F1CF  c7 42 04 00 00 00 00            mov dword ptr [edx + 4], 0
0040F1D6  8b 45 f0                        mov eax, dword ptr [ebp - 0x10]
0040F1D9  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0040F1DC  64 89 0d 00 00 00 00            mov dword ptr fs:[0], ecx
0040F1E3  8b e5                           mov esp, ebp
0040F1E5  5d                              pop ebp
0040F1E6  c2 08 00                        ret 8

; VA 0x0040F1E9, file offset 0xF1E9, length 884, SHA-256 e34eba1a8411729bb848a59a7d44a1c6346806f2f3ba649f6cf109ebcdb5e4a3
0040F1E9  55                              push ebp
0040F1EA  8b ec                           mov ebp, esp
0040F1EC  6a ff                           push -1
0040F1EE  68 8b 9c 4a 00                  push 0x4a9c8b
0040F1F3  64 a1 00 00 00 00               mov eax, dword ptr fs:[0]
0040F1F9  50                              push eax
0040F1FA  64 89 25 00 00 00 00            mov dword ptr fs:[0], esp
0040F201  81 ec 80 00 00 00               sub esp, 0x80
0040F207  89 4d 84                        mov dword ptr [ebp - 0x7c], ecx
0040F20A  8d 45 dc                        lea eax, [ebp - 0x24]
0040F20D  50                              push eax
0040F20E  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F211  e8 49 01 02 00                  call 0x42f35f
0040F216  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
0040F21D  8d 4d c0                        lea ecx, [ebp - 0x40]
0040F220  51                              push ecx
0040F221  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F224  e8 05 00 02 00                  call 0x42f22e
0040F229  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0040F22D  dd 45 e4                        fld qword ptr [ebp - 0x1c]
0040F230  e8 77 d3 08 00                  call 0x49c5ac
0040F235  50                              push eax
0040F236  dd 45 dc                        fld qword ptr [ebp - 0x24]
0040F239  e8 6e d3 08 00                  call 0x49c5ac
0040F23E  50                              push eax
0040F23F  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040F244  e8 17 ec ff ff                  call 0x40de60
0040F249  8b 10                           mov edx, dword ptr [eax]
0040F24B  89 55 bc                        mov dword ptr [ebp - 0x44], edx
0040F24E  8b 45 08                        mov eax, dword ptr [ebp + 8]
0040F251  db 40 44                        fild dword ptr [eax + 0x44]
0040F254  dc 6d c8                        fsubr qword ptr [ebp - 0x38]
0040F257  e8 50 d3 08 00                  call 0x49c5ac
0040F25C  89 45 80                        mov dword ptr [ebp - 0x80], eax
0040F25F  db 45 80                        fild dword ptr [ebp - 0x80]
0040F262  83 ec 08                        sub esp, 8
0040F265  dd 1c 24                        fstp qword ptr [esp]
0040F268  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040F26D  e8 de ec ff ff                  call 0x40df50
0040F272  e8 35 d3 08 00                  call 0x49c5ac
0040F277  89 45 b8                        mov dword ptr [ebp - 0x48], eax
0040F27A  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
0040F27D  51                              push ecx
0040F27E  dd 45 c0                        fld qword ptr [ebp - 0x40]
0040F281  e8 26 d3 08 00                  call 0x49c5ac
0040F286  50                              push eax
0040F287  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040F28C  e8 cf eb ff ff                  call 0x40de60
0040F291  8b 10                           mov edx, dword ptr [eax]
0040F293  89 55 d8                        mov dword ptr [ebp - 0x28], edx
0040F296  8b 4d 0c                        mov ecx, dword ptr [ebp + 0xc]
0040F299  81 e1 ff 00 00 00               and ecx, 0xff
0040F29F  85 c9                           test ecx, ecx
0040F2A1  0f 85 1b 02 00 00               jne 0x40f4c2
0040F2A7  68 dc f1 4b 00                  push 0x4bf1dc
0040F2AC  e8 af 05 00 00                  call 0x40f860
0040F2B1  83 c4 04                        add esp, 4
0040F2B4  83 7d d8 00                     cmp dword ptr [ebp - 0x28], 0
0040F2B8  74 4c                           je 0x40f306
0040F2BA  8b 55 d8                        mov edx, dword ptr [ebp - 0x28]
0040F2BD  81 e2 80 00 00 00               and edx, 0x80
0040F2C3  85 d2                           test edx, edx
0040F2C5  75 3f                           jne 0x40f306
0040F2C7  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F2CA  e8 ea 05 02 00                  call 0x42f8b9
0040F2CF  68 f8 f1 4b 00                  push 0x4bf1f8
0040F2D4  e8 87 05 00 00                  call 0x40f860
0040F2D9  83 c4 04                        add esp, 4
0040F2DC  c7 45 b4 01 00 00 00            mov dword ptr [ebp - 0x4c], 1
0040F2E3  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
0040F2E7  8d 4d c0                        lea ecx, [ebp - 0x40]
0040F2EA  e8 c1 4b ff ff                  call 0x403eb0
0040F2EF  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0040F2F6  8d 4d dc                        lea ecx, [ebp - 0x24]
0040F2F9  e8 b2 4b ff ff                  call 0x403eb0
0040F2FE  8b 45 b4                        mov eax, dword ptr [ebp - 0x4c]
0040F301  e9 47 02 00 00                  jmp 0x40f54d
0040F306  8b 45 d8                        mov eax, dword ptr [ebp - 0x28]
0040F309  25 00 10 00 00                  and eax, 0x1000
0040F30E  85 c0                           test eax, eax
0040F310  0f 84 98 00 00 00               je 0x40f3ae
0040F316  8b 4d 84                        mov ecx, dword ptr [ebp - 0x7c]
0040F319  dd 41 08                        fld qword ptr [ecx + 8]
0040F31C  dc 1d a8 55 4c 00               fcomp qword ptr [0x4c55a8]
0040F322  df e0                           fnstsw ax
0040F324  f6 c4 41                        test ah, 0x41
0040F327  0f 85 81 00 00 00               jne 0x40f3ae
0040F32D  6a 30                           push 0x30
0040F32F  e8 74 ce 08 00                  call 0x49c1a8
0040F334  83 c4 04                        add esp, 4
0040F337  89 45 ac                        mov dword ptr [ebp - 0x54], eax
0040F33A  c6 45 fc 02                     mov byte ptr [ebp - 4], 2
0040F33E  83 7d ac 00                     cmp dword ptr [ebp - 0x54], 0
0040F342  74 10                           je 0x40f354
0040F344  8b 4d ac                        mov ecx, dword ptr [ebp - 0x54]
0040F347  e8 32 f9 ff ff                  call 0x40ec7e
0040F34C  89 85 7c ff ff ff               mov dword ptr [ebp - 0x84], eax
0040F352  eb 0a                           jmp 0x40f35e
0040F354  c7 85 7c ff ff ff 00 00 00 00   mov dword ptr [ebp - 0x84], 0
0040F35E  8b 95 7c ff ff ff               mov edx, dword ptr [ebp - 0x84]
0040F364  89 55 b0                        mov dword ptr [ebp - 0x50], edx
0040F367  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0040F36B  8b 45 b0                        mov eax, dword ptr [ebp - 0x50]
0040F36E  50                              push eax
0040F36F  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F372  e8 1f f3 01 00                  call 0x42e696
0040F377  68 0c f2 4b 00                  push 0x4bf20c
0040F37C  e8 df 04 00 00                  call 0x40f860
0040F381  83 c4 04                        add esp, 4
0040F384  c7 45 a8 01 00 00 00            mov dword ptr [ebp - 0x58], 1
0040F38B  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
0040F38F  8d 4d c0                        lea ecx, [ebp - 0x40]
0040F392  e8 19 4b ff ff                  call 0x403eb0
0040F397  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0040F39E  8d 4d dc                        lea ecx, [ebp - 0x24]
0040F3A1  e8 0a 4b ff ff                  call 0x403eb0
0040F3A6  8b 45 a8                        mov eax, dword ptr [ebp - 0x58]
0040F3A9  e9 9f 01 00 00                  jmp 0x40f54d
0040F3AE  83 7d bc 00                     cmp dword ptr [ebp - 0x44], 0
0040F3B2  0f 84 81 00 00 00               je 0x40f439
0040F3B8  6a 28                           push 0x28
0040F3BA  e8 e9 cd 08 00                  call 0x49c1a8
0040F3BF  83 c4 04                        add esp, 4
0040F3C2  89 45 a0                        mov dword ptr [ebp - 0x60], eax
0040F3C5  c6 45 fc 03                     mov byte ptr [ebp - 4], 3
0040F3C9  83 7d a0 00                     cmp dword ptr [ebp - 0x60], 0
0040F3CD  74 10                           je 0x40f3df
0040F3CF  8b 4d a0                        mov ecx, dword ptr [ebp - 0x60]
0040F3D2  e8 c9 6b 00 00                  call 0x415fa0
0040F3D7  89 85 78 ff ff ff               mov dword ptr [ebp - 0x88], eax
0040F3DD  eb 0a                           jmp 0x40f3e9
0040F3DF  c7 85 78 ff ff ff 00 00 00 00   mov dword ptr [ebp - 0x88], 0
0040F3E9  8b 8d 78 ff ff ff               mov ecx, dword ptr [ebp - 0x88]
0040F3EF  89 4d a4                        mov dword ptr [ebp - 0x5c], ecx
0040F3F2  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0040F3F6  8b 55 a4                        mov edx, dword ptr [ebp - 0x5c]
0040F3F9  52                              push edx
0040F3FA  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F3FD  e8 94 f2 01 00                  call 0x42e696
0040F402  68 24 f2 4b 00                  push 0x4bf224
0040F407  e8 54 04 00 00                  call 0x40f860
0040F40C  83 c4 04                        add esp, 4
0040F40F  c7 45 9c 01 00 00 00            mov dword ptr [ebp - 0x64], 1
0040F416  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
0040F41A  8d 4d c0                        lea ecx, [ebp - 0x40]
0040F41D  e8 8e 4a ff ff                  call 0x403eb0
0040F422  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0040F429  8d 4d dc                        lea ecx, [ebp - 0x24]
0040F42C  e8 7f 4a ff ff                  call 0x403eb0
0040F431  8b 45 9c                        mov eax, dword ptr [ebp - 0x64]
0040F434  e9 14 01 00 00                  jmp 0x40f54d
0040F439  8b 45 84                        mov eax, dword ptr [ebp - 0x7c]
0040F43C  dd 40 08                        fld qword ptr [eax + 8]
0040F43F  dc 1d 98 55 4c 00               fcomp qword ptr [0x4c5598]
0040F445  df e0                           fnstsw ax
0040F447  f6 c4 40                        test ah, 0x40
0040F44A  74 74                           je 0x40f4c0
0040F44C  6a 28                           push 0x28
0040F44E  e8 55 cd 08 00                  call 0x49c1a8
0040F453  83 c4 04                        add esp, 4
0040F456  89 45 94                        mov dword ptr [ebp - 0x6c], eax
0040F459  c6 45 fc 04                     mov byte ptr [ebp - 4], 4
0040F45D  83 7d 94 00                     cmp dword ptr [ebp - 0x6c], 0
0040F461  74 10                           je 0x40f473
0040F463  8b 4d 94                        mov ecx, dword ptr [ebp - 0x6c]
0040F466  e8 35 6b 00 00                  call 0x415fa0
0040F46B  89 85 74 ff ff ff               mov dword ptr [ebp - 0x8c], eax
0040F471  eb 0a                           jmp 0x40f47d
0040F473  c7 85 74 ff ff ff 00 00 00 00   mov dword ptr [ebp - 0x8c], 0
0040F47D  8b 8d 74 ff ff ff               mov ecx, dword ptr [ebp - 0x8c]
0040F483  89 4d 98                        mov dword ptr [ebp - 0x68], ecx
0040F486  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0040F48A  8b 55 98                        mov edx, dword ptr [ebp - 0x68]
0040F48D  52                              push edx
0040F48E  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F491  e8 00 f2 01 00                  call 0x42e696
0040F496  c7 45 90 01 00 00 00            mov dword ptr [ebp - 0x70], 1
0040F49D  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
0040F4A1  8d 4d c0                        lea ecx, [ebp - 0x40]
0040F4A4  e8 07 4a ff ff                  call 0x403eb0
0040F4A9  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0040F4B0  8d 4d dc                        lea ecx, [ebp - 0x24]
0040F4B3  e8 f8 49 ff ff                  call 0x403eb0
0040F4B8  8b 45 90                        mov eax, dword ptr [ebp - 0x70]
0040F4BB  e9 8d 00 00 00                  jmp 0x40f54d
0040F4C0  eb 66                           jmp 0x40f528
0040F4C2  68 38 f2 4b 00                  push 0x4bf238
0040F4C7  e8 94 03 00 00                  call 0x40f860
0040F4CC  83 c4 04                        add esp, 4
0040F4CF  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F4D2  e8 71 f6 01 00                  call 0x42eb48
0040F4D7  25 ff 00 00 00                  and eax, 0xff
0040F4DC  85 c0                           test eax, eax
0040F4DE  74 48                           je 0x40f528
0040F4E0  8b 45 0c                        mov eax, dword ptr [ebp + 0xc]
0040F4E3  25 ff 00 00 00                  and eax, 0xff
0040F4E8  85 c0                           test eax, eax
0040F4EA  74 3c                           je 0x40f528
0040F4EC  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F4EF  e8 77 01 02 00                  call 0x42f66b
0040F4F4  68 54 f2 4b 00                  push 0x4bf254
0040F4F9  e8 62 03 00 00                  call 0x40f860
0040F4FE  83 c4 04                        add esp, 4
0040F501  c7 45 8c 00 00 00 00            mov dword ptr [ebp - 0x74], 0
0040F508  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
0040F50C  8d 4d c0                        lea ecx, [ebp - 0x40]
0040F50F  e8 9c 49 ff ff                  call 0x403eb0
0040F514  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0040F51B  8d 4d dc                        lea ecx, [ebp - 0x24]
0040F51E  e8 8d 49 ff ff                  call 0x403eb0
0040F523  8b 45 8c                        mov eax, dword ptr [ebp - 0x74]
0040F526  eb 25                           jmp 0x40f54d
0040F528  c7 45 88 00 00 00 00            mov dword ptr [ebp - 0x78], 0
0040F52F  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
0040F533  8d 4d c0                        lea ecx, [ebp - 0x40]
0040F536  e8 75 49 ff ff                  call 0x403eb0
0040F53B  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0040F542  8d 4d dc                        lea ecx, [ebp - 0x24]
0040F545  e8 66 49 ff ff                  call 0x403eb0
0040F54A  8b 45 88                        mov eax, dword ptr [ebp - 0x78]
0040F54D  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0040F550  64 89 0d 00 00 00 00            mov dword ptr fs:[0], ecx
0040F557  8b e5                           mov esp, ebp
0040F559  5d                              pop ebp
0040F55A  c2 08 00                        ret 8

; VA 0x0040F55D, file offset 0xF55D, length 155, SHA-256 9c6e9dbb8e4faa8d9df08b352c3aae1fc95d10f29672b23ab1fe27dbc421edf0
0040F55D  55                              push ebp
0040F55E  8b ec                           mov ebp, esp
0040F560  83 ec 14                        sub esp, 0x14
0040F563  56                              push esi
0040F564  89 4d f4                        mov dword ptr [ebp - 0xc], ecx
0040F567  c7 45 f8 00 00 00 00            mov dword ptr [ebp - 8], 0
0040F56E  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
0040F575  8b 45 08                        mov eax, dword ptr [ebp + 8]
0040F578  83 78 40 01                     cmp dword ptr [eax + 0x40], 1
0040F57C  75 25                           jne 0x40f5a3
0040F57E  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F581  dd 41 08                        fld qword ptr [ecx + 8]
0040F584  e8 23 d0 08 00                  call 0x49c5ac
0040F589  83 c0 01                        add eax, 1
0040F58C  89 45 f0                        mov dword ptr [ebp - 0x10], eax
0040F58F  db 45 f0                        fild dword ptr [ebp - 0x10]
0040F592  8b 55 08                        mov edx, dword ptr [ebp + 8]
0040F595  dc 62 08                        fsub qword ptr [edx + 8]
0040F598  dc 35 b0 10 4b 00               fdiv qword ptr [0x4b10b0]
0040F59E  dd 5d f8                        fstp qword ptr [ebp - 8]
0040F5A1  eb 20                           jmp 0x40f5c3
0040F5A3  8b 75 08                        mov esi, dword ptr [ebp + 8]
0040F5A6  8b 45 08                        mov eax, dword ptr [ebp + 8]
0040F5A9  dd 40 08                        fld qword ptr [eax + 8]
0040F5AC  e8 fb cf 08 00                  call 0x49c5ac
0040F5B1  89 45 ec                        mov dword ptr [ebp - 0x14], eax
0040F5B4  db 45 ec                        fild dword ptr [ebp - 0x14]
0040F5B7  dc 6e 08                        fsubr qword ptr [esi + 8]
0040F5BA  dc 35 b0 10 4b 00               fdiv qword ptr [0x4b10b0]
0040F5C0  dd 5d f8                        fstp qword ptr [ebp - 8]
0040F5C3  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0040F5C6  33 d2                           xor edx, edx
0040F5C8  8a 51 28                        mov dl, byte ptr [ecx + 0x28]
0040F5CB  85 d2                           test edx, edx
0040F5CD  74 1f                           je 0x40f5ee
0040F5CF  dd 45 f8                        fld qword ptr [ebp - 8]
0040F5D2  dc 1d 38 03 4b 00               fcomp qword ptr [0x4b0338]
0040F5D8  df e0                           fnstsw ax
0040F5DA  f6 c4 40                        test ah, 0x40
0040F5DD  74 0f                           je 0x40f5ee
0040F5DF  dd 05 d0 02 4b 00               fld qword ptr [0x4b02d0]
0040F5E5  dc 35 b0 10 4b 00               fdiv qword ptr [0x4b10b0]
0040F5EB  dd 5d f8                        fstp qword ptr [ebp - 8]
0040F5EE  dd 45 f8                        fld qword ptr [ebp - 8]
0040F5F1  5e                              pop esi
0040F5F2  8b e5                           mov esp, ebp
0040F5F4  5d                              pop ebp
0040F5F5  c2 04 00                        ret 4

; VA 0x0040F5F8, file offset 0xF5F8, length 264, SHA-256 0eeb5e2880f3acabfc30e212d29b9969fff1bb7fd97b1b5d2c9b45e8fc557ba6
0040F5F8  55                              push ebp
0040F5F9  8b ec                           mov ebp, esp
0040F5FB  83 ec 18                        sub esp, 0x18
0040F5FE  89 4d f0                        mov dword ptr [ebp - 0x10], ecx
0040F601  c7 45 f8 00 00 00 00            mov dword ptr [ebp - 8], 0
0040F608  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
0040F60F  c7 45 f4 01 00 00 00            mov dword ptr [ebp - 0xc], 1
0040F616  eb 09                           jmp 0x40f621
0040F618  8b 45 f4                        mov eax, dword ptr [ebp - 0xc]
0040F61B  83 c0 01                        add eax, 1
0040F61E  89 45 f4                        mov dword ptr [ebp - 0xc], eax
0040F621  83 7d f4 32                     cmp dword ptr [ebp - 0xc], 0x32
0040F625  7d 66                           jge 0x40f68d
0040F627  8b 4d f0                        mov ecx, dword ptr [ebp - 0x10]
0040F62A  8b 55 f4                        mov edx, dword ptr [ebp - 0xc]
0040F62D  dd 41 08                        fld qword ptr [ecx + 8]
0040F630  dc 1c d5 08 54 4c 00            fcomp qword ptr [edx*8 + 0x4c5408]
0040F637  df e0                           fnstsw ax
0040F639  f6 c4 01                        test ah, 1
0040F63C  75 4d                           jne 0x40f68b
0040F63E  8b 45 f0                        mov eax, dword ptr [ebp - 0x10]
0040F641  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0040F644  dd 40 08                        fld qword ptr [eax + 8]
0040F647  dc 1c cd 10 54 4c 00            fcomp qword ptr [ecx*8 + 0x4c5410]
0040F64E  df e0                           fnstsw ax
0040F650  f6 c4 01                        test ah, 1
0040F653  74 36                           je 0x40f68b
0040F655  8b 55 f4                        mov edx, dword ptr [ebp - 0xc]
0040F658  8b 45 f0                        mov eax, dword ptr [ebp - 0x10]
0040F65B  dd 04 d5 10 54 4c 00            fld qword ptr [edx*8 + 0x4c5410]
0040F662  dc 60 08                        fsub qword ptr [eax + 8]
0040F665  dd 5d f8                        fstp qword ptr [ebp - 8]
0040F668  83 7d f4 19                     cmp dword ptr [ebp - 0xc], 0x19
0040F66C  7d 08                           jge 0x40f676
0040F66E  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0040F671  89 4d ec                        mov dword ptr [ebp - 0x14], ecx
0040F674  eb 0b                           jmp 0x40f681
0040F676  ba 31 00 00 00                  mov edx, 0x31
0040F67B  2b 55 f4                        sub edx, dword ptr [ebp - 0xc]
0040F67E  89 55 ec                        mov dword ptr [ebp - 0x14], edx
0040F681  8b 45 08                        mov eax, dword ptr [ebp + 8]
0040F684  8b 4d ec                        mov ecx, dword ptr [ebp - 0x14]
0040F687  89 08                           mov dword ptr [eax], ecx
0040F689  eb 02                           jmp 0x40f68d
0040F68B  eb 8b                           jmp 0x40f618
0040F68D  8b 55 f0                        mov edx, dword ptr [ebp - 0x10]
0040F690  33 c0                           xor eax, eax
0040F692  8a 42 28                        mov al, byte ptr [edx + 0x28]
0040F695  85 c0                           test eax, eax
0040F697  74 5e                           je 0x40f6f7
0040F699  dd 45 f8                        fld qword ptr [ebp - 8]
0040F69C  dc 1d 38 03 4b 00               fcomp qword ptr [0x4b0338]
0040F6A2  df e0                           fnstsw ax
0040F6A4  f6 c4 40                        test ah, 0x40
0040F6A7  74 4e                           je 0x40f6f7
0040F6A9  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0040F6AC  83 c1 01                        add ecx, 1
0040F6AF  83 f9 32                        cmp ecx, 0x32
0040F6B2  7d 43                           jge 0x40f6f7
0040F6B4  8b 55 f4                        mov edx, dword ptr [ebp - 0xc]
0040F6B7  8b 45 f4                        mov eax, dword ptr [ebp - 0xc]
0040F6BA  dd 04 d5 18 54 4c 00            fld qword ptr [edx*8 + 0x4c5418]
0040F6C1  dc 24 c5 10 54 4c 00            fsub qword ptr [eax*8 + 0x4c5410]
0040F6C8  dd 5d f8                        fstp qword ptr [ebp - 8]
0040F6CB  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0040F6CE  83 c1 01                        add ecx, 1
0040F6D1  83 f9 19                        cmp ecx, 0x19
0040F6D4  7d 0b                           jge 0x40f6e1
0040F6D6  8b 55 f4                        mov edx, dword ptr [ebp - 0xc]
0040F6D9  83 c2 01                        add edx, 1
0040F6DC  89 55 e8                        mov dword ptr [ebp - 0x18], edx
0040F6DF  eb 0e                           jmp 0x40f6ef
0040F6E1  b8 31 00 00 00                  mov eax, 0x31
0040F6E6  2b 45 f4                        sub eax, dword ptr [ebp - 0xc]
0040F6E9  83 e8 01                        sub eax, 1
0040F6EC  89 45 e8                        mov dword ptr [ebp - 0x18], eax
0040F6EF  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F6F2  8b 55 e8                        mov edx, dword ptr [ebp - 0x18]
0040F6F5  89 11                           mov dword ptr [ecx], edx
0040F6F7  dd 45 f8                        fld qword ptr [ebp - 8]
0040F6FA  8b e5                           mov esp, ebp
0040F6FC  5d                              pop ebp
0040F6FD  c2 04 00                        ret 4

; VA 0x0040F700, file offset 0xF700, length 260, SHA-256 0dd480aa60d9ea9290c13b09f994003b598162511da2af0361c98f13c0881232
0040F700  55                              push ebp
0040F701  8b ec                           mov ebp, esp
0040F703  83 ec 2c                        sub esp, 0x2c
0040F706  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
0040F70D  eb 09                           jmp 0x40f718
0040F70F  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0040F712  83 c0 01                        add eax, 1
0040F715  89 45 fc                        mov dword ptr [ebp - 4], eax
0040F718  83 7d fc 18                     cmp dword ptr [ebp - 4], 0x18
0040F71C  0f 8f de 00 00 00               jg 0x40f800
0040F722  db 45 fc                        fild dword ptr [ebp - 4]
0040F725  dd 5d dc                        fstp qword ptr [ebp - 0x24]
0040F728  dd 05 08 08 4b 00               fld qword ptr [0x4b0808]
0040F72E  dc 0d 08 08 4b 00               fmul qword ptr [0x4b0808]
0040F734  dd 05 f8 03 4b 00               fld qword ptr [0x4b03f8]
0040F73A  dc 0d 00 08 4b 00               fmul qword ptr [0x4b0800]
0040F740  dc 4d dc                        fmul qword ptr [ebp - 0x24]
0040F743  de c1                           faddp st(1)
0040F745  dd 5d f4                        fstp qword ptr [ebp - 0xc]
0040F748  8b 4d f8                        mov ecx, dword ptr [ebp - 8]
0040F74B  51                              push ecx
0040F74C  8b 55 f4                        mov edx, dword ptr [ebp - 0xc]
0040F74F  52                              push edx
0040F750  e8 eb 00 00 00                  call 0x40f840
0040F755  83 c4 08                        add esp, 8
0040F758  dd 5d d4                        fstp qword ptr [ebp - 0x2c]
0040F75B  dd 05 08 08 4b 00               fld qword ptr [0x4b0808]
0040F761  d9 e0                           fchs
0040F763  dc 45 d4                        fadd qword ptr [ebp - 0x2c]
0040F766  dc 35 00 08 4b 00               fdiv qword ptr [0x4b0800]
0040F76C  dd 5d e4                        fstp qword ptr [ebp - 0x1c]
0040F76F  dd 05 08 08 4b 00               fld qword ptr [0x4b0808]
0040F775  d9 e0                           fchs
0040F777  dc 65 d4                        fsub qword ptr [ebp - 0x2c]
0040F77A  dc 35 00 08 4b 00               fdiv qword ptr [0x4b0800]
0040F780  dd 5d ec                        fstp qword ptr [ebp - 0x14]
0040F783  dd 45 e4                        fld qword ptr [ebp - 0x1c]
0040F786  dc 1d a8 55 4c 00               fcomp qword ptr [0x4c55a8]
0040F78C  df e0                           fnstsw ax
0040F78E  f6 c4 01                        test ah, 1
0040F791  74 35                           je 0x40f7c8
0040F793  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0040F796  8b 4d e4                        mov ecx, dword ptr [ebp - 0x1c]
0040F799  89 0c c5 10 54 4c 00            mov dword ptr [eax*8 + 0x4c5410], ecx
0040F7A0  8b 55 e8                        mov edx, dword ptr [ebp - 0x18]
0040F7A3  89 14 c5 14 54 4c 00            mov dword ptr [eax*8 + 0x4c5414], edx
0040F7AA  b8 31 00 00 00                  mov eax, 0x31
0040F7AF  2b 45 fc                        sub eax, dword ptr [ebp - 4]
0040F7B2  8b 4d ec                        mov ecx, dword ptr [ebp - 0x14]
0040F7B5  89 0c c5 10 54 4c 00            mov dword ptr [eax*8 + 0x4c5410], ecx
0040F7BC  8b 55 f0                        mov edx, dword ptr [ebp - 0x10]
0040F7BF  89 14 c5 14 54 4c 00            mov dword ptr [eax*8 + 0x4c5414], edx
0040F7C6  eb 33                           jmp 0x40f7fb
0040F7C8  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0040F7CB  8b 4d ec                        mov ecx, dword ptr [ebp - 0x14]
0040F7CE  89 0c c5 10 54 4c 00            mov dword ptr [eax*8 + 0x4c5410], ecx
0040F7D5  8b 55 f0                        mov edx, dword ptr [ebp - 0x10]
0040F7D8  89 14 c5 14 54 4c 00            mov dword ptr [eax*8 + 0x4c5414], edx
0040F7DF  b8 31 00 00 00                  mov eax, 0x31
0040F7E4  2b 45 fc                        sub eax, dword ptr [ebp - 4]
0040F7E7  8b 4d e4                        mov ecx, dword ptr [ebp - 0x1c]
0040F7EA  89 0c c5 10 54 4c 00            mov dword ptr [eax*8 + 0x4c5410], ecx
0040F7F1  8b 55 e8                        mov edx, dword ptr [ebp - 0x18]
0040F7F4  89 14 c5 14 54 4c 00            mov dword ptr [eax*8 + 0x4c5414], edx
0040F7FB  e9 0f ff ff ff                  jmp 0x40f70f
0040F800  8b e5                           mov esp, ebp
0040F802  5d                              pop ebp
0040F803  c3                              ret

; VA 0x0040F840, file offset 0xF840, length 21, SHA-256 14fde52da5e457074616150486ffa3ae944044bf39862b688c6ed830b7f02e27
0040F840  55                              push ebp
0040F841  8b ec                           mov ebp, esp
0040F843  8b 45 0c                        mov eax, dword ptr [ebp + 0xc]
0040F846  50                              push eax
0040F847  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040F84A  51                              push ecx
0040F84B  e8 74 d1 08 00                  call 0x49c9c4
0040F850  83 c4 08                        add esp, 8
0040F853  5d                              pop ebp
0040F854  c3                              ret

; VA 0x0042EEA3, file offset 0x2EEA3, length 47, SHA-256 79274e8f53fdbfaddf4bd0a8a6f565e90b1cc683dcd4599d5709eddadced3881
0042EEA3  55                              push ebp
0042EEA4  8b ec                           mov ebp, esp
0042EEA6  51                              push ecx
0042EEA7  89 4d fc                        mov dword ptr [ebp - 4], ecx
0042EEAA  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0042EEAD  dd 45 08                        fld qword ptr [ebp + 8]
0042EEB0  dc a0 60 03 00 00               fsub qword ptr [eax + 0x360]
0042EEB6  83 ec 08                        sub esp, 8
0042EEB9  dd 1c 24                        fstp qword ptr [esp]
0042EEBC  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0042EEC1  e8 fa ef fd ff                  call 0x40dec0
0042EEC6  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
0042EEC9  dd 59 08                        fstp qword ptr [ecx + 8]
0042EECC  8b e5                           mov esp, ebp
0042EECE  5d                              pop ebp
0042EECF  c2 08 00                        ret 8

; VA 0x0042F35F, file offset 0x2F35F, length 149, SHA-256 030bc891f4fdb30afd4e9b0585aa25e9595eaf36a05c2c657942554beda300bd
0042F35F  55                              push ebp
0042F360  8b ec                           mov ebp, esp
0042F362  6a ff                           push -1
0042F364  68 5a b0 4a 00                  push 0x4ab05a
0042F369  64 a1 00 00 00 00               mov eax, dword ptr fs:[0]
0042F36F  50                              push eax
0042F370  64 89 25 00 00 00 00            mov dword ptr fs:[0], esp
0042F377  83 ec 20                        sub esp, 0x20
0042F37A  56                              push esi
0042F37B  57                              push edi
0042F37C  89 4d d4                        mov dword ptr [ebp - 0x2c], ecx
0042F37F  c7 45 d8 00 00 00 00            mov dword ptr [ebp - 0x28], 0
0042F386  8d 4d dc                        lea ecx, [ebp - 0x24]
0042F389  e8 12 73 fd ff                  call 0x4066a0
0042F38E  8b 45 d4                        mov eax, dword ptr [ebp - 0x2c]
0042F391  8b 4d d4                        mov ecx, dword ptr [ebp - 0x2c]
0042F394  dd 81 50 03 00 00               fld qword ptr [ecx + 0x350]
0042F39A  dc 40 08                        fadd qword ptr [eax + 8]
0042F39D  dd 5d dc                        fstp qword ptr [ebp - 0x24]
0042F3A0  8b 55 d4                        mov edx, dword ptr [ebp - 0x2c]
0042F3A3  8b 45 d4                        mov eax, dword ptr [ebp - 0x2c]
0042F3A6  dd 80 58 03 00 00               fld qword ptr [eax + 0x358]
0042F3AC  dc 42 10                        fadd qword ptr [edx + 0x10]
0042F3AF  dd 5d e4                        fstp qword ptr [ebp - 0x1c]
0042F3B2  8b 4d d4                        mov ecx, dword ptr [ebp - 0x2c]
0042F3B5  8b 51 18                        mov edx, dword ptr [ecx + 0x18]
0042F3B8  89 55 ec                        mov dword ptr [ebp - 0x14], edx
0042F3BB  8b 41 1c                        mov eax, dword ptr [ecx + 0x1c]
0042F3BE  89 45 f0                        mov dword ptr [ebp - 0x10], eax
0042F3C1  b9 06 00 00 00                  mov ecx, 6
0042F3C6  8d 75 dc                        lea esi, [ebp - 0x24]
0042F3C9  8b 7d 08                        mov edi, dword ptr [ebp + 8]
0042F3CC  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
0042F3CE  8b 4d d8                        mov ecx, dword ptr [ebp - 0x28]
0042F3D1  83 c9 01                        or ecx, 1
0042F3D4  89 4d d8                        mov dword ptr [ebp - 0x28], ecx
0042F3D7  8d 4d dc                        lea ecx, [ebp - 0x24]
0042F3DA  e8 d1 4a fd ff                  call 0x403eb0
0042F3DF  8b 45 08                        mov eax, dword ptr [ebp + 8]
0042F3E2  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0042F3E5  64 89 0d 00 00 00 00            mov dword ptr fs:[0], ecx
0042F3EC  5f                              pop edi
0042F3ED  5e                              pop esi
0042F3EE  8b e5                           mov esp, ebp
0042F3F0  5d                              pop ebp
0042F3F1  c2 04 00                        ret 4

; VA 0x00418390, file offset 0x18390, length 47, SHA-256 254616efe4eeecb6423131c308a44bd17d051f5651e8a0adebbe4d242ca65772
00418390  55                              push ebp
00418391  8b ec                           mov ebp, esp
00418393  51                              push ecx
00418394  89 4d fc                        mov dword ptr [ebp - 4], ecx
00418397  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0041839A  c7 00 40 0b 4b 00               mov dword ptr [eax], 0x4b0b40
004183A0  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
004183A3  c7 41 08 00 00 00 00            mov dword ptr [ecx + 8], 0
004183AA  c7 41 0c 00 00 00 00            mov dword ptr [ecx + 0xc], 0
004183B1  8b 55 fc                        mov edx, dword ptr [ebp - 4]
004183B4  c6 42 10 00                     mov byte ptr [edx + 0x10], 0
004183B8  8b 45 fc                        mov eax, dword ptr [ebp - 4]
004183BB  8b e5                           mov esp, ebp
004183BD  5d                              pop ebp
004183BE  c3                              ret

; VA 0x00415FA0, file offset 0x15FA0, length 72, SHA-256 47f4c097829951b2df6234b31be6d06fd266a4e97f0034113238d15a9b65afd1
00415FA0  55                              push ebp
00415FA1  8b ec                           mov ebp, esp
00415FA3  51                              push ecx
00415FA4  89 4d fc                        mov dword ptr [ebp - 4], ecx
00415FA7  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
00415FAA  e8 e1 23 00 00                  call 0x418390
00415FAF  8b 45 fc                        mov eax, dword ptr [ebp - 4]
00415FB2  c7 00 20 0a 4b 00               mov dword ptr [eax], 0x4b0a20
00415FB8  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
00415FBB  c7 41 18 00 00 00 00            mov dword ptr [ecx + 0x18], 0
00415FC2  c7 41 1c 00 00 00 00            mov dword ptr [ecx + 0x1c], 0
00415FC9  8b 55 fc                        mov edx, dword ptr [ebp - 4]
00415FCC  c7 42 14 13 00 00 00            mov dword ptr [edx + 0x14], 0x13
00415FD3  8b 45 fc                        mov eax, dword ptr [ebp - 4]
00415FD6  c6 40 11 00                     mov byte ptr [eax + 0x11], 0
00415FDA  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
00415FDD  c6 41 20 00                     mov byte ptr [ecx + 0x20], 0
00415FE1  8b 45 fc                        mov eax, dword ptr [ebp - 4]
00415FE4  8b e5                           mov esp, ebp
00415FE6  5d                              pop ebp
00415FE7  c3                              ret

; VA 0x00404D30, file offset 0x4D30, length 16, SHA-256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936
00404D30  55                              push ebp
00404D31  8b ec                           mov ebp, esp
00404D33  51                              push ecx
00404D34  89 4d fc                        mov dword ptr [ebp - 4], ecx
00404D37  8b 45 fc                        mov eax, dword ptr [ebp - 4]
00404D3A  8b 00                           mov eax, dword ptr [eax]
00404D3C  8b e5                           mov esp, ebp
00404D3E  5d                              pop ebp
00404D3F  c3                              ret
