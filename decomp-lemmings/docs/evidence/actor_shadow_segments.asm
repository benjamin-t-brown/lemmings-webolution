; Original x86 instruction bytes; static disassembly, not runtime evidence.
; Reference SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Decoder: Capstone 5.0.7, x86 32-bit; addresses are virtual addresses.

; VA 0x00459557, file offset 0x59557, length 998, SHA-256 fb8c4b16cb86d0cf1f14a8b550791262138a1d7bfeffe951bd79389cf706cd27
00459557  55                              push ebp
00459558  8b ec                           mov ebp, esp
0045955A  6a ff                           push -1
0045955C  68 f5 d0 4a 00                  push 0x4ad0f5
00459561  64 a1 00 00 00 00               mov eax, dword ptr fs:[0]
00459567  50                              push eax
00459568  64 89 25 00 00 00 00            mov dword ptr fs:[0], esp
0045956F  81 ec b8 00 00 00               sub esp, 0xb8
00459575  56                              push esi
00459576  57                              push edi
00459577  89 8d 68 ff ff ff               mov dword ptr [ebp - 0x98], ecx
0045957D  c7 45 d4 00 00 00 00            mov dword ptr [ebp - 0x2c], 0
00459584  8d 4d b8                        lea ecx, [ebp - 0x48]
00459587  e8 14 d1 fa ff                  call 0x4066a0
0045958C  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
00459593  8b b5 68 ff ff ff               mov esi, dword ptr [ebp - 0x98]
00459599  81 c6 b0 03 00 00               add esi, 0x3b0
0045959F  b9 06 00 00 00                  mov ecx, 6
004595A4  8d 7d a0                        lea edi, [ebp - 0x60]
004595A7  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
004595A9  8b 85 68 ff ff ff               mov eax, dword ptr [ebp - 0x98]
004595AF  dd 80 b8 03 00 00               fld qword ptr [eax + 0x3b8]
004595B5  e8 f2 2f 04 00                  call 0x49c5ac
004595BA  89 85 64 ff ff ff               mov dword ptr [ebp - 0x9c], eax
004595C0  db 85 64 ff ff ff               fild dword ptr [ebp - 0x9c]
004595C6  83 ec 08                        sub esp, 8
004595C9  dd 1c 24                        fstp qword ptr [esp]
004595CC  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
004595D1  e8 7a 49 fb ff                  call 0x40df50
004595D6  e8 d1 2f 04 00                  call 0x49c5ac
004595DB  89 45 ec                        mov dword ptr [ebp - 0x14], eax
004595DE  dd 45 a0                        fld qword ptr [ebp - 0x60]
004595E1  e8 c6 2f 04 00                  call 0x49c5ac
004595E6  83 e8 01                        sub eax, 1
004595E9  89 45 d0                        mov dword ptr [ebp - 0x30], eax
004595EC  dd 45 a0                        fld qword ptr [ebp - 0x60]
004595EF  e8 b8 2f 04 00                  call 0x49c5ac
004595F4  83 c0 01                        add eax, 1
004595F7  89 45 f0                        mov dword ptr [ebp - 0x10], eax
004595FA  8b 4d 0c                        mov ecx, dword ptr [ebp + 0xc]
004595FD  89 4d e0                        mov dword ptr [ebp - 0x20], ecx
00459600  8b 55 10                        mov edx, dword ptr [ebp + 0x10]
00459603  89 55 e4                        mov dword ptr [ebp - 0x1c], edx
00459606  db 45 ec                        fild dword ptr [ebp - 0x14]
00459609  dd 5d d8                        fstp qword ptr [ebp - 0x28]
0045960C  8b 45 d0                        mov eax, dword ptr [ebp - 0x30]
0045960F  89 45 e8                        mov dword ptr [ebp - 0x18], eax
00459612  eb 09                           jmp 0x45961d
00459614  8b 4d e8                        mov ecx, dword ptr [ebp - 0x18]
00459617  83 c1 01                        add ecx, 1
0045961A  89 4d e8                        mov dword ptr [ebp - 0x18], ecx
0045961D  8b 55 e8                        mov edx, dword ptr [ebp - 0x18]
00459620  3b 55 f0                        cmp edx, dword ptr [ebp - 0x10]
00459623  0f 8f f3 02 00 00               jg 0x45991c
00459629  db 45 e8                        fild dword ptr [ebp - 0x18]
0045962C  83 ec 08                        sub esp, 8
0045962F  dd 1c 24                        fstp qword ptr [esp]
00459632  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00459637  e8 84 48 fb ff                  call 0x40dec0
0045963C  e8 6b 2f 04 00                  call 0x49c5ac
00459641  89 45 9c                        mov dword ptr [ebp - 0x64], eax
00459644  8b 45 ec                        mov eax, dword ptr [ebp - 0x14]
00459647  50                              push eax
00459648  8b 4d 9c                        mov ecx, dword ptr [ebp - 0x64]
0045964B  51                              push ecx
0045964C  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00459651  e8 0a 48 fb ff                  call 0x40de60
00459656  8b 10                           mov edx, dword ptr [eax]
00459658  83 e2 01                        and edx, 1
0045965B  85 d2                           test edx, edx
0045965D  0f 84 b1 00 00 00               je 0x459714
00459663  8b 45 ec                        mov eax, dword ptr [ebp - 0x14]
00459666  83 c0 01                        add eax, 1
00459669  89 85 60 ff ff ff               mov dword ptr [ebp - 0xa0], eax
0045966F  db 85 60 ff ff ff               fild dword ptr [ebp - 0xa0]
00459675  83 ec 08                        sub esp, 8
00459678  dd 1c 24                        fstp qword ptr [esp]
0045967B  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00459680  e8 cb 48 fb ff                  call 0x40df50
00459685  e8 22 2f 04 00                  call 0x49c5ac
0045968A  50                              push eax
0045968B  8b 4d 9c                        mov ecx, dword ptr [ebp - 0x64]
0045968E  51                              push ecx
0045968F  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00459694  e8 c7 47 fb ff                  call 0x40de60
00459699  8b 10                           mov edx, dword ptr [eax]
0045969B  83 e2 01                        and edx, 1
0045969E  85 d2                           test edx, edx
004596A0  74 1c                           je 0x4596be
004596A2  8b 45 9c                        mov eax, dword ptr [ebp - 0x64]
004596A5  3b 45 f0                        cmp eax, dword ptr [ebp - 0x10]
004596A8  75 12                           jne 0x4596bc
004596AA  8b 4d 14                        mov ecx, dword ptr [ebp + 0x14]
004596AD  89 4d e0                        mov dword ptr [ebp - 0x20], ecx
004596B0  8b 55 18                        mov edx, dword ptr [ebp + 0x18]
004596B3  89 55 e4                        mov dword ptr [ebp - 0x1c], edx
004596B6  db 45 ec                        fild dword ptr [ebp - 0x14]
004596B9  dd 5d d8                        fstp qword ptr [ebp - 0x28]
004596BC  eb 51                           jmp 0x45970f
004596BE  db 45 9c                        fild dword ptr [ebp - 0x64]
004596C1  dc 5d a0                        fcomp qword ptr [ebp - 0x60]
004596C4  df e0                           fnstsw ax
004596C6  f6 c4 01                        test ah, 1
004596C9  74 23                           je 0x4596ee
004596CB  8b 45 0c                        mov eax, dword ptr [ebp + 0xc]
004596CE  89 45 e0                        mov dword ptr [ebp - 0x20], eax
004596D1  8b 4d 10                        mov ecx, dword ptr [ebp + 0x10]
004596D4  89 4d e4                        mov dword ptr [ebp - 0x1c], ecx
004596D7  8b 55 ec                        mov edx, dword ptr [ebp - 0x14]
004596DA  83 c2 01                        add edx, 1
004596DD  89 95 5c ff ff ff               mov dword ptr [ebp - 0xa4], edx
004596E3  db 85 5c ff ff ff               fild dword ptr [ebp - 0xa4]
004596E9  dd 5d d8                        fstp qword ptr [ebp - 0x28]
004596EC  eb 21                           jmp 0x45970f
004596EE  8b 45 14                        mov eax, dword ptr [ebp + 0x14]
004596F1  89 45 e0                        mov dword ptr [ebp - 0x20], eax
004596F4  8b 4d 18                        mov ecx, dword ptr [ebp + 0x18]
004596F7  89 4d e4                        mov dword ptr [ebp - 0x1c], ecx
004596FA  8b 55 ec                        mov edx, dword ptr [ebp - 0x14]
004596FD  83 c2 01                        add edx, 1
00459700  89 95 58 ff ff ff               mov dword ptr [ebp - 0xa8], edx
00459706  db 85 58 ff ff ff               fild dword ptr [ebp - 0xa8]
0045970C  dd 5d d8                        fstp qword ptr [ebp - 0x28]
0045970F  e9 37 01 00 00                  jmp 0x45984b
00459714  8b 45 ec                        mov eax, dword ptr [ebp - 0x14]
00459717  83 e8 01                        sub eax, 1
0045971A  89 85 54 ff ff ff               mov dword ptr [ebp - 0xac], eax
00459720  db 85 54 ff ff ff               fild dword ptr [ebp - 0xac]
00459726  83 ec 08                        sub esp, 8
00459729  dd 1c 24                        fstp qword ptr [esp]
0045972C  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00459731  e8 1a 48 fb ff                  call 0x40df50
00459736  e8 71 2e 04 00                  call 0x49c5ac
0045973B  50                              push eax
0045973C  8b 4d 9c                        mov ecx, dword ptr [ebp - 0x64]
0045973F  51                              push ecx
00459740  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00459745  e8 16 47 fb ff                  call 0x40de60
0045974A  83 38 00                        cmp dword ptr [eax], 0
0045974D  0f 85 c0 00 00 00               jne 0x459813
00459753  8b 55 ec                        mov edx, dword ptr [ebp - 0x14]
00459756  83 ea 02                        sub edx, 2
00459759  89 95 50 ff ff ff               mov dword ptr [ebp - 0xb0], edx
0045975F  db 85 50 ff ff ff               fild dword ptr [ebp - 0xb0]
00459765  83 ec 08                        sub esp, 8
00459768  dd 1c 24                        fstp qword ptr [esp]
0045976B  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00459770  e8 db 47 fb ff                  call 0x40df50
00459775  e8 32 2e 04 00                  call 0x49c5ac
0045977A  50                              push eax
0045977B  8b 45 9c                        mov eax, dword ptr [ebp - 0x64]
0045977E  50                              push eax
0045977F  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00459784  e8 d7 46 fb ff                  call 0x40de60
00459789  83 38 00                        cmp dword ptr [eax], 0
0045978C  75 32                           jne 0x4597c0
0045978E  8b 4d 9c                        mov ecx, dword ptr [ebp - 0x64]
00459791  83 c1 01                        add ecx, 1
00459794  89 8d 4c ff ff ff               mov dword ptr [ebp - 0xb4], ecx
0045979A  db 85 4c ff ff ff               fild dword ptr [ebp - 0xb4]
004597A0  dd 5d e0                        fstp qword ptr [ebp - 0x20]
004597A3  db 45 9c                        fild dword ptr [ebp - 0x64]
004597A6  dc 5d a0                        fcomp qword ptr [ebp - 0x60]
004597A9  df e0                           fnstsw ax
004597AB  f6 c4 01                        test ah, 1
004597AE  74 08                           je 0x4597b8
004597B0  db 45 ec                        fild dword ptr [ebp - 0x14]
004597B3  dd 5d d8                        fstp qword ptr [ebp - 0x28]
004597B6  eb 06                           jmp 0x4597be
004597B8  db 45 ec                        fild dword ptr [ebp - 0x14]
004597BB  dd 5d d8                        fstp qword ptr [ebp - 0x28]
004597BE  eb 51                           jmp 0x459811
004597C0  db 45 9c                        fild dword ptr [ebp - 0x64]
004597C3  dc 5d a0                        fcomp qword ptr [ebp - 0x60]
004597C6  df e0                           fnstsw ax
004597C8  f6 c4 01                        test ah, 1
004597CB  74 23                           je 0x4597f0
004597CD  8b 55 0c                        mov edx, dword ptr [ebp + 0xc]
004597D0  89 55 e0                        mov dword ptr [ebp - 0x20], edx
004597D3  8b 45 10                        mov eax, dword ptr [ebp + 0x10]
004597D6  89 45 e4                        mov dword ptr [ebp - 0x1c], eax
004597D9  8b 4d ec                        mov ecx, dword ptr [ebp - 0x14]
004597DC  83 e9 01                        sub ecx, 1
004597DF  89 8d 48 ff ff ff               mov dword ptr [ebp - 0xb8], ecx
004597E5  db 85 48 ff ff ff               fild dword ptr [ebp - 0xb8]
004597EB  dd 5d d8                        fstp qword ptr [ebp - 0x28]
004597EE  eb 21                           jmp 0x459811
004597F0  8b 55 14                        mov edx, dword ptr [ebp + 0x14]
004597F3  89 55 e0                        mov dword ptr [ebp - 0x20], edx
004597F6  8b 45 18                        mov eax, dword ptr [ebp + 0x18]
004597F9  89 45 e4                        mov dword ptr [ebp - 0x1c], eax
004597FC  8b 4d ec                        mov ecx, dword ptr [ebp - 0x14]
004597FF  83 e9 01                        sub ecx, 1
00459802  89 8d 44 ff ff ff               mov dword ptr [ebp - 0xbc], ecx
00459808  db 85 44 ff ff ff               fild dword ptr [ebp - 0xbc]
0045980E  dd 5d d8                        fstp qword ptr [ebp - 0x28]
00459811  eb 38                           jmp 0x45984b
00459813  db 45 ec                        fild dword ptr [ebp - 0x14]
00459816  dd 5d d8                        fstp qword ptr [ebp - 0x28]
00459819  8b 55 9c                        mov edx, dword ptr [ebp - 0x64]
0045981C  3b 55 d0                        cmp edx, dword ptr [ebp - 0x30]
0045981F  75 0e                           jne 0x45982f
00459821  8b 45 0c                        mov eax, dword ptr [ebp + 0xc]
00459824  89 45 e0                        mov dword ptr [ebp - 0x20], eax
00459827  8b 4d 10                        mov ecx, dword ptr [ebp + 0x10]
0045982A  89 4d e4                        mov dword ptr [ebp - 0x1c], ecx
0045982D  eb 1c                           jmp 0x45984b
0045982F  8b 55 9c                        mov edx, dword ptr [ebp - 0x64]
00459832  3b 55 f0                        cmp edx, dword ptr [ebp - 0x10]
00459835  75 0e                           jne 0x459845
00459837  8b 45 14                        mov eax, dword ptr [ebp + 0x14]
0045983A  89 45 e0                        mov dword ptr [ebp - 0x20], eax
0045983D  8b 4d 18                        mov ecx, dword ptr [ebp + 0x18]
00459840  89 4d e4                        mov dword ptr [ebp - 0x1c], ecx
00459843  eb 06                           jmp 0x45984b
00459845  db 45 9c                        fild dword ptr [ebp - 0x64]
00459848  dd 5d e0                        fstp qword ptr [ebp - 0x20]
0045984B  8b 55 d4                        mov edx, dword ptr [ebp - 0x2c]
0045984E  83 c2 01                        add edx, 1
00459851  52                              push edx
00459852  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00459855  e8 06 7a fc ff                  call 0x421260
0045985A  6a 00                           push 0
0045985C  6a 00                           push 0
0045985E  8b 45 dc                        mov eax, dword ptr [ebp - 0x24]
00459861  50                              push eax
00459862  8b 4d d8                        mov ecx, dword ptr [ebp - 0x28]
00459865  51                              push ecx
00459866  8b 55 e4                        mov edx, dword ptr [ebp - 0x1c]
00459869  52                              push edx
0045986A  8b 45 e0                        mov eax, dword ptr [ebp - 0x20]
0045986D  50                              push eax
0045986E  8d 4d 84                        lea ecx, [ebp - 0x7c]
00459871  e8 7a 7f fa ff                  call 0x4017f0
00459876  50                              push eax
00459877  8b 4d d4                        mov ecx, dword ptr [ebp - 0x2c]
0045987A  89 8d 40 ff ff ff               mov dword ptr [ebp - 0xc0], ecx
00459880  8b 95 40 ff ff ff               mov edx, dword ptr [ebp - 0xc0]
00459886  52                              push edx
00459887  8b 45 d4                        mov eax, dword ptr [ebp - 0x2c]
0045988A  83 c0 01                        add eax, 1
0045988D  89 45 d4                        mov dword ptr [ebp - 0x2c], eax
00459890  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00459893  e8 a8 79 fc ff                  call 0x421240
00459898  8b c8                           mov ecx, eax
0045989A  e8 61 ce fa ff                  call 0x406700
0045989F  dd 45 a0                        fld qword ptr [ebp - 0x60]
004598A2  e8 05 2d 04 00                  call 0x49c5ac
004598A7  39 45 9c                        cmp dword ptr [ebp - 0x64], eax
004598AA  75 6b                           jne 0x459917
004598AC  db 45 f0                        fild dword ptr [ebp - 0x10]
004598AF  dc 5d 14                        fcomp qword ptr [ebp + 0x14]
004598B2  df e0                           fnstsw ax
004598B4  f6 c4 01                        test ah, 1
004598B7  74 5e                           je 0x459917
004598B9  8b 4d d4                        mov ecx, dword ptr [ebp - 0x2c]
004598BC  83 c1 01                        add ecx, 1
004598BF  51                              push ecx
004598C0  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
004598C3  e8 98 79 fc ff                  call 0x421260
004598C8  6a 00                           push 0
004598CA  6a 00                           push 0
004598CC  8b 55 dc                        mov edx, dword ptr [ebp - 0x24]
004598CF  52                              push edx
004598D0  8b 45 d8                        mov eax, dword ptr [ebp - 0x28]
004598D3  50                              push eax
004598D4  dd 45 e0                        fld qword ptr [ebp - 0x20]
004598D7  dc 05 d0 02 4b 00               fadd qword ptr [0x4b02d0]
004598DD  83 ec 08                        sub esp, 8
004598E0  dd 1c 24                        fstp qword ptr [esp]
004598E3  8d 8d 6c ff ff ff               lea ecx, [ebp - 0x94]
004598E9  e8 02 7f fa ff                  call 0x4017f0
004598EE  50                              push eax
004598EF  8b 4d d4                        mov ecx, dword ptr [ebp - 0x2c]
004598F2  89 8d 3c ff ff ff               mov dword ptr [ebp - 0xc4], ecx
004598F8  8b 95 3c ff ff ff               mov edx, dword ptr [ebp - 0xc4]
004598FE  52                              push edx
004598FF  8b 45 d4                        mov eax, dword ptr [ebp - 0x2c]
00459902  83 c0 01                        add eax, 1
00459905  89 45 d4                        mov dword ptr [ebp - 0x2c], eax
00459908  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0045990B  e8 30 79 fc ff                  call 0x421240
00459910  8b c8                           mov ecx, eax
00459912  e8 e9 cd fa ff                  call 0x406700
00459917  e9 f8 fc ff ff                  jmp 0x459614
0045991C  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
00459923  8d 4d b8                        lea ecx, [ebp - 0x48]
00459926  e8 85 a5 fa ff                  call 0x403eb0
0045992B  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0045992E  64 89 0d 00 00 00 00            mov dword ptr fs:[0], ecx
00459935  5f                              pop edi
00459936  5e                              pop esi
00459937  8b e5                           mov esp, ebp
00459939  5d                              pop ebp
0045993A  c2 14 00                        ret 0x14
