; Original x86 instruction bytes; static disassembly, not runtime evidence.
; Reference SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Decoder: Capstone 5.0.7, x86 32-bit; addresses are virtual addresses.

; VA 0x00458186, file offset 0x58186, length 4775, SHA-256 f58dc109bbc1701770603607a5c7453613f11a85e3c20c57b6a174bc09fddc86
00458186  55                              push ebp
00458187  8b ec                           mov ebp, esp
00458189  6a ff                           push -1
0045818B  68 e2 d0 4a 00                  push 0x4ad0e2
00458190  64 a1 00 00 00 00               mov eax, dword ptr fs:[0]
00458196  50                              push eax
00458197  64 89 25 00 00 00 00            mov dword ptr fs:[0], esp
0045819E  81 ec 20 06 00 00               sub esp, 0x620
004581A4  56                              push esi
004581A5  57                              push edi
004581A6  89 8d d4 f9 ff ff               mov dword ptr [ebp - 0x62c], ecx
004581AC  8d 4d a8                        lea ecx, [ebp - 0x58]
004581AF  e8 3c a8 fc ff                  call 0x4229f0
004581B4  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
004581BB  6a 01                           push 1
004581BD  8d 4d a8                        lea ecx, [ebp - 0x58]
004581C0  e8 9b 90 fc ff                  call 0x421260
004581C5  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
004581CB  8b b0 a8 03 00 00               mov esi, dword ptr [eax + 0x3a8]
004581D1  83 c6 08                        add esi, 8
004581D4  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
004581DA  81 c7 b0 03 00 00               add edi, 0x3b0
004581E0  b9 06 00 00 00                  mov ecx, 6
004581E5  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
004581E7  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004581ED  8b 91 a8 03 00 00               mov edx, dword ptr [ecx + 0x3a8]
004581F3  83 7a 44 ff                     cmp dword ptr [edx + 0x44], -1
004581F7  74 12                           je 0x45820b
004581F9  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
004581FF  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458205  83 79 54 00                     cmp dword ptr [ecx + 0x54], 0
00458209  74 21                           je 0x45822c
0045820B  8b 95 d4 f9 ff ff               mov edx, dword ptr [ebp - 0x62c]
00458211  c6 82 ad 03 00 00 00            mov byte ptr [edx + 0x3ad], 0
00458218  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0045821F  8d 4d a8                        lea ecx, [ebp - 0x58]
00458222  e8 09 a8 fc ff                  call 0x422a30
00458227  e9 f1 11 00 00                  jmp 0x45941d
0045822C  6a 23                           push 0x23
0045822E  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458234  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
0045823A  e8 8a 6a fd ff                  call 0x42ecc9
0045823F  25 ff 00 00 00                  and eax, 0xff
00458244  85 c0                           test eax, eax
00458246  74 21                           je 0x458269
00458248  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
0045824E  c6 81 ad 03 00 00 00            mov byte ptr [ecx + 0x3ad], 0
00458255  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0045825C  8d 4d a8                        lea ecx, [ebp - 0x58]
0045825F  e8 cc a7 fc ff                  call 0x422a30
00458264  e9 b4 11 00 00                  jmp 0x45941d
00458269  8b 95 d4 f9 ff ff               mov edx, dword ptr [ebp - 0x62c]
0045826F  8b 8a a8 03 00 00               mov ecx, dword ptr [edx + 0x3a8]
00458275  e8 99 66 fd ff                  call 0x42e913
0045827A  25 ff 00 00 00                  and eax, 0xff
0045827F  85 c0                           test eax, eax
00458281  0f 85 d8 00 00 00               jne 0x45835f
00458287  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
0045828D  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458293  e8 f0 74 fd ff                  call 0x42f788
00458298  dd 9d 7c ff ff ff               fstp qword ptr [ebp - 0x84]
0045829E  dd 85 7c ff ff ff               fld qword ptr [ebp - 0x84]
004582A4  dc 1d 38 03 4b 00               fcomp qword ptr [0x4b0338]
004582AA  df e0                           fnstsw ax
004582AC  f6 c4 41                        test ah, 0x41
004582AF  0f 85 87 00 00 00               jne 0x45833c
004582B5  dd 85 7c ff ff ff               fld qword ptr [ebp - 0x84]
004582BB  dc 1d e8 03 4b 00               fcomp qword ptr [0x4b03e8]
004582C1  df e0                           fnstsw ax
004582C3  f6 c4 01                        test ah, 1
004582C6  74 74                           je 0x45833c
004582C8  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004582CE  c6 81 ad 03 00 00 01            mov byte ptr [ecx + 0x3ad], 1
004582D5  8b 95 d4 f9 ff ff               mov edx, dword ptr [ebp - 0x62c]
004582DB  dd 82 b8 03 00 00               fld qword ptr [edx + 0x3b8]
004582E1  dc a5 7c ff ff ff               fsub qword ptr [ebp - 0x84]
004582E7  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
004582ED  dd 98 b8 03 00 00               fstp qword ptr [eax + 0x3b8]
004582F3  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004582F9  dd 81 b8 03 00 00               fld qword ptr [ecx + 0x3b8]
004582FF  dc 05 d0 02 4b 00               fadd qword ptr [0x4b02d0]
00458305  8b 95 d4 f9 ff ff               mov edx, dword ptr [ebp - 0x62c]
0045830B  dd 9a b8 03 00 00               fstp qword ptr [edx + 0x3b8]
00458311  dd 85 7c ff ff ff               fld qword ptr [ebp - 0x84]
00458317  dc 35 e8 03 4b 00               fdiv qword ptr [0x4b03e8]
0045831D  dc 0d 00 04 4b 00               fmul qword ptr [0x4b0400]
00458323  dc 2d 00 04 4b 00               fsubr qword ptr [0x4b0400]
00458329  e8 7e 42 04 00                  call 0x49c5ac
0045832E  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00458334  89 81 c8 03 00 00               mov dword ptr [ecx + 0x3c8], eax
0045833A  eb 21                           jmp 0x45835d
0045833C  8b 95 d4 f9 ff ff               mov edx, dword ptr [ebp - 0x62c]
00458342  c6 82 ad 03 00 00 00            mov byte ptr [edx + 0x3ad], 0
00458349  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
00458350  8d 4d a8                        lea ecx, [ebp - 0x58]
00458353  e8 d8 a6 fc ff                  call 0x422a30
00458358  e9 c0 10 00 00                  jmp 0x45941d
0045835D  eb 31                           jmp 0x458390
0045835F  c7 05 c8 05 4c 00 00 00 00 00   mov dword ptr [0x4c05c8], 0
00458369  c7 05 cc 05 4c 00 00 00 04 40   mov dword ptr [0x4c05cc], 0x40040000
00458373  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458379  c6 80 ad 03 00 00 01            mov byte ptr [eax + 0x3ad], 1
00458380  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00458386  c7 81 c8 03 00 00 ff 00 00 00   mov dword ptr [ecx + 0x3c8], 0xff
00458390  8b 95 d4 f9 ff ff               mov edx, dword ptr [ebp - 0x62c]
00458396  8b b2 a8 03 00 00               mov esi, dword ptr [edx + 0x3a8]
0045839C  83 c6 08                        add esi, 8
0045839F  b9 06 00 00 00                  mov ecx, 6
004583A4  8d 7d 84                        lea edi, [ebp - 0x7c]
004583A7  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
004583A9  dd 05 c8 05 4c 00               fld qword ptr [0x4c05c8]
004583AF  dc 35 f8 03 4b 00               fdiv qword ptr [0x4b03f8]
004583B5  dc 6d 84                        fsubr qword ptr [ebp - 0x7c]
004583B8  83 ec 08                        sub esp, 8
004583BB  dd 1c 24                        fstp qword ptr [esp]
004583BE  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
004583C3  e8 f8 5a fb ff                  call 0x40dec0
004583C8  dd 5d c0                        fstp qword ptr [ebp - 0x40]
004583CB  dd 05 c8 05 4c 00               fld qword ptr [0x4c05c8]
004583D1  dc 35 f8 03 4b 00               fdiv qword ptr [0x4b03f8]
004583D7  dc 45 84                        fadd qword ptr [ebp - 0x7c]
004583DA  83 ec 08                        sub esp, 8
004583DD  dd 1c 24                        fstp qword ptr [esp]
004583E0  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
004583E5  e8 d6 5a fb ff                  call 0x40dec0
004583EA  dd 5d b8                        fstp qword ptr [ebp - 0x48]
004583ED  8b 45 bc                        mov eax, dword ptr [ebp - 0x44]
004583F0  50                              push eax
004583F1  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
004583F4  51                              push ecx
004583F5  8b 55 c4                        mov edx, dword ptr [ebp - 0x3c]
004583F8  52                              push edx
004583F9  8b 45 c0                        mov eax, dword ptr [ebp - 0x40]
004583FC  50                              push eax
004583FD  8d 4d a8                        lea ecx, [ebp - 0x58]
00458400  51                              push ecx
00458401  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00458407  e8 4b 11 00 00                  call 0x459557
0045840C  8d 4d a8                        lea ecx, [ebp - 0x58]
0045840F  e8 7c 15 00 00                  call 0x459990
00458414  89 45 f0                        mov dword ptr [ebp - 0x10], eax
00458417  8b 55 f0                        mov edx, dword ptr [ebp - 0x10]
0045841A  83 ea 01                        sub edx, 1
0045841D  52                              push edx
0045841E  8d 4d a8                        lea ecx, [ebp - 0x58]
00458421  e8 1a 8e fc ff                  call 0x421240
00458426  8b f0                           mov esi, eax
00458428  6a 00                           push 0
0045842A  8d 4d a8                        lea ecx, [ebp - 0x58]
0045842D  e8 0e 8e fc ff                  call 0x421240
00458432  dd 06                           fld qword ptr [esi]
00458434  dc 20                           fsub qword ptr [eax]
00458436  83 ec 08                        sub esp, 8
00458439  dd 1c 24                        fstp qword ptr [esp]
0045843C  e8 ef 5a fb ff                  call 0x40df30
00458441  83 c4 08                        add esp, 8
00458444  dd 5d 9c                        fstp qword ptr [ebp - 0x64]
00458447  c7 45 a4 00 00 00 00            mov dword ptr [ebp - 0x5c], 0
0045844E  eb 09                           jmp 0x458459
00458450  8b 45 a4                        mov eax, dword ptr [ebp - 0x5c]
00458453  83 c0 01                        add eax, 1
00458456  89 45 a4                        mov dword ptr [ebp - 0x5c], eax
00458459  8b 4d a4                        mov ecx, dword ptr [ebp - 0x5c]
0045845C  3b 4d f0                        cmp ecx, dword ptr [ebp - 0x10]
0045845F  7d 28                           jge 0x458489
00458461  8b 55 a4                        mov edx, dword ptr [ebp - 0x5c]
00458464  52                              push edx
00458465  8d 4d a8                        lea ecx, [ebp - 0x58]
00458468  e8 d3 8d fc ff                  call 0x421240
0045846D  8b f0                           mov esi, eax
0045846F  6a 00                           push 0
00458471  8d 4d a8                        lea ecx, [ebp - 0x58]
00458474  e8 c7 8d fc ff                  call 0x421240
00458479  dd 06                           fld qword ptr [esi]
0045847B  dc 20                           fsub qword ptr [eax]
0045847D  dc 75 9c                        fdiv qword ptr [ebp - 0x64]
00458480  8b 45 a4                        mov eax, dword ptr [ebp - 0x5c]
00458483  dd 5c c5 c8                     fstp qword ptr [ebp + eax*8 - 0x38]
00458487  eb c7                           jmp 0x458450
00458489  8b 0d c0 05 4c 00               mov ecx, dword ptr [0x4c05c0]
0045848F  51                              push ecx
00458490  b9 48 cb 58 00                  mov ecx, 0x58cb48
00458495  e8 76 c5 fb ff                  call 0x414a10
0045849A  8b c8                           mov ecx, eax
0045849C  e8 7e a1 01 00                  call 0x47261f
004584A1  c7 45 e8 52 b8 1e 85            mov dword ptr [ebp - 0x18], 0x851eb852
004584A8  c7 45 ec eb 51 c8 3f            mov dword ptr [ebp - 0x14], 0x3fc851eb
004584AF  8d 4d a8                        lea ecx, [ebp - 0x58]
004584B2  e8 d9 14 00 00                  call 0x459990
004584B7  83 f8 03                        cmp eax, 3
004584BA  0f 85 4a 05 00 00               jne 0x458a0a
004584C0  8b 95 d4 f9 ff ff               mov edx, dword ptr [ebp - 0x62c]
004584C6  c7 82 20 01 00 00 01 00 00 00   mov dword ptr [edx + 0x120], 1
004584D0  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
004584D6  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
004584DC  dd 41 18                        fld qword ptr [ecx + 0x18]
004584DF  dc 05 48 10 4b 00               fadd qword ptr [0x4b1048]
004584E5  83 ec 08                        sub esp, 8
004584E8  dd 1c 24                        fstp qword ptr [esp]
004584EB  6a 00                           push 0
004584ED  8d 4d a8                        lea ecx, [ebp - 0x58]
004584F0  e8 4b 8d fc ff                  call 0x421240
004584F5  dd 45 e8                        fld qword ptr [ebp - 0x18]
004584F8  dc 40 08                        fadd qword ptr [eax + 8]
004584FB  83 ec 08                        sub esp, 8
004584FE  dd 1c 24                        fstp qword ptr [esp]
00458501  6a 00                           push 0
00458503  8d 4d a8                        lea ecx, [ebp - 0x58]
00458506  e8 35 8d fc ff                  call 0x421240
0045850B  8b 50 04                        mov edx, dword ptr [eax + 4]
0045850E  52                              push edx
0045850F  8b 00                           mov eax, dword ptr [eax]
00458511  50                              push eax
00458512  8d 8d 48 ff ff ff               lea ecx, [ebp - 0xb8]
00458518  e8 d3 92 fa ff                  call 0x4017f0
0045851D  50                              push eax
0045851E  8d 8d 60 ff ff ff               lea ecx, [ebp - 0xa0]
00458524  e8 a7 e1 fa ff                  call 0x4066d0
00458529  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0045852D  8d 8d 60 ff ff ff               lea ecx, [ebp - 0xa0]
00458533  51                              push ecx
00458534  8d 95 30 ff ff ff               lea edx, [ebp - 0xd0]
0045853A  52                              push edx
0045853B  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458540  e8 27 50 fe ff                  call 0x43d56c
00458545  8b f0                           mov esi, eax
00458547  b9 06 00 00 00                  mov ecx, 6
0045854C  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458552  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458554  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458558  8d 8d 60 ff ff ff               lea ecx, [ebp - 0xa0]
0045855E  e8 4d b9 fa ff                  call 0x403eb0
00458563  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458569  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
0045856F  dd 41 18                        fld qword ptr [ecx + 0x18]
00458572  dc 25 48 10 4b 00               fsub qword ptr [0x4b1048]
00458578  83 ec 08                        sub esp, 8
0045857B  dd 1c 24                        fstp qword ptr [esp]
0045857E  6a 00                           push 0
00458580  8d 4d a8                        lea ecx, [ebp - 0x58]
00458583  e8 b8 8c fc ff                  call 0x421240
00458588  dd 45 e8                        fld qword ptr [ebp - 0x18]
0045858B  dc 40 08                        fadd qword ptr [eax + 8]
0045858E  83 ec 08                        sub esp, 8
00458591  dd 1c 24                        fstp qword ptr [esp]
00458594  6a 00                           push 0
00458596  8d 4d a8                        lea ecx, [ebp - 0x58]
00458599  e8 a2 8c fc ff                  call 0x421240
0045859E  8b 50 04                        mov edx, dword ptr [eax + 4]
004585A1  52                              push edx
004585A2  8b 00                           mov eax, dword ptr [eax]
004585A4  50                              push eax
004585A5  8d 8d 00 ff ff ff               lea ecx, [ebp - 0x100]
004585AB  e8 40 92 fa ff                  call 0x4017f0
004585B0  50                              push eax
004585B1  8d 8d 18 ff ff ff               lea ecx, [ebp - 0xe8]
004585B7  e8 14 e1 fa ff                  call 0x4066d0
004585BC  c6 45 fc 02                     mov byte ptr [ebp - 4], 2
004585C0  8d 8d 18 ff ff ff               lea ecx, [ebp - 0xe8]
004585C6  51                              push ecx
004585C7  8d 95 e8 fe ff ff               lea edx, [ebp - 0x118]
004585CD  52                              push edx
004585CE  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
004585D3  e8 94 4f fe ff                  call 0x43d56c
004585D8  8b f0                           mov esi, eax
004585DA  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
004585E0  83 c7 18                        add edi, 0x18
004585E3  b9 06 00 00 00                  mov ecx, 6
004585E8  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
004585EA  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
004585EE  8d 8d 18 ff ff ff               lea ecx, [ebp - 0xe8]
004585F4  e8 b7 b8 fa ff                  call 0x403eb0
004585F9  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
004585FF  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458605  dd 41 18                        fld qword ptr [ecx + 0x18]
00458608  dc 05 48 10 4b 00               fadd qword ptr [0x4b1048]
0045860E  83 ec 08                        sub esp, 8
00458611  dd 1c 24                        fstp qword ptr [esp]
00458614  6a 00                           push 0
00458616  8d 4d a8                        lea ecx, [ebp - 0x58]
00458619  e8 22 8c fc ff                  call 0x421240
0045861E  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458621  dc 40 08                        fadd qword ptr [eax + 8]
00458624  83 ec 08                        sub esp, 8
00458627  dd 1c 24                        fstp qword ptr [esp]
0045862A  6a 01                           push 1
0045862C  8d 4d a8                        lea ecx, [ebp - 0x58]
0045862F  e8 0c 8c fc ff                  call 0x421240
00458634  8b 50 04                        mov edx, dword ptr [eax + 4]
00458637  52                              push edx
00458638  8b 00                           mov eax, dword ptr [eax]
0045863A  50                              push eax
0045863B  8d 8d b8 fe ff ff               lea ecx, [ebp - 0x148]
00458641  e8 aa 91 fa ff                  call 0x4017f0
00458646  50                              push eax
00458647  8d 8d d0 fe ff ff               lea ecx, [ebp - 0x130]
0045864D  e8 7e e0 fa ff                  call 0x4066d0
00458652  c6 45 fc 03                     mov byte ptr [ebp - 4], 3
00458656  8d 8d d0 fe ff ff               lea ecx, [ebp - 0x130]
0045865C  51                              push ecx
0045865D  8d 95 a0 fe ff ff               lea edx, [ebp - 0x160]
00458663  52                              push edx
00458664  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458669  e8 fe 4e fe ff                  call 0x43d56c
0045866E  8b f0                           mov esi, eax
00458670  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458676  83 c7 30                        add edi, 0x30
00458679  b9 06 00 00 00                  mov ecx, 6
0045867E  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458680  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458684  8d 8d d0 fe ff ff               lea ecx, [ebp - 0x130]
0045868A  e8 21 b8 fa ff                  call 0x403eb0
0045868F  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458695  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
0045869B  dd 41 18                        fld qword ptr [ecx + 0x18]
0045869E  dc 25 48 10 4b 00               fsub qword ptr [0x4b1048]
004586A4  83 ec 08                        sub esp, 8
004586A7  dd 1c 24                        fstp qword ptr [esp]
004586AA  6a 00                           push 0
004586AC  8d 4d a8                        lea ecx, [ebp - 0x58]
004586AF  e8 8c 8b fc ff                  call 0x421240
004586B4  dd 45 e8                        fld qword ptr [ebp - 0x18]
004586B7  dc 40 08                        fadd qword ptr [eax + 8]
004586BA  83 ec 08                        sub esp, 8
004586BD  dd 1c 24                        fstp qword ptr [esp]
004586C0  6a 01                           push 1
004586C2  8d 4d a8                        lea ecx, [ebp - 0x58]
004586C5  e8 76 8b fc ff                  call 0x421240
004586CA  8b 50 04                        mov edx, dword ptr [eax + 4]
004586CD  52                              push edx
004586CE  8b 00                           mov eax, dword ptr [eax]
004586D0  50                              push eax
004586D1  8d 8d 70 fe ff ff               lea ecx, [ebp - 0x190]
004586D7  e8 14 91 fa ff                  call 0x4017f0
004586DC  50                              push eax
004586DD  8d 8d 88 fe ff ff               lea ecx, [ebp - 0x178]
004586E3  e8 e8 df fa ff                  call 0x4066d0
004586E8  c6 45 fc 04                     mov byte ptr [ebp - 4], 4
004586EC  8d 8d 88 fe ff ff               lea ecx, [ebp - 0x178]
004586F2  51                              push ecx
004586F3  8d 95 58 fe ff ff               lea edx, [ebp - 0x1a8]
004586F9  52                              push edx
004586FA  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
004586FF  e8 68 4e fe ff                  call 0x43d56c
00458704  8b f0                           mov esi, eax
00458706  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
0045870C  83 c7 48                        add edi, 0x48
0045870F  b9 06 00 00 00                  mov ecx, 6
00458714  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458716  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
0045871A  8d 8d 88 fe ff ff               lea ecx, [ebp - 0x178]
00458720  e8 8b b7 fa ff                  call 0x403eb0
00458725  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
0045872B  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458731  dd 41 18                        fld qword ptr [ecx + 0x18]
00458734  dc 05 48 10 4b 00               fadd qword ptr [0x4b1048]
0045873A  83 ec 08                        sub esp, 8
0045873D  dd 1c 24                        fstp qword ptr [esp]
00458740  6a 02                           push 2
00458742  8d 4d a8                        lea ecx, [ebp - 0x58]
00458745  e8 f6 8a fc ff                  call 0x421240
0045874A  dd 45 e8                        fld qword ptr [ebp - 0x18]
0045874D  dc 40 08                        fadd qword ptr [eax + 8]
00458750  83 ec 08                        sub esp, 8
00458753  dd 1c 24                        fstp qword ptr [esp]
00458756  6a 01                           push 1
00458758  8d 4d a8                        lea ecx, [ebp - 0x58]
0045875B  e8 e0 8a fc ff                  call 0x421240
00458760  8b 50 04                        mov edx, dword ptr [eax + 4]
00458763  52                              push edx
00458764  8b 00                           mov eax, dword ptr [eax]
00458766  50                              push eax
00458767  8d 8d 28 fe ff ff               lea ecx, [ebp - 0x1d8]
0045876D  e8 7e 90 fa ff                  call 0x4017f0
00458772  50                              push eax
00458773  8d 8d 40 fe ff ff               lea ecx, [ebp - 0x1c0]
00458779  e8 52 df fa ff                  call 0x4066d0
0045877E  c6 45 fc 05                     mov byte ptr [ebp - 4], 5
00458782  8d 8d 40 fe ff ff               lea ecx, [ebp - 0x1c0]
00458788  51                              push ecx
00458789  8d 95 10 fe ff ff               lea edx, [ebp - 0x1f0]
0045878F  52                              push edx
00458790  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458795  e8 d2 4d fe ff                  call 0x43d56c
0045879A  8b f0                           mov esi, eax
0045879C  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
004587A2  83 c7 60                        add edi, 0x60
004587A5  b9 06 00 00 00                  mov ecx, 6
004587AA  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
004587AC  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
004587B0  8d 8d 40 fe ff ff               lea ecx, [ebp - 0x1c0]
004587B6  e8 f5 b6 fa ff                  call 0x403eb0
004587BB  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
004587C1  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
004587C7  dd 41 18                        fld qword ptr [ecx + 0x18]
004587CA  dc 25 48 10 4b 00               fsub qword ptr [0x4b1048]
004587D0  83 ec 08                        sub esp, 8
004587D3  dd 1c 24                        fstp qword ptr [esp]
004587D6  6a 02                           push 2
004587D8  8d 4d a8                        lea ecx, [ebp - 0x58]
004587DB  e8 60 8a fc ff                  call 0x421240
004587E0  dd 45 e8                        fld qword ptr [ebp - 0x18]
004587E3  dc 40 08                        fadd qword ptr [eax + 8]
004587E6  83 ec 08                        sub esp, 8
004587E9  dd 1c 24                        fstp qword ptr [esp]
004587EC  6a 01                           push 1
004587EE  8d 4d a8                        lea ecx, [ebp - 0x58]
004587F1  e8 4a 8a fc ff                  call 0x421240
004587F6  8b 50 04                        mov edx, dword ptr [eax + 4]
004587F9  52                              push edx
004587FA  8b 00                           mov eax, dword ptr [eax]
004587FC  50                              push eax
004587FD  8d 8d e0 fd ff ff               lea ecx, [ebp - 0x220]
00458803  e8 e8 8f fa ff                  call 0x4017f0
00458808  50                              push eax
00458809  8d 8d f8 fd ff ff               lea ecx, [ebp - 0x208]
0045880F  e8 bc de fa ff                  call 0x4066d0
00458814  c6 45 fc 06                     mov byte ptr [ebp - 4], 6
00458818  8d 8d f8 fd ff ff               lea ecx, [ebp - 0x208]
0045881E  51                              push ecx
0045881F  8d 95 c8 fd ff ff               lea edx, [ebp - 0x238]
00458825  52                              push edx
00458826  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0045882B  e8 3c 4d fe ff                  call 0x43d56c
00458830  8b f0                           mov esi, eax
00458832  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458838  83 c7 78                        add edi, 0x78
0045883B  b9 06 00 00 00                  mov ecx, 6
00458840  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458842  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458846  8d 8d f8 fd ff ff               lea ecx, [ebp - 0x208]
0045884C  e8 5f b6 fa ff                  call 0x403eb0
00458851  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458857  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
0045885D  dd 41 18                        fld qword ptr [ecx + 0x18]
00458860  dc 05 48 10 4b 00               fadd qword ptr [0x4b1048]
00458866  83 ec 08                        sub esp, 8
00458869  dd 1c 24                        fstp qword ptr [esp]
0045886C  6a 02                           push 2
0045886E  8d 4d a8                        lea ecx, [ebp - 0x58]
00458871  e8 ca 89 fc ff                  call 0x421240
00458876  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458879  dc 40 08                        fadd qword ptr [eax + 8]
0045887C  83 ec 08                        sub esp, 8
0045887F  dd 1c 24                        fstp qword ptr [esp]
00458882  6a 02                           push 2
00458884  8d 4d a8                        lea ecx, [ebp - 0x58]
00458887  e8 b4 89 fc ff                  call 0x421240
0045888C  8b 50 04                        mov edx, dword ptr [eax + 4]
0045888F  52                              push edx
00458890  8b 00                           mov eax, dword ptr [eax]
00458892  50                              push eax
00458893  8d 8d 98 fd ff ff               lea ecx, [ebp - 0x268]
00458899  e8 52 8f fa ff                  call 0x4017f0
0045889E  50                              push eax
0045889F  8d 8d b0 fd ff ff               lea ecx, [ebp - 0x250]
004588A5  e8 26 de fa ff                  call 0x4066d0
004588AA  c6 45 fc 07                     mov byte ptr [ebp - 4], 7
004588AE  8d 8d b0 fd ff ff               lea ecx, [ebp - 0x250]
004588B4  51                              push ecx
004588B5  8d 95 80 fd ff ff               lea edx, [ebp - 0x280]
004588BB  52                              push edx
004588BC  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
004588C1  e8 a6 4c fe ff                  call 0x43d56c
004588C6  8b f0                           mov esi, eax
004588C8  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
004588CE  81 c7 90 00 00 00               add edi, 0x90
004588D4  b9 06 00 00 00                  mov ecx, 6
004588D9  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
004588DB  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
004588DF  8d 8d b0 fd ff ff               lea ecx, [ebp - 0x250]
004588E5  e8 c6 b5 fa ff                  call 0x403eb0
004588EA  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
004588F0  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
004588F6  dd 41 18                        fld qword ptr [ecx + 0x18]
004588F9  dc 25 48 10 4b 00               fsub qword ptr [0x4b1048]
004588FF  83 ec 08                        sub esp, 8
00458902  dd 1c 24                        fstp qword ptr [esp]
00458905  6a 02                           push 2
00458907  8d 4d a8                        lea ecx, [ebp - 0x58]
0045890A  e8 31 89 fc ff                  call 0x421240
0045890F  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458912  dc 40 08                        fadd qword ptr [eax + 8]
00458915  83 ec 08                        sub esp, 8
00458918  dd 1c 24                        fstp qword ptr [esp]
0045891B  6a 02                           push 2
0045891D  8d 4d a8                        lea ecx, [ebp - 0x58]
00458920  e8 1b 89 fc ff                  call 0x421240
00458925  8b 50 04                        mov edx, dword ptr [eax + 4]
00458928  52                              push edx
00458929  8b 00                           mov eax, dword ptr [eax]
0045892B  50                              push eax
0045892C  8d 8d 50 fd ff ff               lea ecx, [ebp - 0x2b0]
00458932  e8 b9 8e fa ff                  call 0x4017f0
00458937  50                              push eax
00458938  8d 8d 68 fd ff ff               lea ecx, [ebp - 0x298]
0045893E  e8 8d dd fa ff                  call 0x4066d0
00458943  c6 45 fc 08                     mov byte ptr [ebp - 4], 8
00458947  8d 8d 68 fd ff ff               lea ecx, [ebp - 0x298]
0045894D  51                              push ecx
0045894E  8d 95 38 fd ff ff               lea edx, [ebp - 0x2c8]
00458954  52                              push edx
00458955  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0045895A  e8 0d 4c fe ff                  call 0x43d56c
0045895F  8b f0                           mov esi, eax
00458961  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458967  81 c7 a8 00 00 00               add edi, 0xa8
0045896D  b9 06 00 00 00                  mov ecx, 6
00458972  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458974  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458978  8d 8d 68 fd ff ff               lea ecx, [ebp - 0x298]
0045897E  e8 2d b5 fa ff                  call 0x403eb0
00458983  6a 00                           push 0
00458985  6a 00                           push 0
00458987  8b 45 cc                        mov eax, dword ptr [ebp - 0x34]
0045898A  50                              push eax
0045898B  8b 4d c8                        mov ecx, dword ptr [ebp - 0x38]
0045898E  51                              push ecx
0045898F  6a 00                           push 0
00458991  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00458997  81 c1 28 01 00 00               add ecx, 0x128
0045899D  e8 be ab fb ff                  call 0x413560
004589A2  68 00 00 f0 3f                  push 0x3ff00000
004589A7  6a 00                           push 0
004589A9  8b 55 cc                        mov edx, dword ptr [ebp - 0x34]
004589AC  52                              push edx
004589AD  8b 45 c8                        mov eax, dword ptr [ebp - 0x38]
004589B0  50                              push eax
004589B1  6a 01                           push 1
004589B3  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004589B9  81 c1 28 01 00 00               add ecx, 0x128
004589BF  e8 9c ab fb ff                  call 0x413560
004589C4  68 00 00 f0 3f                  push 0x3ff00000
004589C9  6a 00                           push 0
004589CB  8b 4d d4                        mov ecx, dword ptr [ebp - 0x2c]
004589CE  51                              push ecx
004589CF  8b 55 d0                        mov edx, dword ptr [ebp - 0x30]
004589D2  52                              push edx
004589D3  6a 02                           push 2
004589D5  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004589DB  81 c1 28 01 00 00               add ecx, 0x128
004589E1  e8 7a ab fb ff                  call 0x413560
004589E6  6a 00                           push 0
004589E8  6a 00                           push 0
004589EA  8b 45 d4                        mov eax, dword ptr [ebp - 0x2c]
004589ED  50                              push eax
004589EE  8b 4d d0                        mov ecx, dword ptr [ebp - 0x30]
004589F1  51                              push ecx
004589F2  6a 03                           push 3
004589F4  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004589FA  81 c1 28 01 00 00               add ecx, 0x128
00458A00  e8 5b ab fb ff                  call 0x413560
00458A05  e9 04 0a 00 00                  jmp 0x45940e
00458A0A  8d 4d a8                        lea ecx, [ebp - 0x58]
00458A0D  e8 7e 0f 00 00                  call 0x459990
00458A12  83 f8 04                        cmp eax, 4
00458A15  0f 85 f3 09 00 00               jne 0x45940e
00458A1B  8b 95 d4 f9 ff ff               mov edx, dword ptr [ebp - 0x62c]
00458A21  c7 82 20 01 00 00 05 00 00 00   mov dword ptr [edx + 0x120], 5
00458A2B  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458A31  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458A37  dd 41 18                        fld qword ptr [ecx + 0x18]
00458A3A  dc 05 48 10 4b 00               fadd qword ptr [0x4b1048]
00458A40  83 ec 08                        sub esp, 8
00458A43  dd 1c 24                        fstp qword ptr [esp]
00458A46  6a 00                           push 0
00458A48  8d 4d a8                        lea ecx, [ebp - 0x58]
00458A4B  e8 f0 87 fc ff                  call 0x421240
00458A50  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458A53  dc 40 08                        fadd qword ptr [eax + 8]
00458A56  83 ec 08                        sub esp, 8
00458A59  dd 1c 24                        fstp qword ptr [esp]
00458A5C  6a 00                           push 0
00458A5E  8d 4d a8                        lea ecx, [ebp - 0x58]
00458A61  e8 da 87 fc ff                  call 0x421240
00458A66  8b 50 04                        mov edx, dword ptr [eax + 4]
00458A69  52                              push edx
00458A6A  8b 00                           mov eax, dword ptr [eax]
00458A6C  50                              push eax
00458A6D  8d 8d 08 fd ff ff               lea ecx, [ebp - 0x2f8]
00458A73  e8 78 8d fa ff                  call 0x4017f0
00458A78  50                              push eax
00458A79  8d 8d 20 fd ff ff               lea ecx, [ebp - 0x2e0]
00458A7F  e8 4c dc fa ff                  call 0x4066d0
00458A84  c6 45 fc 09                     mov byte ptr [ebp - 4], 9
00458A88  8d 8d 20 fd ff ff               lea ecx, [ebp - 0x2e0]
00458A8E  51                              push ecx
00458A8F  8d 95 f0 fc ff ff               lea edx, [ebp - 0x310]
00458A95  52                              push edx
00458A96  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458A9B  e8 cc 4a fe ff                  call 0x43d56c
00458AA0  8b f0                           mov esi, eax
00458AA2  b9 06 00 00 00                  mov ecx, 6
00458AA7  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458AAD  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458AAF  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458AB3  8d 8d 20 fd ff ff               lea ecx, [ebp - 0x2e0]
00458AB9  e8 f2 b3 fa ff                  call 0x403eb0
00458ABE  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458AC4  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458ACA  dd 41 18                        fld qword ptr [ecx + 0x18]
00458ACD  dc 25 48 10 4b 00               fsub qword ptr [0x4b1048]
00458AD3  83 ec 08                        sub esp, 8
00458AD6  dd 1c 24                        fstp qword ptr [esp]
00458AD9  6a 00                           push 0
00458ADB  8d 4d a8                        lea ecx, [ebp - 0x58]
00458ADE  e8 5d 87 fc ff                  call 0x421240
00458AE3  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458AE6  dc 40 08                        fadd qword ptr [eax + 8]
00458AE9  83 ec 08                        sub esp, 8
00458AEC  dd 1c 24                        fstp qword ptr [esp]
00458AEF  6a 00                           push 0
00458AF1  8d 4d a8                        lea ecx, [ebp - 0x58]
00458AF4  e8 47 87 fc ff                  call 0x421240
00458AF9  8b 50 04                        mov edx, dword ptr [eax + 4]
00458AFC  52                              push edx
00458AFD  8b 00                           mov eax, dword ptr [eax]
00458AFF  50                              push eax
00458B00  8d 8d c0 fc ff ff               lea ecx, [ebp - 0x340]
00458B06  e8 e5 8c fa ff                  call 0x4017f0
00458B0B  50                              push eax
00458B0C  8d 8d d8 fc ff ff               lea ecx, [ebp - 0x328]
00458B12  e8 b9 db fa ff                  call 0x4066d0
00458B17  c6 45 fc 0a                     mov byte ptr [ebp - 4], 0xa
00458B1B  8d 8d d8 fc ff ff               lea ecx, [ebp - 0x328]
00458B21  51                              push ecx
00458B22  8d 95 a8 fc ff ff               lea edx, [ebp - 0x358]
00458B28  52                              push edx
00458B29  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458B2E  e8 39 4a fe ff                  call 0x43d56c
00458B33  8b f0                           mov esi, eax
00458B35  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458B3B  83 c7 18                        add edi, 0x18
00458B3E  b9 06 00 00 00                  mov ecx, 6
00458B43  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458B45  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458B49  8d 8d d8 fc ff ff               lea ecx, [ebp - 0x328]
00458B4F  e8 5c b3 fa ff                  call 0x403eb0
00458B54  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458B5A  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458B60  dd 41 18                        fld qword ptr [ecx + 0x18]
00458B63  dc 05 48 10 4b 00               fadd qword ptr [0x4b1048]
00458B69  83 ec 08                        sub esp, 8
00458B6C  dd 1c 24                        fstp qword ptr [esp]
00458B6F  6a 00                           push 0
00458B71  8d 4d a8                        lea ecx, [ebp - 0x58]
00458B74  e8 c7 86 fc ff                  call 0x421240
00458B79  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458B7C  dc 40 08                        fadd qword ptr [eax + 8]
00458B7F  83 ec 08                        sub esp, 8
00458B82  dd 1c 24                        fstp qword ptr [esp]
00458B85  6a 01                           push 1
00458B87  8d 4d a8                        lea ecx, [ebp - 0x58]
00458B8A  e8 b1 86 fc ff                  call 0x421240
00458B8F  8b 50 04                        mov edx, dword ptr [eax + 4]
00458B92  52                              push edx
00458B93  8b 00                           mov eax, dword ptr [eax]
00458B95  50                              push eax
00458B96  8d 8d 78 fc ff ff               lea ecx, [ebp - 0x388]
00458B9C  e8 4f 8c fa ff                  call 0x4017f0
00458BA1  50                              push eax
00458BA2  8d 8d 90 fc ff ff               lea ecx, [ebp - 0x370]
00458BA8  e8 23 db fa ff                  call 0x4066d0
00458BAD  c6 45 fc 0b                     mov byte ptr [ebp - 4], 0xb
00458BB1  8d 8d 90 fc ff ff               lea ecx, [ebp - 0x370]
00458BB7  51                              push ecx
00458BB8  8d 95 60 fc ff ff               lea edx, [ebp - 0x3a0]
00458BBE  52                              push edx
00458BBF  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458BC4  e8 a3 49 fe ff                  call 0x43d56c
00458BC9  8b f0                           mov esi, eax
00458BCB  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458BD1  83 c7 30                        add edi, 0x30
00458BD4  b9 06 00 00 00                  mov ecx, 6
00458BD9  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458BDB  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458BDF  8d 8d 90 fc ff ff               lea ecx, [ebp - 0x370]
00458BE5  e8 c6 b2 fa ff                  call 0x403eb0
00458BEA  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458BF0  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458BF6  dd 41 18                        fld qword ptr [ecx + 0x18]
00458BF9  dc 25 48 10 4b 00               fsub qword ptr [0x4b1048]
00458BFF  83 ec 08                        sub esp, 8
00458C02  dd 1c 24                        fstp qword ptr [esp]
00458C05  6a 00                           push 0
00458C07  8d 4d a8                        lea ecx, [ebp - 0x58]
00458C0A  e8 31 86 fc ff                  call 0x421240
00458C0F  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458C12  dc 40 08                        fadd qword ptr [eax + 8]
00458C15  83 ec 08                        sub esp, 8
00458C18  dd 1c 24                        fstp qword ptr [esp]
00458C1B  6a 01                           push 1
00458C1D  8d 4d a8                        lea ecx, [ebp - 0x58]
00458C20  e8 1b 86 fc ff                  call 0x421240
00458C25  8b 50 04                        mov edx, dword ptr [eax + 4]
00458C28  52                              push edx
00458C29  8b 00                           mov eax, dword ptr [eax]
00458C2B  50                              push eax
00458C2C  8d 8d 30 fc ff ff               lea ecx, [ebp - 0x3d0]
00458C32  e8 b9 8b fa ff                  call 0x4017f0
00458C37  50                              push eax
00458C38  8d 8d 48 fc ff ff               lea ecx, [ebp - 0x3b8]
00458C3E  e8 8d da fa ff                  call 0x4066d0
00458C43  c6 45 fc 0c                     mov byte ptr [ebp - 4], 0xc
00458C47  8d 8d 48 fc ff ff               lea ecx, [ebp - 0x3b8]
00458C4D  51                              push ecx
00458C4E  8d 95 18 fc ff ff               lea edx, [ebp - 0x3e8]
00458C54  52                              push edx
00458C55  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458C5A  e8 0d 49 fe ff                  call 0x43d56c
00458C5F  8b f0                           mov esi, eax
00458C61  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458C67  83 c7 48                        add edi, 0x48
00458C6A  b9 06 00 00 00                  mov ecx, 6
00458C6F  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458C71  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458C75  8d 8d 48 fc ff ff               lea ecx, [ebp - 0x3b8]
00458C7B  e8 30 b2 fa ff                  call 0x403eb0
00458C80  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458C86  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458C8C  dd 41 18                        fld qword ptr [ecx + 0x18]
00458C8F  dc 05 48 10 4b 00               fadd qword ptr [0x4b1048]
00458C95  83 ec 08                        sub esp, 8
00458C98  dd 1c 24                        fstp qword ptr [esp]
00458C9B  6a 01                           push 1
00458C9D  8d 4d a8                        lea ecx, [ebp - 0x58]
00458CA0  e8 9b 85 fc ff                  call 0x421240
00458CA5  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458CA8  dc 40 08                        fadd qword ptr [eax + 8]
00458CAB  83 ec 08                        sub esp, 8
00458CAE  dd 1c 24                        fstp qword ptr [esp]
00458CB1  6a 01                           push 1
00458CB3  8d 4d a8                        lea ecx, [ebp - 0x58]
00458CB6  e8 85 85 fc ff                  call 0x421240
00458CBB  8b 50 04                        mov edx, dword ptr [eax + 4]
00458CBE  52                              push edx
00458CBF  8b 00                           mov eax, dword ptr [eax]
00458CC1  50                              push eax
00458CC2  8d 8d e8 fb ff ff               lea ecx, [ebp - 0x418]
00458CC8  e8 23 8b fa ff                  call 0x4017f0
00458CCD  50                              push eax
00458CCE  8d 8d 00 fc ff ff               lea ecx, [ebp - 0x400]
00458CD4  e8 f7 d9 fa ff                  call 0x4066d0
00458CD9  c6 45 fc 0d                     mov byte ptr [ebp - 4], 0xd
00458CDD  8d 8d 00 fc ff ff               lea ecx, [ebp - 0x400]
00458CE3  51                              push ecx
00458CE4  8d 95 d0 fb ff ff               lea edx, [ebp - 0x430]
00458CEA  52                              push edx
00458CEB  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458CF0  e8 77 48 fe ff                  call 0x43d56c
00458CF5  8b f0                           mov esi, eax
00458CF7  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458CFD  83 c7 60                        add edi, 0x60
00458D00  b9 06 00 00 00                  mov ecx, 6
00458D05  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458D07  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458D0B  8d 8d 00 fc ff ff               lea ecx, [ebp - 0x400]
00458D11  e8 9a b1 fa ff                  call 0x403eb0
00458D16  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458D1C  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458D22  dd 41 18                        fld qword ptr [ecx + 0x18]
00458D25  dc 25 48 10 4b 00               fsub qword ptr [0x4b1048]
00458D2B  83 ec 08                        sub esp, 8
00458D2E  dd 1c 24                        fstp qword ptr [esp]
00458D31  6a 01                           push 1
00458D33  8d 4d a8                        lea ecx, [ebp - 0x58]
00458D36  e8 05 85 fc ff                  call 0x421240
00458D3B  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458D3E  dc 40 08                        fadd qword ptr [eax + 8]
00458D41  83 ec 08                        sub esp, 8
00458D44  dd 1c 24                        fstp qword ptr [esp]
00458D47  6a 01                           push 1
00458D49  8d 4d a8                        lea ecx, [ebp - 0x58]
00458D4C  e8 ef 84 fc ff                  call 0x421240
00458D51  8b 50 04                        mov edx, dword ptr [eax + 4]
00458D54  52                              push edx
00458D55  8b 00                           mov eax, dword ptr [eax]
00458D57  50                              push eax
00458D58  8d 8d a0 fb ff ff               lea ecx, [ebp - 0x460]
00458D5E  e8 8d 8a fa ff                  call 0x4017f0
00458D63  50                              push eax
00458D64  8d 8d b8 fb ff ff               lea ecx, [ebp - 0x448]
00458D6A  e8 61 d9 fa ff                  call 0x4066d0
00458D6F  c6 45 fc 0e                     mov byte ptr [ebp - 4], 0xe
00458D73  8d 8d b8 fb ff ff               lea ecx, [ebp - 0x448]
00458D79  51                              push ecx
00458D7A  8d 95 88 fb ff ff               lea edx, [ebp - 0x478]
00458D80  52                              push edx
00458D81  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458D86  e8 e1 47 fe ff                  call 0x43d56c
00458D8B  8b f0                           mov esi, eax
00458D8D  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458D93  83 c7 78                        add edi, 0x78
00458D96  b9 06 00 00 00                  mov ecx, 6
00458D9B  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458D9D  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458DA1  8d 8d b8 fb ff ff               lea ecx, [ebp - 0x448]
00458DA7  e8 04 b1 fa ff                  call 0x403eb0
00458DAC  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458DB2  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458DB8  dd 41 18                        fld qword ptr [ecx + 0x18]
00458DBB  dc 05 48 10 4b 00               fadd qword ptr [0x4b1048]
00458DC1  83 ec 08                        sub esp, 8
00458DC4  dd 1c 24                        fstp qword ptr [esp]
00458DC7  6a 02                           push 2
00458DC9  8d 4d a8                        lea ecx, [ebp - 0x58]
00458DCC  e8 6f 84 fc ff                  call 0x421240
00458DD1  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458DD4  dc 40 08                        fadd qword ptr [eax + 8]
00458DD7  83 ec 08                        sub esp, 8
00458DDA  dd 1c 24                        fstp qword ptr [esp]
00458DDD  6a 02                           push 2
00458DDF  8d 4d a8                        lea ecx, [ebp - 0x58]
00458DE2  e8 59 84 fc ff                  call 0x421240
00458DE7  8b 50 04                        mov edx, dword ptr [eax + 4]
00458DEA  52                              push edx
00458DEB  8b 00                           mov eax, dword ptr [eax]
00458DED  50                              push eax
00458DEE  8d 8d 58 fb ff ff               lea ecx, [ebp - 0x4a8]
00458DF4  e8 f7 89 fa ff                  call 0x4017f0
00458DF9  50                              push eax
00458DFA  8d 8d 70 fb ff ff               lea ecx, [ebp - 0x490]
00458E00  e8 cb d8 fa ff                  call 0x4066d0
00458E05  c6 45 fc 0f                     mov byte ptr [ebp - 4], 0xf
00458E09  8d 8d 70 fb ff ff               lea ecx, [ebp - 0x490]
00458E0F  51                              push ecx
00458E10  8d 95 40 fb ff ff               lea edx, [ebp - 0x4c0]
00458E16  52                              push edx
00458E17  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458E1C  e8 4b 47 fe ff                  call 0x43d56c
00458E21  8b f0                           mov esi, eax
00458E23  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458E29  81 c7 90 00 00 00               add edi, 0x90
00458E2F  b9 06 00 00 00                  mov ecx, 6
00458E34  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458E36  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458E3A  8d 8d 70 fb ff ff               lea ecx, [ebp - 0x490]
00458E40  e8 6b b0 fa ff                  call 0x403eb0
00458E45  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458E4B  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458E51  dd 41 18                        fld qword ptr [ecx + 0x18]
00458E54  dc 25 48 10 4b 00               fsub qword ptr [0x4b1048]
00458E5A  83 ec 08                        sub esp, 8
00458E5D  dd 1c 24                        fstp qword ptr [esp]
00458E60  6a 02                           push 2
00458E62  8d 4d a8                        lea ecx, [ebp - 0x58]
00458E65  e8 d6 83 fc ff                  call 0x421240
00458E6A  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458E6D  dc 40 08                        fadd qword ptr [eax + 8]
00458E70  83 ec 08                        sub esp, 8
00458E73  dd 1c 24                        fstp qword ptr [esp]
00458E76  6a 02                           push 2
00458E78  8d 4d a8                        lea ecx, [ebp - 0x58]
00458E7B  e8 c0 83 fc ff                  call 0x421240
00458E80  8b 50 04                        mov edx, dword ptr [eax + 4]
00458E83  52                              push edx
00458E84  8b 00                           mov eax, dword ptr [eax]
00458E86  50                              push eax
00458E87  8d 8d 10 fb ff ff               lea ecx, [ebp - 0x4f0]
00458E8D  e8 5e 89 fa ff                  call 0x4017f0
00458E92  50                              push eax
00458E93  8d 8d 28 fb ff ff               lea ecx, [ebp - 0x4d8]
00458E99  e8 32 d8 fa ff                  call 0x4066d0
00458E9E  c6 45 fc 10                     mov byte ptr [ebp - 4], 0x10
00458EA2  8d 8d 28 fb ff ff               lea ecx, [ebp - 0x4d8]
00458EA8  51                              push ecx
00458EA9  8d 95 f8 fa ff ff               lea edx, [ebp - 0x508]
00458EAF  52                              push edx
00458EB0  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458EB5  e8 b2 46 fe ff                  call 0x43d56c
00458EBA  8b f0                           mov esi, eax
00458EBC  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458EC2  81 c7 a8 00 00 00               add edi, 0xa8
00458EC8  b9 06 00 00 00                  mov ecx, 6
00458ECD  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458ECF  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458ED3  8d 8d 28 fb ff ff               lea ecx, [ebp - 0x4d8]
00458ED9  e8 d2 af fa ff                  call 0x403eb0
00458EDE  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458EE4  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458EEA  dd 41 18                        fld qword ptr [ecx + 0x18]
00458EED  dc 05 48 10 4b 00               fadd qword ptr [0x4b1048]
00458EF3  83 ec 08                        sub esp, 8
00458EF6  dd 1c 24                        fstp qword ptr [esp]
00458EF9  6a 03                           push 3
00458EFB  8d 4d a8                        lea ecx, [ebp - 0x58]
00458EFE  e8 3d 83 fc ff                  call 0x421240
00458F03  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458F06  dc 40 08                        fadd qword ptr [eax + 8]
00458F09  83 ec 08                        sub esp, 8
00458F0C  dd 1c 24                        fstp qword ptr [esp]
00458F0F  6a 02                           push 2
00458F11  8d 4d a8                        lea ecx, [ebp - 0x58]
00458F14  e8 27 83 fc ff                  call 0x421240
00458F19  8b 50 04                        mov edx, dword ptr [eax + 4]
00458F1C  52                              push edx
00458F1D  8b 00                           mov eax, dword ptr [eax]
00458F1F  50                              push eax
00458F20  8d 8d c8 fa ff ff               lea ecx, [ebp - 0x538]
00458F26  e8 c5 88 fa ff                  call 0x4017f0
00458F2B  50                              push eax
00458F2C  8d 8d e0 fa ff ff               lea ecx, [ebp - 0x520]
00458F32  e8 99 d7 fa ff                  call 0x4066d0
00458F37  c6 45 fc 11                     mov byte ptr [ebp - 4], 0x11
00458F3B  8d 8d e0 fa ff ff               lea ecx, [ebp - 0x520]
00458F41  51                              push ecx
00458F42  8d 95 b0 fa ff ff               lea edx, [ebp - 0x550]
00458F48  52                              push edx
00458F49  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458F4E  e8 19 46 fe ff                  call 0x43d56c
00458F53  8b f0                           mov esi, eax
00458F55  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458F5B  81 c7 c0 00 00 00               add edi, 0xc0
00458F61  b9 06 00 00 00                  mov ecx, 6
00458F66  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00458F68  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00458F6C  8d 8d e0 fa ff ff               lea ecx, [ebp - 0x520]
00458F72  e8 39 af fa ff                  call 0x403eb0
00458F77  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00458F7D  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
00458F83  dd 41 18                        fld qword ptr [ecx + 0x18]
00458F86  dc 25 48 10 4b 00               fsub qword ptr [0x4b1048]
00458F8C  83 ec 08                        sub esp, 8
00458F8F  dd 1c 24                        fstp qword ptr [esp]
00458F92  6a 03                           push 3
00458F94  8d 4d a8                        lea ecx, [ebp - 0x58]
00458F97  e8 a4 82 fc ff                  call 0x421240
00458F9C  dd 45 e8                        fld qword ptr [ebp - 0x18]
00458F9F  dc 40 08                        fadd qword ptr [eax + 8]
00458FA2  83 ec 08                        sub esp, 8
00458FA5  dd 1c 24                        fstp qword ptr [esp]
00458FA8  6a 02                           push 2
00458FAA  8d 4d a8                        lea ecx, [ebp - 0x58]
00458FAD  e8 8e 82 fc ff                  call 0x421240
00458FB2  8b 50 04                        mov edx, dword ptr [eax + 4]
00458FB5  52                              push edx
00458FB6  8b 00                           mov eax, dword ptr [eax]
00458FB8  50                              push eax
00458FB9  8d 8d 80 fa ff ff               lea ecx, [ebp - 0x580]
00458FBF  e8 2c 88 fa ff                  call 0x4017f0
00458FC4  50                              push eax
00458FC5  8d 8d 98 fa ff ff               lea ecx, [ebp - 0x568]
00458FCB  e8 00 d7 fa ff                  call 0x4066d0
00458FD0  c6 45 fc 12                     mov byte ptr [ebp - 4], 0x12
00458FD4  8d 8d 98 fa ff ff               lea ecx, [ebp - 0x568]
00458FDA  51                              push ecx
00458FDB  8d 95 68 fa ff ff               lea edx, [ebp - 0x598]
00458FE1  52                              push edx
00458FE2  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00458FE7  e8 80 45 fe ff                  call 0x43d56c
00458FEC  8b f0                           mov esi, eax
00458FEE  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00458FF4  81 c7 d8 00 00 00               add edi, 0xd8
00458FFA  b9 06 00 00 00                  mov ecx, 6
00458FFF  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00459001  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00459005  8d 8d 98 fa ff ff               lea ecx, [ebp - 0x568]
0045900B  e8 a0 ae fa ff                  call 0x403eb0
00459010  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
00459016  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
0045901C  dd 41 18                        fld qword ptr [ecx + 0x18]
0045901F  dc 05 48 10 4b 00               fadd qword ptr [0x4b1048]
00459025  83 ec 08                        sub esp, 8
00459028  dd 1c 24                        fstp qword ptr [esp]
0045902B  6a 03                           push 3
0045902D  8d 4d a8                        lea ecx, [ebp - 0x58]
00459030  e8 0b 82 fc ff                  call 0x421240
00459035  dd 45 e8                        fld qword ptr [ebp - 0x18]
00459038  dc 40 08                        fadd qword ptr [eax + 8]
0045903B  83 ec 08                        sub esp, 8
0045903E  dd 1c 24                        fstp qword ptr [esp]
00459041  6a 03                           push 3
00459043  8d 4d a8                        lea ecx, [ebp - 0x58]
00459046  e8 f5 81 fc ff                  call 0x421240
0045904B  8b 50 04                        mov edx, dword ptr [eax + 4]
0045904E  52                              push edx
0045904F  8b 00                           mov eax, dword ptr [eax]
00459051  50                              push eax
00459052  8d 8d 38 fa ff ff               lea ecx, [ebp - 0x5c8]
00459058  e8 93 87 fa ff                  call 0x4017f0
0045905D  50                              push eax
0045905E  8d 8d 50 fa ff ff               lea ecx, [ebp - 0x5b0]
00459064  e8 67 d6 fa ff                  call 0x4066d0
00459069  c6 45 fc 13                     mov byte ptr [ebp - 4], 0x13
0045906D  8d 8d 50 fa ff ff               lea ecx, [ebp - 0x5b0]
00459073  51                              push ecx
00459074  8d 95 20 fa ff ff               lea edx, [ebp - 0x5e0]
0045907A  52                              push edx
0045907B  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00459080  e8 e7 44 fe ff                  call 0x43d56c
00459085  8b f0                           mov esi, eax
00459087  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
0045908D  81 c7 f0 00 00 00               add edi, 0xf0
00459093  b9 06 00 00 00                  mov ecx, 6
00459098  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
0045909A  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
0045909E  8d 8d 50 fa ff ff               lea ecx, [ebp - 0x5b0]
004590A4  e8 07 ae fa ff                  call 0x403eb0
004590A9  8b 85 d4 f9 ff ff               mov eax, dword ptr [ebp - 0x62c]
004590AF  8b 88 a8 03 00 00               mov ecx, dword ptr [eax + 0x3a8]
004590B5  dd 41 18                        fld qword ptr [ecx + 0x18]
004590B8  dc 25 48 10 4b 00               fsub qword ptr [0x4b1048]
004590BE  83 ec 08                        sub esp, 8
004590C1  dd 1c 24                        fstp qword ptr [esp]
004590C4  6a 03                           push 3
004590C6  8d 4d a8                        lea ecx, [ebp - 0x58]
004590C9  e8 72 81 fc ff                  call 0x421240
004590CE  dd 45 e8                        fld qword ptr [ebp - 0x18]
004590D1  dc 40 08                        fadd qword ptr [eax + 8]
004590D4  83 ec 08                        sub esp, 8
004590D7  dd 1c 24                        fstp qword ptr [esp]
004590DA  6a 03                           push 3
004590DC  8d 4d a8                        lea ecx, [ebp - 0x58]
004590DF  e8 5c 81 fc ff                  call 0x421240
004590E4  8b 50 04                        mov edx, dword ptr [eax + 4]
004590E7  52                              push edx
004590E8  8b 00                           mov eax, dword ptr [eax]
004590EA  50                              push eax
004590EB  8d 8d f0 f9 ff ff               lea ecx, [ebp - 0x610]
004590F1  e8 fa 86 fa ff                  call 0x4017f0
004590F6  50                              push eax
004590F7  8d 8d 08 fa ff ff               lea ecx, [ebp - 0x5f8]
004590FD  e8 ce d5 fa ff                  call 0x4066d0
00459102  c6 45 fc 14                     mov byte ptr [ebp - 4], 0x14
00459106  8d 8d 08 fa ff ff               lea ecx, [ebp - 0x5f8]
0045910C  51                              push ecx
0045910D  8d 95 d8 f9 ff ff               lea edx, [ebp - 0x628]
00459113  52                              push edx
00459114  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00459119  e8 4e 44 fe ff                  call 0x43d56c
0045911E  8b f0                           mov esi, eax
00459120  8b bd d4 f9 ff ff               mov edi, dword ptr [ebp - 0x62c]
00459126  81 c7 08 01 00 00               add edi, 0x108
0045912C  b9 06 00 00 00                  mov ecx, 6
00459131  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00459133  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00459137  8d 8d 08 fa ff ff               lea ecx, [ebp - 0x5f8]
0045913D  e8 6e ad fa ff                  call 0x403eb0
00459142  c7 85 78 ff ff ff 01 00 00 00   mov dword ptr [ebp - 0x88], 1
0045914C  6a 00                           push 0
0045914E  6a 00                           push 0
00459150  8b 45 cc                        mov eax, dword ptr [ebp - 0x34]
00459153  50                              push eax
00459154  8b 4d c8                        mov ecx, dword ptr [ebp - 0x38]
00459157  51                              push ecx
00459158  6a 00                           push 0
0045915A  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00459160  81 c1 28 01 00 00               add ecx, 0x128
00459166  e8 f5 a3 fb ff                  call 0x413560
0045916B  68 00 00 f0 3f                  push 0x3ff00000
00459170  6a 00                           push 0
00459172  8b 55 cc                        mov edx, dword ptr [ebp - 0x34]
00459175  52                              push edx
00459176  8b 45 c8                        mov eax, dword ptr [ebp - 0x38]
00459179  50                              push eax
0045917A  6a 01                           push 1
0045917C  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00459182  81 c1 28 01 00 00               add ecx, 0x128
00459188  e8 d3 a3 fb ff                  call 0x413560
0045918D  68 00 00 f0 3f                  push 0x3ff00000
00459192  6a 00                           push 0
00459194  8b 4d d4                        mov ecx, dword ptr [ebp - 0x2c]
00459197  51                              push ecx
00459198  8b 55 d0                        mov edx, dword ptr [ebp - 0x30]
0045919B  52                              push edx
0045919C  6a 02                           push 2
0045919E  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004591A4  81 c1 28 01 00 00               add ecx, 0x128
004591AA  e8 b1 a3 fb ff                  call 0x413560
004591AF  6a 00                           push 0
004591B1  6a 00                           push 0
004591B3  8b 45 d4                        mov eax, dword ptr [ebp - 0x2c]
004591B6  50                              push eax
004591B7  8b 4d d0                        mov ecx, dword ptr [ebp - 0x30]
004591BA  51                              push ecx
004591BB  6a 03                           push 3
004591BD  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004591C3  81 c1 28 01 00 00               add ecx, 0x128
004591C9  e8 92 a3 fb ff                  call 0x413560
004591CE  6a 00                           push 0
004591D0  6a 00                           push 0
004591D2  db 85 78 ff ff ff               fild dword ptr [ebp - 0x88]
004591D8  dc 6d d0                        fsubr qword ptr [ebp - 0x30]
004591DB  83 ec 08                        sub esp, 8
004591DE  dd 1c 24                        fstp qword ptr [esp]
004591E1  6a 00                           push 0
004591E3  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004591E9  81 c1 a8 01 00 00               add ecx, 0x1a8
004591EF  e8 6c a3 fb ff                  call 0x413560
004591F4  68 00 00 f0 3f                  push 0x3ff00000
004591F9  6a 00                           push 0
004591FB  db 85 78 ff ff ff               fild dword ptr [ebp - 0x88]
00459201  dc 6d d0                        fsubr qword ptr [ebp - 0x30]
00459204  83 ec 08                        sub esp, 8
00459207  dd 1c 24                        fstp qword ptr [esp]
0045920A  6a 01                           push 1
0045920C  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00459212  81 c1 a8 01 00 00               add ecx, 0x1a8
00459218  e8 43 a3 fb ff                  call 0x413560
0045921D  68 00 00 f0 3f                  push 0x3ff00000
00459222  6a 00                           push 0
00459224  db 85 78 ff ff ff               fild dword ptr [ebp - 0x88]
0045922A  dc 45 d0                        fadd qword ptr [ebp - 0x30]
0045922D  83 ec 08                        sub esp, 8
00459230  dd 1c 24                        fstp qword ptr [esp]
00459233  6a 02                           push 2
00459235  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
0045923B  81 c1 a8 01 00 00               add ecx, 0x1a8
00459241  e8 1a a3 fb ff                  call 0x413560
00459246  6a 00                           push 0
00459248  6a 00                           push 0
0045924A  db 85 78 ff ff ff               fild dword ptr [ebp - 0x88]
00459250  dc 45 d0                        fadd qword ptr [ebp - 0x30]
00459253  83 ec 08                        sub esp, 8
00459256  dd 1c 24                        fstp qword ptr [esp]
00459259  6a 03                           push 3
0045925B  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00459261  81 c1 a8 01 00 00               add ecx, 0x1a8
00459267  e8 f4 a2 fb ff                  call 0x413560
0045926C  6a 00                           push 0
0045926E  6a 00                           push 0
00459270  8b 55 d4                        mov edx, dword ptr [ebp - 0x2c]
00459273  52                              push edx
00459274  8b 45 d0                        mov eax, dword ptr [ebp - 0x30]
00459277  50                              push eax
00459278  6a 00                           push 0
0045927A  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00459280  81 c1 28 02 00 00               add ecx, 0x228
00459286  e8 d5 a2 fb ff                  call 0x413560
0045928B  68 00 00 f0 3f                  push 0x3ff00000
00459290  6a 00                           push 0
00459292  8b 4d d4                        mov ecx, dword ptr [ebp - 0x2c]
00459295  51                              push ecx
00459296  8b 55 d0                        mov edx, dword ptr [ebp - 0x30]
00459299  52                              push edx
0045929A  6a 01                           push 1
0045929C  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004592A2  81 c1 28 02 00 00               add ecx, 0x228
004592A8  e8 b3 a2 fb ff                  call 0x413560
004592AD  68 00 00 f0 3f                  push 0x3ff00000
004592B2  6a 00                           push 0
004592B4  8b 45 dc                        mov eax, dword ptr [ebp - 0x24]
004592B7  50                              push eax
004592B8  8b 4d d8                        mov ecx, dword ptr [ebp - 0x28]
004592BB  51                              push ecx
004592BC  6a 02                           push 2
004592BE  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004592C4  81 c1 28 02 00 00               add ecx, 0x228
004592CA  e8 91 a2 fb ff                  call 0x413560
004592CF  6a 00                           push 0
004592D1  6a 00                           push 0
004592D3  8b 55 dc                        mov edx, dword ptr [ebp - 0x24]
004592D6  52                              push edx
004592D7  8b 45 d8                        mov eax, dword ptr [ebp - 0x28]
004592DA  50                              push eax
004592DB  6a 03                           push 3
004592DD  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004592E3  81 c1 28 02 00 00               add ecx, 0x228
004592E9  e8 72 a2 fb ff                  call 0x413560
004592EE  6a 00                           push 0
004592F0  6a 00                           push 0
004592F2  db 85 78 ff ff ff               fild dword ptr [ebp - 0x88]
004592F8  dc 6d d8                        fsubr qword ptr [ebp - 0x28]
004592FB  83 ec 08                        sub esp, 8
004592FE  dd 1c 24                        fstp qword ptr [esp]
00459301  6a 00                           push 0
00459303  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00459309  81 c1 a8 02 00 00               add ecx, 0x2a8
0045930F  e8 4c a2 fb ff                  call 0x413560
00459314  68 00 00 f0 3f                  push 0x3ff00000
00459319  6a 00                           push 0
0045931B  db 85 78 ff ff ff               fild dword ptr [ebp - 0x88]
00459321  dc 6d d8                        fsubr qword ptr [ebp - 0x28]
00459324  83 ec 08                        sub esp, 8
00459327  dd 1c 24                        fstp qword ptr [esp]
0045932A  6a 01                           push 1
0045932C  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00459332  81 c1 a8 02 00 00               add ecx, 0x2a8
00459338  e8 23 a2 fb ff                  call 0x413560
0045933D  68 00 00 f0 3f                  push 0x3ff00000
00459342  6a 00                           push 0
00459344  db 85 78 ff ff ff               fild dword ptr [ebp - 0x88]
0045934A  dc 45 d8                        fadd qword ptr [ebp - 0x28]
0045934D  83 ec 08                        sub esp, 8
00459350  dd 1c 24                        fstp qword ptr [esp]
00459353  6a 02                           push 2
00459355  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
0045935B  81 c1 a8 02 00 00               add ecx, 0x2a8
00459361  e8 fa a1 fb ff                  call 0x413560
00459366  6a 00                           push 0
00459368  6a 00                           push 0
0045936A  db 85 78 ff ff ff               fild dword ptr [ebp - 0x88]
00459370  dc 45 d8                        fadd qword ptr [ebp - 0x28]
00459373  83 ec 08                        sub esp, 8
00459376  dd 1c 24                        fstp qword ptr [esp]
00459379  6a 03                           push 3
0045937B  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00459381  81 c1 a8 02 00 00               add ecx, 0x2a8
00459387  e8 d4 a1 fb ff                  call 0x413560
0045938C  6a 00                           push 0
0045938E  6a 00                           push 0
00459390  8b 4d dc                        mov ecx, dword ptr [ebp - 0x24]
00459393  51                              push ecx
00459394  8b 55 d8                        mov edx, dword ptr [ebp - 0x28]
00459397  52                              push edx
00459398  6a 00                           push 0
0045939A  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004593A0  81 c1 28 03 00 00               add ecx, 0x328
004593A6  e8 b5 a1 fb ff                  call 0x413560
004593AB  68 00 00 f0 3f                  push 0x3ff00000
004593B0  6a 00                           push 0
004593B2  8b 45 dc                        mov eax, dword ptr [ebp - 0x24]
004593B5  50                              push eax
004593B6  8b 4d d8                        mov ecx, dword ptr [ebp - 0x28]
004593B9  51                              push ecx
004593BA  6a 01                           push 1
004593BC  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004593C2  81 c1 28 03 00 00               add ecx, 0x328
004593C8  e8 93 a1 fb ff                  call 0x413560
004593CD  68 00 00 f0 3f                  push 0x3ff00000
004593D2  6a 00                           push 0
004593D4  8b 55 e4                        mov edx, dword ptr [ebp - 0x1c]
004593D7  52                              push edx
004593D8  8b 45 e0                        mov eax, dword ptr [ebp - 0x20]
004593DB  50                              push eax
004593DC  6a 02                           push 2
004593DE  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
004593E4  81 c1 28 03 00 00               add ecx, 0x328
004593EA  e8 71 a1 fb ff                  call 0x413560
004593EF  6a 00                           push 0
004593F1  6a 00                           push 0
004593F3  8b 4d e4                        mov ecx, dword ptr [ebp - 0x1c]
004593F6  51                              push ecx
004593F7  8b 55 e0                        mov edx, dword ptr [ebp - 0x20]
004593FA  52                              push edx
004593FB  6a 03                           push 3
004593FD  8b 8d d4 f9 ff ff               mov ecx, dword ptr [ebp - 0x62c]
00459403  81 c1 28 03 00 00               add ecx, 0x328
00459409  e8 52 a1 fb ff                  call 0x413560
0045940E  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
00459415  8d 4d a8                        lea ecx, [ebp - 0x58]
00459418  e8 13 96 fc ff                  call 0x422a30
0045941D  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
00459420  64 89 0d 00 00 00 00            mov dword ptr fs:[0], ecx
00459427  5f                              pop edi
00459428  5e                              pop esi
00459429  8b e5                           mov esp, ebp
0045942B  5d                              pop ebp
0045942C  c3                              ret

; VA 0x00413560, file offset 0x13560, length 59, SHA-256 e10637d1d1606b2cd36380cca7f81c8917aa7893dc17ffcb46e440a2bc70f821
00413560  55                              push ebp
00413561  8b ec                           mov ebp, esp
00413563  51                              push ecx
00413564  89 4d fc                        mov dword ptr [ebp - 4], ecx
00413567  8b 45 08                        mov eax, dword ptr [ebp + 8]
0041356A  6b c0 18                        imul eax, eax, 0x18
0041356D  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
00413570  8b 55 0c                        mov edx, dword ptr [ebp + 0xc]
00413573  89 54 01 28                     mov dword ptr [ecx + eax + 0x28], edx
00413577  8b 55 10                        mov edx, dword ptr [ebp + 0x10]
0041357A  89 54 01 2c                     mov dword ptr [ecx + eax + 0x2c], edx
0041357E  8b 45 08                        mov eax, dword ptr [ebp + 8]
00413581  6b c0 18                        imul eax, eax, 0x18
00413584  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
00413587  8b 55 14                        mov edx, dword ptr [ebp + 0x14]
0041358A  89 54 01 30                     mov dword ptr [ecx + eax + 0x30], edx
0041358E  8b 55 18                        mov edx, dword ptr [ebp + 0x18]
00413591  89 54 01 34                     mov dword ptr [ecx + eax + 0x34], edx
00413595  8b e5                           mov esp, ebp
00413597  5d                              pop ebp
00413598  c2 14 00                        ret 0x14
