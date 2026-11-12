; Executable SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f

; VA 0x0043F510; PE offset 259344; length 1308; SHA-256 a45ce7e60823fc491696e2756b7f7c5966e5c139a60bbfe169b57c140c3f4c07
0043f510: 55                                               push ebp
0043f511: 8b ec                                            mov ebp, esp
0043f513: 6a ff                                            push -1
0043f515: 68 97 be 4a 00                                   push 0x4abe97
0043f51a: 64 a1 00 00 00 00                                mov eax, dword ptr fs:[0]
0043f520: 50                                               push eax
0043f521: 64 89 25 00 00 00 00                             mov dword ptr fs:[0], esp
0043f528: 81 ec 3c 01 00 00                                sub esp, 0x13c
0043f52e: 56                                               push esi
0043f52f: 57                                               push edi
0043f530: 83 7d 0c 00                                      cmp dword ptr [ebp + 0xc], 0
0043f534: 75 10                                            jne 0x43f546
0043f536: c7 45 e0 07 00 00 00                             mov dword ptr [ebp - 0x20], 7
0043f53d: c7 45 e4 28 15 4b 00                             mov dword ptr [ebp - 0x1c], 0x4b1528
0043f544: eb 0e                                            jmp 0x43f554
0043f546: c7 45 e0 09 00 00 00                             mov dword ptr [ebp - 0x20], 9
0043f54d: c7 45 e4 5c 15 4b 00                             mov dword ptr [ebp - 0x1c], 0x4b155c
0043f554: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
0043f557: db 45 e0                                         fild dword ptr [ebp - 0x20]
0043f55a: dc 35 f8 03 4b 00                                fdiv qword ptr [0x4b03f8]
0043f560: dc 68 08                                         fsubr qword ptr [eax + 8]
0043f563: 83 ec 08                                         sub esp, 8
0043f566: dd 1c 24                                         fstp qword ptr [esp]
0043f569: b9 10 6a 4c 00                                   mov ecx, 0x4c6a10
0043f56e: e8 dd e9 fc ff                                   call 0x40df50
0043f573: e8 34 d0 05 00                                   call 0x49c5ac
0043f578: 89 45 e8                                         mov dword ptr [ebp - 0x18], eax
0043f57b: db 45 e0                                         fild dword ptr [ebp - 0x20]
0043f57e: dc 35 f8 03 4b 00                                fdiv qword ptr [0x4b03f8]
0043f584: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
0043f587: dc 29                                            fsubr qword ptr [ecx]
0043f589: 83 ec 08                                         sub esp, 8
0043f58c: dd 1c 24                                         fstp qword ptr [esp]
0043f58f: b9 10 6a 4c 00                                   mov ecx, 0x4c6a10
0043f594: e8 27 e9 fc ff                                   call 0x40dec0
0043f599: e8 0e d0 05 00                                   call 0x49c5ac
0043f59e: 89 45 f0                                         mov dword ptr [ebp - 0x10], eax
0043f5a1: 8b 55 e8                                         mov edx, dword ptr [ebp - 0x18]
0043f5a4: 89 55 dc                                         mov dword ptr [ebp - 0x24], edx
0043f5a7: c7 45 d8 00 00 00 00                             mov dword ptr [ebp - 0x28], 0
0043f5ae: eb 09                                            jmp 0x43f5b9
0043f5b0: 8b 45 d8                                         mov eax, dword ptr [ebp - 0x28]
0043f5b3: 83 c0 01                                         add eax, 1
0043f5b6: 89 45 d8                                         mov dword ptr [ebp - 0x28], eax
0043f5b9: 8b 4d d8                                         mov ecx, dword ptr [ebp - 0x28]
0043f5bc: 3b 4d e0                                         cmp ecx, dword ptr [ebp - 0x20]
0043f5bf: 0f 8d 6b 02 00 00                                jge 0x43f830
0043f5c5: 8b 55 f0                                         mov edx, dword ptr [ebp - 0x10]
0043f5c8: 89 55 bc                                         mov dword ptr [ebp - 0x44], edx
0043f5cb: c7 45 b8 00 00 00 00                             mov dword ptr [ebp - 0x48], 0
0043f5d2: eb 09                                            jmp 0x43f5dd
0043f5d4: 8b 45 b8                                         mov eax, dword ptr [ebp - 0x48]
0043f5d7: 83 c0 01                                         add eax, 1
0043f5da: 89 45 b8                                         mov dword ptr [ebp - 0x48], eax
0043f5dd: 8b 4d b8                                         mov ecx, dword ptr [ebp - 0x48]
0043f5e0: 3b 4d e0                                         cmp ecx, dword ptr [ebp - 0x20]
0043f5e3: 0f 8d 28 02 00 00                                jge 0x43f811
0043f5e9: 8b 55 e4                                         mov edx, dword ptr [ebp - 0x1c]
0043f5ec: 33 c0                                            xor eax, eax
0043f5ee: 8a 02                                            mov al, byte ptr [edx]
0043f5f0: 8b 4d e4                                         mov ecx, dword ptr [ebp - 0x1c]
0043f5f3: 83 c1 01                                         add ecx, 1
0043f5f6: 89 4d e4                                         mov dword ptr [ebp - 0x1c], ecx
0043f5f9: 85 c0                                            test eax, eax
0043f5fb: 0f 84 fd 01 00 00                                je 0x43f7fe
0043f601: 8b 55 dc                                         mov edx, dword ptr [ebp - 0x24]
0043f604: 52                                               push edx
0043f605: 8b 45 bc                                         mov eax, dword ptr [ebp - 0x44]
0043f608: 50                                               push eax
0043f609: b9 10 6a 4c 00                                   mov ecx, 0x4c6a10
0043f60e: e8 4d e8 fc ff                                   call 0x40de60
0043f613: 89 45 b4                                         mov dword ptr [ebp - 0x4c], eax
0043f616: 8b 4d b4                                         mov ecx, dword ptr [ebp - 0x4c]
0043f619: 8b 11                                            mov edx, dword ptr [ecx]
0043f61b: 83 e2 02                                         and edx, 2
0043f61e: 85 d2                                            test edx, edx
0043f620: 0f 85 d8 01 00 00                                jne 0x43f7fe
0043f626: 8b 45 b4                                         mov eax, dword ptr [ebp - 0x4c]
0043f629: 8b 08                                            mov ecx, dword ptr [eax]
0043f62b: 81 e1 00 02 00 00                                and ecx, 0x200
0043f631: 85 c9                                            test ecx, ecx
0043f633: 0f 85 c5 01 00 00                                jne 0x43f7fe
0043f639: 8b 55 b4                                         mov edx, dword ptr [ebp - 0x4c]
0043f63c: 8b 02                                            mov eax, dword ptr [edx]
0043f63e: 25 00 08 00 00                                   and eax, 0x800
0043f643: 85 c0                                            test eax, eax
0043f645: 0f 85 b3 01 00 00                                jne 0x43f7fe
0043f64b: 8b 4d b4                                         mov ecx, dword ptr [ebp - 0x4c]
0043f64e: 8b 11                                            mov edx, dword ptr [ecx]
0043f650: 83 e2 01                                         and edx, 1
0043f653: 85 d2                                            test edx, edx
0043f655: 0f 84 9a 01 00 00                                je 0x43f7f5
0043f65b: 8b 45 bc                                         mov eax, dword ptr [ebp - 0x44]
0043f65e: 25 03 00 00 80                                   and eax, 0x80000003
0043f663: 79 05                                            jns 0x43f66a
0043f665: 48                                               dec eax
0043f666: 83 c8 fc                                         or eax, 0xfffffffc
0043f669: 40                                               inc eax
0043f66a: 85 c0                                            test eax, eax
0043f66c: 0f 85 83 01 00 00                                jne 0x43f7f5
0043f672: 8b 4d dc                                         mov ecx, dword ptr [ebp - 0x24]
0043f675: 81 e1 03 00 00 80                                and ecx, 0x80000003
0043f67b: 79 05                                            jns 0x43f682
0043f67d: 49                                               dec ecx
0043f67e: 83 c9 fc                                         or ecx, 0xfffffffc
0043f681: 41                                               inc ecx
0043f682: 85 c9                                            test ecx, ecx
0043f684: 0f 85 6b 01 00 00                                jne 0x43f7f5
0043f68a: 68 48 07 00 00                                   push 0x748
0043f68f: e8 14 cb 05 00                                   call 0x49c1a8
0043f694: 83 c4 04                                         add esp, 4
0043f697: 89 45 90                                         mov dword ptr [ebp - 0x70], eax
0043f69a: c7 45 fc 00 00 00 00                             mov dword ptr [ebp - 4], 0
0043f6a1: 83 7d 90 00                                      cmp dword ptr [ebp - 0x70], 0
0043f6a5: 74 14                                            je 0x43f6bb
0043f6a7: 8b 55 08                                         mov edx, dword ptr [ebp + 8]
0043f6aa: 52                                               push edx
0043f6ab: 8b 4d 90                                         mov ecx, dword ptr [ebp - 0x70]
0043f6ae: e8 ad d2 fc ff                                   call 0x40c960
0043f6b3: 89 85 c4 fe ff ff                                mov dword ptr [ebp - 0x13c], eax
0043f6b9: eb 0a                                            jmp 0x43f6c5
0043f6bb: c7 85 c4 fe ff ff 00 00 00 00                    mov dword ptr [ebp - 0x13c], 0
0043f6c5: 8b 85 c4 fe ff ff                                mov eax, dword ptr [ebp - 0x13c]
0043f6cb: 89 45 94                                         mov dword ptr [ebp - 0x6c], eax
0043f6ce: c7 45 fc ff ff ff ff                             mov dword ptr [ebp - 4], 0xffffffff
0043f6d5: 8b 4d 94                                         mov ecx, dword ptr [ebp - 0x6c]
0043f6d8: 89 4d 98                                         mov dword ptr [ebp - 0x68], ecx
0043f6db: 68 e1 7a 84 3f                                   push 0x3f847ae1
0043f6e0: 68 7b 14 ae 47                                   push 0x47ae147b
0043f6e5: 8b 4d 98                                         mov ecx, dword ptr [ebp - 0x68]
0043f6e8: e8 33 d2 fc ff                                   call 0x40c920
0043f6ed: 6a 00                                            push 0
0043f6ef: 8b 55 98                                         mov edx, dword ptr [ebp - 0x68]
0043f6f2: 8b 02                                            mov eax, dword ptr [edx]
0043f6f4: 8b 4d 98                                         mov ecx, dword ptr [ebp - 0x68]
0043f6f7: ff 50 3c                                         call dword ptr [eax + 0x3c]
0043f6fa: 6a 04                                            push 4
0043f6fc: 8b 4d 98                                         mov ecx, dword ptr [ebp - 0x68]
0043f6ff: e8 ec e4 fc ff                                   call 0x40dbf0
0043f704: 8b 75 08                                         mov esi, dword ptr [ebp + 8]
0043f707: b9 06 00 00 00                                   mov ecx, 6
0043f70c: 8d 7d 9c                                         lea edi, [ebp - 0x64]
0043f70f: f3 a5                                            rep movsd dword ptr es:[edi], dword ptr [esi]
0043f711: c7 45 fc 01 00 00 00                             mov dword ptr [ebp - 4], 1
0043f718: dd 45 ac                                         fld qword ptr [ebp - 0x54]
0043f71b: dc 05 f8 03 4b 00                                fadd qword ptr [0x4b03f8]
0043f721: dd 5d ac                                         fstp qword ptr [ebp - 0x54]
0043f724: dd 45 a4                                         fld qword ptr [ebp - 0x5c]
0043f727: dc 05 e8 03 4b 00                                fadd qword ptr [0x4b03e8]
0043f72d: dd 5d a4                                         fstp qword ptr [ebp - 0x5c]
0043f730: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
0043f733: 51                                               push ecx
0043f734: 8d 95 60 ff ff ff                                lea edx, [ebp - 0xa0]
0043f73a: 52                                               push edx
0043f73b: b9 10 6a 4c 00                                   mov ecx, 0x4c6a10
0043f740: e8 27 de ff ff                                   call 0x43d56c
0043f745: 50                                               push eax
0043f746: 8d 85 48 ff ff ff                                lea eax, [ebp - 0xb8]
0043f74c: 50                                               push eax
0043f74d: 8d 4d 9c                                         lea ecx, [ebp - 0x64]
0043f750: 51                                               push ecx
0043f751: 8d 95 78 ff ff ff                                lea edx, [ebp - 0x88]
0043f757: 52                                               push edx
0043f758: b9 10 6a 4c 00                                   mov ecx, 0x4c6a10
0043f75d: e8 0a de ff ff                                   call 0x43d56c
0043f762: 8b c8                                            mov ecx, eax
0043f764: e8 c7 20 fc ff                                   call 0x401830
0043f769: 50                                               push eax
0043f76a: 8b 45 98                                         mov eax, dword ptr [ebp - 0x68]
0043f76d: 8b 10                                            mov edx, dword ptr [eax]
0043f76f: 8b 4d 98                                         mov ecx, dword ptr [ebp - 0x68]
0043f772: ff 52 54                                         call dword ptr [edx + 0x54]
0043f775: 6a 48                                            push 0x48
0043f777: e8 2c ca 05 00                                   call 0x49c1a8
0043f77c: 83 c4 04                                         add esp, 4
0043f77f: 89 85 3c ff ff ff                                mov dword ptr [ebp - 0xc4], eax
0043f785: c6 45 fc 02                                      mov byte ptr [ebp - 4], 2
0043f789: 83 bd 3c ff ff ff 00                             cmp dword ptr [ebp - 0xc4], 0
0043f790: 74 1d                                            je 0x43f7af
0043f792: 6a 01                                            push 1
0043f794: 8b 45 98                                         mov eax, dword ptr [ebp - 0x68]
0043f797: 50                                               push eax
0043f798: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
0043f79b: 51                                               push ecx
0043f79c: 8b 8d 3c ff ff ff                                mov ecx, dword ptr [ebp - 0xc4]
0043f7a2: e8 d9 28 ff ff                                   call 0x432080
0043f7a7: 89 85 c0 fe ff ff                                mov dword ptr [ebp - 0x140], eax
0043f7ad: eb 0a                                            jmp 0x43f7b9
0043f7af: c7 85 c0 fe ff ff 00 00 00 00                    mov dword ptr [ebp - 0x140], 0
0043f7b9: 8b 95 c0 fe ff ff                                mov edx, dword ptr [ebp - 0x140]
0043f7bf: 89 95 40 ff ff ff                                mov dword ptr [ebp - 0xc0], edx
0043f7c5: c6 45 fc 01                                      mov byte ptr [ebp - 4], 1
0043f7c9: 8b 85 40 ff ff ff                                mov eax, dword ptr [ebp - 0xc0]
0043f7cf: 89 85 44 ff ff ff                                mov dword ptr [ebp - 0xbc], eax
0043f7d5: 8d 8d 44 ff ff ff                                lea ecx, [ebp - 0xbc]
0043f7db: 51                                               push ecx
0043f7dc: b9 a8 88 4c 00                                   mov ecx, 0x4c88a8
0043f7e1: e8 aa 20 fc ff                                   call 0x401890
0043f7e6: c7 45 fc ff ff ff ff                             mov dword ptr [ebp - 4], 0xffffffff
0043f7ed: 8d 4d 9c                                         lea ecx, [ebp - 0x64]
0043f7f0: e8 bb 46 fc ff                                   call 0x403eb0
0043f7f5: 8b 55 b4                                         mov edx, dword ptr [ebp - 0x4c]
0043f7f8: c7 02 00 00 00 00                                mov dword ptr [edx], 0
0043f7fe: 8b 45 bc                                         mov eax, dword ptr [ebp - 0x44]
0043f801: 83 c0 01                                         add eax, 1
0043f804: 25 ff 00 00 00                                   and eax, 0xff
0043f809: 89 45 bc                                         mov dword ptr [ebp - 0x44], eax
0043f80c: e9 c3 fd ff ff                                   jmp 0x43f5d4
0043f811: 8b 4d dc                                         mov ecx, dword ptr [ebp - 0x24]
0043f814: 83 c1 01                                         add ecx, 1
0043f817: 89 4d dc                                         mov dword ptr [ebp - 0x24], ecx
0043f81a: b9 10 6a 4c 00                                   mov ecx, 0x4c6a10
0043f81f: e8 7c c2 fc ff                                   call 0x40baa0
0043f824: 39 45 dc                                         cmp dword ptr [ebp - 0x24], eax
0043f827: 75 02                                            jne 0x43f82b
0043f829: eb 05                                            jmp 0x43f830
0043f82b: e9 80 fd ff ff                                   jmp 0x43f5b0
0043f830: 8b 55 e8                                         mov edx, dword ptr [ebp - 0x18]
0043f833: 03 55 e0                                         add edx, dword ptr [ebp - 0x20]
0043f836: 52                                               push edx
0043f837: 8b 45 f0                                         mov eax, dword ptr [ebp - 0x10]
0043f83a: 03 45 e0                                         add eax, dword ptr [ebp - 0x20]
0043f83d: 50                                               push eax
0043f83e: 8b 4d e8                                         mov ecx, dword ptr [ebp - 0x18]
0043f841: 51                                               push ecx
0043f842: 8b 55 f0                                         mov edx, dword ptr [ebp - 0x10]
0043f845: 52                                               push edx
0043f846: b9 10 6a 4c 00                                   mov ecx, 0x4c6a10
0043f84b: e8 51 b8 ff ff                                   call 0x43b0a1
0043f850: 8b 75 08                                         mov esi, dword ptr [ebp + 8]
0043f853: b9 06 00 00 00                                   mov ecx, 6
0043f858: 8d 7d c0                                         lea edi, [ebp - 0x40]
0043f85b: f3 a5                                            rep movsd dword ptr es:[edi], dword ptr [esi]
0043f85d: c7 45 fc 03 00 00 00                             mov dword ptr [ebp - 4], 3
0043f864: dd 45 c8                                         fld qword ptr [ebp - 0x38]
0043f867: dc 05 b0 15 4b 00                                fadd qword ptr [0x4b15b0]
0043f86d: dd 5d c8                                         fstp qword ptr [ebp - 0x38]
0043f870: dd 45 d0                                         fld qword ptr [ebp - 0x30]
0043f873: dc 05 e8 03 4b 00                                fadd qword ptr [0x4b03e8]
0043f879: dd 5d d0                                         fstp qword ptr [ebp - 0x30]
0043f87c: 68 58 01 00 00                                   push 0x158
0043f881: e8 22 c9 05 00                                   call 0x49c1a8
0043f886: 83 c4 04                                         add esp, 4
0043f889: 89 85 34 ff ff ff                                mov dword ptr [ebp - 0xcc], eax
0043f88f: c6 45 fc 04                                      mov byte ptr [ebp - 4], 4
0043f893: 83 bd 34 ff ff ff 00                             cmp dword ptr [ebp - 0xcc], 0
0043f89a: 74 13                                            je 0x43f8af
0043f89c: 8b 8d 34 ff ff ff                                mov ecx, dword ptr [ebp - 0xcc]
0043f8a2: e8 c9 2f fc ff                                   call 0x402870
0043f8a7: 89 85 bc fe ff ff                                mov dword ptr [ebp - 0x144], eax
0043f8ad: eb 0a                                            jmp 0x43f8b9
0043f8af: c7 85 bc fe ff ff 00 00 00 00                    mov dword ptr [ebp - 0x144], 0
0043f8b9: 8b 85 bc fe ff ff                                mov eax, dword ptr [ebp - 0x144]
0043f8bf: 89 85 38 ff ff ff                                mov dword ptr [ebp - 0xc8], eax
0043f8c5: c6 45 fc 03                                      mov byte ptr [ebp - 4], 3
0043f8c9: 8b 8d 38 ff ff ff                                mov ecx, dword ptr [ebp - 0xc8]
0043f8cf: 89 4d ec                                         mov dword ptr [ebp - 0x14], ecx
0043f8d2: 68 00 00 40 40                                   push 0x40400000
0043f8d7: 6a 00                                            push 0
0043f8d9: 68 00 00 40 40                                   push 0x40400000
0043f8de: 6a 00                                            push 0
0043f8e0: 68 00 e0 6f 40                                   push 0x406fe000
0043f8e5: 6a 00                                            push 0
0043f8e7: 8d 8d 1c ff ff ff                                lea ecx, [ebp - 0xe4]
0043f8ed: e8 fe 1e fc ff                                   call 0x4017f0
0043f8f2: 50                                               push eax
0043f8f3: 8b 4d ec                                         mov ecx, dword ptr [ebp - 0x14]
0043f8f6: 81 c1 b0 00 00 00                                add ecx, 0xb0
0043f8fc: e8 2f 01 00 00                                   call 0x43fa30
0043f901: 6a 64                                            push 0x64
0043f903: 8b 4d ec                                         mov ecx, dword ptr [ebp - 0x14]
0043f906: e8 d5 33 fc ff                                   call 0x402ce0
0043f90b: 8b 55 08                                         mov edx, dword ptr [ebp + 8]
0043f90e: 52                                               push edx
0043f90f: 8b 45 ec                                         mov eax, dword ptr [ebp - 0x14]
0043f912: 8b 10                                            mov edx, dword ptr [eax]
0043f914: 8b 4d ec                                         mov ecx, dword ptr [ebp - 0x14]
0043f917: ff 52 08                                         call dword ptr [edx + 8]
0043f91a: 68 00 00 e0 3f                                   push 0x3fe00000
0043f91f: 6a 00                                            push 0
0043f921: 68 00 00 00 40                                   push 0x40000000
0043f926: 6a 00                                            push 0
0043f928: 8b 4d ec                                         mov ecx, dword ptr [ebp - 0x14]
0043f92b: e8 50 34 fc ff                                   call 0x402d80
0043f930: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
0043f933: 50                                               push eax
0043f934: 8d 8d ec fe ff ff                                lea ecx, [ebp - 0x114]
0043f93a: 51                                               push ecx
0043f93b: b9 10 6a 4c 00                                   mov ecx, 0x4c6a10
0043f940: e8 27 dc ff ff                                   call 0x43d56c
0043f945: 50                                               push eax
0043f946: 8d 95 d4 fe ff ff                                lea edx, [ebp - 0x12c]
0043f94c: 52                                               push edx
0043f94d: 8d 45 c0                                         lea eax, [ebp - 0x40]
0043f950: 50                                               push eax
0043f951: 8d 8d 04 ff ff ff                                lea ecx, [ebp - 0xfc]
0043f957: 51                                               push ecx
0043f958: b9 10 6a 4c 00                                   mov ecx, 0x4c6a10
0043f95d: e8 0a dc ff ff                                   call 0x43d56c
0043f962: 8b c8                                            mov ecx, eax
0043f964: e8 c7 1e fc ff                                   call 0x401830
0043f969: 50                                               push eax
0043f96a: 8b 55 ec                                         mov edx, dword ptr [ebp - 0x14]
0043f96d: 8b 02                                            mov eax, dword ptr [edx]
0043f96f: 8b 4d ec                                         mov ecx, dword ptr [ebp - 0x14]
0043f972: ff 50 54                                         call dword ptr [eax + 0x54]
0043f975: 6a 01                                            push 1
0043f977: 8b 4d ec                                         mov ecx, dword ptr [ebp - 0x14]
0043f97a: e8 91 44 fc ff                                   call 0x403e10
0043f97f: 6a 07                                            push 7
0043f981: 8b 4d ec                                         mov ecx, dword ptr [ebp - 0x14]
0043f984: e8 37 34 fc ff                                   call 0x402dc0
0043f989: 8b 4d 14                                         mov ecx, dword ptr [ebp + 0x14]
0043f98c: 51                                               push ecx
0043f98d: 8b 55 10                                         mov edx, dword ptr [ebp + 0x10]
0043f990: 52                                               push edx
0043f991: 8b 45 ec                                         mov eax, dword ptr [ebp - 0x14]
0043f994: 8b 10                                            mov edx, dword ptr [eax]
0043f996: 8b 4d ec                                         mov ecx, dword ptr [ebp - 0x14]
0043f999: ff 52 2c                                         call dword ptr [edx + 0x2c]
0043f99c: 6a 48                                            push 0x48
0043f99e: e8 05 c8 05 00                                   call 0x49c1a8
0043f9a3: 83 c4 04                                         add esp, 4
0043f9a6: 89 85 c8 fe ff ff                                mov dword ptr [ebp - 0x138], eax
0043f9ac: c6 45 fc 05                                      mov byte ptr [ebp - 4], 5
0043f9b0: 83 bd c8 fe ff ff 00                             cmp dword ptr [ebp - 0x138], 0
0043f9b7: 74 1d                                            je 0x43f9d6
0043f9b9: 6a 01                                            push 1
0043f9bb: 8b 45 ec                                         mov eax, dword ptr [ebp - 0x14]
0043f9be: 50                                               push eax
0043f9bf: 8b 4d 08                                         mov ecx, dword ptr [ebp + 8]
0043f9c2: 51                                               push ecx
0043f9c3: 8b 8d c8 fe ff ff                                mov ecx, dword ptr [ebp - 0x138]
0043f9c9: e8 b2 26 ff ff                                   call 0x432080
0043f9ce: 89 85 b8 fe ff ff                                mov dword ptr [ebp - 0x148], eax
0043f9d4: eb 0a                                            jmp 0x43f9e0
0043f9d6: c7 85 b8 fe ff ff 00 00 00 00                    mov dword ptr [ebp - 0x148], 0
0043f9e0: 8b 95 b8 fe ff ff                                mov edx, dword ptr [ebp - 0x148]
0043f9e6: 89 95 cc fe ff ff                                mov dword ptr [ebp - 0x134], edx
0043f9ec: c6 45 fc 03                                      mov byte ptr [ebp - 4], 3
0043f9f0: 8b 85 cc fe ff ff                                mov eax, dword ptr [ebp - 0x134]
0043f9f6: 89 85 d0 fe ff ff                                mov dword ptr [ebp - 0x130], eax
0043f9fc: 8d 8d d0 fe ff ff                                lea ecx, [ebp - 0x130]
0043fa02: 51                                               push ecx
0043fa03: b9 a8 88 4c 00                                   mov ecx, 0x4c88a8
0043fa08: e8 83 1e fc ff                                   call 0x401890
0043fa0d: c7 45 fc ff ff ff ff                             mov dword ptr [ebp - 4], 0xffffffff
0043fa14: 8d 4d c0                                         lea ecx, [ebp - 0x40]
0043fa17: e8 94 44 fc ff                                   call 0x403eb0
0043fa1c: 8b 4d f4                                         mov ecx, dword ptr [ebp - 0xc]
0043fa1f: 64 89 0d 00 00 00 00                             mov dword ptr fs:[0], ecx
0043fa26: 5f                                               pop edi
0043fa27: 5e                                               pop esi
0043fa28: 8b e5                                            mov esp, ebp
0043fa2a: 5d                                               pop ebp
0043fa2b: c3                                               ret

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

; VA 0x00401830; PE offset 6192; length 94; SHA-256 6def4c8304bc663043e2b7b8fa17dd87218c10f868015bb5694902da9d90c3c4
00401830: 55                                               push ebp
00401831: 8b ec                                            mov ebp, esp
00401833: 83 ec 1c                                         sub esp, 0x1c
00401836: 56                                               push esi
00401837: 57                                               push edi
00401838: 89 4d e4                                         mov dword ptr [ebp - 0x1c], ecx
0040183b: 8b 45 e4                                         mov eax, dword ptr [ebp - 0x1c]
0040183e: 8b 4d 0c                                         mov ecx, dword ptr [ebp + 0xc]
00401841: dd 40 10                                         fld qword ptr [eax + 0x10]
00401844: dc 61 10                                         fsub qword ptr [ecx + 0x10]
00401847: 83 ec 08                                         sub esp, 8
0040184a: dd 1c 24                                         fstp qword ptr [esp]
0040184d: 8b 55 e4                                         mov edx, dword ptr [ebp - 0x1c]
00401850: 8b 45 0c                                         mov eax, dword ptr [ebp + 0xc]
00401853: dd 42 08                                         fld qword ptr [edx + 8]
00401856: dc 60 08                                         fsub qword ptr [eax + 8]
00401859: 83 ec 08                                         sub esp, 8
0040185c: dd 1c 24                                         fstp qword ptr [esp]
0040185f: 8b 4d e4                                         mov ecx, dword ptr [ebp - 0x1c]
00401862: 8b 55 0c                                         mov edx, dword ptr [ebp + 0xc]
00401865: dd 01                                            fld qword ptr [ecx]
00401867: dc 22                                            fsub qword ptr [edx]
00401869: 83 ec 08                                         sub esp, 8
0040186c: dd 1c 24                                         fstp qword ptr [esp]
0040186f: 8d 4d e8                                         lea ecx, [ebp - 0x18]
00401872: e8 79 ff ff ff                                   call 0x4017f0
00401877: 8b f0                                            mov esi, eax
00401879: b9 06 00 00 00                                   mov ecx, 6
0040187e: 8b 7d 08                                         mov edi, dword ptr [ebp + 8]
00401881: f3 a5                                            rep movsd dword ptr es:[edi], dword ptr [esi]
00401883: 8b 45 08                                         mov eax, dword ptr [ebp + 8]
00401886: 5f                                               pop edi
00401887: 5e                                               pop esi
00401888: 8b e5                                            mov esp, ebp
0040188a: 5d                                               pop ebp
0040188b: c2 08 00                                         ret 8

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

; data 0x004B0338: 00 00 00 00 00 00 00 00

; data 0x004B0698: 00 00 00 e0 8e 79 45 3e

; data 0x004B07A8: 00 00 00 00 00 00 70 40

; data 0x004B03E8: 00 00 00 00 00 00 24 40

; data 0x004B03F8: 00 00 00 00 00 00 00 40

; data 0x004B15B0: 00 00 00 00 00 00 44 40

; data 0x004B1528: 00 00 01 01 01 00 00 00 01 01 01 01 01 00 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 00 01 01 01 01 01 00 00 00 01 01 01 00 00

; data 0x004B155C: 00 00 00 01 01 01 00 00 00 00 01 01 01 01 01 01 01 00 00 01 01 01 01 01 01 01 00 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 01 00 01 01 01 01 01 01 01 00 00 01 01 01 01 01 01 01 00 00 00 00 01 01 01 00 00 00
