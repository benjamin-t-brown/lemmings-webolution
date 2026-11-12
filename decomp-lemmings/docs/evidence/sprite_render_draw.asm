; VA 0046D460 file offset 6D460 length 421 SHA256 286c1a738a0b6c583cbc27f4eb4d092ef780df5c29037e704b1dae2986a751b4
0046D460 55                         push ebp
0046D461 8bec                       mov ebp, esp
0046D463 83ec10                     sub esp, 0x10
0046D466 56                         push esi
0046D467 894df0                     mov dword ptr [ebp - 0x10], ecx
0046D46A 33c0                       xor eax, eax
0046D46C a050d55800                 mov al, byte ptr [0x58d550]
0046D471 83e001                     and eax, 1
0046D474 85c0                       test eax, eax
0046D476 7526                       jne 0x46d49e
0046D478 8a0d50d55800               mov cl, byte ptr [0x58d550]
0046D47E 80c901                     or cl, 1
0046D481 880d50d55800               mov byte ptr [0x58d550], cl
0046D487 b940d55800                 mov ecx, 0x58d540
0046D48C e86f57f9ff                 call 0x402c00
0046D491 6805d64600                 push 0x46d605
0046D496 e88ef60200                 call 0x49cb29
0046D49B 83c404                     add esp, 4
0046D49E 8b4df0                     mov ecx, dword ptr [ebp - 0x10]
0046D4A1 e81a0a0000                 call 0x46dec0
0046D4A6 50                         push eax
0046D4A7 b940d55800                 mov ecx, 0x58d540
0046D4AC e87f0b0000                 call 0x46e030
0046D4B1 c745fc00000000             mov dword ptr [ebp - 4], 0
0046D4B8 eb09                       jmp 0x46d4c3
0046D4BA 8b55fc                     mov edx, dword ptr [ebp - 4]
0046D4BD 83c201                     add edx, 1
0046D4C0 8955fc                     mov dword ptr [ebp - 4], edx
0046D4C3 8b4df0                     mov ecx, dword ptr [ebp - 0x10]
0046D4C6 e8f5090000                 call 0x46dec0
0046D4CB 3945fc                     cmp dword ptr [ebp - 4], eax
0046D4CE 0f8df0000000               jge 0x46d5c4
0046D4D4 8b45fc                     mov eax, dword ptr [ebp - 4]
0046D4D7 50                         push eax
0046D4D8 b940d55800                 mov ecx, 0x58d540
0046D4DD e81e0b0000                 call 0x46e000
0046D4E2 8945f8                     mov dword ptr [ebp - 8], eax
0046D4E5 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046D4E8 51                         push ecx
0046D4E9 8b4df0                     mov ecx, dword ptr [ebp - 0x10]
0046D4EC e84f75faff                 call 0x414a40
0046D4F1 8b10                       mov edx, dword ptr [eax]
0046D4F3 8955f4                     mov dword ptr [ebp - 0xc], edx
0046D4F6 8b45f4                     mov eax, dword ptr [ebp - 0xc]
0046D4F9 dd4018                     fld qword ptr [eax + 0x18]
0046D4FC 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046D4FF d919                       fstp dword ptr [ecx]
0046D501 8b55f4                     mov edx, dword ptr [ebp - 0xc]
0046D504 dd4220                     fld qword ptr [edx + 0x20]
0046D507 8b45f8                     mov eax, dword ptr [ebp - 8]
0046D50A d95804                     fstp dword ptr [eax + 4]
0046D50D 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046D510 dd4110                     fld qword ptr [ecx + 0x10]
0046D513 8b55f8                     mov edx, dword ptr [ebp - 8]
0046D516 d95a08                     fstp dword ptr [edx + 8]
0046D519 8b45f8                     mov eax, dword ptr [ebp - 8]
0046D51C d94008                     fld dword ptr [eax + 8]
0046D51F dc3dd8194b00               fdivr qword ptr [0x4b19d8]
0046D525 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046D528 d9590c                     fstp dword ptr [ecx + 0xc]
0046D52B 8b55f4                     mov edx, dword ptr [ebp - 0xc]
0046D52E dd4250                     fld qword ptr [edx + 0x50]
0046D531 e876f00200                 call 0x49c5ac
0046D536 8bf0                       mov esi, eax
0046D538 c1e618                     shl esi, 0x18
0046D53B 6a00                       push 0
0046D53D 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046D540 83c138                     add ecx, 0x38
0046D543 e828090000                 call 0x46de70
0046D548 dd00                       fld qword ptr [eax]
0046D54A e85df00200                 call 0x49c5ac
0046D54F c1e010                     shl eax, 0x10
0046D552 0bf0                       or esi, eax
0046D554 6a01                       push 1
0046D556 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046D559 83c138                     add ecx, 0x38
0046D55C e80f090000                 call 0x46de70
0046D561 dd00                       fld qword ptr [eax]
0046D563 e844f00200                 call 0x49c5ac
0046D568 c1e008                     shl eax, 8
0046D56B 0bf0                       or esi, eax
0046D56D 6a02                       push 2
0046D56F 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046D572 83c138                     add ecx, 0x38
0046D575 e8f6080000                 call 0x46de70
0046D57A dd00                       fld qword ptr [eax]
0046D57C e82bf00200                 call 0x49c5ac
0046D581 0bf0                       or esi, eax
0046D583 8b45f8                     mov eax, dword ptr [ebp - 8]
0046D586 897010                     mov dword ptr [eax + 0x10], esi
0046D589 6a00                       push 0
0046D58B 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046D58E 83c128                     add ecx, 0x28
0046D591 e8fa080000                 call 0x46de90
0046D596 dd05a0d25800               fld qword ptr [0x58d2a0]
0046D59C dc00                       fadd qword ptr [eax]
0046D59E 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046D5A1 d95918                     fstp dword ptr [ecx + 0x18]
0046D5A4 6a01                       push 1
0046D5A6 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046D5A9 83c128                     add ecx, 0x28
0046D5AC e8df080000                 call 0x46de90
0046D5B1 dd05a8d25800               fld qword ptr [0x58d2a8]
0046D5B7 dc00                       fadd qword ptr [eax]
0046D5B9 8b55f8                     mov edx, dword ptr [ebp - 8]
0046D5BC d95a1c                     fstp dword ptr [edx + 0x1c]
0046D5BF e9f6feffff                 jmp 0x46d4ba
0046D5C4 6a00                       push 0
0046D5C6 b940d55800                 mov ecx, 0x58d540
0046D5CB e8500a0000                 call 0x46e020
0046D5D0 50                         push eax
0046D5D1 6a00                       push 0
0046D5D3 b940d55800                 mov ecx, 0x58d540
0046D5D8 e8230a0000                 call 0x46e000
0046D5DD 50                         push eax
0046D5DE 68c4010000                 push 0x1c4
0046D5E3 6a06                       push 6
0046D5E5 a1b8cd5800                 mov eax, dword ptr [0x58cdb8]
0046D5EA 8b08                       mov ecx, dword ptr [eax]
0046D5EC 8b15b8cd5800               mov edx, dword ptr [0x58cdb8]
0046D5F2 52                         push edx
0046D5F3 ff5170                     call dword ptr [ecx + 0x70]
0046D5F6 b940cb5800                 mov ecx, 0x58cb40
0046D5FB e8b0080000                 call 0x46deb0
0046D600 5e                         pop esi
0046D601 8be5                       mov esp, ebp
0046D603 5d                         pop ebp
0046D604 c3                         ret

; VA 0046DE70 file offset 6DE70 length 22 SHA256 dfb4f7584787b079a479a612acfdfe25e88554f36f15b23a16e771443d887b78
0046DE70 55                         push ebp
0046DE71 8bec                       mov ebp, esp
0046DE73 51                         push ecx
0046DE74 894dfc                     mov dword ptr [ebp - 4], ecx
0046DE77 8b4508                     mov eax, dword ptr [ebp + 8]
0046DE7A 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046DE7D 8d04c1                     lea eax, [ecx + eax*8]
0046DE80 8be5                       mov esp, ebp
0046DE82 5d                         pop ebp
0046DE83 c20400                     ret 4

; VA 0046DE90 file offset 6DE90 length 22 SHA256 dfb4f7584787b079a479a612acfdfe25e88554f36f15b23a16e771443d887b78
0046DE90 55                         push ebp
0046DE91 8bec                       mov ebp, esp
0046DE93 51                         push ecx
0046DE94 894dfc                     mov dword ptr [ebp - 4], ecx
0046DE97 8b4508                     mov eax, dword ptr [ebp + 8]
0046DE9A 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046DE9D 8d04c1                     lea eax, [ecx + eax*8]
0046DEA0 8be5                       mov esp, ebp
0046DEA2 5d                         pop ebp
0046DEA3 c20400                     ret 4

; VA 0046DEB0 file offset 6DEB0 length 11 SHA256 012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5
0046DEB0 55                         push ebp
0046DEB1 8bec                       mov ebp, esp
0046DEB3 51                         push ecx
0046DEB4 894dfc                     mov dword ptr [ebp - 4], ecx
0046DEB7 8be5                       mov esp, ebp
0046DEB9 5d                         pop ebp
0046DEBA c3                         ret

; VA 0046E020 file offset 6E020 length 16 SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936
0046E020 55                         push ebp
0046E021 8bec                       mov ebp, esp
0046E023 51                         push ecx
0046E024 894dfc                     mov dword ptr [ebp - 4], ecx
0046E027 8b45fc                     mov eax, dword ptr [ebp - 4]
0046E02A 8b00                       mov eax, dword ptr [eax]
0046E02C 8be5                       mov esp, ebp
0046E02E 5d                         pop ebp
0046E02F c3                         ret

; VA 0046E000 file offset 6E000 length 27 SHA256 fa8f6fe634cfefa96ff89f7d358d9e11d4b2e0f5bc08c4a00b99622ca58448d9
0046E000 55                         push ebp
0046E001 8bec                       mov ebp, esp
0046E003 51                         push ecx
0046E004 894dfc                     mov dword ptr [ebp - 4], ecx
0046E007 8b4508                     mov eax, dword ptr [ebp + 8]
0046E00A c1e005                     shl eax, 5
0046E00D 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046E010 8b510c                     mov edx, dword ptr [ecx + 0xc]
0046E013 03c2                       add eax, edx
0046E015 8be5                       mov esp, ebp
0046E017 5d                         pop ebp
0046E018 c20400                     ret 4

; VA 0046E030 file offset 6E030 length 274 SHA256 482930eb17fb17443ee2875e170f6221829683e138006abab3a1ce889cb8e22c
0046E030 55                         push ebp
0046E031 8bec                       mov ebp, esp
0046E033 83ec18                     sub esp, 0x18
0046E036 56                         push esi
0046E037 57                         push edi
0046E038 894de8                     mov dword ptr [ebp - 0x18], ecx
0046E03B 8b45e8                     mov eax, dword ptr [ebp - 0x18]
0046E03E 8b4d08                     mov ecx, dword ptr [ebp + 8]
0046E041 3b4804                     cmp ecx, dword ptr [eax + 4]
0046E044 0f8ee8000000               jle 0x46e132
0046E04A 8b55e8                     mov edx, dword ptr [ebp - 0x18]
0046E04D 837a0801                   cmp dword ptr [edx + 8], 1
0046E051 750b                       jne 0x46e05e
0046E053 8b45e8                     mov eax, dword ptr [ebp - 0x18]
0046E056 8b4d08                     mov ecx, dword ptr [ebp + 8]
0046E059 894804                     mov dword ptr [eax + 4], ecx
0046E05C eb1d                       jmp 0x46e07b
0046E05E 8b55e8                     mov edx, dword ptr [ebp - 0x18]
0046E061 8b4204                     mov eax, dword ptr [edx + 4]
0046E064 8b4de8                     mov ecx, dword ptr [ebp - 0x18]
0046E067 034108                     add eax, dword ptr [ecx + 8]
0046E06A 8b55e8                     mov edx, dword ptr [ebp - 0x18]
0046E06D 894204                     mov dword ptr [edx + 4], eax
0046E070 8b45e8                     mov eax, dword ptr [ebp - 0x18]
0046E073 8b4804                     mov ecx, dword ptr [eax + 4]
0046E076 3b4d08                     cmp ecx, dword ptr [ebp + 8]
0046E079 7ee3                       jle 0x46e05e
0046E07B 8b55e8                     mov edx, dword ptr [ebp - 0x18]
0046E07E 837a0400                   cmp dword ptr [edx + 4], 0
0046E082 0f8e82000000               jle 0x46e10a
0046E088 8b45e8                     mov eax, dword ptr [ebp - 0x18]
0046E08B 8b4804                     mov ecx, dword ptr [eax + 4]
0046E08E c1e105                     shl ecx, 5
0046E091 51                         push ecx
0046E092 e811e10200                 call 0x49c1a8
0046E097 83c404                     add esp, 4
0046E09A 8945f4                     mov dword ptr [ebp - 0xc], eax
0046E09D 8b55f4                     mov edx, dword ptr [ebp - 0xc]
0046E0A0 8955fc                     mov dword ptr [ebp - 4], edx
0046E0A3 8b45e8                     mov eax, dword ptr [ebp - 0x18]
0046E0A6 83780c00                   cmp dword ptr [eax + 0xc], 0
0046E0AA 7453                       je 0x46e0ff
0046E0AC c745f800000000             mov dword ptr [ebp - 8], 0
0046E0B3 eb09                       jmp 0x46e0be
0046E0B5 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046E0B8 83c101                     add ecx, 1
0046E0BB 894df8                     mov dword ptr [ebp - 8], ecx
0046E0BE 8b55e8                     mov edx, dword ptr [ebp - 0x18]
0046E0C1 8b45f8                     mov eax, dword ptr [ebp - 8]
0046E0C4 3b02                       cmp eax, dword ptr [edx]
0046E0C6 7d22                       jge 0x46e0ea
0046E0C8 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046E0CB c1e105                     shl ecx, 5
0046E0CE 8b55e8                     mov edx, dword ptr [ebp - 0x18]
0046E0D1 8b720c                     mov esi, dword ptr [edx + 0xc]
0046E0D4 03f1                       add esi, ecx
0046E0D6 8b45f8                     mov eax, dword ptr [ebp - 8]
0046E0D9 c1e005                     shl eax, 5
0046E0DC 8b7dfc                     mov edi, dword ptr [ebp - 4]
0046E0DF 03f8                       add edi, eax
0046E0E1 b908000000                 mov ecx, 8
0046E0E6 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046E0E8 ebcb                       jmp 0x46e0b5
0046E0EA 8b4de8                     mov ecx, dword ptr [ebp - 0x18]
0046E0ED 8b510c                     mov edx, dword ptr [ecx + 0xc]
0046E0F0 8955f0                     mov dword ptr [ebp - 0x10], edx
0046E0F3 8b45f0                     mov eax, dword ptr [ebp - 0x10]
0046E0F6 50                         push eax
0046E0F7 e841e00200                 call 0x49c13d
0046E0FC 83c404                     add esp, 4
0046E0FF 8b4de8                     mov ecx, dword ptr [ebp - 0x18]
0046E102 8b55fc                     mov edx, dword ptr [ebp - 4]
0046E105 89510c                     mov dword ptr [ecx + 0xc], edx
0046E108 eb28                       jmp 0x46e132
0046E10A 8b45e8                     mov eax, dword ptr [ebp - 0x18]
0046E10D 83780c00                   cmp dword ptr [eax + 0xc], 0
0046E111 7415                       je 0x46e128
0046E113 8b4de8                     mov ecx, dword ptr [ebp - 0x18]
0046E116 8b510c                     mov edx, dword ptr [ecx + 0xc]
0046E119 8955ec                     mov dword ptr [ebp - 0x14], edx
0046E11C 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0046E11F 50                         push eax
0046E120 e818e00200                 call 0x49c13d
0046E125 83c404                     add esp, 4
0046E128 8b4de8                     mov ecx, dword ptr [ebp - 0x18]
0046E12B c7410c00000000             mov dword ptr [ecx + 0xc], 0
0046E132 8b55e8                     mov edx, dword ptr [ebp - 0x18]
0046E135 8b4508                     mov eax, dword ptr [ebp + 8]
0046E138 8902                       mov dword ptr [edx], eax
0046E13A 5f                         pop edi
0046E13B 5e                         pop esi
0046E13C 8be5                       mov esp, ebp
0046E13E 5d                         pop ebp
0046E13F c20400                     ret 4

; VA 00402C00 file offset 2C00 length 53 SHA256 1f0030291fa3a68771361e044671fd4c4ed373926b81c41092ccd2071eedc704
00402C00 55                         push ebp
00402C01 8bec                       mov ebp, esp
00402C03 51                         push ecx
00402C04 894dfc                     mov dword ptr [ebp - 4], ecx
00402C07 8b45fc                     mov eax, dword ptr [ebp - 4]
00402C0A c70000000000               mov dword ptr [eax], 0
00402C10 8b4dfc                     mov ecx, dword ptr [ebp - 4]
00402C13 c7410400000000             mov dword ptr [ecx + 4], 0
00402C1A 8b55fc                     mov edx, dword ptr [ebp - 4]
00402C1D c7420801000000             mov dword ptr [edx + 8], 1
00402C24 8b45fc                     mov eax, dword ptr [ebp - 4]
00402C27 c7400c00000000             mov dword ptr [eax + 0xc], 0
00402C2E 8b45fc                     mov eax, dword ptr [ebp - 4]
00402C31 8be5                       mov esp, ebp
00402C33 5d                         pop ebp
00402C34 c3                         ret

; VA 0046D605 file offset 6D605 length 15 SHA256 365d99be722447d8521395f66c77dc49fb222acb8bfc9dcb6082a7ae3e6cdc23
0046D605 55                         push ebp
0046D606 8bec                       mov ebp, esp
0046D608 b940d55800                 mov ecx, 0x58d540
0046D60D e82e56f9ff                 call 0x402c40
0046D612 5d                         pop ebp
0046D613 c3                         ret

; VA 00402C40 file offset 2C40 length 19 SHA256 1285b14ba797a34e441d926941f0d7a44c94890b97a269beb1c1a9cc46414ff5
00402C40 55                         push ebp
00402C41 8bec                       mov ebp, esp
00402C43 51                         push ecx
00402C44 894dfc                     mov dword ptr [ebp - 4], ecx
00402C47 8b4dfc                     mov ecx, dword ptr [ebp - 4]
00402C4A e8410c0000                 call 0x403890
00402C4F 8be5                       mov esp, ebp
00402C51 5d                         pop ebp
00402C52 c3                         ret

; VA 00403890 file offset 3890 length 71 SHA256 b6cfa875644c706c3dc2e0d0c0a964d82c1384fecfd400eb7dbc1d4ba1e1673d
00403890 55                         push ebp
00403891 8bec                       mov ebp, esp
00403893 83ec08                     sub esp, 8
00403896 894df8                     mov dword ptr [ebp - 8], ecx
00403899 8b45f8                     mov eax, dword ptr [ebp - 8]
0040389C 83780c00                   cmp dword ptr [eax + 0xc], 0
004038A0 7429                       je 0x4038cb
004038A2 8b4df8                     mov ecx, dword ptr [ebp - 8]
004038A5 c7410400000000             mov dword ptr [ecx + 4], 0
004038AC 8b55f8                     mov edx, dword ptr [ebp - 8]
004038AF 8b420c                     mov eax, dword ptr [edx + 0xc]
004038B2 8945fc                     mov dword ptr [ebp - 4], eax
004038B5 8b4dfc                     mov ecx, dword ptr [ebp - 4]
004038B8 51                         push ecx
004038B9 e87f880900                 call 0x49c13d
004038BE 83c404                     add esp, 4
004038C1 8b55f8                     mov edx, dword ptr [ebp - 8]
004038C4 c7420c00000000             mov dword ptr [edx + 0xc], 0
004038CB 8b4df8                     mov ecx, dword ptr [ebp - 8]
004038CE e8ed050000                 call 0x403ec0
004038D3 8be5                       mov esp, ebp
004038D5 5d                         pop ebp
004038D6 c3                         ret

; VA 00403EC0 file offset 3EC0 length 20 SHA256 a068869c343013a89039a5c55be7cdb6c83ff8952b8a0522ce59771b0170b434
00403EC0 55                         push ebp
00403EC1 8bec                       mov ebp, esp
00403EC3 51                         push ecx
00403EC4 894dfc                     mov dword ptr [ebp - 4], ecx
00403EC7 8b45fc                     mov eax, dword ptr [ebp - 4]
00403ECA c70000000000               mov dword ptr [eax], 0
00403ED0 8be5                       mov esp, ebp
00403ED2 5d                         pop ebp
00403ED3 c3                         ret

; VA 0046DEC0 file offset 6DEC0 length 16 SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936
0046DEC0 55                         push ebp
0046DEC1 8bec                       mov ebp, esp
0046DEC3 51                         push ecx
0046DEC4 894dfc                     mov dword ptr [ebp - 4], ecx
0046DEC7 8b45fc                     mov eax, dword ptr [ebp - 4]
0046DECA 8b00                       mov eax, dword ptr [eax]
0046DECC 8be5                       mov esp, ebp
0046DECE 5d                         pop ebp
0046DECF c3                         ret

; VA 00414A40 file offset 14A40 length 25 SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be
00414A40 55                         push ebp
00414A41 8bec                       mov ebp, esp
00414A43 51                         push ecx
00414A44 894dfc                     mov dword ptr [ebp - 4], ecx
00414A47 8b45fc                     mov eax, dword ptr [ebp - 4]
00414A4A 8b480c                     mov ecx, dword ptr [eax + 0xc]
00414A4D 8b5508                     mov edx, dword ptr [ebp + 8]
00414A50 8d0491                     lea eax, [ecx + edx*4]
00414A53 8be5                       mov esp, ebp
00414A55 5d                         pop ebp
00414A56 c20400                     ret 4

; VA 0049C5AC file offset 9C5AC length 39 SHA256 d0a2aca68648a0ae9d5cbfae1b276d4a64d4f797fc0c60f28b64096cdfa3148c
0049C5AC 55                         push ebp
0049C5AD 8bec                       mov ebp, esp
0049C5AF 83c4f4                     add esp, -0xc
0049C5B2 9b                         wait
0049C5B3 d97dfe                     fnstcw word ptr [ebp - 2]
0049C5B6 9b                         wait
0049C5B7 668b45fe                   mov ax, word ptr [ebp - 2]
0049C5BB 80cc0c                     or ah, 0xc
0049C5BE 668945fc                   mov word ptr [ebp - 4], ax
0049C5C2 d96dfc                     fldcw word ptr [ebp - 4]
0049C5C5 df7df4                     fistp qword ptr [ebp - 0xc]
0049C5C8 d96dfe                     fldcw word ptr [ebp - 2]
0049C5CB 8b45f4                     mov eax, dword ptr [ebp - 0xc]
0049C5CE 8b55f8                     mov edx, dword ptr [ebp - 8]
0049C5D1 c9                         leave
0049C5D2 c3                         ret
; constant4B19D8 000000000000d03f
