; Reference executable SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Original bytes and Capstone 5.0.7 disassembly.

; VA 0x0042E58C, offset 189836, size 232, SHA-256 d88c75814e54a55fb290ce88b1afe116f37c5a73d8d13d032dbc6a0cdd9f05cb
0042E58C  8b 55 a4                         mov edx, dword ptr [ebp - 0x5c]
0042E58F  83 7a 44 01                      cmp dword ptr [edx + 0x44], 1
0042E593  75 20                            jne 0x42e5b5
0042E595  6a 2b                            push 0x2b
0042E597  8b 4d a4                         mov ecx, dword ptr [ebp - 0x5c]
0042E59A  e8 2a 07 00 00                   call 0x42ecc9
0042E59F  25 ff 00 00 00                   and eax, 0xff
0042E5A4  85 c0                            test eax, eax
0042E5A6  75 0d                            jne 0x42e5b5
0042E5A8  8b 45 a4                         mov eax, dword ptr [ebp - 0x5c]
0042E5AB  c7 80 d8 02 00 00 00 00 00 00    mov dword ptr [eax + 0x2d8], 0
0042E5B5  8b 4d a4                         mov ecx, dword ptr [ebp - 0x5c]
0042E5B8  83 79 44 ff                      cmp dword ptr [ecx + 0x44], -1
0042E5BC  75 20                            jne 0x42e5de
0042E5BE  6a 2b                            push 0x2b
0042E5C0  8b 4d a4                         mov ecx, dword ptr [ebp - 0x5c]
0042E5C3  e8 01 07 00 00                   call 0x42ecc9
0042E5C8  25 ff 00 00 00                   and eax, 0xff
0042E5CD  85 c0                            test eax, eax
0042E5CF  75 0d                            jne 0x42e5de
0042E5D1  8b 55 a4                         mov edx, dword ptr [ebp - 0x5c]
0042E5D4  c7 82 d8 02 00 00 01 00 00 00    mov dword ptr [edx + 0x2d8], 1
0042E5DE  8b 45 a4                         mov eax, dword ptr [ebp - 0x5c]
0042E5E1  83 c0 08                         add eax, 8
0042E5E4  50                               push eax
0042E5E5  8b 4d a4                         mov ecx, dword ptr [ebp - 0x5c]
0042E5E8  83 c1 78                         add ecx, 0x78
0042E5EB  8b 55 a4                         mov edx, dword ptr [ebp - 0x5c]
0042E5EE  8b 42 78                         mov eax, dword ptr [edx + 0x78]
0042E5F1  ff 50 08                         call dword ptr [eax + 8]
0042E5F4  68 14 f8 4b 00                   push 0x4bf814
0042E5F9  8b 4d a4                         mov ecx, dword ptr [ebp - 0x5c]
0042E5FC  8b 91 f0 02 00 00                mov edx, dword ptr [ecx + 0x2f0]
0042E602  52                               push edx
0042E603  8b 4d a4                         mov ecx, dword ptr [ebp - 0x5c]
0042E606  81 c1 28 01 00 00                add ecx, 0x128
0042E60C  e8 ef be fe ff                   call 0x41a500
0042E611  8b c8                            mov ecx, eax
0042E613  e8 c8 be fe ff                   call 0x41a4e0
0042E618  8b c8                            mov ecx, eax
0042E61A  83 c1 18                         add ecx, 0x18
0042E61D  e8 fe 50 fd ff                   call 0x403720
0042E622  50                               push eax
0042E623  e8 48 e2 06 00                   call 0x49c870
0042E628  83 c4 08                         add esp, 8
0042E62B  85 c0                            test eax, eax
0042E62D  74 37                            je 0x42e666
0042E62F  8b 45 a4                         mov eax, dword ptr [ebp - 0x5c]
0042E632  dd 40 10                         fld qword ptr [eax + 0x10]
0042E635  e8 72 df 06 00                   call 0x49c5ac
0042E63A  50                               push eax
0042E63B  8b 4d a4                         mov ecx, dword ptr [ebp - 0x5c]
0042E63E  dd 41 08                         fld qword ptr [ecx + 8]
0042E641  e8 66 df 06 00                   call 0x49c5ac
0042E646  50                               push eax
0042E647  b9 10 6a 4c 00                   mov ecx, 0x4c6a10
0042E64C  e8 ef 58 fd ff                   call 0x403f40
0042E651  89 45 c0                         mov dword ptr [ebp - 0x40], eax
0042E654  8b 55 c0                         mov edx, dword ptr [ebp - 0x40]
0042E657  52                               push edx
0042E658  8b 4d a4                         mov ecx, dword ptr [ebp - 0x5c]
0042E65B  81 c1 28 01 00 00                add ecx, 0x128
0042E661  e8 ba 51 fd ff                   call 0x403820
0042E666  8b 45 a4                         mov eax, dword ptr [ebp - 0x5c]
0042E669  8b 88 44 03 00 00                mov ecx, dword ptr [eax + 0x344]
0042E66F  e8 12 9b 02 00                   call 0x458186

; VA 0x00403F40, offset 16192, size 83, SHA-256 ea2146855495c43c0f52a09e87c2b3414198596a1c8ad0597a6585d19b8f7fbf
00403F40  55                               push ebp
00403F41  8b ec                            mov ebp, esp
00403F43  51                               push ecx
00403F44  89 4d fc                         mov dword ptr [ebp - 4], ecx
00403F47  8b 45 08                         mov eax, dword ptr [ebp + 8]
00403F4A  25 ff 00 00 00                   and eax, 0xff
00403F4F  99                               cdq
00403F50  2b c2                            sub eax, edx
00403F52  d1 f8                            sar eax, 1
00403F54  89 45 08                         mov dword ptr [ebp + 8], eax
00403F57  8b 45 0c                         mov eax, dword ptr [ebp + 0xc]
00403F5A  89 45 0c                         mov dword ptr [ebp + 0xc], eax
00403F5D  8b 4d 0c                         mov ecx, dword ptr [ebp + 0xc]
00403F60  51                               push ecx
00403F61  8b 55 08                         mov edx, dword ptr [ebp + 8]
00403F64  52                               push edx
00403F65  8b 4d fc                         mov ecx, dword ptr [ebp - 4]
00403F68  e8 53 00 00 00                   call 0x403fc0
00403F6D  50                               push eax
00403F6E  8b 4d fc                         mov ecx, dword ptr [ebp - 4]
00403F71  81 c1 64 06 00 00                add ecx, 0x664
00403F77  e8 24 00 00 00                   call 0x403fa0
00403F7C  33 c9                            xor ecx, ecx
00403F7E  8a 08                            mov cl, byte ptr [eax]
00403F80  6b c9 18                         imul ecx, ecx, 0x18
00403F83  8b 55 fc                         mov edx, dword ptr [ebp - 4]
00403F86  8d 84 0a 78 06 00 00             lea eax, [edx + ecx + 0x678]
00403F8D  8b e5                            mov esp, ebp
00403F8F  5d                               pop ebp
00403F90  c2 08 00                         ret 8

; VA 0x00403FA0, offset 16288, size 22, SHA-256 38b9321c0f2f58e27d286e746e4e6e727ed39abdd983cad0ed8fb86c4aa65569
00403FA0  55                               push ebp
00403FA1  8b ec                            mov ebp, esp
00403FA3  51                               push ecx
00403FA4  89 4d fc                         mov dword ptr [ebp - 4], ecx
00403FA7  8b 45 fc                         mov eax, dword ptr [ebp - 4]
00403FAA  8b 40 0c                         mov eax, dword ptr [eax + 0xc]
00403FAD  03 45 08                         add eax, dword ptr [ebp + 8]
00403FB0  8b e5                            mov esp, ebp
00403FB2  5d                               pop ebp
00403FB3  c2 04 00                         ret 4

; VA 0x00403FC0, offset 16320, size 29, SHA-256 f5f6290fedae9b78ff635315f8fefd8b67873dc29303f01d83cdf79fc55ea51e
00403FC0  55                               push ebp
00403FC1  8b ec                            mov ebp, esp
00403FC3  51                               push ecx
00403FC4  89 4d fc                         mov dword ptr [ebp - 4], ecx
00403FC7  8b 45 0c                         mov eax, dword ptr [ebp + 0xc]
00403FCA  c1 e0 08                         shl eax, 8
00403FCD  99                               cdq
00403FCE  2b c2                            sub eax, edx
00403FD0  d1 f8                            sar eax, 1
00403FD2  8b 4d 08                         mov ecx, dword ptr [ebp + 8]
00403FD5  03 c1                            add eax, ecx
00403FD7  8b e5                            mov esp, ebp
00403FD9  5d                               pop ebp
00403FDA  c2 08 00                         ret 8

; VA 0x00403820, offset 14368, size 108, SHA-256 1c1917eab1053ab387b6cec82ed61fbdd2d713b350ea0e49bd861f0b5ed02e95
00403820  55                               push ebp
00403821  8b ec                            mov ebp, esp
00403823  51                               push ecx
00403824  56                               push esi
00403825  57                               push edi
00403826  89 4d fc                         mov dword ptr [ebp - 4], ecx
00403829  8b 75 08                         mov esi, dword ptr [ebp + 8]
0040382C  8b 7d fc                         mov edi, dword ptr [ebp - 4]
0040382F  81 c7 78 01 00 00                add edi, 0x178
00403835  b9 06 00 00 00                   mov ecx, 6
0040383A  f3 a5                            rep movsd dword ptr es:[edi], dword ptr [esi]
0040383C  8b 75 fc                         mov esi, dword ptr [ebp - 4]
0040383F  81 c6 78 01 00 00                add esi, 0x178
00403845  8b 7d fc                         mov edi, dword ptr [ebp - 4]
00403848  81 c7 10 01 00 00                add edi, 0x110
0040384E  b9 06 00 00 00                   mov ecx, 6
00403853  f3 a5                            rep movsd dword ptr es:[edi], dword ptr [esi]
00403855  8b 75 fc                         mov esi, dword ptr [ebp - 4]
00403858  81 c6 10 01 00 00                add esi, 0x110
0040385E  8b 7d fc                         mov edi, dword ptr [ebp - 4]
00403861  81 c7 a8 00 00 00                add edi, 0xa8
00403867  b9 06 00 00 00                   mov ecx, 6
0040386C  f3 a5                            rep movsd dword ptr es:[edi], dword ptr [esi]
0040386E  8b 75 fc                         mov esi, dword ptr [ebp - 4]
00403871  81 c6 a8 00 00 00                add esi, 0xa8
00403877  8b 7d fc                         mov edi, dword ptr [ebp - 4]
0040387A  83 c7 40                         add edi, 0x40
0040387D  b9 06 00 00 00                   mov ecx, 6
00403882  f3 a5                            rep movsd dword ptr es:[edi], dword ptr [esi]
00403884  5f                               pop edi
00403885  5e                               pop esi
00403886  8b e5                            mov esp, ebp
00403888  5d                               pop ebp
00403889  c2 04 00                         ret 4

; VA 0x0041A500, offset 107776, size 20, SHA-256 83773417e7a4f1befacf0638a811f8a5b3bbeb9eeaf20d092ce68f2d0298da02
0041A500  55                               push ebp
0041A501  8b ec                            mov ebp, esp
0041A503  51                               push ecx
0041A504  89 4d fc                         mov dword ptr [ebp - 4], ecx
0041A507  8b 45 fc                         mov eax, dword ptr [ebp - 4]
0041A50A  8b 80 f8 01 00 00                mov eax, dword ptr [eax + 0x1f8]
0041A510  8b e5                            mov esp, ebp
0041A512  5d                               pop ebp
0041A513  c3                               ret

; VA 0x004658D6, offset 415958, size 42, SHA-256 cac3984f894e28afcd554c5060d5a84fc2cb4cbfb4cb47bedbe9809714037236
004658D6  55                               push ebp
004658D7  8b ec                            mov ebp, esp
004658D9  83 ec 1c                         sub esp, 0x1c
004658DC  89 4d e4                         mov dword ptr [ebp - 0x1c], ecx
004658DF  8b 45 08                         mov eax, dword ptr [ebp + 8]
004658E2  50                               push eax
004658E3  8d 4d e8                         lea ecx, [ebp - 0x18]
004658E6  51                               push ecx
004658E7  b9 10 6a 4c 00                   mov ecx, 0x4c6a10
004658EC  e8 7b 7c fd ff                   call 0x43d56c
004658F1  50                               push eax
004658F2  8b 4d e4                         mov ecx, dword ptr [ebp - 0x1c]
004658F5  e8 06 00 00 00                   call 0x465900
004658FA  8b e5                            mov esp, ebp
004658FC  5d                               pop ebp
004658FD  c2 04 00                         ret 4

; VA 0x00465900, offset 416000, size 36, SHA-256 08c93f4542f307e2bc33864d37d92f196711c9c1a7f1786ac782ba6a9d9f61fc
00465900  55                               push ebp
00465901  8b ec                            mov ebp, esp
00465903  51                               push ecx
00465904  56                               push esi
00465905  57                               push edi
00465906  89 4d fc                         mov dword ptr [ebp - 4], ecx
00465909  8b 75 08                         mov esi, dword ptr [ebp + 8]
0046590C  8b 7d fc                         mov edi, dword ptr [ebp - 4]
0046590F  81 c7 90 00 00 00                add edi, 0x90
00465915  b9 06 00 00 00                   mov ecx, 6
0046591A  f3 a5                            rep movsd dword ptr es:[edi], dword ptr [esi]
0046591C  5f                               pop edi
0046591D  5e                               pop esi
0046591E  8b e5                            mov esp, ebp
00465920  5d                               pop ebp
00465921  c2 04 00                         ret 4
