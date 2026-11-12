; Original x86 instruction bytes; static disassembly, not runtime evidence.
; Reference SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Decoder: Capstone 5.0.7, x86 32-bit; addresses are virtual addresses.

; VA 0x0040B3A0, file offset 0xB3A0, length 48, SHA-256 4d91ce609376caf9a4d1b52e8178680d6a4317cb979143c19204d8fd30d1fa5f
0040B3A0  55                              push ebp
0040B3A1  8b ec                           mov ebp, esp
0040B3A3  51                              push ecx
0040B3A4  89 4d fc                        mov dword ptr [ebp - 4], ecx
0040B3A7  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
0040B3AA  e8 21 5d ff ff                  call 0x4010d0
0040B3AF  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0040B3B2  c7 00 68 06 4b 00               mov dword ptr [eax], 0x4b0668
0040B3B8  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
0040B3BB  c7 41 14 1a 00 00 00            mov dword ptr [ecx + 0x14], 0x1a
0040B3C2  8b 55 fc                        mov edx, dword ptr [ebp - 4]
0040B3C5  c6 42 11 00                     mov byte ptr [edx + 0x11], 0
0040B3C9  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0040B3CC  8b e5                           mov esp, ebp
0040B3CE  5d                              pop ebp
0040B3CF  c3                              ret

; VA 0x0040B3EC, file offset 0xB3EC, length 1542, SHA-256 bcae2c0c12e932e925309cf5c18207c38cadc831ae92c237711c8d8d8bf7955e
0040B3EC  55                              push ebp
0040B3ED  8b ec                           mov ebp, esp
0040B3EF  6a ff                           push -1
0040B3F1  68 86 99 4a 00                  push 0x4a9986
0040B3F6  64 a1 00 00 00 00               mov eax, dword ptr fs:[0]
0040B3FC  50                              push eax
0040B3FD  64 89 25 00 00 00 00            mov dword ptr fs:[0], esp
0040B404  81 ec c8 01 00 00               sub esp, 0x1c8
0040B40A  56                              push esi
0040B40B  57                              push edi
0040B40C  89 8d 50 fe ff ff               mov dword ptr [ebp - 0x1b0], ecx
0040B412  8b 75 08                        mov esi, dword ptr [ebp + 8]
0040B415  83 c6 08                        add esi, 8
0040B418  b9 06 00 00 00                  mov ecx, 6
0040B41D  8d 7d c0                        lea edi, [ebp - 0x40]
0040B420  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
0040B422  c7 45 fc 00 00 00 00            mov dword ptr [ebp - 4], 0
0040B429  dd 45 c8                        fld qword ptr [ebp - 0x38]
0040B42C  dc 1d 38 03 4b 00               fcomp qword ptr [0x4b0338]
0040B432  df e0                           fnstsw ax
0040B434  f6 c4 01                        test ah, 1
0040B437  74 0e                           je 0x40b447
0040B439  c7 45 c8 00 00 00 00            mov dword ptr [ebp - 0x38], 0
0040B440  c7 45 cc 00 00 00 00            mov dword ptr [ebp - 0x34], 0
0040B447  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040B44C  e8 4f 06 00 00                  call 0x40baa0
0040B451  89 85 4c fe ff ff               mov dword ptr [ebp - 0x1b4], eax
0040B457  db 85 4c fe ff ff               fild dword ptr [ebp - 0x1b4]
0040B45D  dc 5d c8                        fcomp qword ptr [ebp - 0x38]
0040B460  df e0                           fnstsw ax
0040B462  f6 c4 01                        test ah, 1
0040B465  74 19                           je 0x40b480
0040B467  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040B46C  e8 2f 06 00 00                  call 0x40baa0
0040B471  89 85 48 fe ff ff               mov dword ptr [ebp - 0x1b8], eax
0040B477  db 85 48 fe ff ff               fild dword ptr [ebp - 0x1b8]
0040B47D  dd 5d c8                        fstp qword ptr [ebp - 0x38]
0040B480  b9 06 00 00 00                  mov ecx, 6
0040B485  8d 75 c0                        lea esi, [ebp - 0x40]
0040B488  8d 7d d8                        lea edi, [ebp - 0x28]
0040B48B  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
0040B48D  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0040B491  8b 45 08                        mov eax, dword ptr [ebp + 8]
0040B494  8b 48 44                        mov ecx, dword ptr [eax + 0x44]
0040B497  c1 e1 02                        shl ecx, 2
0040B49A  89 8d 44 fe ff ff               mov dword ptr [ebp - 0x1bc], ecx
0040B4A0  db 85 44 fe ff ff               fild dword ptr [ebp - 0x1bc]
0040B4A6  dc 45 e0                        fadd qword ptr [ebp - 0x20]
0040B4A9  dd 5d e0                        fstp qword ptr [ebp - 0x20]
0040B4AC  8b 55 08                        mov edx, dword ptr [ebp + 8]
0040B4AF  c6 42 28 00                     mov byte ptr [edx + 0x28], 0
0040B4B3  6a 00                           push 0
0040B4B5  a1 e0 32 4e 00                  mov eax, dword ptr [0x4e32e0]
0040B4BA  50                              push eax
0040B4BB  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040B4BE  e8 3d b5 ff ff                  call 0x406a00
0040B4C3  0f bf c8                        movsx ecx, ax
0040B4C6  51                              push ecx
0040B4C7  b9 08 6a 4c 00                  mov ecx, 0x4c6a08
0040B4CC  e8 8a 9e 03 00                  call 0x44535b
0040B4D1  8b 55 08                        mov edx, dword ptr [ebp + 8]
0040B4D4  83 7a 4c 03                     cmp dword ptr [edx + 0x4c], 3
0040B4D8  0f 84 bc 01 00 00               je 0x40b69a
0040B4DE  68 28 01 00 00                  push 0x128
0040B4E3  e8 c0 0c 09 00                  call 0x49c1a8
0040B4E8  83 c4 04                        add esp, 4
0040B4EB  89 85 78 ff ff ff               mov dword ptr [ebp - 0x88], eax
0040B4F1  c6 45 fc 02                     mov byte ptr [ebp - 4], 2
0040B4F5  83 bd 78 ff ff ff 00            cmp dword ptr [ebp - 0x88], 0
0040B4FC  74 13                           je 0x40b511
0040B4FE  8b 8d 78 ff ff ff               mov ecx, dword ptr [ebp - 0x88]
0040B504  e8 d7 63 ff ff                  call 0x4018e0
0040B509  89 85 40 fe ff ff               mov dword ptr [ebp - 0x1c0], eax
0040B50F  eb 0a                           jmp 0x40b51b
0040B511  c7 85 40 fe ff ff 00 00 00 00   mov dword ptr [ebp - 0x1c0], 0
0040B51B  8b 85 40 fe ff ff               mov eax, dword ptr [ebp - 0x1c0]
0040B521  89 85 7c ff ff ff               mov dword ptr [ebp - 0x84], eax
0040B527  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0040B52B  8b 8d 7c ff ff ff               mov ecx, dword ptr [ebp - 0x84]
0040B531  89 4d b8                        mov dword ptr [ebp - 0x48], ecx
0040B534  6a 06                           push 6
0040B536  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
0040B539  e8 72 66 ff ff                  call 0x401bb0
0040B53E  8d 55 c0                        lea edx, [ebp - 0x40]
0040B541  52                              push edx
0040B542  8b 45 b8                        mov eax, dword ptr [ebp - 0x48]
0040B545  8b 10                           mov edx, dword ptr [eax]
0040B547  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
0040B54A  ff 52 08                        call dword ptr [edx + 8]
0040B54D  68 00 00 e0 3f                  push 0x3fe00000
0040B552  6a 00                           push 0
0040B554  68 00 00 10 40                  push 0x40100000
0040B559  6a 00                           push 0
0040B55B  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
0040B55E  e8 8d 69 ff ff                  call 0x401ef0
0040B563  6a 05                           push 5
0040B565  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
0040B568  e8 e3 69 ff ff                  call 0x401f50
0040B56D  8d 45 c0                        lea eax, [ebp - 0x40]
0040B570  50                              push eax
0040B571  8d 8d 48 ff ff ff               lea ecx, [ebp - 0xb8]
0040B577  51                              push ecx
0040B578  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040B57D  e8 ea 1f 03 00                  call 0x43d56c
0040B582  50                              push eax
0040B583  8d 95 30 ff ff ff               lea edx, [ebp - 0xd0]
0040B589  52                              push edx
0040B58A  8d 45 d8                        lea eax, [ebp - 0x28]
0040B58D  50                              push eax
0040B58E  8d 8d 60 ff ff ff               lea ecx, [ebp - 0xa0]
0040B594  51                              push ecx
0040B595  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040B59A  e8 cd 1f 03 00                  call 0x43d56c
0040B59F  8b c8                           mov ecx, eax
0040B5A1  e8 8a 62 ff ff                  call 0x401830
0040B5A6  50                              push eax
0040B5A7  8b 55 b8                        mov edx, dword ptr [ebp - 0x48]
0040B5AA  8b 02                           mov eax, dword ptr [edx]
0040B5AC  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
0040B5AF  ff 50 54                        call dword ptr [eax + 0x54]
0040B5B2  dd 45 e0                        fld qword ptr [ebp - 0x20]
0040B5B5  e8 f2 0f 09 00                  call 0x49c5ac
0040B5BA  50                              push eax
0040B5BB  dd 45 d8                        fld qword ptr [ebp - 0x28]
0040B5BE  e8 e9 0f 09 00                  call 0x49c5ac
0040B5C3  50                              push eax
0040B5C4  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040B5C9  e8 72 89 ff ff                  call 0x403f40
0040B5CE  8b f0                           mov esi, eax
0040B5D0  b9 06 00 00 00                  mov ecx, 6
0040B5D5  8d 7d a0                        lea edi, [ebp - 0x60]
0040B5D8  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
0040B5DA  c7 45 bc 00 00 00 00            mov dword ptr [ebp - 0x44], 0
0040B5E1  eb 09                           jmp 0x40b5ec
0040B5E3  8b 4d bc                        mov ecx, dword ptr [ebp - 0x44]
0040B5E6  83 c1 01                        add ecx, 1
0040B5E9  89 4d bc                        mov dword ptr [ebp - 0x44], ecx
0040B5EC  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
0040B5EF  81 c1 b0 00 00 00               add ecx, 0xb0
0040B5F5  e8 d6 62 ff ff                  call 0x4018d0
0040B5FA  39 45 bc                        cmp dword ptr [ebp - 0x44], eax
0040B5FD  7d 25                           jge 0x40b624
0040B5FF  8d 55 a0                        lea edx, [ebp - 0x60]
0040B602  52                              push edx
0040B603  8b 45 bc                        mov eax, dword ptr [ebp - 0x44]
0040B606  50                              push eax
0040B607  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
0040B60A  81 c1 b0 00 00 00               add ecx, 0xb0
0040B610  e8 9b 62 ff ff                  call 0x4018b0
0040B615  8b c8                           mov ecx, eax
0040B617  81 c1 b0 00 00 00               add ecx, 0xb0
0040B61D  e8 fe 81 ff ff                  call 0x403820
0040B622  eb bf                           jmp 0x40b5e3
0040B624  6a 48                           push 0x48
0040B626  e8 7d 0b 09 00                  call 0x49c1a8
0040B62B  83 c4 04                        add esp, 4
0040B62E  89 85 24 ff ff ff               mov dword ptr [ebp - 0xdc], eax
0040B634  c6 45 fc 03                     mov byte ptr [ebp - 4], 3
0040B638  83 bd 24 ff ff ff 00            cmp dword ptr [ebp - 0xdc], 0
0040B63F  74 1d                           je 0x40b65e
0040B641  6a 01                           push 1
0040B643  8b 4d b8                        mov ecx, dword ptr [ebp - 0x48]
0040B646  51                              push ecx
0040B647  8d 55 c0                        lea edx, [ebp - 0x40]
0040B64A  52                              push edx
0040B64B  8b 8d 24 ff ff ff               mov ecx, dword ptr [ebp - 0xdc]
0040B651  e8 2a 6a 02 00                  call 0x432080
0040B656  89 85 3c fe ff ff               mov dword ptr [ebp - 0x1c4], eax
0040B65C  eb 0a                           jmp 0x40b668
0040B65E  c7 85 3c fe ff ff 00 00 00 00   mov dword ptr [ebp - 0x1c4], 0
0040B668  8b 85 3c fe ff ff               mov eax, dword ptr [ebp - 0x1c4]
0040B66E  89 85 28 ff ff ff               mov dword ptr [ebp - 0xd8], eax
0040B674  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0040B678  8b 8d 28 ff ff ff               mov ecx, dword ptr [ebp - 0xd8]
0040B67E  89 8d 2c ff ff ff               mov dword ptr [ebp - 0xd4], ecx
0040B684  8d 95 2c ff ff ff               lea edx, [ebp - 0xd4]
0040B68A  52                              push edx
0040B68B  b9 a8 88 4c 00                  mov ecx, 0x4c88a8
0040B690  e8 fb 61 ff ff                  call 0x401890
0040B695  e9 b7 01 00 00                  jmp 0x40b851
0040B69A  68 28 01 00 00                  push 0x128
0040B69F  e8 04 0b 09 00                  call 0x49c1a8
0040B6A4  83 c4 04                        add esp, 4
0040B6A7  89 85 1c ff ff ff               mov dword ptr [ebp - 0xe4], eax
0040B6AD  c6 45 fc 04                     mov byte ptr [ebp - 4], 4
0040B6B1  83 bd 1c ff ff ff 00            cmp dword ptr [ebp - 0xe4], 0
0040B6B8  74 13                           je 0x40b6cd
0040B6BA  8b 8d 1c ff ff ff               mov ecx, dword ptr [ebp - 0xe4]
0040B6C0  e8 db bb ff ff                  call 0x4072a0
0040B6C5  89 85 38 fe ff ff               mov dword ptr [ebp - 0x1c8], eax
0040B6CB  eb 0a                           jmp 0x40b6d7
0040B6CD  c7 85 38 fe ff ff 00 00 00 00   mov dword ptr [ebp - 0x1c8], 0
0040B6D7  8b 85 38 fe ff ff               mov eax, dword ptr [ebp - 0x1c8]
0040B6DD  89 85 20 ff ff ff               mov dword ptr [ebp - 0xe0], eax
0040B6E3  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0040B6E7  8b 8d 20 ff ff ff               mov ecx, dword ptr [ebp - 0xe0]
0040B6ED  89 4d 98                        mov dword ptr [ebp - 0x68], ecx
0040B6F0  6a 06                           push 6
0040B6F2  8b 4d 98                        mov ecx, dword ptr [ebp - 0x68]
0040B6F5  e8 66 be ff ff                  call 0x407560
0040B6FA  8d 55 c0                        lea edx, [ebp - 0x40]
0040B6FD  52                              push edx
0040B6FE  8b 45 98                        mov eax, dword ptr [ebp - 0x68]
0040B701  8b 10                           mov edx, dword ptr [eax]
0040B703  8b 4d 98                        mov ecx, dword ptr [ebp - 0x68]
0040B706  ff 52 08                        call dword ptr [edx + 8]
0040B709  68 00 00 e0 3f                  push 0x3fe00000
0040B70E  6a 00                           push 0
0040B710  68 00 00 10 40                  push 0x40100000
0040B715  6a 00                           push 0
0040B717  8b 4d 98                        mov ecx, dword ptr [ebp - 0x68]
0040B71A  e8 b1 bf ff ff                  call 0x4076d0
0040B71F  6a 05                           push 5
0040B721  8b 4d 98                        mov ecx, dword ptr [ebp - 0x68]
0040B724  e8 07 c0 ff ff                  call 0x407730
0040B729  8d 45 c0                        lea eax, [ebp - 0x40]
0040B72C  50                              push eax
0040B72D  8d 8d ec fe ff ff               lea ecx, [ebp - 0x114]
0040B733  51                              push ecx
0040B734  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040B739  e8 2e 1e 03 00                  call 0x43d56c
0040B73E  50                              push eax
0040B73F  8d 95 d4 fe ff ff               lea edx, [ebp - 0x12c]
0040B745  52                              push edx
0040B746  8d 45 d8                        lea eax, [ebp - 0x28]
0040B749  50                              push eax
0040B74A  8d 8d 04 ff ff ff               lea ecx, [ebp - 0xfc]
0040B750  51                              push ecx
0040B751  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040B756  e8 11 1e 03 00                  call 0x43d56c
0040B75B  8b c8                           mov ecx, eax
0040B75D  e8 ce 60 ff ff                  call 0x401830
0040B762  50                              push eax
0040B763  8b 55 98                        mov edx, dword ptr [ebp - 0x68]
0040B766  8b 02                           mov eax, dword ptr [edx]
0040B768  8b 4d 98                        mov ecx, dword ptr [ebp - 0x68]
0040B76B  ff 50 54                        call dword ptr [eax + 0x54]
0040B76E  dd 45 e0                        fld qword ptr [ebp - 0x20]
0040B771  e8 36 0e 09 00                  call 0x49c5ac
0040B776  50                              push eax
0040B777  dd 45 d8                        fld qword ptr [ebp - 0x28]
0040B77A  e8 2d 0e 09 00                  call 0x49c5ac
0040B77F  50                              push eax
0040B780  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040B785  e8 b6 87 ff ff                  call 0x403f40
0040B78A  8b f0                           mov esi, eax
0040B78C  b9 06 00 00 00                  mov ecx, 6
0040B791  8d 7d 80                        lea edi, [ebp - 0x80]
0040B794  f3 a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
0040B796  c7 45 9c 00 00 00 00            mov dword ptr [ebp - 0x64], 0
0040B79D  eb 09                           jmp 0x40b7a8
0040B79F  8b 4d 9c                        mov ecx, dword ptr [ebp - 0x64]
0040B7A2  83 c1 01                        add ecx, 1
0040B7A5  89 4d 9c                        mov dword ptr [ebp - 0x64], ecx
0040B7A8  8b 4d 98                        mov ecx, dword ptr [ebp - 0x68]
0040B7AB  81 c1 b0 00 00 00               add ecx, 0xb0
0040B7B1  e8 1a 61 ff ff                  call 0x4018d0
0040B7B6  39 45 9c                        cmp dword ptr [ebp - 0x64], eax
0040B7B9  7d 25                           jge 0x40b7e0
0040B7BB  8d 55 80                        lea edx, [ebp - 0x80]
0040B7BE  52                              push edx
0040B7BF  8b 45 9c                        mov eax, dword ptr [ebp - 0x64]
0040B7C2  50                              push eax
0040B7C3  8b 4d 98                        mov ecx, dword ptr [ebp - 0x68]
0040B7C6  81 c1 b0 00 00 00               add ecx, 0xb0
0040B7CC  e8 df 60 ff ff                  call 0x4018b0
0040B7D1  8b c8                           mov ecx, eax
0040B7D3  81 c1 b0 00 00 00               add ecx, 0xb0
0040B7D9  e8 42 80 ff ff                  call 0x403820
0040B7DE  eb bf                           jmp 0x40b79f
0040B7E0  6a 48                           push 0x48
0040B7E2  e8 c1 09 09 00                  call 0x49c1a8
0040B7E7  83 c4 04                        add esp, 4
0040B7EA  89 85 c8 fe ff ff               mov dword ptr [ebp - 0x138], eax
0040B7F0  c6 45 fc 05                     mov byte ptr [ebp - 4], 5
0040B7F4  83 bd c8 fe ff ff 00            cmp dword ptr [ebp - 0x138], 0
0040B7FB  74 1d                           je 0x40b81a
0040B7FD  6a 01                           push 1
0040B7FF  8b 4d 98                        mov ecx, dword ptr [ebp - 0x68]
0040B802  51                              push ecx
0040B803  8d 55 c0                        lea edx, [ebp - 0x40]
0040B806  52                              push edx
0040B807  8b 8d c8 fe ff ff               mov ecx, dword ptr [ebp - 0x138]
0040B80D  e8 6e 68 02 00                  call 0x432080
0040B812  89 85 34 fe ff ff               mov dword ptr [ebp - 0x1cc], eax
0040B818  eb 0a                           jmp 0x40b824
0040B81A  c7 85 34 fe ff ff 00 00 00 00   mov dword ptr [ebp - 0x1cc], 0
0040B824  8b 85 34 fe ff ff               mov eax, dword ptr [ebp - 0x1cc]
0040B82A  89 85 cc fe ff ff               mov dword ptr [ebp - 0x134], eax
0040B830  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0040B834  8b 8d cc fe ff ff               mov ecx, dword ptr [ebp - 0x134]
0040B83A  89 8d d0 fe ff ff               mov dword ptr [ebp - 0x130], ecx
0040B840  8d 95 d0 fe ff ff               lea edx, [ebp - 0x130]
0040B846  52                              push edx
0040B847  b9 a8 88 4c 00                  mov ecx, 0x4c88a8
0040B84C  e8 3f 60 ff ff                  call 0x401890
0040B851  68 58 01 00 00                  push 0x158
0040B856  e8 4d 09 09 00                  call 0x49c1a8
0040B85B  83 c4 04                        add esp, 4
0040B85E  89 85 c0 fe ff ff               mov dword ptr [ebp - 0x140], eax
0040B864  c6 45 fc 06                     mov byte ptr [ebp - 4], 6
0040B868  83 bd c0 fe ff ff 00            cmp dword ptr [ebp - 0x140], 0
0040B86F  74 13                           je 0x40b884
0040B871  8b 8d c0 fe ff ff               mov ecx, dword ptr [ebp - 0x140]
0040B877  e8 f4 6f ff ff                  call 0x402870
0040B87C  89 85 30 fe ff ff               mov dword ptr [ebp - 0x1d0], eax
0040B882  eb 0a                           jmp 0x40b88e
0040B884  c7 85 30 fe ff ff 00 00 00 00   mov dword ptr [ebp - 0x1d0], 0
0040B88E  8b 85 30 fe ff ff               mov eax, dword ptr [ebp - 0x1d0]
0040B894  89 85 c4 fe ff ff               mov dword ptr [ebp - 0x13c], eax
0040B89A  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0040B89E  8b 8d c4 fe ff ff               mov ecx, dword ptr [ebp - 0x13c]
0040B8A4  89 4d f0                        mov dword ptr [ebp - 0x10], ecx
0040B8A7  6a 0a                           push 0xa
0040B8A9  8b 4d f0                        mov ecx, dword ptr [ebp - 0x10]
0040B8AC  e8 2f 74 ff ff                  call 0x402ce0
0040B8B1  68 00 c0 5f 40                  push 0x405fc000
0040B8B6  6a 00                           push 0
0040B8B8  68 00 c0 5f 40                  push 0x405fc000
0040B8BD  6a 00                           push 0
0040B8BF  68 00 e0 6f 40                  push 0x406fe000
0040B8C4  6a 00                           push 0
0040B8C6  8d 8d a8 fe ff ff               lea ecx, [ebp - 0x158]
0040B8CC  e8 1f 5f ff ff                  call 0x4017f0
0040B8D1  50                              push eax
0040B8D2  8b 4d f0                        mov ecx, dword ptr [ebp - 0x10]
0040B8D5  81 c1 b0 00 00 00               add ecx, 0xb0
0040B8DB  e8 f0 5e ff ff                  call 0x4017d0
0040B8E0  8d 55 d8                        lea edx, [ebp - 0x28]
0040B8E3  52                              push edx
0040B8E4  8b 45 f0                        mov eax, dword ptr [ebp - 0x10]
0040B8E7  8b 10                           mov edx, dword ptr [eax]
0040B8E9  8b 4d f0                        mov ecx, dword ptr [ebp - 0x10]
0040B8EC  ff 52 08                        call dword ptr [edx + 8]
0040B8EF  68 00 00 e0 3f                  push 0x3fe00000
0040B8F4  6a 00                           push 0
0040B8F6  68 00 00 10 40                  push 0x40100000
0040B8FB  6a 00                           push 0
0040B8FD  8b 4d f0                        mov ecx, dword ptr [ebp - 0x10]
0040B900  e8 7b 74 ff ff                  call 0x402d80
0040B905  6a 05                           push 5
0040B907  8b 4d f0                        mov ecx, dword ptr [ebp - 0x10]
0040B90A  e8 b1 74 ff ff                  call 0x402dc0
0040B90F  8d 45 c0                        lea eax, [ebp - 0x40]
0040B912  50                              push eax
0040B913  8d 8d 78 fe ff ff               lea ecx, [ebp - 0x188]
0040B919  51                              push ecx
0040B91A  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040B91F  e8 48 1c 03 00                  call 0x43d56c
0040B924  50                              push eax
0040B925  8d 95 60 fe ff ff               lea edx, [ebp - 0x1a0]
0040B92B  52                              push edx
0040B92C  8d 45 d8                        lea eax, [ebp - 0x28]
0040B92F  50                              push eax
0040B930  8d 8d 90 fe ff ff               lea ecx, [ebp - 0x170]
0040B936  51                              push ecx
0040B937  b9 10 6a 4c 00                  mov ecx, 0x4c6a10
0040B93C  e8 2b 1c 03 00                  call 0x43d56c
0040B941  8b c8                           mov ecx, eax
0040B943  e8 e8 5e ff ff                  call 0x401830
0040B948  50                              push eax
0040B949  8b 55 f0                        mov edx, dword ptr [ebp - 0x10]
0040B94C  8b 02                           mov eax, dword ptr [edx]
0040B94E  8b 4d f0                        mov ecx, dword ptr [ebp - 0x10]
0040B951  ff 50 54                        call dword ptr [eax + 0x54]
0040B954  6a 48                           push 0x48
0040B956  e8 4d 08 09 00                  call 0x49c1a8
0040B95B  83 c4 04                        add esp, 4
0040B95E  89 85 54 fe ff ff               mov dword ptr [ebp - 0x1ac], eax
0040B964  c6 45 fc 07                     mov byte ptr [ebp - 4], 7
0040B968  83 bd 54 fe ff ff 00            cmp dword ptr [ebp - 0x1ac], 0
0040B96F  74 1d                           je 0x40b98e
0040B971  6a 01                           push 1
0040B973  8b 4d f0                        mov ecx, dword ptr [ebp - 0x10]
0040B976  51                              push ecx
0040B977  8d 55 c0                        lea edx, [ebp - 0x40]
0040B97A  52                              push edx
0040B97B  8b 8d 54 fe ff ff               mov ecx, dword ptr [ebp - 0x1ac]
0040B981  e8 fa 66 02 00                  call 0x432080
0040B986  89 85 2c fe ff ff               mov dword ptr [ebp - 0x1d4], eax
0040B98C  eb 0a                           jmp 0x40b998
0040B98E  c7 85 2c fe ff ff 00 00 00 00   mov dword ptr [ebp - 0x1d4], 0
0040B998  8b 85 2c fe ff ff               mov eax, dword ptr [ebp - 0x1d4]
0040B99E  89 85 58 fe ff ff               mov dword ptr [ebp - 0x1a8], eax
0040B9A4  c6 45 fc 01                     mov byte ptr [ebp - 4], 1
0040B9A8  8b 8d 58 fe ff ff               mov ecx, dword ptr [ebp - 0x1a8]
0040B9AE  89 8d 5c fe ff ff               mov dword ptr [ebp - 0x1a4], ecx
0040B9B4  8d 95 5c fe ff ff               lea edx, [ebp - 0x1a4]
0040B9BA  52                              push edx
0040B9BB  b9 a8 88 4c 00                  mov ecx, 0x4c88a8
0040B9C0  e8 cb 5e ff ff                  call 0x401890
0040B9C5  c6 45 fc 00                     mov byte ptr [ebp - 4], 0
0040B9C9  8d 4d d8                        lea ecx, [ebp - 0x28]
0040B9CC  e8 df 84 ff ff                  call 0x403eb0
0040B9D1  c7 45 fc ff ff ff ff            mov dword ptr [ebp - 4], 0xffffffff
0040B9D8  8d 4d c0                        lea ecx, [ebp - 0x40]
0040B9DB  e8 d0 84 ff ff                  call 0x403eb0
0040B9E0  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0040B9E3  64 89 0d 00 00 00 00            mov dword ptr fs:[0], ecx
0040B9EA  5f                              pop edi
0040B9EB  5e                              pop esi
0040B9EC  8b e5                           mov esp, ebp
0040B9EE  5d                              pop ebp
0040B9EF  c2 04 00                        ret 4

; VA 0x0040B9F2, file offset 0xB9F2, length 104, SHA-256 f5737cd3bb0aed25041d2a5462df5d47394decffea281a32467e1fd112c08fdb
0040B9F2  55                              push ebp
0040B9F3  8b ec                           mov ebp, esp
0040B9F5  51                              push ecx
0040B9F6  89 4d fc                        mov dword ptr [ebp - 4], ecx
0040B9F9  8b 45 0c                        mov eax, dword ptr [ebp + 0xc]
0040B9FC  8b 48 04                        mov ecx, dword ptr [eax + 4]
0040B9FF  51                              push ecx
0040BA00  8b 10                           mov edx, dword ptr [eax]
0040BA02  52                              push edx
0040BA03  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040BA06  83 c1 78                        add ecx, 0x78
0040BA09  8b 45 08                        mov eax, dword ptr [ebp + 8]
0040BA0C  8b 50 78                        mov edx, dword ptr [eax + 0x78]
0040BA0F  ff 52 2c                        call dword ptr [edx + 0x2c]
0040BA12  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0040BA15  8b 4d 0c                        mov ecx, dword ptr [ebp + 0xc]
0040BA18  dd 40 08                        fld qword ptr [eax + 8]
0040BA1B  dc 01                           fadd qword ptr [ecx]
0040BA1D  8b 55 fc                        mov edx, dword ptr [ebp - 4]
0040BA20  dd 5a 08                        fstp qword ptr [edx + 8]
0040BA23  8b 45 0c                        mov eax, dword ptr [ebp + 0xc]
0040BA26  c7 00 00 00 00 00               mov dword ptr [eax], 0
0040BA2C  c7 40 04 00 00 00 00            mov dword ptr [eax + 4], 0
0040BA33  8b 4d 08                        mov ecx, dword ptr [ebp + 8]
0040BA36  83 c1 78                        add ecx, 0x78
0040BA39  8b 55 08                        mov edx, dword ptr [ebp + 8]
0040BA3C  8b 42 78                        mov eax, dword ptr [edx + 0x78]
0040BA3F  ff 50 44                        call dword ptr [eax + 0x44]
0040BA42  25 ff 00 00 00                  and eax, 0xff
0040BA47  85 c0                           test eax, eax
0040BA49  74 07                           je 0x40ba52
0040BA4B  b8 02 00 00 00                  mov eax, 2
0040BA50  eb 02                           jmp 0x40ba54
0040BA52  33 c0                           xor eax, eax
0040BA54  8b e5                           mov esp, ebp
0040BA56  5d                              pop ebp
0040BA57  c2 08 00                        ret 8

; VA 0x0040BA60, file offset 0xBA60, length 13, SHA-256 8a964d1d7ff7ce1a3c60997027339b1b12723566ff413f0bb34cb92f56441d3a
0040BA60  55                              push ebp
0040BA61  8b ec                           mov ebp, esp
0040BA63  51                              push ecx
0040BA64  89 4d fc                        mov dword ptr [ebp - 4], ecx
0040BA67  8b e5                           mov esp, ebp
0040BA69  5d                              pop ebp
0040BA6A  c2 04 00                        ret 4

; VA 0x004010D0, file offset 0x10D0, length 31, SHA-256 38fd038f41a342d0889685b8ddb612588565f0a9b1c5bdfe2a2cdb905e7103de
004010D0  55                              push ebp
004010D1  8b ec                           mov ebp, esp
004010D3  51                              push ecx
004010D4  89 4d fc                        mov dword ptr [ebp - 4], ecx
004010D7  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
004010DA  e8 b1 72 01 00                  call 0x418390
004010DF  8b 45 fc                        mov eax, dword ptr [ebp - 4]
004010E2  c7 00 74 02 4b 00               mov dword ptr [eax], 0x4b0274
004010E8  8b 45 fc                        mov eax, dword ptr [ebp - 4]
004010EB  8b e5                           mov esp, ebp
004010ED  5d                              pop ebp
004010EE  c3                              ret

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
