; Original x86 instruction bytes; static disassembly, not runtime evidence.
; Reference SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Decoder: Capstone 5.0.7, x86 32-bit; addresses are virtual addresses.

; VA 0x00413750, file offset 0x13750, length 55, SHA-256 35c7883489ced993577b3a1b5458ce6a5e8002e955fe538d17cf09c190b53f3b
00413750  55                              push ebp
00413751  8b ec                           mov ebp, esp
00413753  51                              push ecx
00413754  89 4d fc                        mov dword ptr [ebp - 4], ecx
00413757  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
0041375A  e8 31 4c 00 00                  call 0x418390
0041375F  8b 45 fc                        mov eax, dword ptr [ebp - 4]
00413762  c7 00 60 09 4b 00               mov dword ptr [eax], 0x4b0960
00413768  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
0041376B  c7 41 14 00 00 00 00            mov dword ptr [ecx + 0x14], 0
00413772  8b 55 fc                        mov edx, dword ptr [ebp - 4]
00413775  c6 42 11 01                     mov byte ptr [edx + 0x11], 1
00413779  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0041377C  c6 40 10 01                     mov byte ptr [eax + 0x10], 1
00413780  8b 45 fc                        mov eax, dword ptr [ebp - 4]
00413783  8b e5                           mov esp, ebp
00413785  5d                              pop ebp
00413786  c3                              ret

; VA 0x004137A3, file offset 0x137A3, length 26, SHA-256 762efafb3ef0779159d4b79982c59fbc9025ee40f096341e11f8a2083ba3609c
004137A3  55                              push ebp
004137A4  8b ec                           mov ebp, esp
004137A6  51                              push ecx
004137A7  89 4d fc                        mov dword ptr [ebp - 4], ecx
004137AA  68 d4 f2 4b 00                  push 0x4bf2d4
004137AF  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
004137B2  e8 6e c5 01 00                  call 0x42fd25
004137B7  8b e5                           mov esp, ebp
004137B9  5d                              pop ebp
004137BA  c2 04 00                        ret 4

; VA 0x004137CA, file offset 0x137CA, length 346, SHA-256 0f6b9ef825ea04454fd9fc8e5d46dbac3001069b7c2f67a0bf6a8a51ff25f8d6
004137CA  55                              push ebp
004137CB  8b ec                           mov ebp, esp
004137CD  83 ec 1c                        sub esp, 0x1c
004137D0  89 4d e4                        mov dword ptr [ebp - 0x1c], ecx
004137D3  8b 45 08                        mov eax, dword ptr [ebp + 8]
004137D6  83 78 50 03                     cmp dword ptr [eax + 0x50], 3
004137DA  75 10                           jne 0x4137ec
004137DC  8b 4d 0c                        mov ecx, dword ptr [ebp + 0xc]
004137DF  dd 01                           fld qword ptr [ecx]
004137E1  dc 35 f8 03 4b 00               fdiv qword ptr [0x4b03f8]
004137E7  8b 55 0c                        mov edx, dword ptr [ebp + 0xc]
004137EA  dd 1a                           fstp qword ptr [edx]
004137EC  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
004137F3  8b 45 0c                        mov eax, dword ptr [ebp + 0xc]
004137F6  dd 00                           fld qword ptr [eax]
004137F8  dc 1d 38 03 4b 00               fcomp qword ptr [0x4b0338]
004137FE  df e0                           fnstsw ax
00413800  f6 c4 41                        test ah, 0x41
00413803  0f 85 fc 00 00 00               jne 0x413905
00413809  83 7d fc 00                     cmp dword ptr [ebp - 4], 0
0041380D  0f 85 f2 00 00 00               jne 0x413905
00413813  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00413816  51                              push ecx
00413817  8b 4d e4                        mov ecx, dword ptr [ebp - 0x1c]
0041381A  e8 05 01 00 00                  call 0x413924
0041381F  dd 5d f0                        fstp qword ptr [ebp - 0x10]
00413822  c7 45 e8 00 00 00 00            mov dword ptr [ebp - 0x18], 0
00413829  c7 45 ec 00 00 00 00            mov dword ptr [ebp - 0x14], 0
00413830  c6 45 f8 00                     mov byte ptr [ebp - 8], 0
00413834  8b 55 0c                        mov edx, dword ptr [ebp + 0xc]
00413837  dd 45 f0                        fld qword ptr [ebp - 0x10]
0041383A  dc 1a                           fcomp qword ptr [edx]
0041383C  df e0                           fnstsw ax
0041383E  f6 c4 41                        test ah, 0x41
00413841  74 12                           je 0x413855
00413843  c6 45 f8 01                     mov byte ptr [ebp - 8], 1
00413847  8b 45 f0                        mov eax, dword ptr [ebp - 0x10]
0041384A  89 45 e8                        mov dword ptr [ebp - 0x18], eax
0041384D  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
00413850  89 4d ec                        mov dword ptr [ebp - 0x14], ecx
00413853  eb 0e                           jmp 0x413863
00413855  8b 55 0c                        mov edx, dword ptr [ebp + 0xc]
00413858  8b 02                           mov eax, dword ptr [edx]
0041385A  89 45 e8                        mov dword ptr [ebp - 0x18], eax
0041385D  8b 4a 04                        mov ecx, dword ptr [edx + 4]
00413860  89 4d ec                        mov dword ptr [ebp - 0x14], ecx
00413863  8b 55 08                        mov edx, dword ptr [ebp + 8]
00413866  8b 45 08                        mov eax, dword ptr [ebp + 8]
00413869  db 40 44                        fild dword ptr [eax + 0x44]
0041386C  dc 0d b0 10 4b 00               fmul qword ptr [0x4b10b0]
00413872  dc 4d e8                        fmul qword ptr [ebp - 0x18]
00413875  dc 42 10                        fadd qword ptr [edx + 0x10]
00413878  83 ec 08                        sub esp, 8
0041387B  dd 1c 24                        fstp qword ptr [esp]
0041387E  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00413883  e8 c8 a6 ff ff                  call 0x40df50
00413888  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0041388B  dd 59 10                        fstp qword ptr [ecx + 0x10]
0041388E  8b 55 ec                        mov edx, dword ptr [ebp - 0x14]
00413891  52                              push edx
00413892  8b 45 e8                        mov eax, dword ptr [ebp - 0x18]
00413895  50                              push eax
00413896  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00413899  83 c1 78                        add ecx, 0x78
0041389C  8b 55 08                        mov edx, dword ptr [ebp + 8]
0041389F  8b 42 78                        mov eax, dword ptr [edx + 0x78]
004138A2  ff 50 2c                        call dword ptr [eax + 0x2c]
004138A5  8b 4d 0c                        mov ecx, dword ptr [ebp + 0xc]
004138A8  dd 01                           fld qword ptr [ecx]
004138AA  dc 65 e8                        fsub qword ptr [ebp - 0x18]
004138AD  8b 55 0c                        mov edx, dword ptr [ebp + 0xc]
004138B0  dd 1a                           fstp qword ptr [edx]
004138B2  8b 45 f8                        mov eax, dword ptr [ebp - 8]
004138B5  25 ff 00 00 00                  and eax, 0xff
004138BA  85 c0                           test eax, eax
004138BC  74 3b                           je 0x4138f9
004138BE  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
004138C1  8b 51 14                        mov edx, dword ptr [ecx + 0x14]
004138C4  52                              push edx
004138C5  8b 41 10                        mov eax, dword ptr [ecx + 0x10]
004138C8  50                              push eax
004138C9  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
004138CC  8b 51 0c                        mov edx, dword ptr [ecx + 0xc]
004138CF  52                              push edx
004138D0  8b 41 08                        mov eax, dword ptr [ecx + 8]
004138D3  50                              push eax
004138D4  68 dc f2 4b 00                  push 0x4bf2dc
004138D9  e8 82 bf ff ff                  call 0x40f860
004138DE  83 c4 14                        add esp, 0x14
004138E1  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
004138E4  51                              push ecx
004138E5  8b 4d e4                        mov ecx, dword ptr [ebp - 0x1c]
004138E8  e8 d2 00 00 00                  call 0x4139bf
004138ED  89 45 fc                        mov dword ptr [ebp - 4], eax
004138F0  8b 55 e4                        mov edx, dword ptr [ebp - 0x1c]
004138F3  c6 42 18 01                     mov byte ptr [edx + 0x18], 1
004138F7  eb 07                           jmp 0x413900
004138F9  8b 45 e4                        mov eax, dword ptr [ebp - 0x1c]
004138FC  c6 40 18 00                     mov byte ptr [eax + 0x18], 0
00413900  e9 ee fe ff ff                  jmp 0x4137f3
00413905  83 7d fc 00                     cmp dword ptr [ebp - 4], 0
00413909  75 10                           jne 0x41391b
0041390B  8b 4d 0c                        mov ecx, dword ptr [ebp + 0xc]
0041390E  c7 01 00 00 00 00               mov dword ptr [ecx], 0
00413914  c7 41 04 00 00 00 00            mov dword ptr [ecx + 4], 0
0041391B  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0041391E  8b e5                           mov esp, ebp
00413920  5d                              pop ebp
00413921  c2 08 00                        ret 8

; VA 0x00413924, file offset 0x13924, length 155, SHA-256 d98558c3558d0206d7e4876b80c309bb202a10d1180e62bcd2c468af4cfbe984
00413924  55                              push ebp
00413925  8b ec                           mov ebp, esp
00413927  83 ec 14                        sub esp, 0x14
0041392A  56                              push esi
0041392B  89 4d f4                        mov dword ptr [ebp - 0xc], ecx
0041392E  c7 45 f8 00 00 00 00            mov dword ptr [ebp - 8], 0
00413935  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
0041393C  8b 45 08                        mov eax, dword ptr [ebp + 8]
0041393F  83 78 44 01                     cmp dword ptr [eax + 0x44], 1
00413943  75 25                           jne 0x41396a
00413945  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00413948  dd 41 10                        fld qword ptr [ecx + 0x10]
0041394B  e8 5c 8c 08 00                  call 0x49c5ac
00413950  83 c0 01                        add eax, 1
00413953  89 45 f0                        mov dword ptr [ebp - 0x10], eax
00413956  db 45 f0                        fild dword ptr [ebp - 0x10]
00413959  8b 55 08                        mov edx, dword ptr [ebp + 8]
0041395C  dc 62 10                        fsub qword ptr [edx + 0x10]
0041395F  dc 35 b0 10 4b 00               fdiv qword ptr [0x4b10b0]
00413965  dd 5d f8                        fstp qword ptr [ebp - 8]
00413968  eb 20                           jmp 0x41398a
0041396A  8b 75 08                        mov esi, dword ptr [ebp + 8]
0041396D  8b 45 08                        mov eax, dword ptr [ebp + 8]
00413970  dd 40 10                        fld qword ptr [eax + 0x10]
00413973  e8 34 8c 08 00                  call 0x49c5ac
00413978  89 45 ec                        mov dword ptr [ebp - 0x14], eax
0041397B  db 45 ec                        fild dword ptr [ebp - 0x14]
0041397E  dc 6e 10                        fsubr qword ptr [esi + 0x10]
00413981  dc 35 b0 10 4b 00               fdiv qword ptr [0x4b10b0]
00413987  dd 5d f8                        fstp qword ptr [ebp - 8]
0041398A  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0041398D  33 d2                           xor edx, edx
0041398F  8a 51 18                        mov dl, byte ptr [ecx + 0x18]
00413992  85 d2                           test edx, edx
00413994  74 1f                           je 0x4139b5
00413996  dd 45 f8                        fld qword ptr [ebp - 8]
00413999  dc 1d 38 03 4b 00               fcomp qword ptr [0x4b0338]
0041399F  df e0                           fnstsw ax
004139A1  f6 c4 40                        test ah, 0x40
004139A4  74 0f                           je 0x4139b5
004139A6  dd 05 d0 02 4b 00               fld qword ptr [0x4b02d0]
004139AC  dc 35 b0 10 4b 00               fdiv qword ptr [0x4b10b0]
004139B2  dd 5d f8                        fstp qword ptr [ebp - 8]
004139B5  dd 45 f8                        fld qword ptr [ebp - 8]
004139B8  5e                              pop esi
004139B9  8b e5                           mov esp, ebp
004139BB  5d                              pop ebp
004139BC  c2 04 00                        ret 4

; VA 0x004139BF, file offset 0x139BF, length 615, SHA-256 7c9f158b967a796755cb2c142ea292c794b3a839746e9c254464dc4c189d3069
004139BF  55                              push ebp
004139C0  8b ec                           mov ebp, esp
004139C2  6a ff                           push -1
004139C4  68 4f 9e 4a 00                  push 0x4a9e4f
004139C9  64 a1 00 00 00 00               mov eax, dword ptr fs:[0]
004139CF  50                              push eax
004139D0  64 89 25 00 00 00 00            mov dword ptr fs:[0], esp
004139D7  83 ec 54                        sub esp, 0x54
004139DA  89 4d ac                        mov dword ptr [ebp - 0x54], ecx
004139DD  8d 45 dc                        lea eax, [ebp - 0x24]
004139E0  50                              push eax
004139E1  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
004139E4  e8 76 b9 01 00                  call 0x42f35f
004139E9  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
004139F0  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
004139F3  db 41 40                        fild dword ptr [ecx + 0x40]
004139F6  dc 6d dc                        fsubr qword ptr [ebp - 0x24]
004139F9  83 ec 08                        sub esp, 8
004139FC  dd 1c 24                        fstp qword ptr [esp]
004139FF  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00413A04  e8 b7 a4 ff ff                  call 0x40dec0
00413A09  e8 9e 8b 08 00                  call 0x49c5ac
00413A0E  89 45 d8                        mov dword ptr [ebp - 0x28], eax
00413A11  dd 45 e4                        fld qword ptr [ebp - 0x1c]
00413A14  e8 93 8b 08 00                  call 0x49c5ac
00413A19  89 45 d4                        mov dword ptr [ebp - 0x2c], eax
00413A1C  db 45 d4                        fild dword ptr [ebp - 0x2c]
00413A1F  83 ec 08                        sub esp, 8
00413A22  dd 1c 24                        fstp qword ptr [esp]
00413A25  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00413A2A  e8 21 a5 ff ff                  call 0x40df50
00413A2F  e8 78 8b 08 00                  call 0x49c5ac
00413A34  50                              push eax
00413A35  db 45 d8                        fild dword ptr [ebp - 0x28]
00413A38  83 ec 08                        sub esp, 8
00413A3B  dd 1c 24                        fstp qword ptr [esp]
00413A3E  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00413A43  e8 78 a4 ff ff                  call 0x40dec0
00413A48  e8 5f 8b 08 00                  call 0x49c5ac
00413A4D  50                              push eax
00413A4E  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00413A53  e8 08 a4 ff ff                  call 0x40de60
00413A58  83 38 00                        cmp dword ptr [eax], 0
00413A5B  75 13                           jne 0x413a70
00413A5D  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00413A62  e8 39 80 ff ff                  call 0x40baa0
00413A67  39 45 d4                        cmp dword ptr [ebp - 0x2c], eax
00413A6A  0f 8e 9c 00 00 00               jle 0x413b0c
00413A70  8b 55 08                        mov edx, dword ptr [ebp + 8]
00413A73  8b 45 08                        mov eax, dword ptr [ebp + 8]
00413A76  db 40 40                        fild dword ptr [eax + 0x40]
00413A79  dc 6a 08                        fsubr qword ptr [edx + 8]
00413A7C  83 ec 08                        sub esp, 8
00413A7F  dd 1c 24                        fstp qword ptr [esp]
00413A82  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00413A87  e8 34 a4 ff ff                  call 0x40dec0
00413A8C  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00413A8F  dd 59 08                        fstp qword ptr [ecx + 8]
00413A92  68 fc f2 4b 00                  push 0x4bf2fc
00413A97  e8 c4 bd ff ff                  call 0x40f860
00413A9C  83 c4 04                        add esp, 4
00413A9F  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00413AA2  e8 c4 bb 01 00                  call 0x42f66b
00413AA7  6a 28                           push 0x28
00413AA9  e8 fa 86 08 00                  call 0x49c1a8
00413AAE  83 c4 04                        add esp, 4
00413AB1  89 45 c4                        mov dword ptr [ebp - 0x3c], eax
00413AB4  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
00413AB8  83 7d c4 00                     cmp dword ptr [ebp - 0x3c], 0
00413ABC  74 0d                           je 0x413acb
00413ABE  8b 4d c4                        mov ecx, dword ptr [ebp - 0x3c]
00413AC1  e8 da 24 00 00                  call 0x415fa0
00413AC6  89 45 a8                        mov dword ptr [ebp - 0x58], eax
00413AC9  eb 07                           jmp 0x413ad2
00413ACB  c7 45 a8 00 00 00 00            mov dword ptr [ebp - 0x58], 0
00413AD2  8b 55 a8                        mov edx, dword ptr [ebp - 0x58]
00413AD5  89 55 c8                        mov dword ptr [ebp - 0x38], edx
00413AD8  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00413ADC  8b 45 c8                        mov eax, dword ptr [ebp - 0x38]
00413ADF  89 45 d0                        mov dword ptr [ebp - 0x30], eax
00413AE2  8b 4d d0                        mov ecx, dword ptr [ebp - 0x30]
00413AE5  51                              push ecx
00413AE6  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00413AE9  e8 a8 ab 01 00                  call 0x42e696
00413AEE  c7 45 c0 01 00 00 00            mov dword ptr [ebp - 0x40], 1
00413AF5  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
00413AFC  8d 4d dc                        lea ecx, [ebp - 0x24]
00413AFF  e8 ac 03 ff ff                  call 0x403eb0
00413B04  8b 45 c0                        mov eax, dword ptr [ebp - 0x40]
00413B07  e9 0a 01 00 00                  jmp 0x413c16
00413B0C  dd 45 dc                        fld qword ptr [ebp - 0x24]
00413B0F  e8 98 8a 08 00                  call 0x49c5ac
00413B14  89 45 d8                        mov dword ptr [ebp - 0x28], eax
00413B17  dd 45 e4                        fld qword ptr [ebp - 0x1c]
00413B1A  e8 8d 8a 08 00                  call 0x49c5ac
00413B1F  8b 55 08                        mov edx, dword ptr [ebp + 8]
00413B22  2b 42 44                        sub eax, dword ptr [edx + 0x44]
00413B25  89 45 a4                        mov dword ptr [ebp - 0x5c], eax
00413B28  db 45 a4                        fild dword ptr [ebp - 0x5c]
00413B2B  83 ec 08                        sub esp, 8
00413B2E  dd 1c 24                        fstp qword ptr [esp]
00413B31  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00413B36  e8 15 a4 ff ff                  call 0x40df50
00413B3B  e8 6c 8a 08 00                  call 0x49c5ac
00413B40  89 45 d4                        mov dword ptr [ebp - 0x2c], eax
00413B43  8b 45 d4                        mov eax, dword ptr [ebp - 0x2c]
00413B46  50                              push eax
00413B47  8b 4d d8                        mov ecx, dword ptr [ebp - 0x28]
00413B4A  51                              push ecx
00413B4B  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
00413B50  e8 0b a3 ff ff                  call 0x40de60
00413B55  83 38 00                        cmp dword ptr [eax], 0
00413B58  0f 85 9f 00 00 00               jne 0x413bfd
00413B5E  68 10 f3 4b 00                  push 0x4bf310
00413B63  e8 f8 bc ff ff                  call 0x40f860
00413B68  83 c4 04                        add esp, 4
00413B6B  8b 55 08                        mov edx, dword ptr [ebp + 8]
00413B6E  8b 42 14                        mov eax, dword ptr [edx + 0x14]
00413B71  50                              push eax
00413B72  8b 4a 10                        mov ecx, dword ptr [edx + 0x10]
00413B75  51                              push ecx
00413B76  8b 55 08                        mov edx, dword ptr [ebp + 8]
00413B79  8b 42 0c                        mov eax, dword ptr [edx + 0xc]
00413B7C  50                              push eax
00413B7D  8b 4a 08                        mov ecx, dword ptr [edx + 8]
00413B80  51                              push ecx
00413B81  68 24 f3 4b 00                  push 0x4bf324
00413B86  e8 d5 bc ff ff                  call 0x40f860
00413B8B  83 c4 14                        add esp, 0x14
00413B8E  6a 18                           push 0x18
00413B90  e8 13 86 08 00                  call 0x49c1a8
00413B95  83 c4 04                        add esp, 4
00413B98  89 45 b8                        mov dword ptr [ebp - 0x48], eax
00413B9B  c6 45 fc 02                     mov byte ptr [ebp - 4], 2
00413B9F  83 7d b8 00                     cmp dword ptr [ebp - 0x48], 0
00413BA3  74 0d                           je 0x413bb2
00413BA5  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
00413BA8  e8 d3 40 00 00                  call 0x417c80
00413BAD  89 45 a0                        mov dword ptr [ebp - 0x60], eax
00413BB0  eb 07                           jmp 0x413bb9
00413BB2  c7 45 a0 00 00 00 00            mov dword ptr [ebp - 0x60], 0
00413BB9  8b 55 a0                        mov edx, dword ptr [ebp - 0x60]
00413BBC  89 55 bc                        mov dword ptr [ebp - 0x44], edx
00413BBF  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
00413BC3  8b 45 bc                        mov eax, dword ptr [ebp - 0x44]
00413BC6  89 45 cc                        mov dword ptr [ebp - 0x34], eax
00413BC9  8b 4d cc                        mov ecx, dword ptr [ebp - 0x34]
00413BCC  51                              push ecx
00413BCD  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00413BD0  e8 c1 aa 01 00                  call 0x42e696
00413BD5  68 40 f3 4b 00                  push 0x4bf340
00413BDA  e8 81 bc ff ff                  call 0x40f860
00413BDF  83 c4 04                        add esp, 4
00413BE2  c7 45 b4 01 00 00 00            mov dword ptr [ebp - 0x4c], 1
00413BE9  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
00413BF0  8d 4d dc                        lea ecx, [ebp - 0x24]
00413BF3  e8 b8 02 ff ff                  call 0x403eb0
00413BF8  8b 45 b4                        mov eax, dword ptr [ebp - 0x4c]
00413BFB  eb 19                           jmp 0x413c16
00413BFD  c7 45 b0 00 00 00 00            mov dword ptr [ebp - 0x50], 0
00413C04  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
00413C0B  8d 4d dc                        lea ecx, [ebp - 0x24]
00413C0E  e8 9d 02 ff ff                  call 0x403eb0
00413C13  8b 45 b0                        mov eax, dword ptr [ebp - 0x50]
00413C16  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
00413C19  64 89 0d 00 00 00 00            mov dword ptr fs:[0], ecx
00413C20  8b e5                           mov esp, ebp
00413C22  5d                              pop ebp
00413C23  c2 04 00                        ret 4

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

; VA 0x00417C80, file offset 0x17C80, length 48, SHA-256 c4a6045c0b25e3464c8c09698b888785bbf81fda93192af4ea6cbdddf39c0b6c
00417C80  55                              push ebp
00417C81  8b ec                           mov ebp, esp
00417C83  51                              push ecx
00417C84  89 4d fc                        mov dword ptr [ebp - 4], ecx
00417C87  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
00417C8A  e8 01 07 00 00                  call 0x418390
00417C8F  8b 45 fc                        mov eax, dword ptr [ebp - 4]
00417C92  c7 00 f8 0a 4b 00               mov dword ptr [eax], 0x4b0af8
00417C98  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
00417C9B  c7 41 14 1e 00 00 00            mov dword ptr [ecx + 0x14], 0x1e
00417CA2  8b 55 fc                        mov edx, dword ptr [ebp - 4]
00417CA5  c6 42 11 00                     mov byte ptr [edx + 0x11], 0
00417CA9  8b 45 fc                        mov eax, dword ptr [ebp - 4]
00417CAC  8b e5                           mov esp, ebp
00417CAE  5d                              pop ebp
00417CAF  c3                              ret

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
