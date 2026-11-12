; 0x43ce00
0043CE00  55                        push ebp
0043CE01  8bec                      mov ebp, esp
0043CE03  6aff                      push -1
0043CE05  68b9bc4a00                push 0x4abcb9
0043CE0A  64a100000000              mov eax, dword ptr fs:[0]
0043CE10  50                        push eax
0043CE11  64892500000000            mov dword ptr fs:[0], esp
0043CE18  51                        push ecx
0043CE19  894df0                    mov dword ptr [ebp - 0x10], ecx
0043CE1C  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
0043CE1F  e81c8a0200                call 0x465840
0043CE24  c745fc00000000            mov dword ptr [ebp - 4], 0
0043CE2B  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
0043CE2E  81c1b0000000              add ecx, 0xb0
0043CE34  e8b7b20200                call 0x4680f0
0043CE39  8b45f0                    mov eax, dword ptr [ebp - 0x10]
0043CE3C  c700a8144b00              mov dword ptr [eax], 0x4b14a8
0043CE42  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
0043CE45  c681b802000001            mov byte ptr [ecx + 0x2b8], 1
0043CE4C  8b55f0                    mov edx, dword ptr [ebp - 0x10]
0043CE4F  c782c002000000000000      mov dword ptr [edx + 0x2c0], 0
0043CE59  c782c402000000000000      mov dword ptr [edx + 0x2c4], 0
0043CE63  8b45f0                    mov eax, dword ptr [ebp - 0x10]
0043CE66  c7808800000001000000      mov dword ptr [eax + 0x88], 1
0043CE70  c745fcffffffff            mov dword ptr [ebp - 4], 0xffffffff
0043CE77  8b45f0                    mov eax, dword ptr [ebp - 0x10]
0043CE7A  8b4df4                    mov ecx, dword ptr [ebp - 0xc]
0043CE7D  64890d00000000            mov dword ptr fs:[0], ecx
0043CE84  8be5                      mov esp, ebp
0043CE86  5d                        pop ebp
0043CE87  c3                        ret

; 0x465840
00465840  55                        push ebp
00465841  8bec                      mov ebp, esp
00465843  6aff                      push -1
00465845  683cd94a00                push 0x4ad93c
0046584A  64a100000000              mov eax, dword ptr fs:[0]
00465850  50                        push eax
00465851  64892500000000            mov dword ptr fs:[0], esp
00465858  51                        push ecx
00465859  894df0                    mov dword ptr [ebp - 0x10], ecx
0046585C  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
0046585F  83c108                    add ecx, 8
00465862  e8294cfbff                call 0x41a490
00465867  c745fc00000000            mov dword ptr [ebp - 4], 0
0046586E  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
00465871  81c190000000              add ecx, 0x90
00465877  e804c3f9ff                call 0x401b80
0046587C  8b45f0                    mov eax, dword ptr [ebp - 0x10]
0046587F  c70058184b00              mov dword ptr [eax], 0x4b1858
00465885  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
00465888  c781a800000001000000      mov dword ptr [ecx + 0xa8], 1
00465892  8b55f0                    mov edx, dword ptr [ebp - 0x10]
00465895  c7828800000003000000      mov dword ptr [edx + 0x88], 3
0046589F  c745fcffffffff            mov dword ptr [ebp - 4], 0xffffffff
004658A6  8b45f0                    mov eax, dword ptr [ebp - 0x10]
004658A9  8b4df4                    mov ecx, dword ptr [ebp - 0xc]
004658AC  64890d00000000            mov dword ptr fs:[0], ecx
004658B3  8be5                      mov esp, ebp
004658B5  5d                        pop ebp
004658B6  c3                        ret

; 0x4680f0
004680F0  55                        push ebp
004680F1  8bec                      mov ebp, esp
004680F3  6aff                      push -1
004680F5  68d9da4a00                push 0x4adad9
004680FA  64a100000000              mov eax, dword ptr fs:[0]
00468100  50                        push eax
00468101  64892500000000            mov dword ptr fs:[0], esp
00468108  83ec1c                    sub esp, 0x1c
0046810B  56                        push esi
0046810C  57                        push edi
0046810D  894dd8                    mov dword ptr [ebp - 0x28], ecx
00468110  8b4dd8                    mov ecx, dword ptr [ebp - 0x28]
00468113  e8c888fdff                call 0x4409e0
00468118  c745fc00000000            mov dword ptr [ebp - 4], 0
0046811F  8b4dd8                    mov ecx, dword ptr [ebp - 0x28]
00468122  81c1e0010000              add ecx, 0x1e0
00468128  e8539af9ff                call 0x401b80
0046812D  8b45d8                    mov eax, dword ptr [ebp - 0x28]
00468130  c780d001000001000000      mov dword ptr [eax + 0x1d0], 1
0046813A  8b4dd8                    mov ecx, dword ptr [ebp - 0x28]
0046813D  c6810002000000            mov byte ptr [ecx + 0x200], 0
00468144  8b55d8                    mov edx, dword ptr [ebp - 0x28]
00468147  c782d801000000000000      mov dword ptr [edx + 0x1d8], 0
00468151  c782dc01000000000000      mov dword ptr [edx + 0x1dc], 0
0046815B  8b45d8                    mov eax, dword ptr [ebp - 0x28]
0046815E  c780f801000000000000      mov dword ptr [eax + 0x1f8], 0
00468168  8b4dd8                    mov ecx, dword ptr [ebp - 0x28]
0046816B  c781c801000000000000      mov dword ptr [ecx + 0x1c8], 0
00468175  8b55d8                    mov edx, dword ptr [ebp - 0x28]
00468178  c782cc01000000000000      mov dword ptr [edx + 0x1cc], 0
00468182  8b45d8                    mov eax, dword ptr [ebp - 0x28]
00468185  c780c001000000000000      mov dword ptr [eax + 0x1c0], 0
0046818F  c780c401000000000000      mov dword ptr [eax + 0x1c4], 0
00468199  8b4dd8                    mov ecx, dword ptr [ebp - 0x28]
0046819C  c781b001000000000000      mov dword ptr [ecx + 0x1b0], 0
004681A6  8b55d8                    mov edx, dword ptr [ebp - 0x28]
004681A9  c6820102000001            mov byte ptr [edx + 0x201], 1
004681B0  8b45d8                    mov eax, dword ptr [ebp - 0x28]
004681B3  c780fc010000ffffffff      mov dword ptr [eax + 0x1fc], 0xffffffff
004681BD  8b4dd8                    mov ecx, dword ptr [ebp - 0x28]
004681C0  c781b801000000000000      mov dword ptr [ecx + 0x1b8], 0
004681CA  680000f0bf                push 0xbff00000
004681CF  6a00                      push 0
004681D1  6a00                      push 0
004681D3  6a00                      push 0
004681D5  6a00                      push 0
004681D7  6a00                      push 0
004681D9  8d4ddc                    lea ecx, [ebp - 0x24]
004681DC  e80f96f9ff                call 0x4017f0
004681E1  8bf0                      mov esi, eax
004681E3  8b7dd8                    mov edi, dword ptr [ebp - 0x28]
004681E6  81c7e0010000              add edi, 0x1e0
004681EC  b906000000                mov ecx, 6
004681F1  f3a5                      rep movsd dword ptr es:[edi], dword ptr [esi]
004681F3  c745fcffffffff            mov dword ptr [ebp - 4], 0xffffffff
004681FA  8b45d8                    mov eax, dword ptr [ebp - 0x28]
004681FD  8b4df4                    mov ecx, dword ptr [ebp - 0xc]
00468200  64890d00000000            mov dword ptr fs:[0], ecx
00468207  5f                        pop edi
00468208  5e                        pop esi
00468209  8be5                      mov esp, ebp
0046820B  5d                        pop ebp
0046820C  c3                        ret

; 0x41a490
0041A490  55                        push ebp
0041A491  8bec                      mov ebp, esp
0041A493  51                        push ecx
0041A494  894dfc                    mov dword ptr [ebp - 4], ecx
0041A497  8b45fc                    mov eax, dword ptr [ebp - 4]
0041A49A  c60000                    mov byte ptr [eax], 0
0041A49D  8b45fc                    mov eax, dword ptr [ebp - 4]
0041A4A0  8be5                      mov esp, ebp
0041A4A2  5d                        pop ebp
0041A4A3  c3                        ret

; 0x4409e0
004409E0  55                        push ebp
004409E1  8bec                      mov ebp, esp
004409E3  51                        push ecx
004409E4  894dfc                    mov dword ptr [ebp - 4], ecx
004409E7  8b4dfc                    mov ecx, dword ptr [ebp - 4]
004409EA  e821000000                call 0x440a10
004409EF  8b45fc                    mov eax, dword ptr [ebp - 4]
004409F2  c780a8010000ffffffff      mov dword ptr [eax + 0x1a8], 0xffffffff
004409FC  8b45fc                    mov eax, dword ptr [ebp - 4]
004409FF  8be5                      mov esp, ebp
00440A01  5d                        pop ebp
00440A02  c3                        ret

; 0x440a10
00440A10  55                        push ebp
00440A11  8bec                      mov ebp, esp
00440A13  6aff                      push -1
00440A15  6806bf4a00                push 0x4abf06
00440A1A  64a100000000              mov eax, dword ptr fs:[0]
00440A20  50                        push eax
00440A21  64892500000000            mov dword ptr fs:[0], esp
00440A28  83ec64                    sub esp, 0x64
00440A2B  56                        push esi
00440A2C  57                        push edi
00440A2D  894d90                    mov dword ptr [ebp - 0x70], ecx
00440A30  68a00d4200                push 0x420da0
00440A35  68600d4200                push 0x420d60
00440A3A  6a04                      push 4
00440A3C  6a68                      push 0x68
00440A3E  8b4590                    mov eax, dword ptr [ebp - 0x70]
00440A41  83c008                    add eax, 8
00440A44  50                        push eax
00440A45  e889bb0500                call 0x49c5d3
00440A4A  c745fc00000000            mov dword ptr [ebp - 4], 0
00440A51  8b4d90                    mov ecx, dword ptr [ebp - 0x70]
00440A54  c70100000000              mov dword ptr [ecx], 0
00440A5A  8b5590                    mov edx, dword ptr [ebp - 0x70]
00440A5D  a1c0154b00                mov eax, dword ptr [0x4b15c0]
00440A62  894218                    mov dword ptr [edx + 0x18], eax
00440A65  8b0dc4154b00              mov ecx, dword ptr [0x4b15c4]
00440A6B  894a1c                    mov dword ptr [edx + 0x1c], ecx
00440A6E  8b5590                    mov edx, dword ptr [ebp - 0x70]
00440A71  a1c0154b00                mov eax, dword ptr [0x4b15c0]
00440A76  898280000000              mov dword ptr [edx + 0x80], eax
00440A7C  8b0dc4154b00              mov ecx, dword ptr [0x4b15c4]
00440A82  898a84000000              mov dword ptr [edx + 0x84], ecx
00440A88  8b5590                    mov edx, dword ptr [ebp - 0x70]
00440A8B  a1c0154b00                mov eax, dword ptr [0x4b15c0]
00440A90  8982e8000000              mov dword ptr [edx + 0xe8], eax
00440A96  8b0dc4154b00              mov ecx, dword ptr [0x4b15c4]
00440A9C  898aec000000              mov dword ptr [edx + 0xec], ecx
00440AA2  8b5590                    mov edx, dword ptr [ebp - 0x70]
00440AA5  a1c0154b00                mov eax, dword ptr [0x4b15c0]
00440AAA  898250010000              mov dword ptr [edx + 0x150], eax
00440AB0  8b0dc4154b00              mov ecx, dword ptr [0x4b15c4]
00440AB6  898a54010000              mov dword ptr [edx + 0x154], ecx
00440ABC  6800e06f40                push 0x406fe000
00440AC1  6a00                      push 0
00440AC3  6800e06f40                push 0x406fe000
00440AC8  6a00                      push 0
00440ACA  6800e06f40                push 0x406fe000
00440ACF  6a00                      push 0
00440AD1  8d4ddc                    lea ecx, [ebp - 0x24]
00440AD4  e8170dfcff                call 0x4017f0
00440AD9  8bf0                      mov esi, eax
00440ADB  8b7d90                    mov edi, dword ptr [ebp - 0x70]
00440ADE  83c740                    add edi, 0x40
00440AE1  b906000000                mov ecx, 6
00440AE6  f3a5                      rep movsd dword ptr es:[edi], dword ptr [esi]
00440AE8  6800e06f40                push 0x406fe000
00440AED  6a00                      push 0
00440AEF  6800e06f40                push 0x406fe000
00440AF4  6a00                      push 0
00440AF6  6800e06f40                push 0x406fe000
00440AFB  6a00                      push 0
00440AFD  8d4dc4                    lea ecx, [ebp - 0x3c]
00440B00  e8eb0cfcff                call 0x4017f0
00440B05  8bf0                      mov esi, eax
00440B07  8b7d90                    mov edi, dword ptr [ebp - 0x70]
00440B0A  81c7a8000000              add edi, 0xa8
00440B10  b906000000                mov ecx, 6
00440B15  f3a5                      rep movsd dword ptr es:[edi], dword ptr [esi]
00440B17  6800e06f40                push 0x406fe000
00440B1C  6a00                      push 0
00440B1E  6800e06f40                push 0x406fe000
00440B23  6a00                      push 0
00440B25  6800e06f40                push 0x406fe000
00440B2A  6a00                      push 0
00440B2C  8d4dac                    lea ecx, [ebp - 0x54]
00440B2F  e8bc0cfcff                call 0x4017f0
00440B34  8bf0                      mov esi, eax
00440B36  8b7d90                    mov edi, dword ptr [ebp - 0x70]
00440B39  81c710010000              add edi, 0x110
00440B3F  b906000000                mov ecx, 6
00440B44  f3a5                      rep movsd dword ptr es:[edi], dword ptr [esi]
00440B46  6800e06f40                push 0x406fe000
00440B4B  6a00                      push 0
00440B4D  6800e06f40                push 0x406fe000
00440B52  6a00                      push 0
00440B54  6800e06f40                push 0x406fe000
00440B59  6a00                      push 0
00440B5B  8d4d94                    lea ecx, [ebp - 0x6c]
00440B5E  e88d0cfcff                call 0x4017f0
00440B63  8bf0                      mov esi, eax
00440B65  8b7d90                    mov edi, dword ptr [ebp - 0x70]
00440B68  81c778010000              add edi, 0x178
00440B6E  b906000000                mov ecx, 6
00440B73  f3a5                      rep movsd dword ptr es:[edi], dword ptr [esi]
00440B75  8b5590                    mov edx, dword ptr [ebp - 0x70]
00440B78  c7425800000000            mov dword ptr [edx + 0x58], 0
00440B7F  c7425c00e06f40            mov dword ptr [edx + 0x5c], 0x406fe000
00440B86  8b4590                    mov eax, dword ptr [ebp - 0x70]
00440B89  c780c000000000000000      mov dword ptr [eax + 0xc0], 0
00440B93  c780c400000000e06f40      mov dword ptr [eax + 0xc4], 0x406fe000
00440B9D  8b4d90                    mov ecx, dword ptr [ebp - 0x70]
00440BA0  c7812801000000000000      mov dword ptr [ecx + 0x128], 0
00440BAA  c7812c01000000e06f40      mov dword ptr [ecx + 0x12c], 0x406fe000
00440BB4  8b5590                    mov edx, dword ptr [ebp - 0x70]
00440BB7  c7829001000000000000      mov dword ptr [edx + 0x190], 0
00440BC1  c7829401000000e06f40      mov dword ptr [edx + 0x194], 0x406fe000
00440BCB  c745fcffffffff            mov dword ptr [ebp - 4], 0xffffffff
00440BD2  8b4590                    mov eax, dword ptr [ebp - 0x70]
00440BD5  8b4df4                    mov ecx, dword ptr [ebp - 0xc]
00440BD8  64890d00000000            mov dword ptr fs:[0], ecx
00440BDF  5f                        pop edi
00440BE0  5e                        pop esi
00440BE1  8be5                      mov esp, ebp
00440BE3  5d                        pop ebp
00440BE4  c3                        ret

; 0x420d60
00420D60  55                        push ebp
00420D61  8bec                      mov ebp, esp
00420D63  51                        push ecx
00420D64  894dfc                    mov dword ptr [ebp - 4], ecx
00420D67  8b4dfc                    mov ecx, dword ptr [ebp - 4]
00420D6A  e8110efeff                call 0x401b80
00420D6F  8b4dfc                    mov ecx, dword ptr [ebp - 4]
00420D72  83c118                    add ecx, 0x18
00420D75  e856140600                call 0x4821d0
00420D7A  8b4dfc                    mov ecx, dword ptr [ebp - 4]
00420D7D  83c128                    add ecx, 0x28
00420D80  e84b140600                call 0x4821d0
00420D85  8b4dfc                    mov ecx, dword ptr [ebp - 4]
00420D88  83c138                    add ecx, 0x38
00420D8B  e8f00dfeff                call 0x401b80
00420D90  8b45fc                    mov eax, dword ptr [ebp - 4]
00420D93  8be5                      mov esp, ebp
00420D95  5d                        pop ebp
00420D96  c3                        ret

; 0x420da0
00420DA0  55                        push ebp
00420DA1  8bec                      mov ebp, esp
00420DA3  51                        push ecx
00420DA4  894dfc                    mov dword ptr [ebp - 4], ecx
00420DA7  8be5                      mov esp, ebp
00420DA9  5d                        pop ebp
00420DAA  c3                        ret

; 0x49c5d3
0049C5D3  55                        push ebp
0049C5D4  8bec                      mov ebp, esp
0049C5D6  6aff                      push -1
0049C5D8  68b83f4b00                push 0x4b3fb8
0049C5DD  6800f24900                push 0x49f200
0049C5E2  64a100000000              mov eax, dword ptr fs:[0]
0049C5E8  50                        push eax
0049C5E9  64892500000000            mov dword ptr fs:[0], esp
0049C5F0  83ec10                    sub esp, 0x10
0049C5F3  53                        push ebx
0049C5F4  56                        push esi
0049C5F5  57                        push edi
0049C5F6  33c0                      xor eax, eax
0049C5F8  8945e0                    mov dword ptr [ebp - 0x20], eax
0049C5FB  8945fc                    mov dword ptr [ebp - 4], eax
0049C5FE  8945e4                    mov dword ptr [ebp - 0x1c], eax
0049C601  8b45e4                    mov eax, dword ptr [ebp - 0x1c]
0049C604  3b4510                    cmp eax, dword ptr [ebp + 0x10]
0049C607  7d13                      jge 0x49c61c
0049C609  8b7508                    mov esi, dword ptr [ebp + 8]
0049C60C  8bce                      mov ecx, esi
0049C60E  ff5514                    call dword ptr [ebp + 0x14]
0049C611  03750c                    add esi, dword ptr [ebp + 0xc]
0049C614  897508                    mov dword ptr [ebp + 8], esi
0049C617  ff45e4                    inc dword ptr [ebp - 0x1c]
0049C61A  ebe5                      jmp 0x49c601
0049C61C  c745e001000000            mov dword ptr [ebp - 0x20], 1
0049C623  834dfcff                  or dword ptr [ebp - 4], 0xffffffff
0049C627  e811000000                call 0x49c63d
0049C62C  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
0049C62F  64890d00000000            mov dword ptr fs:[0], ecx
0049C636  5f                        pop edi
0049C637  5e                        pop esi
0049C638  5b                        pop ebx
0049C639  c9                        leave
0049C63A  c21400                    ret 0x14

; 0x49c63d
0049C63D  837de000                  cmp dword ptr [ebp - 0x20], 0
0049C641  7511                      jne 0x49c654
0049C643  ff7518                    push dword ptr [ebp + 0x18]
0049C646  ff75e4                    push dword ptr [ebp - 0x1c]
0049C649  ff750c                    push dword ptr [ebp + 0xc]
0049C64C  ff7508                    push dword ptr [ebp + 8]
0049C64F  e881000000                call 0x49c6d5
0049C654  c3                        ret
