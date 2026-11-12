; Executable SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f

; VA 0x0044A050; PE offset 303184; length 128; SHA-256 2980ff59bd5433cc1c6be17943ee79716f080b010344036cf30c9f1cbcf3c056
0044a050: 55                                               push ebp
0044a051: 8b ec                                            mov ebp, esp
0044a053: 83 ec 0c                                         sub esp, 0xc
0044a056: 89 4d f4                                         mov dword ptr [ebp - 0xc], ecx
0044a059: 8b 4d f4                                         mov ecx, dword ptr [ebp - 0xc]
0044a05c: 83 c1 04                                         add ecx, 4
0044a05f: e8 fc 66 fe ff                                   call 0x430760
0044a064: 89 45 fc                                         mov dword ptr [ebp - 4], eax
0044a067: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0044a06a: 50                                               push eax
0044a06b: 8b 4d f4                                         mov ecx, dword ptr [ebp - 0xc]
0044a06e: 83 c1 04                                         add ecx, 4
0044a071: e8 0a 67 fe ff                                   call 0x430780
0044a076: 25 ff 00 00 00                                   and eax, 0xff
0044a07b: 85 c0                                            test eax, eax
0044a07d: 75 4d                                            jne 0x44a0cc
0044a07f: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0044a082: 83 79 08 00                                      cmp dword ptr [ecx + 8], 0
0044a086: 75 39                                            jne 0x44a0c1
0044a088: 8b 55 fc                                         mov edx, dword ptr [ebp - 4]
0044a08b: 89 55 f8                                         mov dword ptr [ebp - 8], edx
0044a08e: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0044a091: 8b 48 0c                                         mov ecx, dword ptr [eax + 0xc]
0044a094: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
0044a097: 8b 55 f8                                         mov edx, dword ptr [ebp - 8]
0044a09a: 8b 02                                            mov eax, dword ptr [edx]
0044a09c: 50                                               push eax
0044a09d: 8b 4d f4                                         mov ecx, dword ptr [ebp - 0xc]
0044a0a0: 8b 11                                            mov edx, dword ptr [ecx]
0044a0a2: 8b 4d f4                                         mov ecx, dword ptr [ebp - 0xc]
0044a0a5: ff 52 10                                         call dword ptr [edx + 0x10]
0044a0a8: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
0044a0ab: e8 50 1d 00 00                                   call 0x44be00
0044a0b0: 8b 45 f4                                         mov eax, dword ptr [ebp - 0xc]
0044a0b3: 8b 48 2c                                         mov ecx, dword ptr [eax + 0x2c]
0044a0b6: 83 e9 01                                         sub ecx, 1
0044a0b9: 8b 55 f4                                         mov edx, dword ptr [ebp - 0xc]
0044a0bc: 89 4a 2c                                         mov dword ptr [edx + 0x2c], ecx
0044a0bf: eb 09                                            jmp 0x44a0ca
0044a0c1: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0044a0c4: 8b 48 0c                                         mov ecx, dword ptr [eax + 0xc]
0044a0c7: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
0044a0ca: eb 9b                                            jmp 0x44a067
0044a0cc: 8b e5                                            mov esp, ebp
0044a0ce: 5d                                               pop ebp
0044a0cf: c3                                               ret

; VA 0x00485380; PE offset 545664; length 119; SHA-256 17521df7dddb9bf7b35758205af9f4ad60b088d24dec346d6d7471be42ef2716
00485380: 55                                               push ebp
00485381: 8b ec                                            mov ebp, esp
00485383: 83 ec 08                                         sub esp, 8
00485386: 89 4d f8                                         mov dword ptr [ebp - 8], ecx
00485389: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
0048538c: e8 bf 4c fc ff                                   call 0x44a050
00485391: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
00485394: 83 c1 04                                         add ecx, 4
00485397: e8 c4 b3 fa ff                                   call 0x430760
0048539c: 89 45 fc                                         mov dword ptr [ebp - 4], eax
0048539f: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
004853a2: 50                                               push eax
004853a3: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
004853a6: 83 c1 04                                         add ecx, 4
004853a9: e8 d2 b3 fa ff                                   call 0x430780
004853ae: 25 ff 00 00 00                                   and eax, 0xff
004853b3: 85 c0                                            test eax, eax
004853b5: 75 3c                                            jne 0x4853f3
004853b7: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
004853ba: 83 39 00                                         cmp dword ptr [ecx], 0
004853bd: 74 29                                            je 0x4853e8
004853bf: 8b 55 fc                                         mov edx, dword ptr [ebp - 4]
004853c2: 8b 02                                            mov eax, dword ptr [edx]
004853c4: 50                                               push eax
004853c5: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
004853c8: 8b 11                                            mov edx, dword ptr [ecx]
004853ca: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
004853cd: ff 52 10                                         call dword ptr [edx + 0x10]
004853d0: 8b 45 f8                                         mov eax, dword ptr [ebp - 8]
004853d3: 8b 48 2c                                         mov ecx, dword ptr [eax + 0x2c]
004853d6: 83 e9 01                                         sub ecx, 1
004853d9: 8b 55 f8                                         mov edx, dword ptr [ebp - 8]
004853dc: 89 4a 2c                                         mov dword ptr [edx + 0x2c], ecx
004853df: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
004853e2: c7 00 00 00 00 00                                mov dword ptr [eax], 0
004853e8: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
004853eb: 8b 51 0c                                         mov edx, dword ptr [ecx + 0xc]
004853ee: 89 55 fc                                         mov dword ptr [ebp - 4], edx
004853f1: eb ac                                            jmp 0x48539f
004853f3: 8b e5                                            mov esp, ebp
004853f5: 5d                                               pop ebp
004853f6: c3                                               ret

; VA 0x00485400; PE offset 545792; length 108; SHA-256 7f51d37fb8bbb35d412d659e1520a925aee38497aa68f57aee85f028a4645935
00485400: 55                                               push ebp
00485401: 8b ec                                            mov ebp, esp
00485403: 83 ec 08                                         sub esp, 8
00485406: 89 4d f8                                         mov dword ptr [ebp - 8], ecx
00485409: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
0048540c: e8 3f 4c fc ff                                   call 0x44a050
00485411: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
00485414: 83 c1 04                                         add ecx, 4
00485417: e8 44 b3 fa ff                                   call 0x430760
0048541c: 89 45 fc                                         mov dword ptr [ebp - 4], eax
0048541f: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00485422: 50                                               push eax
00485423: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
00485426: 83 c1 04                                         add ecx, 4
00485429: e8 52 b3 fa ff                                   call 0x430780
0048542e: 25 ff 00 00 00                                   and eax, 0xff
00485433: 85 c0                                            test eax, eax
00485435: 75 31                                            jne 0x485468
00485437: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0048543a: 8b 51 04                                         mov edx, dword ptr [ecx + 4]
0048543d: 52                                               push edx
0048543e: 8b 45 f8                                         mov eax, dword ptr [ebp - 8]
00485441: 8b 10                                            mov edx, dword ptr [eax]
00485443: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
00485446: ff 52 0c                                         call dword ptr [edx + 0xc]
00485449: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0048544c: 89 01                                            mov dword ptr [ecx], eax
0048544e: 8b 55 f8                                         mov edx, dword ptr [ebp - 8]
00485451: 8b 42 2c                                         mov eax, dword ptr [edx + 0x2c]
00485454: 83 c0 01                                         add eax, 1
00485457: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
0048545a: 89 41 2c                                         mov dword ptr [ecx + 0x2c], eax
0048545d: 8b 55 fc                                         mov edx, dword ptr [ebp - 4]
00485460: 8b 42 0c                                         mov eax, dword ptr [edx + 0xc]
00485463: 89 45 fc                                         mov dword ptr [ebp - 4], eax
00485466: eb b7                                            jmp 0x48541f
00485468: 8b e5                                            mov esp, ebp
0048546a: 5d                                               pop ebp
0048546b: c3                                               ret

; VA 0x004854D0; PE offset 546000; length 82; SHA-256 e96bdc0739f627eb8cd5ccb9a9967b534585b126bcdac6b45ff370ca6337271f
004854d0: 55                                               push ebp
004854d1: 8b ec                                            mov ebp, esp
004854d3: 83 ec 08                                         sub esp, 8
004854d6: 89 4d f8                                         mov dword ptr [ebp - 8], ecx
004854d9: c7 45 fc 00 00 00 00                             mov dword ptr [ebp - 4], 0
004854e0: eb 09                                            jmp 0x4854eb
004854e2: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
004854e5: 83 c0 01                                         add eax, 1
004854e8: 89 45 fc                                         mov dword ptr [ebp - 4], eax
004854eb: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
004854ee: 83 c1 30                                         add ecx, 0x30
004854f1: e8 4a 11 f8 ff                                   call 0x406640
004854f6: 39 45 fc                                         cmp dword ptr [ebp - 4], eax
004854f9: 7d 1b                                            jge 0x485516
004854fb: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
004854fe: 51                                               push ecx
004854ff: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
00485502: 83 c1 30                                         add ecx, 0x30
00485505: e8 96 b2 fa ff                                   call 0x4307a0
0048550a: c7 80 80 00 00 00 00 00 00 00                    mov dword ptr [eax + 0x80], 0
00485514: eb cc                                            jmp 0x4854e2
00485516: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
00485519: e8 62 fe ff ff                                   call 0x485380
0048551e: 8b e5                                            mov esp, ebp
00485520: 5d                                               pop ebp
00485521: c3                                               ret

; VA 0x00485530; PE offset 546096; length 94; SHA-256 a5731f0116afc2a1d31429ae66fae6e2c457bd0790c99cfaf93ed081a5eec0d2
00485530: 55                                               push ebp
00485531: 8b ec                                            mov ebp, esp
00485533: 83 ec 08                                         sub esp, 8
00485536: 56                                               push esi
00485537: 89 4d f8                                         mov dword ptr [ebp - 8], ecx
0048553a: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
0048553d: e8 be fe ff ff                                   call 0x485400
00485542: c7 45 fc 00 00 00 00                             mov dword ptr [ebp - 4], 0
00485549: eb 09                                            jmp 0x485554
0048554b: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0048554e: 83 c0 01                                         add eax, 1
00485551: 89 45 fc                                         mov dword ptr [ebp - 4], eax
00485554: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
00485557: 83 c1 30                                         add ecx, 0x30
0048555a: e8 e1 10 f8 ff                                   call 0x406640
0048555f: 39 45 fc                                         cmp dword ptr [ebp - 4], eax
00485562: 7d 25                                            jge 0x485589
00485564: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
00485567: 51                                               push ecx
00485568: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
0048556b: e8 80 b1 fa ff                                   call 0x4306f0
00485570: 8b f0                                            mov esi, eax
00485572: 8b 55 fc                                         mov edx, dword ptr [ebp - 4]
00485575: 52                                               push edx
00485576: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
00485579: 83 c1 30                                         add ecx, 0x30
0048557c: e8 1f b2 fa ff                                   call 0x4307a0
00485581: 89 b0 80 00 00 00                                mov dword ptr [eax + 0x80], esi
00485587: eb c2                                            jmp 0x48554b
00485589: 5e                                               pop esi
0048558a: 8b e5                                            mov esp, ebp
0048558c: 5d                                               pop ebp
0048558d: c3                                               ret

; VA 0x00449FB0; PE offset 303024; length 157; SHA-256 d0846021a0b8ba1b65a98969ece55b4c5291a8118a85f405c7294c6e03ee4dd5
00449fb0: 55                                               push ebp
00449fb1: 8b ec                                            mov ebp, esp
00449fb3: 83 ec 08                                         sub esp, 8
00449fb6: 89 4d f8                                         mov dword ptr [ebp - 8], ecx
00449fb9: c7 45 fc 00 00 00 00                             mov dword ptr [ebp - 4], 0
00449fc0: eb 09                                            jmp 0x449fcb
00449fc2: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00449fc5: 83 c0 01                                         add eax, 1
00449fc8: 89 45 fc                                         mov dword ptr [ebp - 4], eax
00449fcb: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
00449fce: 83 c1 30                                         add ecx, 0x30
00449fd1: e8 6a c6 fb ff                                   call 0x406640
00449fd6: 39 45 fc                                         cmp dword ptr [ebp - 4], eax
00449fd9: 7d 66                                            jge 0x44a041
00449fdb: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
00449fde: 51                                               push ecx
00449fdf: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
00449fe2: 83 c1 30                                         add ecx, 0x30
00449fe5: e8 b6 67 fe ff                                   call 0x4307a0
00449fea: 33 d2                                            xor edx, edx
00449fec: 8a 90 84 00 00 00                                mov dl, byte ptr [eax + 0x84]
00449ff2: 85 d2                                            test edx, edx
00449ff4: 74 49                                            je 0x44a03f
00449ff6: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00449ff9: 50                                               push eax
00449ffa: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
00449ffd: 83 c1 30                                         add ecx, 0x30
0044a000: e8 9b 67 fe ff                                   call 0x4307a0
0044a005: 83 b8 80 00 00 00 00                             cmp dword ptr [eax + 0x80], 0
0044a00c: 74 1b                                            je 0x44a029
0044a00e: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0044a011: 51                                               push ecx
0044a012: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
0044a015: 83 c1 30                                         add ecx, 0x30
0044a018: e8 83 67 fe ff                                   call 0x4307a0
0044a01d: 8b 90 80 00 00 00                                mov edx, dword ptr [eax + 0x80]
0044a023: 83 7a 08 00                                      cmp dword ptr [edx + 8], 0
0044a027: 75 16                                            jne 0x44a03f
0044a029: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0044a02c: 50                                               push eax
0044a02d: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
0044a030: 83 c1 30                                         add ecx, 0x30
0044a033: e8 68 67 fe ff                                   call 0x4307a0
0044a038: c6 80 84 00 00 00 00                             mov byte ptr [eax + 0x84], 0
0044a03f: eb 81                                            jmp 0x449fc2
0044a041: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
0044a044: e8 07 00 00 00                                   call 0x44a050
0044a049: 8b e5                                            mov esp, ebp
0044a04b: 5d                                               pop ebp
0044a04c: c3                                               ret

; VA 0x0044BE00; PE offset 310784; length 83; SHA-256 04b91154d74e1bdbdf66c92beed1f12361b9786019d4cd960c75e373e8b40f6e
0044be00: 55                                               push ebp
0044be01: 8b ec                                            mov ebp, esp
0044be03: 83 ec 10                                         sub esp, 0x10
0044be06: 89 4d f4                                         mov dword ptr [ebp - 0xc], ecx
0044be09: 8b 45 f4                                         mov eax, dword ptr [ebp - 0xc]
0044be0c: 8b 48 10                                         mov ecx, dword ptr [eax + 0x10]
0044be0f: 8b 55 f4                                         mov edx, dword ptr [ebp - 0xc]
0044be12: 8b 42 0c                                         mov eax, dword ptr [edx + 0xc]
0044be15: 89 41 0c                                         mov dword ptr [ecx + 0xc], eax
0044be18: 8b 4d f4                                         mov ecx, dword ptr [ebp - 0xc]
0044be1b: 8b 51 0c                                         mov edx, dword ptr [ecx + 0xc]
0044be1e: 8b 45 f4                                         mov eax, dword ptr [ebp - 0xc]
0044be21: 8b 48 10                                         mov ecx, dword ptr [eax + 0x10]
0044be24: 89 4a 10                                         mov dword ptr [edx + 0x10], ecx
0044be27: 8b 55 f4                                         mov edx, dword ptr [ebp - 0xc]
0044be2a: 89 55 f8                                         mov dword ptr [ebp - 8], edx
0044be2d: 8b 45 f8                                         mov eax, dword ptr [ebp - 8]
0044be30: 89 45 fc                                         mov dword ptr [ebp - 4], eax
0044be33: 83 7d fc 00                                      cmp dword ptr [ebp - 4], 0
0044be37: 74 0f                                            je 0x44be48
0044be39: 6a 01                                            push 1
0044be3b: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0044be3e: e8 1d 02 00 00                                   call 0x44c060
0044be43: 89 45 f0                                         mov dword ptr [ebp - 0x10], eax
0044be46: eb 07                                            jmp 0x44be4f
0044be48: c7 45 f0 00 00 00 00                             mov dword ptr [ebp - 0x10], 0
0044be4f: 8b e5                                            mov esp, ebp
0044be51: 5d                                               pop ebp
0044be52: c3                                               ret

; VA 0x0044C060; PE offset 311392; length 46; SHA-256 2e436a1f2aef81ceb70d00bd612286e23e46a5916c0e124289f7357dfc7786ac
0044c060: 55                                               push ebp
0044c061: 8b ec                                            mov ebp, esp
0044c063: 51                                               push ecx
0044c064: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
0044c067: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0044c06a: e8 21 00 00 00                                   call 0x44c090
0044c06f: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
0044c072: 83 e0 01                                         and eax, 1
0044c075: 85 c0                                            test eax, eax
0044c077: 74 0c                                            je 0x44c085
0044c079: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
0044c07c: 51                                               push ecx
0044c07d: e8 bb 00 05 00                                   call 0x49c13d
0044c082: 83 c4 04                                         add esp, 4
0044c085: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0044c088: 8b e5                                            mov esp, ebp
0044c08a: 5d                                               pop ebp
0044c08b: c2 04 00                                         ret 4

; VA 0x0044C090; PE offset 311440; length 11; SHA-256 012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5
0044c090: 55                                               push ebp
0044c091: 8b ec                                            mov ebp, esp
0044c093: 51                                               push ecx
0044c094: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
0044c097: 8b e5                                            mov esp, ebp
0044c099: 5d                                               pop ebp
0044c09a: c3                                               ret

; VA 0x00430760; PE offset 198496; length 17; SHA-256 a26cd3064709e57ee483cfbbb2753f8da501718ce4ced1e834a5123dd2fcaa3d
00430760: 55                                               push ebp
00430761: 8b ec                                            mov ebp, esp
00430763: 51                                               push ecx
00430764: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00430767: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0043076a: 8b 40 0c                                         mov eax, dword ptr [eax + 0xc]
0043076d: 8b e5                                            mov esp, ebp
0043076f: 5d                                               pop ebp
00430770: c3                                               ret

; VA 0x00430780; PE offset 198528; length 29; SHA-256 279bb61281a416389bf3313e5358634b5d927b0d349db8573597cecd1ad504d0
00430780: 55                                               push ebp
00430781: 8b ec                                            mov ebp, esp
00430783: 51                                               push ecx
00430784: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00430787: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0043078a: 83 c0 14                                         add eax, 0x14
0043078d: 33 c9                                            xor ecx, ecx
0043078f: 39 45 08                                         cmp dword ptr [ebp + 8], eax
00430792: 0f 94 c1                                         sete cl
00430795: 8a c1                                            mov al, cl
00430797: 8b e5                                            mov esp, ebp
00430799: 5d                                               pop ebp
0043079a: c2 04 00                                         ret 4

; VA 0x004306F0; PE offset 198384; length 106; SHA-256 c9ac76eb0fdf7ee30c39be22275c04a317e44ba8e990c4aa6d4030d792b31b72
004306f0: 55                                               push ebp
004306f1: 8b ec                                            mov ebp, esp
004306f3: 83 ec 08                                         sub esp, 8
004306f6: 89 4d f8                                         mov dword ptr [ebp - 8], ecx
004306f9: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
004306fc: 83 c1 04                                         add ecx, 4
004306ff: e8 5c 00 00 00                                   call 0x430760
00430704: 89 45 fc                                         mov dword ptr [ebp - 4], eax
00430707: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0043070a: 50                                               push eax
0043070b: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
0043070e: 83 c1 04                                         add ecx, 4
00430711: e8 6a 00 00 00                                   call 0x430780
00430716: 25 ff 00 00 00                                   and eax, 0xff
0043071b: 85 c0                                            test eax, eax
0043071d: 75 16                                            jne 0x430735
0043071f: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
00430722: 8b 51 04                                         mov edx, dword ptr [ecx + 4]
00430725: 3b 55 08                                         cmp edx, dword ptr [ebp + 8]
00430728: 74 0b                                            je 0x430735
0043072a: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0043072d: 8b 48 0c                                         mov ecx, dword ptr [eax + 0xc]
00430730: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00430733: eb d2                                            jmp 0x430707
00430735: 8b 55 fc                                         mov edx, dword ptr [ebp - 4]
00430738: 52                                               push edx
00430739: 8b 4d f8                                         mov ecx, dword ptr [ebp - 8]
0043073c: 83 c1 04                                         add ecx, 4
0043073f: e8 3c 00 00 00                                   call 0x430780
00430744: 25 ff 00 00 00                                   and eax, 0xff
00430749: 85 c0                                            test eax, eax
0043074b: 74 04                                            je 0x430751
0043074d: 33 c0                                            xor eax, eax
0043074f: eb 03                                            jmp 0x430754
00430751: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
00430754: 8b e5                                            mov esp, ebp
00430756: 5d                                               pop ebp
00430757: c2 04 00                                         ret 4

; VA 0x004307A0; PE offset 198560; length 30; SHA-256 3f6d2d5c7711f92761fa78394636a1ea22836c94e313ce6fc2378ef7e44d1c34
004307a0: 55                                               push ebp
004307a1: 8b ec                                            mov ebp, esp
004307a3: 51                                               push ecx
004307a4: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
004307a7: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
004307aa: 69 c0 88 00 00 00                                imul eax, eax, 0x88
004307b0: 8b 4d fc                                         mov ecx, dword ptr [ebp - 4]
004307b3: 8b 51 0c                                         mov edx, dword ptr [ecx + 0xc]
004307b6: 03 c2                                            add eax, edx
004307b8: 8b e5                                            mov esp, ebp
004307ba: 5d                                               pop ebp
004307bb: c2 04 00                                         ret 4

; VA 0x00406640; PE offset 26176; length 16; SHA-256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936
00406640: 55                                               push ebp
00406641: 8b ec                                            mov ebp, esp
00406643: 51                                               push ecx
00406644: 89 4d fc                                         mov dword ptr [ebp - 4], ecx
00406647: 8b 45 fc                                         mov eax, dword ptr [ebp - 4]
0040664a: 8b 00                                            mov eax, dword ptr [eax]
0040664c: 8b e5                                            mov esp, ebp
0040664e: 5d                                               pop ebp
0040664f: c3                                               ret
