; Executable SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f

; VA 0x00457F40; PE offset 360256; length 557; SHA-256 c1930a02a6306f4e888904a7996871933e77d195934a375d63461431f4aa72ae
00457f40: 55                                               push ebp
00457f41: 8b ec                                            mov ebp, esp
00457f43: 6a ff                                            push -1
00457f45: 68 df cf 4a 00                                   push 0x4acfdf
00457f4a: 64 a1 00 00 00 00                                mov eax, dword ptr fs:[0]
00457f50: 50                                               push eax
00457f51: 64 89 25 00 00 00 00                             mov dword ptr fs:[0], esp
00457f58: 83 ec 38                                         sub esp, 0x38
00457f5b: 56                                               push esi
00457f5c: 57                                               push edi
00457f5d: 89 4d bc                                         mov dword ptr [ebp - 0x44], ecx
00457f60: 68 80 1b 40 00                                   push 0x401b80
00457f65: 6a 0c                                            push 0xc
00457f67: 6a 18                                            push 0x18
00457f69: 8b 45 bc                                         mov eax, dword ptr [ebp - 0x44]
00457f6c: 50                                               push eax
00457f6d: e8 2e 92 fa ff                                   call 0x4011a0
00457f72: 68 40 99 45 00                                   push 0x459940
00457f77: 6a 05                                            push 5
00457f79: 68 80 00 00 00                                   push 0x80
00457f7e: 8b 4d bc                                         mov ecx, dword ptr [ebp - 0x44]
00457f81: 81 c1 28 01 00 00                                add ecx, 0x128
00457f87: 51                                               push ecx
00457f88: e8 13 92 fa ff                                   call 0x4011a0
00457f8d: 8b 4d bc                                         mov ecx, dword ptr [ebp - 0x44]
00457f90: 81 c1 b0 03 00 00                                add ecx, 0x3b0
00457f96: e8 05 e7 fa ff                                   call 0x4066a0
00457f9b: c7 45 fc 00 00 00 00                             mov dword ptr [ebp - 4], 0
00457fa2: 8b 55 bc                                         mov edx, dword ptr [ebp - 0x44]
00457fa5: c6 82 ac 03 00 00 00                             mov byte ptr [edx + 0x3ac], 0
00457fac: 6a 00                                            push 0
00457fae: 6a 00                                            push 0
00457fb0: 68 00 00 f0 3f                                   push 0x3ff00000
00457fb5: 6a 00                                            push 0
00457fb7: 6a 00                                            push 0
00457fb9: 6a 00                                            push 0
00457fbb: 8d 4d c0                                         lea ecx, [ebp - 0x40]
00457fbe: e8 2d 98 fa ff                                   call 0x4017f0
00457fc3: 8b f0                                            mov esi, eax
00457fc5: b9 06 00 00 00                                   mov ecx, 6
00457fca: 8d 7d dc                                         lea edi, [ebp - 0x24]
00457fcd: f3 a5                                            rep movsd dword ptr es:[edi], dword ptr [esi]
00457fcf: c7 45 d8 00 00 00 00                             mov dword ptr [ebp - 0x28], 0
00457fd6: eb 09                                            jmp 0x457fe1
00457fd8: 8b 45 d8                                         mov eax, dword ptr [ebp - 0x28]
00457fdb: 83 c0 01                                         add eax, 1
00457fde: 89 45 d8                                         mov dword ptr [ebp - 0x28], eax
00457fe1: 83 7d d8 05                                      cmp dword ptr [ebp - 0x28], 5
00457fe5: 0f 8d 4d 01 00 00                                jge 0x458138
00457feb: 8b 4d d8                                         mov ecx, dword ptr [ebp - 0x28]
00457fee: d1 e1                                            shl ecx, 1
00457ff0: 51                                               push ecx
00457ff1: 6a 00                                            push 0
00457ff3: 8b 55 d8                                         mov edx, dword ptr [ebp - 0x28]
00457ff6: c1 e2 07                                         shl edx, 7
00457ff9: 8b 45 bc                                         mov eax, dword ptr [ebp - 0x44]
00457ffc: 8d 8c 10 28 01 00 00                             lea ecx, [eax + edx + 0x128]
00458003: e8 98 b5 fb ff                                   call 0x4135a0
00458008: 8b 4d d8                                         mov ecx, dword ptr [ebp - 0x28]
0045800b: 8d 54 09 01                                      lea edx, [ecx + ecx + 1]
0045800f: 52                                               push edx
00458010: 6a 01                                            push 1
00458012: 8b 45 d8                                         mov eax, dword ptr [ebp - 0x28]
00458015: c1 e0 07                                         shl eax, 7
00458018: 8b 4d bc                                         mov ecx, dword ptr [ebp - 0x44]
0045801b: 8d 8c 01 28 01 00 00                             lea ecx, [ecx + eax + 0x128]
00458022: e8 79 b5 fb ff                                   call 0x4135a0
00458027: 8b 55 d8                                         mov edx, dword ptr [ebp - 0x28]
0045802a: 8d 44 12 03                                      lea eax, [edx + edx + 3]
0045802e: 50                                               push eax
0045802f: 6a 02                                            push 2
00458031: 8b 4d d8                                         mov ecx, dword ptr [ebp - 0x28]
00458034: c1 e1 07                                         shl ecx, 7
00458037: 8b 55 bc                                         mov edx, dword ptr [ebp - 0x44]
0045803a: 8d 8c 0a 28 01 00 00                             lea ecx, [edx + ecx + 0x128]
00458041: e8 5a b5 fb ff                                   call 0x4135a0
00458046: 8b 45 d8                                         mov eax, dword ptr [ebp - 0x28]
00458049: 8d 4c 00 02                                      lea ecx, [eax + eax + 2]
0045804d: 51                                               push ecx
0045804e: 6a 03                                            push 3
00458050: 8b 55 d8                                         mov edx, dword ptr [ebp - 0x28]
00458053: c1 e2 07                                         shl edx, 7
00458056: 8b 45 bc                                         mov eax, dword ptr [ebp - 0x44]
00458059: 8d 8c 10 28 01 00 00                             lea ecx, [eax + edx + 0x128]
00458060: e8 3b b5 fb ff                                   call 0x4135a0
00458065: 6a 00                                            push 0
00458067: 6a 00                                            push 0
00458069: 6a 00                                            push 0
0045806b: 6a 00                                            push 0
0045806d: 6a 00                                            push 0
0045806f: 8b 4d d8                                         mov ecx, dword ptr [ebp - 0x28]
00458072: c1 e1 07                                         shl ecx, 7
00458075: 8b 55 bc                                         mov edx, dword ptr [ebp - 0x44]
00458078: 8d 8c 0a 28 01 00 00                             lea ecx, [edx + ecx + 0x128]
0045807f: e8 dc b4 fb ff                                   call 0x413560
00458084: 68 00 00 f0 3f                                   push 0x3ff00000
00458089: 6a 00                                            push 0
0045808b: 6a 00                                            push 0
0045808d: 6a 00                                            push 0
0045808f: 6a 01                                            push 1
00458091: 8b 45 d8                                         mov eax, dword ptr [ebp - 0x28]
00458094: c1 e0 07                                         shl eax, 7
00458097: 8b 4d bc                                         mov ecx, dword ptr [ebp - 0x44]
0045809a: 8d 8c 01 28 01 00 00                             lea ecx, [ecx + eax + 0x128]
004580a1: e8 ba b4 fb ff                                   call 0x413560
004580a6: 68 00 00 f0 3f                                   push 0x3ff00000
004580ab: 6a 00                                            push 0
004580ad: 68 00 00 f0 3f                                   push 0x3ff00000
004580b2: 6a 00                                            push 0
004580b4: 6a 02                                            push 2
004580b6: 8b 55 d8                                         mov edx, dword ptr [ebp - 0x28]
004580b9: c1 e2 07                                         shl edx, 7
004580bc: 8b 45 bc                                         mov eax, dword ptr [ebp - 0x44]
004580bf: 8d 8c 10 28 01 00 00                             lea ecx, [eax + edx + 0x128]
004580c6: e8 95 b4 fb ff                                   call 0x413560
004580cb: 6a 00                                            push 0
004580cd: 6a 00                                            push 0
004580cf: 68 00 00 f0 3f                                   push 0x3ff00000
004580d4: 6a 00                                            push 0
004580d6: 6a 03                                            push 3
004580d8: 8b 4d d8                                         mov ecx, dword ptr [ebp - 0x28]
004580db: c1 e1 07                                         shl ecx, 7
004580de: 8b 55 bc                                         mov edx, dword ptr [ebp - 0x44]
004580e1: 8d 8c 0a 28 01 00 00                             lea ecx, [edx + ecx + 0x128]
004580e8: e8 73 b4 fb ff                                   call 0x413560
004580ed: 68 00 e0 6f 40                                   push 0x406fe000
004580f2: 6a 00                                            push 0
004580f4: 68 00 e0 6f 40                                   push 0x406fe000
004580f9: 6a 00                                            push 0
004580fb: 68 00 e0 6f 40                                   push 0x406fe000
00458100: 6a 00                                            push 0
00458102: 8b 45 d8                                         mov eax, dword ptr [ebp - 0x28]
00458105: c1 e0 07                                         shl eax, 7
00458108: 8b 4d bc                                         mov ecx, dword ptr [ebp - 0x44]
0045810b: 8d 8c 01 28 01 00 00                             lea ecx, [ecx + eax + 0x128]
00458112: e8 a9 b4 fb ff                                   call 0x4135c0
00458117: 68 00 e0 6f 40                                   push 0x406fe000
0045811c: 6a 00                                            push 0
0045811e: 8b 55 d8                                         mov edx, dword ptr [ebp - 0x28]
00458121: c1 e2 07                                         shl edx, 7
00458124: 8b 45 bc                                         mov eax, dword ptr [ebp - 0x44]
00458127: 8d 8c 10 28 01 00 00                             lea ecx, [eax + edx + 0x128]
0045812e: e8 cd 19 00 00                                   call 0x459b00
00458133: e9 a0 fe ff ff                                   jmp 0x457fd8
00458138: 8b 4d bc                                         mov ecx, dword ptr [ebp - 0x44]
0045813b: c7 81 20 01 00 00 00 00 00 00                    mov dword ptr [ecx + 0x120], 0
00458145: 8b 55 bc                                         mov edx, dword ptr [ebp - 0x44]
00458148: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
0045814b: 89 82 a8 03 00 00                                mov dword ptr [edx + 0x3a8], eax
00458151: c7 45 fc ff ff ff ff                             mov dword ptr [ebp - 4], 0xffffffff
00458158: 8b 45 bc                                         mov eax, dword ptr [ebp - 0x44]
0045815b: 8b 4d f4                                         mov ecx, dword ptr [ebp - 0xc]
0045815e: 64 89 0d 00 00 00 00                             mov dword ptr fs:[0], ecx
00458165: 5f                                               pop edi
00458166: 5e                                               pop esi
00458167: 8b e5                                            mov esp, ebp
00458169: 5d                                               pop ebp
0045816a: c2 04 00                                         ret 4

; VA 0x004011A0; PE offset 4512; length 39; SHA-256 c946d75a8e84b4c660af2c6b118aaef8aba4dbcbe0a5a4e6dd820282e646e700
004011a0: 55                                               push ebp
004011a1: 8b ec                                            mov ebp, esp
004011a3: 8b 45 10                                         mov eax, dword ptr [ebp + 0x10]
004011a6: 83 e8 01                                         sub eax, 1
004011a9: 89 45 10                                         mov dword ptr [ebp + 0x10], eax
004011ac: 83 7d 10 00                                      cmp dword ptr [ebp + 0x10], 0
004011b0: 7c 11                                            jl 0x4011c3
004011b2: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
004011b5: ff 55 14                                         call dword ptr [ebp + 0x14]
004011b8: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
004011bb: 03 4d 0c                                         add ecx, dword ptr [ebp + 0xc]
004011be: 89 4d 08                                         mov dword ptr [ebp + 8], ecx
004011c1: eb e0                                            jmp 0x4011a3
004011c3: 5d                                               pop ebp
004011c4: c2 10 00                                         ret 0x10

; VA 0x00459940; PE offset 366912; length 22; SHA-256 90c0aa7faa40cddbc21c573c8301978496fb82ff811a68c2c28dd6a094a55521
00459940: 55                                               push ebp
00459941: 8b ec                                            mov ebp, esp
00459943: 51                                               push ecx
00459944: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00459947: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0045994a: e8 11 00 00 00                                   call 0x459960
0045994f: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00459952: 8b e5                                            mov esp, ebp
00459954: 5d                                               pop ebp
00459955: c3                                               ret

; VA 0x00459960; PE offset 366944; length 43; SHA-256 31c8e2ae1f03c23b1efcfa0193f266730cd9070163f6892d7cabe3926c5cb579
00459960: 55                                               push ebp
00459961: 8b ec                                            mov ebp, esp
00459963: 51                                               push ecx
00459964: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00459967: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0045996a: e8 11 82 fa ff                                   call 0x401b80
0045996f: 68 b0 00 44 00                                   push 0x4400b0
00459974: 6a 04                                            push 4
00459976: 6a 18                                            push 0x18
00459978: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0045997b: 83 c0 20                                         add eax, 0x20
0045997e: 50                                               push eax
0045997f: e8 1c 78 fa ff                                   call 0x4011a0
00459984: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00459987: 8b e5                                            mov esp, ebp
00459989: 5d                                               pop ebp
0045998a: c3                                               ret

; VA 0x004400B0; PE offset 262320; length 25; SHA-256 1e4ae53f1441f11b973d8e31e984ffbf23a0986f1965b869f4afc628fead2a05
004400b0: 55                                               push ebp
004400b1: 8b ec                                            mov ebp, esp
004400b3: 51                                               push ecx
004400b4: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
004400b7: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
004400ba: 83 c1 08                                         add ecx, 8
004400bd: e8 0e 21 04 00                                   call 0x4821d0
004400c2: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
004400c5: 8b e5                                            mov esp, ebp
004400c7: 5d                                               pop ebp
004400c8: c3                                               ret

; VA 0x004821D0; PE offset 532944; length 14; SHA-256 befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c
004821d0: 55                                               push ebp
004821d1: 8b ec                                            mov ebp, esp
004821d3: 51                                               push ecx
004821d4: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
004821d7: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
004821da: 8b e5                                            mov esp, ebp
004821dc: 5d                                               pop ebp
004821dd: c3                                               ret

; VA 0x004135A0; PE offset 79264; length 29; SHA-256 928c42c9f8e5f5ba1a08043a6e57ea8c4fda98c5fd4268074d7cd464bd6bf7d1
004135a0: 55                                               push ebp
004135a1: 8b ec                                            mov ebp, esp
004135a3: 51                                               push ecx
004135a4: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
004135a7: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
004135aa: 6b c0 18                                         imul eax, eax, 0x18
004135ad: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
004135b0: 8b 55 0c                                         mov edx, dword ptr [ebp + 0xc]
004135b3: 89 54 01 20                                      mov dword ptr [ecx + eax + 0x20], edx
004135b7: 8b e5                                            mov esp, ebp
004135b9: 5d                                               pop ebp
004135ba: c2 08 00                                         ret 8

; VA 0x004135C0; PE offset 79296; length 57; SHA-256 c1ea3714d3563268d58297b1021f445ab22ed71f3e812c19770affc860847a44
004135c0: 55                                               push ebp
004135c1: 8b ec                                            mov ebp, esp
004135c3: 51                                               push ecx
004135c4: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
004135c7: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
004135ca: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
004135cd: 89 08                                            mov dword ptr [eax], ecx
004135cf: 8b 55 0c                                         mov edx, dword ptr [ebp + 0xc]
004135d2: 89 50 04                                         mov dword ptr [eax + 4], edx
004135d5: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
004135d8: 8b 4d 10                                         mov ecx, dword ptr [ebp + 0x10]
004135db: 89 48 08                                         mov dword ptr [eax + 8], ecx
004135de: 8b 55 14                                         mov edx, dword ptr [ebp + 0x14]
004135e1: 89 50 0c                                         mov dword ptr [eax + 0xc], edx
004135e4: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
004135e7: 8b 4d 18                                         mov ecx, dword ptr [ebp + 0x18]
004135ea: 89 48 10                                         mov dword ptr [eax + 0x10], ecx
004135ed: 8b 55 1c                                         mov edx, dword ptr [ebp + 0x1c]
004135f0: 89 50 14                                         mov dword ptr [eax + 0x14], edx
004135f3: 8b e5                                            mov esp, ebp
004135f5: 5d                                               pop ebp
004135f6: c2 18 00                                         ret 0x18

; VA 0x00459B00; PE offset 367360; length 28; SHA-256 1da575a340082503f2abfa3d9a2a5d642bf5450b6c780abdde10a109613d5c74
00459b00: 55                                               push ebp
00459b01: 8b ec                                            mov ebp, esp
00459b03: 51                                               push ecx
00459b04: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00459b07: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00459b0a: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
00459b0d: 89 48 18                                         mov dword ptr [eax + 0x18], ecx
00459b10: 8b 55 0c                                         mov edx, dword ptr [ebp + 0xc]
00459b13: 89 50 1c                                         mov dword ptr [eax + 0x1c], edx
00459b16: 8b e5                                            mov esp, ebp
00459b18: 5d                                               pop ebp
00459b19: c2 08 00                                         ret 8

; VA 0x00401B80; PE offset 7040; length 14; SHA-256 befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c
00401b80: 55                                               push ebp
00401b81: 8b ec                                            mov ebp, esp
00401b83: 51                                               push ecx
00401b84: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00401b87: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00401b8a: 8b e5                                            mov esp, ebp
00401b8c: 5d                                               pop ebp
00401b8d: c3                                               ret

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

; VA 0x00413560; PE offset 79200; length 59; SHA-256 e10637d1d1606b2cd36380cca7f81c8917aa7893dc17ffcb46e440a2bc70f821
00413560: 55                                               push ebp
00413561: 8b ec                                            mov ebp, esp
00413563: 51                                               push ecx
00413564: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00413567: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
0041356a: 6b c0 18                                         imul eax, eax, 0x18
0041356d: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
00413570: 8b 55 0c                                         mov edx, dword ptr [ebp + 0xc]
00413573: 89 54 01 28                                      mov dword ptr [ecx + eax + 0x28], edx
00413577: 8b 55 10                                         mov edx, dword ptr [ebp + 0x10]
0041357a: 89 54 01 2c                                      mov dword ptr [ecx + eax + 0x2c], edx
0041357e: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
00413581: 6b c0 18                                         imul eax, eax, 0x18
00413584: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
00413587: 8b 55 14                                         mov edx, dword ptr [ebp + 0x14]
0041358a: 89 54 01 30                                      mov dword ptr [ecx + eax + 0x30], edx
0041358e: 8b 55 18                                         mov edx, dword ptr [ebp + 0x18]
00413591: 89 54 01 34                                      mov dword ptr [ecx + eax + 0x34], edx
00413595: 8b e5                                            mov esp, ebp
00413597: 5d                                               pop ebp
00413598: c2 14 00                                         ret 0x14
