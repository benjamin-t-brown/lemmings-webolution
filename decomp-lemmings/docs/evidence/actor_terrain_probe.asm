; Original x86 instruction bytes; static disassembly, not runtime evidence.
; Reference SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Decoder: Capstone 5.0.7, x86 32-bit; addresses are virtual addresses.

; VA 0x0042F6EC, file offset 0x2F6EC, length 156, SHA-256 a47aaf13325a019ad3242e30894400456f1a5a5a5cc459c9e0a5820fe700caee
0042F6EC  55                              push ebp
0042F6ED  8b ec                           mov ebp, esp
0042F6EF  6a ff                           push -1
0042F6F1  68 a6 b0 4a 00                  push 0x4ab0a6
0042F6F6  64 a1 00 00 00 00               mov eax, dword ptr fs:[0]
0042F6FC  50                              push eax
0042F6FD  64 89 25 00 00 00 00            mov dword ptr fs:[0], esp
0042F704  83 ec 2c                        sub esp, 0x2c
0042F707  89 4d cc                        mov dword ptr [ebp - 0x34], ecx
0042F70A  c7 45 d8 10 6a 4c 00            mov dword ptr [ebp - 0x28], 0x4c6a10
0042F711  8d 45 dc                        lea eax, [ebp - 0x24]
0042F714  50                              push eax
0042F715  8b 4d cc                        mov ecx, dword ptr [ebp - 0x34]
0042F718  e8 11 fb ff ff                  call 0x42f22e
0042F71D  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
0042F724  dd 45 e4                        fld qword ptr [ebp - 0x1c]
0042F727  dc 1d 38 03 4b 00               fcomp qword ptr [0x4b0338]
0042F72D  df e0                           fnstsw ax
0042F72F  f6 c4 01                        test ah, 1
0042F732  75 18                           jne 0x42f74c
0042F734  8b 4d d8                        mov ecx, dword ptr [ebp - 0x28]
0042F737  e8 64 c3 fd ff                  call 0x40baa0
0042F73C  89 45 c8                        mov dword ptr [ebp - 0x38], eax
0042F73F  db 45 c8                        fild dword ptr [ebp - 0x38]
0042F742  dc 5d e4                        fcomp qword ptr [ebp - 0x1c]
0042F745  df e0                           fnstsw ax
0042F747  f6 c4 01                        test ah, 1
0042F74A  74 18                           je 0x42f764
0042F74C  c6 45 d4 00                     mov byte ptr [ebp - 0x2c], 0
0042F750  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0042F757  8d 4d dc                        lea ecx, [ebp - 0x24]
0042F75A  e8 51 47 fd ff                  call 0x403eb0
0042F75F  8a 45 d4                        mov al, byte ptr [ebp - 0x2c]
0042F762  eb 16                           jmp 0x42f77a
0042F764  c6 45 d0 01                     mov byte ptr [ebp - 0x30], 1
0042F768  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0042F76F  8d 4d dc                        lea ecx, [ebp - 0x24]
0042F772  e8 39 47 fd ff                  call 0x403eb0
0042F777  8a 45 d0                        mov al, byte ptr [ebp - 0x30]
0042F77A  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0042F77D  64 89 0d 00 00 00 00            mov dword ptr fs:[0], ecx
0042F784  8b e5                           mov esp, ebp
0042F786  5d                              pop ebp
0042F787  c3                              ret

; VA 0x0042F788, file offset 0x2F788, length 194, SHA-256 5af155c4d6fdc639bd74190564f17b2b78e6c1f3654fd96aea231286b96adc8d
0042F788  55                              push ebp
0042F789  8b ec                           mov ebp, esp
0042F78B  83 ec 10                        sub esp, 0x10
0042F78E  89 4d f0                        mov dword ptr [ebp - 0x10], ecx
0042F791  8b 45 f0                        mov eax, dword ptr [ebp - 0x10]
0042F794  8b 48 10                        mov ecx, dword ptr [eax + 0x10]
0042F797  89 4d f4                        mov dword ptr [ebp - 0xc], ecx
0042F79A  8b 50 14                        mov edx, dword ptr [eax + 0x14]
0042F79D  89 55 f8                        mov dword ptr [ebp - 8], edx
0042F7A0  dd 45 f4                        fld qword ptr [ebp - 0xc]
0042F7A3  e8 04 ce 06 00                  call 0x49c5ac
0042F7A8  89 45 fc                        mov dword ptr [ebp - 4], eax
0042F7AB  83 7d fc ff                     cmp dword ptr [ebp - 4], -1
0042F7AF  0f 8e 8b 00 00 00               jle 0x42f840
0042F7B5  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0042F7BA  e8 e1 c2 fd ff                  call 0x40baa0
0042F7BF  39 45 fc                        cmp dword ptr [ebp - 4], eax
0042F7C2  7d 7c                           jge 0x42f840
0042F7C4  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0042F7C7  50                              push eax
0042F7C8  8b 4d f0                        mov ecx, dword ptr [ebp - 0x10]
0042F7CB  dd 41 08                        fld qword ptr [ecx + 8]
0042F7CE  e8 d9 cd 06 00                  call 0x49c5ac
0042F7D3  50                              push eax
0042F7D4  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0042F7D9  e8 82 e6 fd ff                  call 0x40de60
0042F7DE  83 38 00                        cmp dword ptr [eax], 0
0042F7E1  74 4c                           je 0x42f82f
0042F7E3  8b 55 fc                        mov edx, dword ptr [ebp - 4]
0042F7E6  52                              push edx
0042F7E7  8b 45 f0                        mov eax, dword ptr [ebp - 0x10]
0042F7EA  dd 40 08                        fld qword ptr [eax + 8]
0042F7ED  e8 ba cd 06 00                  call 0x49c5ac
0042F7F2  50                              push eax
0042F7F3  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0042F7F8  e8 63 e6 fd ff                  call 0x40de60
0042F7FD  8b 08                           mov ecx, dword ptr [eax]
0042F7FF  81 e1 00 02 00 00               and ecx, 0x200
0042F805  85 c9                           test ecx, ecx
0042F807  75 26                           jne 0x42f82f
0042F809  8b 55 fc                        mov edx, dword ptr [ebp - 4]
0042F80C  52                              push edx
0042F80D  8b 45 f0                        mov eax, dword ptr [ebp - 0x10]
0042F810  dd 40 08                        fld qword ptr [eax + 8]
0042F813  e8 94 cd 06 00                  call 0x49c5ac
0042F818  50                              push eax
0042F819  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0042F81E  e8 3d e6 fd ff                  call 0x40de60
0042F823  8b 08                           mov ecx, dword ptr [eax]
0042F825  81 e1 80 00 00 00               and ecx, 0x80
0042F82B  85 c9                           test ecx, ecx
0042F82D  74 11                           je 0x42f840
0042F82F  8b 55 f0                        mov edx, dword ptr [ebp - 0x10]
0042F832  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0042F835  2b 42 44                        sub eax, dword ptr [edx + 0x44]
0042F838  89 45 fc                        mov dword ptr [ebp - 4], eax
0042F83B  e9 6b ff ff ff                  jmp 0x42f7ab
0042F840  db 45 fc                        fild dword ptr [ebp - 4]
0042F843  dc 6d f4                        fsubr qword ptr [ebp - 0xc]
0042F846  8b e5                           mov esp, ebp
0042F848  5d                              pop ebp
0042F849  c3                              ret
