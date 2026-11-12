; Reference SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Static disassembly and original bytes; Capstone 5.0.7 x86 32-bit.

; VA 0x00417CCC, file offset 0x17CCC, length 26, SHA-256 7569ef28b556b0304189d342b0c349417949c5a465a5c1e4d4e24e937be0426b
00417CCC  55                              push ebp
00417CCD  8b ec                           mov ebp, esp
00417CCF  51                              push ecx
00417CD0  89 4d fc                        mov dword ptr [ebp - 4], ecx
00417CD3  68 1c f4 4b 00                  push 0x4bf41c
00417CD8  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00417CDB  e8 45 80 01 00                  call 0x42fd25
00417CE0  8b e5                           mov esp, ebp
00417CE2  5d                              pop ebp
00417CE3  c2 04 00                        ret 4

; VA 0x00417CE6, file offset 0x17CE6, length 241, SHA-256 c4c0b09dd5d8ef89fd33b58b365ce09dbcd773274472a4630b73ddbcef25f88a
00417CE6  55                              push ebp
00417CE7  8b ec                           mov ebp, esp
00417CE9  83 ec 18                        sub esp, 0x18
00417CEC  89 4d f8                        mov dword ptr [ebp - 8], ecx
00417CEF  8b 45 0c                        mov eax, dword ptr [ebp + 0xc]
00417CF2  8b 48 04                        mov ecx, dword ptr [eax + 4]
00417CF5  51                              push ecx
00417CF6  8b 10                           mov edx, dword ptr [eax]
00417CF8  52                              push edx
00417CF9  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00417CFC  83 c1 78                        add ecx, 0x78
00417CFF  8b 45 08                        mov eax, dword ptr [ebp + 8]
00417D02  8b 50 78                        mov edx, dword ptr [eax + 0x78]
00417D05  ff 52 2c                        call dword ptr [edx + 0x2c]
00417D08  c7 45 fc 10 6a 4c 00            mov dword ptr [ebp - 4], 0x4c6a10
00417D0F  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00417D12  83 c1 78                        add ecx, 0x78
00417D15  8b 45 08                        mov eax, dword ptr [ebp + 8]
00417D18  8b 50 78                        mov edx, dword ptr [eax + 0x78]
00417D1B  ff 52 44                        call dword ptr [edx + 0x44]
00417D1E  25 ff 00 00 00                  and eax, 0xff
00417D23  85 c0                           test eax, eax
00417D25  0f 84 94 00 00 00               je 0x417dbf
00417D2B  8b 45 08                        mov eax, dword ptr [ebp + 8]
00417D2E  dd 40 08                        fld qword ptr [eax + 8]
00417D31  e8 76 48 08 00                  call 0x49c5ac
00417D36  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00417D39  03 41 40                        add eax, dword ptr [ecx + 0x40]
00417D3C  89 45 f4                        mov dword ptr [ebp - 0xc], eax
00417D3F  db 45 f4                        fild dword ptr [ebp - 0xc]
00417D42  83 ec 08                        sub esp, 8
00417D45  dd 1c 24                        fstp qword ptr [esp]
00417D48  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
00417D4B  e8 70 61 ff ff                  call 0x40dec0
00417D50  e8 57 48 08 00                  call 0x49c5ac
00417D55  89 45 f0                        mov dword ptr [ebp - 0x10], eax
00417D58  db 45 f0                        fild dword ptr [ebp - 0x10]
00417D5B  8b 55 08                        mov edx, dword ptr [ebp + 8]
00417D5E  dd 5a 08                        fstp qword ptr [edx + 8]
00417D61  8b 45 08                        mov eax, dword ptr [ebp + 8]
00417D64  dd 40 10                        fld qword ptr [eax + 0x10]
00417D67  e8 40 48 08 00                  call 0x49c5ac
00417D6C  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00417D6F  8b 51 44                        mov edx, dword ptr [ecx + 0x44]
00417D72  6b d2 03                        imul edx, edx, 3
00417D75  03 c2                           add eax, edx
00417D77  89 45 ec                        mov dword ptr [ebp - 0x14], eax
00417D7A  db 45 ec                        fild dword ptr [ebp - 0x14]
00417D7D  83 ec 08                        sub esp, 8
00417D80  dd 1c 24                        fstp qword ptr [esp]
00417D83  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
00417D86  e8 c5 61 ff ff                  call 0x40df50
00417D8B  e8 1c 48 08 00                  call 0x49c5ac
00417D90  89 45 e8                        mov dword ptr [ebp - 0x18], eax
00417D93  db 45 e8                        fild dword ptr [ebp - 0x18]
00417D96  8b 45 08                        mov eax, dword ptr [ebp + 8]
00417D99  dd 58 10                        fstp qword ptr [eax + 0x10]
00417D9C  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00417D9F  83 c1 78                        add ecx, 0x78
00417DA2  8b 55 08                        mov edx, dword ptr [ebp + 8]
00417DA5  8b 42 78                        mov eax, dword ptr [edx + 0x78]
00417DA8  ff 50 48                        call dword ptr [eax + 0x48]
00417DAB  8b 4d 0c                        mov ecx, dword ptr [ebp + 0xc]
00417DAE  dd 19                           fstp qword ptr [ecx]
00417DB0  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00417DB3  e8 01 7b 01 00                  call 0x42f8b9
00417DB8  b8 01 00 00 00                  mov eax, 1
00417DBD  eb 12                           jmp 0x417dd1
00417DBF  8b 55 0c                        mov edx, dword ptr [ebp + 0xc]
00417DC2  c7 02 00 00 00 00               mov dword ptr [edx], 0
00417DC8  c7 42 04 00 00 00 00            mov dword ptr [edx + 4], 0
00417DCF  33 c0                           xor eax, eax
00417DD1  8b e5                           mov esp, ebp
00417DD3  5d                              pop ebp
00417DD4  c2 08 00                        ret 8

; VA 0x00417DD7, file offset 0x17DD7, length 49, SHA-256 3e90027ff9b98137c87c89cde1f9e434e7163f0794d2629fbfc2ef9f49c4853b
00417DD7  55                              push ebp
00417DD8  8b ec                           mov ebp, esp
00417DDA  51                              push ecx
00417DDB  89 4d fc                        mov dword ptr [ebp - 4], ecx
00417DDE  8b 45 08                        mov eax, dword ptr [ebp + 8]
00417DE1  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00417DE4  db 41 44                        fild dword ptr [ecx + 0x44]
00417DE7  dc 68 10                        fsubr qword ptr [eax + 0x10]
00417DEA  8b 55 08                        mov edx, dword ptr [ebp + 8]
00417DED  dd 5a 10                        fstp qword ptr [edx + 0x10]
00417DF0  8b 45 08                        mov eax, dword ptr [ebp + 8]
00417DF3  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
00417DF6  db 41 40                        fild dword ptr [ecx + 0x40]
00417DF9  dc 68 08                        fsubr qword ptr [eax + 8]
00417DFC  8b 55 08                        mov edx, dword ptr [ebp + 8]
00417DFF  dd 5a 08                        fstp qword ptr [edx + 8]
00417E02  8b e5                           mov esp, ebp
00417E04  5d                              pop ebp
00417E05  c2 04 00                        ret 4
