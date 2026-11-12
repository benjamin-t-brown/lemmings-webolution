; Executable SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f

; VA 0x00416910; PE offset 92432; length 48; SHA-256 85f69e148ff7d2c86459dd86caeca676c622c407f14e75dbe18f4e7ff4f4af8c
00416910: 55                                               push ebp
00416911: 8b ec                                            mov ebp, esp
00416913: 51                                               push ecx
00416914: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00416917: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0041691a: e8 71 1a 00 00                                   call 0x418390
0041691f: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00416922: c7 00 58 0a 4b 00                                mov dword ptr [eax], 0x4b0a58
00416928: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0041692b: c7 41 14 20 00 00 00                             mov dword ptr [ecx + 0x14], 0x20
00416932: 8b 55 fc                                         mov edx, dword ptr [ebp - 4]
00416935: c6 42 11 00                                      mov byte ptr [edx + 0x11], 0
00416939: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0041693c: 8b e5                                            mov esp, ebp
0041693e: 5d                                               pop ebp
0041693f: c3                                               ret

; VA 0x0041695C; PE offset 92508; length 66; SHA-256 e4e562c9fb986547b0ba358a2ec03115c0415e02962df765140650f264f931be
0041695c: 55                                               push ebp
0041695d: 8b ec                                            mov ebp, esp
0041695f: 51                                               push ecx
00416960: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00416963: a1 88 bf 4c 00                                   mov eax, dword ptr [0x4cbf88]
00416968: 50                                               push eax
00416969: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
0041696c: e8 8f 00 ff ff                                   call 0x406a00
00416971: 0f bf c8                                         movsx ecx, ax
00416974: 51                                               push ecx
00416975: 68 bc f3 4b 00                                   push 0x4bf3bc
0041697a: e8 e1 8e ff ff                                   call 0x40f860
0041697f: 83 c4 0c                                         add esp, 0xc
00416982: 8b 55 08                                         mov edx, dword ptr [ebp + 8]
00416985: 83 7a 4c 03                                      cmp dword ptr [edx + 0x4c], 3
00416989: 74 0d                                            je 0x416998
0041698b: 68 e0 f3 4b 00                                   push 0x4bf3e0
00416990: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
00416993: e8 8d 93 01 00                                   call 0x42fd25
00416998: 8b e5                                            mov esp, ebp
0041699a: 5d                                               pop ebp
0041699b: c2 04 00                                         ret 4

; VA 0x0041699E; PE offset 92574; length 531; SHA-256 abfa88e8f3f7ebbb7694dbd6bb96bf5c1df8cfb63c26b5a03775a4cadf83f625
0041699e: 55                                               push ebp
0041699f: 8b ec                                            mov ebp, esp
004169a1: 6a ff                                            push -1
004169a3: 68 74 a0 4a 00                                   push 0x4aa074
004169a8: 64 a1 00 00 00 00                                mov eax, dword ptr fs:[0]
004169ae: 50                                               push eax
004169af: 64 89 25 00 00 00 00                             mov dword ptr fs:[0], esp
004169b6: 83 ec 58                                         sub esp, 0x58
004169b9: 89 4d b0                                         mov dword ptr [ebp - 0x50], ecx
004169bc: c7 45 e8 10 6a 4c 00                             mov dword ptr [ebp - 0x18], 0x4c6a10
004169c3: 8b 45 0c                                         mov eax, dword ptr [ebp + 0xc]
004169c6: 8b 48 04                                         mov ecx, dword ptr [eax + 4]
004169c9: 51                                               push ecx
004169ca: 8b 10                                            mov edx, dword ptr [eax]
004169cc: 52                                               push edx
004169cd: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
004169d0: 83 c1 78                                         add ecx, 0x78
004169d3: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
004169d6: 8b 50 78                                         mov edx, dword ptr [eax + 0x78]
004169d9: ff 52 2c                                         call dword ptr [edx + 0x2c]
004169dc: 8b 45 b0                                         mov eax, dword ptr [ebp - 0x50]
004169df: 8b 4d 0c                                         mov ecx, dword ptr [ebp + 0xc]
004169e2: dd 40 08                                         fld qword ptr [eax + 8]
004169e5: dc 01                                            fadd qword ptr [ecx]
004169e7: 8b 55 b0                                         mov edx, dword ptr [ebp - 0x50]
004169ea: dd 5a 08                                         fstp qword ptr [edx + 8]
004169ed: 8d 45 d0                                         lea eax, [ebp - 0x30]
004169f0: 50                                               push eax
004169f1: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
004169f4: e8 35 88 01 00                                   call 0x42f22e
004169f9: c7 45 fc 00 00 00 00                             mov dword ptr [ebp - 4], 0
00416a00: dd 45 d0                                         fld qword ptr [ebp - 0x30]
00416a03: e8 a4 5b 08 00                                   call 0x49c5ac
00416a08: 89 45 ac                                         mov dword ptr [ebp - 0x54], eax
00416a0b: db 45 ac                                         fild dword ptr [ebp - 0x54]
00416a0e: 83 ec 08                                         sub esp, 8
00416a11: dd 1c 24                                         fstp qword ptr [esp]
00416a14: 8b 4d e8                                         mov ecx, dword ptr [ebp - 0x18]
00416a17: e8 a4 74 ff ff                                   call 0x40dec0
00416a1c: e8 8b 5b 08 00                                   call 0x49c5ac
00416a21: 89 45 f0                                         mov dword ptr [ebp - 0x10], eax
00416a24: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
00416a27: db 41 44                                         fild dword ptr [ecx + 0x44]
00416a2a: dc 6d d8                                         fsubr qword ptr [ebp - 0x28]
00416a2d: e8 7a 5b 08 00                                   call 0x49c5ac
00416a32: 89 45 a8                                         mov dword ptr [ebp - 0x58], eax
00416a35: db 45 a8                                         fild dword ptr [ebp - 0x58]
00416a38: 83 ec 08                                         sub esp, 8
00416a3b: dd 1c 24                                         fstp qword ptr [esp]
00416a3e: 8b 4d e8                                         mov ecx, dword ptr [ebp - 0x18]
00416a41: e8 0a 75 ff ff                                   call 0x40df50
00416a46: e8 61 5b 08 00                                   call 0x49c5ac
00416a4b: 89 45 ec                                         mov dword ptr [ebp - 0x14], eax
00416a4e: 8b 55 ec                                         mov edx, dword ptr [ebp - 0x14]
00416a51: 52                                               push edx
00416a52: 8b 45 f0                                         mov eax, dword ptr [ebp - 0x10]
00416a55: 50                                               push eax
00416a56: 8b 4d e8                                         mov ecx, dword ptr [ebp - 0x18]
00416a59: e8 02 74 ff ff                                   call 0x40de60
00416a5e: 83 38 00                                         cmp dword ptr [eax], 0
00416a61: 75 47                                            jne 0x416aaa
00416a63: 6a 00                                            push 0
00416a65: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
00416a68: e8 5c 82 01 00                                   call 0x42ecc9
00416a6d: 25 ff 00 00 00                                   and eax, 0xff
00416a72: 85 c0                                            test eax, eax
00416a74: 75 34                                            jne 0x416aaa
00416a76: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
00416a79: db 41 44                                         fild dword ptr [ecx + 0x44]
00416a7c: dd 5d a0                                         fstp qword ptr [ebp - 0x60]
00416a7f: 8b 55 0c                                         mov edx, dword ptr [ebp + 0xc]
00416a82: 8b 42 04                                         mov eax, dword ptr [edx + 4]
00416a85: 50                                               push eax
00416a86: 8b 0a                                            mov ecx, dword ptr [edx]
00416a88: 51                                               push ecx
00416a89: 8b 55 08                                         mov edx, dword ptr [ebp + 8]
00416a8c: 52                                               push edx
00416a8d: e8 a6 f5 ff ff                                   call 0x416038
00416a92: 83 c4 0c                                         add esp, 0xc
00416a95: dc 4d a0                                         fmul qword ptr [ebp - 0x60]
00416a98: dd 5d c8                                         fstp qword ptr [ebp - 0x38]
00416a9b: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
00416a9e: dd 40 10                                         fld qword ptr [eax + 0x10]
00416aa1: dc 65 c8                                         fsub qword ptr [ebp - 0x38]
00416aa4: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
00416aa7: dd 59 10                                         fstp qword ptr [ecx + 0x10]
00416aaa: 8b 55 0c                                         mov edx, dword ptr [ebp + 0xc]
00416aad: c7 02 00 00 00 00                                mov dword ptr [edx], 0
00416ab3: c7 42 04 00 00 00 00                             mov dword ptr [edx + 4], 0
00416aba: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
00416abd: 83 c1 78                                         add ecx, 0x78
00416ac0: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
00416ac3: 8b 50 78                                         mov edx, dword ptr [eax + 0x78]
00416ac6: ff 52 44                                         call dword ptr [edx + 0x44]
00416ac9: 25 ff 00 00 00                                   and eax, 0xff
00416ace: 85 c0                                            test eax, eax
00416ad0: 0f 84 b2 00 00 00                                je 0x416b88
00416ad6: 6a 18                                            push 0x18
00416ad8: e8 cb 56 08 00                                   call 0x49c1a8
00416add: 83 c4 04                                         add esp, 4
00416ae0: 89 45 bc                                         mov dword ptr [ebp - 0x44], eax
00416ae3: c6 45 fc 01                                      mov byte ptr [ebp - 4], 1
00416ae7: 83 7d bc 00                                      cmp dword ptr [ebp - 0x44], 0
00416aeb: 74 0d                                            je 0x416afa
00416aed: 8b 4d bc                                         mov ecx, dword ptr [ebp - 0x44]
00416af0: e8 bb 16 ff ff                                   call 0x4081b0
00416af5: 89 45 9c                                         mov dword ptr [ebp - 0x64], eax
00416af8: eb 07                                            jmp 0x416b01
00416afa: c7 45 9c 00 00 00 00                             mov dword ptr [ebp - 0x64], 0
00416b01: 8b 45 9c                                         mov eax, dword ptr [ebp - 0x64]
00416b04: 89 45 c0                                         mov dword ptr [ebp - 0x40], eax
00416b07: c6 45 fc 00                                      mov byte ptr [ebp - 4], 0
00416b0b: 8b 4d c0                                         mov ecx, dword ptr [ebp - 0x40]
00416b0e: 89 4d c4                                         mov dword ptr [ebp - 0x3c], ecx
00416b11: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
00416b14: 83 c1 78                                         add ecx, 0x78
00416b17: 8b 55 08                                         mov edx, dword ptr [ebp + 8]
00416b1a: 8b 42 78                                         mov eax, dword ptr [edx + 0x78]
00416b1d: ff 50 48                                         call dword ptr [eax + 0x48]
00416b20: 8b 4d 0c                                         mov ecx, dword ptr [ebp + 0xc]
00416b23: dd 19                                            fstp qword ptr [ecx]
00416b25: 8b 55 0c                                         mov edx, dword ptr [ebp + 0xc]
00416b28: 8b 42 04                                         mov eax, dword ptr [edx + 4]
00416b2b: 50                                               push eax
00416b2c: 8b 0a                                            mov ecx, dword ptr [edx]
00416b2e: 51                                               push ecx
00416b2f: 8b 15 88 bf 4c 00                                mov edx, dword ptr [0x4cbf88]
00416b35: 52                                               push edx
00416b36: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
00416b39: e8 c2 fe fe ff                                   call 0x406a00
00416b3e: 0f bf c0                                         movsx eax, ax
00416b41: 50                                               push eax
00416b42: 68 e8 f3 4b 00                                   push 0x4bf3e8
00416b47: e8 14 8d ff ff                                   call 0x40f860
00416b4c: 83 c4 14                                         add esp, 0x14
00416b4f: 8b 4d c4                                         mov ecx, dword ptr [ebp - 0x3c]
00416b52: 51                                               push ecx
00416b53: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
00416b56: e8 3b 7b 01 00                                   call 0x42e696
00416b5b: 8b 55 08                                         mov edx, dword ptr [ebp + 8]
00416b5e: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
00416b61: db 40 44                                         fild dword ptr [eax + 0x44]
00416b64: dc 6a 10                                         fsubr qword ptr [edx + 0x10]
00416b67: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
00416b6a: dd 59 10                                         fstp qword ptr [ecx + 0x10]
00416b6d: c7 45 b8 01 00 00 00                             mov dword ptr [ebp - 0x48], 1
00416b74: c7 45 fc ff ff ff ff                             mov dword ptr [ebp - 4], 0xffffffff
00416b7b: 8d 4d d0                                         lea ecx, [ebp - 0x30]
00416b7e: e8 2d d3 fe ff                                   call 0x403eb0
00416b83: 8b 45 b8                                         mov eax, dword ptr [ebp - 0x48]
00416b86: eb 19                                            jmp 0x416ba1
00416b88: c7 45 b4 00 00 00 00                             mov dword ptr [ebp - 0x4c], 0
00416b8f: c7 45 fc ff ff ff ff                             mov dword ptr [ebp - 4], 0xffffffff
00416b96: 8d 4d d0                                         lea ecx, [ebp - 0x30]
00416b99: e8 12 d3 fe ff                                   call 0x403eb0
00416b9e: 8b 45 b4                                         mov eax, dword ptr [ebp - 0x4c]
00416ba1: 8b 4d f4                                         mov ecx, dword ptr [ebp - 0xc]
00416ba4: 64 89 0d 00 00 00 00                             mov dword ptr fs:[0], ecx
00416bab: 8b e5                                            mov esp, ebp
00416bad: 5d                                               pop ebp
00416bae: c2 08 00                                         ret 8

; VA 0x00401320; PE offset 4896; length 13; SHA-256 8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a
00401320: 55                                               push ebp
00401321: 8b ec                                            mov ebp, esp
00401323: 51                                               push ecx
00401324: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00401327: 8b e5                                            mov esp, ebp
00401329: 5d                                               pop ebp
0040132a: c2 04 00                                         ret 4

; VA 0x00418390; PE offset 99216; length 47; SHA-256 254616efe4eeecb6423131c308a44bd17d051f5651e8a0adebbe4d242ca65772
00418390: 55                                               push ebp
00418391: 8b ec                                            mov ebp, esp
00418393: 51                                               push ecx
00418394: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00418397: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0041839a: c7 00 40 0b 4b 00                                mov dword ptr [eax], 0x4b0b40
004183a0: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
004183a3: c7 41 08 00 00 00 00                             mov dword ptr [ecx + 8], 0
004183aa: c7 41 0c 00 00 00 00                             mov dword ptr [ecx + 0xc], 0
004183b1: 8b 55 fc                                         mov edx, dword ptr [ebp - 4]
004183b4: c6 42 10 00                                      mov byte ptr [edx + 0x10], 0
004183b8: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
004183bb: 8b e5                                            mov esp, ebp
004183bd: 5d                                               pop ebp
004183be: c3                                               ret

; VA 0x0042F22E; PE offset 193070; length 149; SHA-256 4f1cb87f28cc4202499cf9e4579bec7868e84fa9718bf67b19fe25f6e625edc1
0042f22e: 55                                               push ebp
0042f22f: 8b ec                                            mov ebp, esp
0042f231: 6a ff                                            push -1
0042f233: 68 18 b0 4a 00                                   push 0x4ab018
0042f238: 64 a1 00 00 00 00                                mov eax, dword ptr fs:[0]
0042f23e: 50                                               push eax
0042f23f: 64 89 25 00 00 00 00                             mov dword ptr fs:[0], esp
0042f246: 83 ec 20                                         sub esp, 0x20
0042f249: 56                                               push esi
0042f24a: 57                                               push edi
0042f24b: 89 4d d4                                         mov dword ptr [ebp - 0x2c], ecx
0042f24e: c7 45 d8 00 00 00 00                             mov dword ptr [ebp - 0x28], 0
0042f255: 8d 4d dc                                         lea ecx, [ebp - 0x24]
0042f258: e8 43 74 fd ff                                   call 0x4066a0
0042f25d: 8b 45 d4                                         mov eax, dword ptr [ebp - 0x2c]
0042f260: 8b 4d d4                                         mov ecx, dword ptr [ebp - 0x2c]
0042f263: dd 81 60 03 00 00                                fld qword ptr [ecx + 0x360]
0042f269: dc 40 08                                         fadd qword ptr [eax + 8]
0042f26c: dd 5d dc                                         fstp qword ptr [ebp - 0x24]
0042f26f: 8b 55 d4                                         mov edx, dword ptr [ebp - 0x2c]
0042f272: 8b 45 d4                                         mov eax, dword ptr [ebp - 0x2c]
0042f275: dd 80 68 03 00 00                                fld qword ptr [eax + 0x368]
0042f27b: dc 42 10                                         fadd qword ptr [edx + 0x10]
0042f27e: dd 5d e4                                         fstp qword ptr [ebp - 0x1c]
0042f281: 8b 4d d4                                         mov ecx, dword ptr [ebp - 0x2c]
0042f284: 8b 51 18                                         mov edx, dword ptr [ecx + 0x18]
0042f287: 89 55 ec                                         mov dword ptr [ebp - 0x14], edx
0042f28a: 8b 41 1c                                         mov eax, dword ptr [ecx + 0x1c]
0042f28d: 89 45 f0                                         mov dword ptr [ebp - 0x10], eax
0042f290: b9 06 00 00 00                                   mov ecx, 6
0042f295: 8d 75 dc                                         lea esi, [ebp - 0x24]
0042f298: 8b 7d 08                                         mov edi, dword ptr [ebp + 8]
0042f29b: f3 a5                                            rep movsd dword ptr es:[edi], dword ptr [esi]
0042f29d: 8b 4d d8                                         mov ecx, dword ptr [ebp - 0x28]
0042f2a0: 83 c9 01                                         or ecx, 1
0042f2a3: 89 4d d8                                         mov dword ptr [ebp - 0x28], ecx
0042f2a6: 8d 4d dc                                         lea ecx, [ebp - 0x24]
0042f2a9: e8 02 4c fd ff                                   call 0x403eb0
0042f2ae: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
0042f2b1: 8b 4d f4                                         mov ecx, dword ptr [ebp - 0xc]
0042f2b4: 64 89 0d 00 00 00 00                             mov dword ptr fs:[0], ecx
0042f2bb: 5f                                               pop edi
0042f2bc: 5e                                               pop esi
0042f2bd: 8b e5                                            mov esp, ebp
0042f2bf: 5d                                               pop ebp
0042f2c0: c2 04 00                                         ret 4

; VA 0x004066A0; PE offset 26272; length 34; SHA-256 e1e10ac6f649c0685caf0737eb44e8209950355d30eb5327b9bc85485086e434
004066a0: 55                                               push ebp
004066a1: 8b ec                                            mov ebp, esp
004066a3: 51                                               push ecx
004066a4: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
004066a7: 6a 00                                            push 0
004066a9: 6a 00                                            push 0
004066ab: 6a 00                                            push 0
004066ad: 6a 00                                            push 0
004066af: 6a 00                                            push 0
004066b1: 6a 00                                            push 0
004066b3: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
004066b6: e8 35 b1 ff ff                                   call 0x4017f0
004066bb: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
004066be: 8b e5                                            mov esp, ebp
004066c0: 5d                                               pop ebp
004066c1: c3                                               ret

; VA 0x00403EB0; PE offset 16048; length 11; SHA-256 012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5
00403eb0: 55                                               push ebp
00403eb1: 8b ec                                            mov ebp, esp
00403eb3: 51                                               push ecx
00403eb4: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00403eb7: 8b e5                                            mov esp, ebp
00403eb9: 5d                                               pop ebp
00403eba: c3                                               ret

; VA 0x004017F0; PE offset 6128; length 60; SHA-256 9a8c820a68a43ee2211d82f4ef1737664908d5bafae0ecc4e37856b948998034
004017f0: 55                                               push ebp
004017f1: 8b ec                                            mov ebp, esp
004017f3: 51                                               push ecx
004017f4: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
004017f7: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
004017fa: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
004017fd: 89 08                                            mov dword ptr [eax], ecx
004017ff: 8b 55 0c                                         mov edx, dword ptr [ebp + 0xc]
00401802: 89 50 04                                         mov dword ptr [eax + 4], edx
00401805: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00401808: 8b 4d 10                                         mov ecx, dword ptr [ebp + 0x10]
0040180b: 89 48 08                                         mov dword ptr [eax + 8], ecx
0040180e: 8b 55 14                                         mov edx, dword ptr [ebp + 0x14]
00401811: 89 50 0c                                         mov dword ptr [eax + 0xc], edx
00401814: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00401817: 8b 4d 18                                         mov ecx, dword ptr [ebp + 0x18]
0040181a: 89 48 10                                         mov dword ptr [eax + 0x10], ecx
0040181d: 8b 55 1c                                         mov edx, dword ptr [ebp + 0x1c]
00401820: 89 50 14                                         mov dword ptr [eax + 0x14], edx
00401823: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00401826: 8b e5                                            mov esp, ebp
00401828: 5d                                               pop ebp
00401829: c2 18 00                                         ret 0x18

; VA 0x00404D10; PE offset 19728; length 25; SHA-256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be
00404d10: 55                                               push ebp
00404d11: 8b ec                                            mov ebp, esp
00404d13: 51                                               push ecx
00404d14: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00404d17: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00404d1a: 8b 48 0c                                         mov ecx, dword ptr [eax + 0xc]
00404d1d: 8b 55 08                                         mov edx, dword ptr [ebp + 8]
00404d20: 8d 04 91                                         lea eax, [ecx + edx*4]
00404d23: 8b e5                                            mov esp, ebp
00404d25: 5d                                               pop ebp
00404d26: c2 04 00                                         ret 4

; VA 0x0040DE60; PE offset 56928; length 92; SHA-256 17c2a6886c6d8a7368b14ec9da66f74bf7347bfb3308a044362d4c55e7c6b6d3
0040de60: 55                                               push ebp
0040de61: 8b ec                                            mov ebp, esp
0040de63: 51                                               push ecx
0040de64: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
0040de67: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
0040de6a: 25 ff 00 00 00                                   and eax, 0xff
0040de6f: 89 45 08                                         mov dword ptr [ebp + 8], eax
0040de72: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0040de75: 8b 55 0c                                         mov edx, dword ptr [ebp + 0xc]
0040de78: 3b 91 18 04 00 00                                cmp edx, dword ptr [ecx + 0x418]
0040de7e: 7c 0f                                            jl 0x40de8f
0040de80: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0040de83: 8b 88 18 04 00 00                                mov ecx, dword ptr [eax + 0x418]
0040de89: 83 e9 01                                         sub ecx, 1
0040de8c: 89 4d 0c                                         mov dword ptr [ebp + 0xc], ecx
0040de8f: 83 7d 0c 00                                      cmp dword ptr [ebp + 0xc], 0
0040de93: 7d 07                                            jge 0x40de9c
0040de95: c7 45 0c 00 00 00 00                             mov dword ptr [ebp + 0xc], 0
0040de9c: 8b 55 0c                                         mov edx, dword ptr [ebp + 0xc]
0040de9f: c1 e2 08                                         shl edx, 8
0040dea2: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
0040dea5: 03 c2                                            add eax, edx
0040dea7: 50                                               push eax
0040dea8: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0040deab: 81 c1 1c 04 00 00                                add ecx, 0x41c
0040deb1: e8 5a 6e ff ff                                   call 0x404d10
0040deb6: 8b e5                                            mov esp, ebp
0040deb8: 5d                                               pop ebp
0040deb9: c2 08 00                                         ret 8

; VA 0x0040DEC0; PE offset 57024; length 110; SHA-256 53e345321ed4e368c010c76ace2c54fbf6f6b7cc080821effae557b57bbe2f78
0040dec0: 55                                               push ebp
0040dec1: 8b ec                                            mov ebp, esp
0040dec3: 51                                               push ecx
0040dec4: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
0040dec7: 8b 45 0c                                         mov eax, dword ptr [ebp + 0xc]
0040deca: 50                                               push eax
0040decb: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
0040dece: 51                                               push ecx
0040decf: e8 5c 00 00 00                                   call 0x40df30
0040ded4: 83 c4 08                                         add esp, 8
0040ded7: dc 1d 98 06 4b 00                                fcomp qword ptr [0x4b0698]
0040dedd: df e0                                            fnstsw ax
0040dedf: f6 c4 01                                         test ah, 1
0040dee2: 74 08                                            je 0x40deec
0040dee4: dd 05 38 03 4b 00                                fld qword ptr [0x4b0338]
0040deea: eb 3c                                            jmp 0x40df28
0040deec: dd 45 08                                         fld qword ptr [ebp + 8]
0040deef: dc 1d 38 03 4b 00                                fcomp qword ptr [0x4b0338]
0040def5: df e0                                            fnstsw ax
0040def7: f6 c4 01                                         test ah, 1
0040defa: 74 0b                                            je 0x40df07
0040defc: dd 45 08                                         fld qword ptr [ebp + 8]
0040deff: dc 05 a8 07 4b 00                                fadd qword ptr [0x4b07a8]
0040df05: eb 21                                            jmp 0x40df28
0040df07: dd 45 08                                         fld qword ptr [ebp + 8]
0040df0a: dc 1d a8 07 4b 00                                fcomp qword ptr [0x4b07a8]
0040df10: df e0                                            fnstsw ax
0040df12: f6 c4 01                                         test ah, 1
0040df15: 75 0e                                            jne 0x40df25
0040df17: dd 45 08                                         fld qword ptr [ebp + 8]
0040df1a: dc 25 a8 07 4b 00                                fsub qword ptr [0x4b07a8]
0040df20: dd 5d 08                                         fstp qword ptr [ebp + 8]
0040df23: eb e2                                            jmp 0x40df07
0040df25: dd 45 08                                         fld qword ptr [ebp + 8]
0040df28: 8b e5                                            mov esp, ebp
0040df2a: 5d                                               pop ebp
0040df2b: c2 08 00                                         ret 8

; VA 0x0040DF30; PE offset 57136; length 21; SHA-256 1f920dc62d954bc949c11d414be840605a1b5147e0e57a289b64f7bb57916c49
0040df30: 55                                               push ebp
0040df31: 8b ec                                            mov ebp, esp
0040df33: 8b 45 0c                                         mov eax, dword ptr [ebp + 0xc]
0040df36: 50                                               push eax
0040df37: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
0040df3a: 51                                               push ecx
0040df3b: e8 b4 e9 08 00                                   call 0x49c8f4
0040df40: 83 c4 08                                         add esp, 8
0040df43: 5d                                               pop ebp
0040df44: c3                                               ret

; VA 0x0040DF50; PE offset 57168; length 85; SHA-256 442923798ef077222d832bef9326e96123682749444ff3de558159101be4d886
0040df50: 55                                               push ebp
0040df51: 8b ec                                            mov ebp, esp
0040df53: 83 ec 0c                                         sub esp, 0xc
0040df56: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
0040df59: dd 45 08                                         fld qword ptr [ebp + 8]
0040df5c: dc 1d 38 03 4b 00                                fcomp qword ptr [0x4b0338]
0040df62: df e0                                            fnstsw ax
0040df64: f6 c4 01                                         test ah, 1
0040df67: 74 08                                            je 0x40df71
0040df69: dd 05 38 03 4b 00                                fld qword ptr [0x4b0338]
0040df6f: eb 2e                                            jmp 0x40df9f
0040df71: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0040df74: e8 27 db ff ff                                   call 0x40baa0
0040df79: 89 45 f8                                         mov dword ptr [ebp - 8], eax
0040df7c: db 45 f8                                         fild dword ptr [ebp - 8]
0040df7f: dc 5d 08                                         fcomp qword ptr [ebp + 8]
0040df82: df e0                                            fnstsw ax
0040df84: f6 c4 41                                         test ah, 0x41
0040df87: 74 13                                            je 0x40df9c
0040df89: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0040df8c: e8 0f db ff ff                                   call 0x40baa0
0040df91: 83 e8 01                                         sub eax, 1
0040df94: 89 45 f4                                         mov dword ptr [ebp - 0xc], eax
0040df97: db 45 f4                                         fild dword ptr [ebp - 0xc]
0040df9a: eb 03                                            jmp 0x40df9f
0040df9c: dd 45 08                                         fld qword ptr [ebp + 8]
0040df9f: 8b e5                                            mov esp, ebp
0040dfa1: 5d                                               pop ebp
0040dfa2: c2 08 00                                         ret 8

; VA 0x0040BAA0; PE offset 47776; length 20; SHA-256 188576ebbe5ebafad22bde4fe5971546cb6997b28c6a73d19d1a686aca4612eb
0040baa0: 55                                               push ebp
0040baa1: 8b ec                                            mov ebp, esp
0040baa3: 51                                               push ecx
0040baa4: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
0040baa7: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0040baaa: 8b 80 18 04 00 00                                mov eax, dword ptr [eax + 0x418]
0040bab0: 8b e5                                            mov esp, ebp
0040bab2: 5d                                               pop ebp
0040bab3: c3                                               ret

; VA 0x0049C5AC; PE offset 640428; length 39; SHA-256 d0a2aca68648a0ae9d5cbfae1b276d4a64d4f797fc0c60f28b64096cdfa3148c
0049c5ac: 55                                               push ebp
0049c5ad: 8b ec                                            mov ebp, esp
0049c5af: 83 c4 f4                                         add esp, -0xc
0049c5b2: 9b                                               wait
0049c5b3: d9 7d fe                                         fnstcw word ptr [ebp - 2]
0049c5b6: 9b                                               wait
0049c5b7: 66 8b 45 fe                                      mov ax, word ptr [ebp - 2]
0049c5bb: 80 cc 0c                                         or ah, 0xc
0049c5be: 66 89 45 fc                                      mov word ptr [ebp - 4], ax
0049c5c2: d9 6d fc                                         fldcw word ptr [ebp - 4]
0049c5c5: df 7d f4                                         fistp qword ptr [ebp - 0xc]
0049c5c8: d9 6d fe                                         fldcw word ptr [ebp - 2]
0049c5cb: 8b 45 f4                                         mov eax, dword ptr [ebp - 0xc]
0049c5ce: 8b 55 f8                                         mov edx, dword ptr [ebp - 8]
0049c5d1: c9                                               leave
0049c5d2: c3                                               ret

; VA 0x0049C8F4; PE offset 641268; length 181; SHA-256 91822f10090dce7961ee7459a9ee05b0e1a92f13bbfda209ed90afa672a58a86
0049c8f4: 55                                               push ebp
0049c8f5: 8b ec                                            mov ebp, esp
0049c8f7: 51                                               push ecx
0049c8f8: 51                                               push ecx
0049c8f9: 56                                               push esi
0049c8fa: be ff ff 00 00                                   mov esi, 0xffff
0049c8ff: 57                                               push edi
0049c900: 56                                               push esi
0049c901: 68 3f 13 00 00                                   push 0x133f
0049c906: e8 4b 33 00 00                                   call 0x49fc56
0049c90b: 8b f8                                            mov edi, eax
0049c90d: 8b 45 0e                                         mov eax, dword ptr [ebp + 0xe]
0049c910: 66 25 f0 7f                                      and ax, 0x7ff0
0049c914: 59                                               pop ecx
0049c915: 66 3d f0 7f                                      cmp ax, 0x7ff0
0049c919: 59                                               pop ecx
0049c91a: 75 6c                                            jne 0x49c988
0049c91c: dd 45 08                                         fld qword ptr [ebp + 8]
0049c91f: 51                                               push ecx
0049c920: 51                                               push ecx
0049c921: dd 1c 24                                         fstp qword ptr [esp]
0049c924: e8 f5 31 00 00                                   call 0x49fb1e
0049c929: 59                                               pop ecx
0049c92a: 48                                               dec eax
0049c92b: 59                                               pop ecx
0049c92c: 74 4c                                            je 0x49c97a
0049c92e: 48                                               dec eax
0049c92f: 74 39                                            je 0x49c96a
0049c931: dd 45 08                                         fld qword ptr [ebp + 8]
0049c934: 48                                               dec eax
0049c935: 57                                               push edi
0049c936: 74 21                                            je 0x49c959
0049c938: dc 05 d0 02 4b 00                                fadd qword ptr [0x4b02d0]
0049c93e: 51                                               push ecx
0049c93f: 51                                               push ecx
0049c940: dd 1c 24                                         fstp qword ptr [esp]
0049c943: dd 45 08                                         fld qword ptr [ebp + 8]
0049c946: 51                                               push ecx
0049c947: 51                                               push ecx
0049c948: dd 1c 24                                         fstp qword ptr [esp]
0049c94b: 6a 15                                            push 0x15
0049c94d: 6a 08                                            push 8
0049c94f: e8 36 2a 00 00                                   call 0x49f38a
0049c954: 83 c4 1c                                         add esp, 0x1c
0049c957: eb 4c                                            jmp 0x49c9a5
0049c959: 51                                               push ecx
0049c95a: 51                                               push ecx
0049c95b: dd 1c 24                                         fstp qword ptr [esp]
0049c95e: 6a 15                                            push 0x15
0049c960: e8 73 29 00 00                                   call 0x49f2d8
0049c965: 83 c4 10                                         add esp, 0x10
0049c968: eb 3b                                            jmp 0x49c9a5
0049c96a: 56                                               push esi
0049c96b: 57                                               push edi
0049c96c: e8 e5 32 00 00                                   call 0x49fc56
0049c971: dd 45 08                                         fld qword ptr [ebp + 8]
0049c974: 59                                               pop ecx
0049c975: d9 e0                                            fchs
0049c977: 59                                               pop ecx
0049c978: eb 2b                                            jmp 0x49c9a5
0049c97a: 56                                               push esi
0049c97b: 57                                               push edi
0049c97c: e8 d5 32 00 00                                   call 0x49fc56
0049c981: dd 45 08                                         fld qword ptr [ebp + 8]
0049c984: 59                                               pop ecx
0049c985: 59                                               pop ecx
0049c986: eb 1d                                            jmp 0x49c9a5
0049c988: 8b 45 0c                                         mov eax, dword ptr [ebp + 0xc]
0049c98b: 56                                               push esi
0049c98c: 25 ff ff ff 7f                                   and eax, 0x7fffffff
0049c991: 57                                               push edi
0049c992: 89 45 fc                                         mov dword ptr [ebp - 4], eax
0049c995: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
0049c998: 89 45 f8                                         mov dword ptr [ebp - 8], eax
0049c99b: e8 b6 32 00 00                                   call 0x49fc56
0049c9a0: dd 45 f8                                         fld qword ptr [ebp - 8]
0049c9a3: 59                                               pop ecx
0049c9a4: 59                                               pop ecx
0049c9a5: 5f                                               pop edi
0049c9a6: 5e                                               pop esi
0049c9a7: c9                                               leave
0049c9a8: c3                                               ret

; VA 0x0049FC56; PE offset 654422; length 35; SHA-256 84749c64e12c57ce2a4dec7bdb50573ab3dd9f851b65e73be2fba62d0cfec3b2
0049fc56: 55                                               push ebp
0049fc57: 8b ec                                            mov ebp, esp
0049fc59: 51                                               push ecx
0049fc5a: 9b                                               wait
0049fc5b: d9 7d fc                                         fnstcw word ptr [ebp - 4]
0049fc5e: 8b 45 0c                                         mov eax, dword ptr [ebp + 0xc]
0049fc61: 8b c8                                            mov ecx, eax
0049fc63: 23 45 08                                         and eax, dword ptr [ebp + 8]
0049fc66: f7 d1                                            not ecx
0049fc68: 23 4d fc                                         and ecx, dword ptr [ebp - 4]
0049fc6b: 0b c8                                            or ecx, eax
0049fc6d: 89 4d 0c                                         mov dword ptr [ebp + 0xc], ecx
0049fc70: d9 6d 0c                                         fldcw word ptr [ebp + 0xc]
0049fc73: 0f bf 45 fc                                      movsx eax, word ptr [ebp - 4]
0049fc77: c9                                               leave
0049fc78: c3                                               ret

; VA 0x00416038; PE offset 90168; length 74; SHA-256 4e79ab47ece9ce6c65dad17bafa033ae914b7f96c7f1c025c1cf62e78558e316
00416038: 55                                               push ebp
00416039: 8b ec                                            mov ebp, esp
0041603b: 83 ec 10                                         sub esp, 0x10
0041603e: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
00416041: 33 c9                                            xor ecx, ecx
00416043: 8a 48 4b                                         mov cl, byte ptr [eax + 0x4b]
00416046: 85 c9                                            test ecx, ecx
00416048: 74 11                                            je 0x41605b
0041604a: dd 05 18 0a 4b 00                                fld qword ptr [0x4b0a18]
00416050: dc 35 f8 03 4b 00                                fdiv qword ptr [0x4b03f8]
00416056: dd 5d f0                                         fstp qword ptr [ebp - 0x10]
00416059: eb 11                                            jmp 0x41606c
0041605b: 8b 15 18 0a 4b 00                                mov edx, dword ptr [0x4b0a18]
00416061: 89 55 f0                                         mov dword ptr [ebp - 0x10], edx
00416064: a1 1c 0a 4b 00                                   mov eax, dword ptr [0x4b0a1c]
00416069: 89 45 f4                                         mov dword ptr [ebp - 0xc], eax
0041606c: 8b 4d f0                                         mov ecx, dword ptr [ebp - 0x10]
0041606f: 89 4d f8                                         mov dword ptr [ebp - 8], ecx
00416072: 8b 55 f4                                         mov edx, dword ptr [ebp - 0xc]
00416075: 89 55 fc                                         mov dword ptr [ebp - 4], edx
00416078: dd 45 f8                                         fld qword ptr [ebp - 8]
0041607b: dc 4d 0c                                         fmul qword ptr [ebp + 0xc]
0041607e: 8b e5                                            mov esp, ebp
00416080: 5d                                               pop ebp
00416081: c3                                               ret

; VA 0x00406A00; PE offset 27136; length 18; SHA-256 45713e9071f67175de33bff8071f8358847bcf139043b6cfbb69f2f7744aaa47
00406a00: 55                                               push ebp
00406a01: 8b ec                                            mov ebp, esp
00406a03: 51                                               push ecx
00406a04: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00406a07: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00406a0a: 66 8b 40 3a                                      mov ax, word ptr [eax + 0x3a]
00406a0e: 8b e5                                            mov esp, ebp
00406a10: 5d                                               pop ebp
00406a11: c3                                               ret

; data 0x004B0338: 00 00 00 00 00 00 00 00

; data 0x004B0698: 00 00 00 e0 8e 79 45 3e

; data 0x004B07A8: 00 00 00 00 00 00 70 40

; data 0x004B03E8: 00 00 00 00 00 00 24 40

; data 0x004B03F8: 00 00 00 00 00 00 00 40

; data 0x004BF3E0: 65 78 70 6c 6f 64 65 00

; data 0x004B0A58: c0 6b 41 00 9e 69 41 00 5c 69 41 00 20 13 40 00
; data 0x004B0A18: 00 00 00 00 00 00 34 40
