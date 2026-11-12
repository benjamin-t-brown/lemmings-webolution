; EXE SHA256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f

; VA 0047A190 PE offset 500112 bytes 981 SHA256 909ce99b037561bda1f93eec51874a02f83feb547c286d0d95dd8a7277d8f94f; recovered
0047A190  55                                 push ebp
0047A191  8b ec                              mov ebp, esp
0047A193  83 ec 14                           sub esp, 0x14
0047A196  89 4d f4                           mov dword ptr [ebp - 0xc], ecx
0047A199  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A19C  8b 88 b8 01 00 00                  mov ecx, dword ptr [eax + 0x1b8]
0047A1A2  51                                 push ecx
0047A1A3  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A1A6  8b 82 cc 01 00 00                  mov eax, dword ptr [edx + 0x1cc]
0047A1AC  50                                 push eax
0047A1AD  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A1B0  8b 91 c8 01 00 00                  mov edx, dword ptr [ecx + 0x1c8]
0047A1B6  52                                 push edx
0047A1B7  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A1BA  8b 88 f8 01 00 00                  mov ecx, dword ptr [eax + 0x1f8]
0047A1C0  e8 3b eb fe ff                     call 0x468d00
0047A1C5  8b c8                              mov ecx, eax
0047A1C7  e8 14 eb fe ff                     call 0x468ce0
0047A1CC  8b c8                              mov ecx, eax
0047A1CE  e8 ed 09 00 00                     call 0x47abc0
0047A1D3  89 45 fc                           mov dword ptr [ebp - 4], eax
0047A1D6  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A1D9  83 b9 b0 01 00 00 00               cmp dword ptr [ecx + 0x1b0], 0
0047A1E0  75 36                              jne 0x47a218
0047A1E2  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A1E5  8b 42 14                           mov eax, dword ptr [edx + 0x14]
0047A1E8  50                                 push eax
0047A1E9  8b 4a 10                           mov ecx, dword ptr [edx + 0x10]
0047A1EC  51                                 push ecx
0047A1ED  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A1F0  8b 42 24                           mov eax, dword ptr [edx + 0x24]
0047A1F3  50                                 push eax
0047A1F4  8b 4a 20                           mov ecx, dword ptr [edx + 0x20]
0047A1F7  51                                 push ecx
0047A1F8  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A1FB  8b 42 1c                           mov eax, dword ptr [edx + 0x1c]
0047A1FE  50                                 push eax
0047A1FF  8b 4a 18                           mov ecx, dword ptr [edx + 0x18]
0047A202  51                                 push ecx
0047A203  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A206  8b 42 2c                           mov eax, dword ptr [edx + 0x2c]
0047A209  50                                 push eax
0047A20A  8b 4a 28                           mov ecx, dword ptr [edx + 0x28]
0047A20D  51                                 push ecx
0047A20E  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A211  e8 3a 6a fc ff                     call 0x440c50
0047A216  eb 34                              jmp 0x47a24c
0047A218  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A21B  8b 42 1c                           mov eax, dword ptr [edx + 0x1c]
0047A21E  50                                 push eax
0047A21F  8b 4a 18                           mov ecx, dword ptr [edx + 0x18]
0047A222  51                                 push ecx
0047A223  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A226  8b 42 24                           mov eax, dword ptr [edx + 0x24]
0047A229  50                                 push eax
0047A22A  8b 4a 20                           mov ecx, dword ptr [edx + 0x20]
0047A22D  51                                 push ecx
0047A22E  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A231  8b 42 14                           mov eax, dword ptr [edx + 0x14]
0047A234  50                                 push eax
0047A235  8b 4a 10                           mov ecx, dword ptr [edx + 0x10]
0047A238  51                                 push ecx
0047A239  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A23C  8b 42 2c                           mov eax, dword ptr [edx + 0x2c]
0047A23F  50                                 push eax
0047A240  8b 4a 28                           mov ecx, dword ptr [edx + 0x28]
0047A243  51                                 push ecx
0047A244  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A247  e8 04 6a fc ff                     call 0x440c50
0047A24C  8b 15 3c d5 58 00                  mov edx, dword ptr [0x58d53c]
0047A252  2b 15 38 cb 58 00                  sub edx, dword ptr [0x58cb38]
0047A258  89 55 f8                           mov dword ptr [ebp - 8], edx
0047A25B  83 7d f8 00                        cmp dword ptr [ebp - 8], 0
0047A25F  0f 85 f3 00 00 00                  jne 0x47a358
0047A265  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A268  db 40 30                           fild dword ptr [eax + 0x30]
0047A26B  dc 45 08                           fadd qword ptr [ebp + 8]
0047A26E  dd 5d 08                           fstp qword ptr [ebp + 8]
0047A271  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A274  db 41 34                           fild dword ptr [ecx + 0x34]
0047A277  dc 6d 10                           fsubr qword ptr [ebp + 0x10]
0047A27A  dd 5d 10                           fstp qword ptr [ebp + 0x10]
0047A27D  dd 45 18                           fld qword ptr [ebp + 0x18]
0047A280  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A286  dc 6d 08                           fsubr qword ptr [ebp + 8]
0047A289  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A28C  db 42 38                           fild dword ptr [edx + 0x38]
0047A28F  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A295  de e9                              fsubp st(1)
0047A297  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A29A  dd 98 88 00 00 00                  fstp qword ptr [eax + 0x88]
0047A2A0  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A2A3  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A2A6  8b 81 88 00 00 00                  mov eax, dword ptr [ecx + 0x88]
0047A2AC  89 42 20                           mov dword ptr [edx + 0x20], eax
0047A2AF  8b 89 8c 00 00 00                  mov ecx, dword ptr [ecx + 0x8c]
0047A2B5  89 4a 24                           mov dword ptr [edx + 0x24], ecx
0047A2B8  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A2BB  dd 45 18                           fld qword ptr [ebp + 0x18]
0047A2BE  dc 42 20                           fadd qword ptr [edx + 0x20]
0047A2C1  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A2C4  dd 98 58 01 00 00                  fstp qword ptr [eax + 0x158]
0047A2CA  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A2CD  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A2D0  8b 81 58 01 00 00                  mov eax, dword ptr [ecx + 0x158]
0047A2D6  89 82 f0 00 00 00                  mov dword ptr [edx + 0xf0], eax
0047A2DC  8b 89 5c 01 00 00                  mov ecx, dword ptr [ecx + 0x15c]
0047A2E2  89 8a f4 00 00 00                  mov dword ptr [edx + 0xf4], ecx
0047A2E8  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A2EB  db 42 3c                           fild dword ptr [edx + 0x3c]
0047A2EE  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A2F4  dc 6d 10                           fsubr qword ptr [ebp + 0x10]
0047A2F7  dd 45 20                           fld qword ptr [ebp + 0x20]
0047A2FA  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A300  de e9                              fsubp st(1)
0047A302  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A305  dd 98 60 01 00 00                  fstp qword ptr [eax + 0x160]
0047A30B  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A30E  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A311  8b 81 60 01 00 00                  mov eax, dword ptr [ecx + 0x160]
0047A317  89 42 28                           mov dword ptr [edx + 0x28], eax
0047A31A  8b 89 64 01 00 00                  mov ecx, dword ptr [ecx + 0x164]
0047A320  89 4a 2c                           mov dword ptr [edx + 0x2c], ecx
0047A323  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A326  dd 45 20                           fld qword ptr [ebp + 0x20]
0047A329  dc 42 28                           fadd qword ptr [edx + 0x28]
0047A32C  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A32F  dd 98 f8 00 00 00                  fstp qword ptr [eax + 0xf8]
0047A335  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A338  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A33B  8b 81 f8 00 00 00                  mov eax, dword ptr [ecx + 0xf8]
0047A341  89 82 90 00 00 00                  mov dword ptr [edx + 0x90], eax
0047A347  8b 89 fc 00 00 00                  mov ecx, dword ptr [ecx + 0xfc]
0047A34D  89 8a 94 00 00 00                  mov dword ptr [edx + 0x94], ecx
0047A353  e9 fb 01 00 00                     jmp 0x47a553
0047A358  83 7d f8 ff                        cmp dword ptr [ebp - 8], -1
0047A35C  0f 85 0b 01 00 00                  jne 0x47a46d
0047A362  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A365  db 42 30                           fild dword ptr [edx + 0x30]
0047A368  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A36E  dc 45 08                           fadd qword ptr [ebp + 8]
0047A371  dd 5d 08                           fstp qword ptr [ebp + 8]
0047A374  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A377  db 40 34                           fild dword ptr [eax + 0x34]
0047A37A  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A380  dc 6d 10                           fsubr qword ptr [ebp + 0x10]
0047A383  dd 5d 10                           fstp qword ptr [ebp + 0x10]
0047A386  dd 45 18                           fld qword ptr [ebp + 0x18]
0047A389  dc 35 c8 02 4b 00                  fdiv qword ptr [0x4b02c8]
0047A38F  dc 6d 08                           fsubr qword ptr [ebp + 8]
0047A392  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A395  db 41 38                           fild dword ptr [ecx + 0x38]
0047A398  dc 35 c8 02 4b 00                  fdiv qword ptr [0x4b02c8]
0047A39E  de e9                              fsubp st(1)
0047A3A0  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A3A3  dd 9a 88 00 00 00                  fstp qword ptr [edx + 0x88]
0047A3A9  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A3AC  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A3AF  8b 90 88 00 00 00                  mov edx, dword ptr [eax + 0x88]
0047A3B5  89 51 20                           mov dword ptr [ecx + 0x20], edx
0047A3B8  8b 80 8c 00 00 00                  mov eax, dword ptr [eax + 0x8c]
0047A3BE  89 41 24                           mov dword ptr [ecx + 0x24], eax
0047A3C1  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A3C4  dd 45 18                           fld qword ptr [ebp + 0x18]
0047A3C7  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A3CD  dc 41 20                           fadd qword ptr [ecx + 0x20]
0047A3D0  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A3D3  dd 9a 58 01 00 00                  fstp qword ptr [edx + 0x158]
0047A3D9  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A3DC  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A3DF  8b 90 58 01 00 00                  mov edx, dword ptr [eax + 0x158]
0047A3E5  89 91 f0 00 00 00                  mov dword ptr [ecx + 0xf0], edx
0047A3EB  8b 80 5c 01 00 00                  mov eax, dword ptr [eax + 0x15c]
0047A3F1  89 81 f4 00 00 00                  mov dword ptr [ecx + 0xf4], eax
0047A3F7  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A3FA  db 41 3c                           fild dword ptr [ecx + 0x3c]
0047A3FD  dc 35 c8 02 4b 00                  fdiv qword ptr [0x4b02c8]
0047A403  dc 6d 10                           fsubr qword ptr [ebp + 0x10]
0047A406  dd 45 20                           fld qword ptr [ebp + 0x20]
0047A409  dc 35 c8 02 4b 00                  fdiv qword ptr [0x4b02c8]
0047A40F  de e9                              fsubp st(1)
0047A411  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A414  dd 9a 60 01 00 00                  fstp qword ptr [edx + 0x160]
0047A41A  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A41D  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A420  8b 90 60 01 00 00                  mov edx, dword ptr [eax + 0x160]
0047A426  89 51 28                           mov dword ptr [ecx + 0x28], edx
0047A429  8b 80 64 01 00 00                  mov eax, dword ptr [eax + 0x164]
0047A42F  89 41 2c                           mov dword ptr [ecx + 0x2c], eax
0047A432  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A435  dd 45 20                           fld qword ptr [ebp + 0x20]
0047A438  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A43E  dc 41 28                           fadd qword ptr [ecx + 0x28]
0047A441  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A444  dd 9a f8 00 00 00                  fstp qword ptr [edx + 0xf8]
0047A44A  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A44D  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A450  8b 90 f8 00 00 00                  mov edx, dword ptr [eax + 0xf8]
0047A456  89 91 90 00 00 00                  mov dword ptr [ecx + 0x90], edx
0047A45C  8b 80 fc 00 00 00                  mov eax, dword ptr [eax + 0xfc]
0047A462  89 81 94 00 00 00                  mov dword ptr [ecx + 0x94], eax
0047A468  e9 e6 00 00 00                     jmp 0x47a553
0047A46D  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A470  8b 51 30                           mov edx, dword ptr [ecx + 0x30]
0047A473  d1 e2                              shl edx, 1
0047A475  89 55 f0                           mov dword ptr [ebp - 0x10], edx
0047A478  db 45 f0                           fild dword ptr [ebp - 0x10]
0047A47B  dc 45 08                           fadd qword ptr [ebp + 8]
0047A47E  dd 5d 08                           fstp qword ptr [ebp + 8]
0047A481  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A484  8b 48 34                           mov ecx, dword ptr [eax + 0x34]
0047A487  d1 e1                              shl ecx, 1
0047A489  89 4d ec                           mov dword ptr [ebp - 0x14], ecx
0047A48C  db 45 ec                           fild dword ptr [ebp - 0x14]
0047A48F  dc 6d 10                           fsubr qword ptr [ebp + 0x10]
0047A492  dd 5d 10                           fstp qword ptr [ebp + 0x10]
0047A495  dd 45 08                           fld qword ptr [ebp + 8]
0047A498  dc 65 18                           fsub qword ptr [ebp + 0x18]
0047A49B  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A49E  da 62 38                           fisub dword ptr [edx + 0x38]
0047A4A1  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A4A4  dd 98 88 00 00 00                  fstp qword ptr [eax + 0x88]
0047A4AA  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A4AD  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A4B0  8b 81 88 00 00 00                  mov eax, dword ptr [ecx + 0x88]
0047A4B6  89 42 20                           mov dword ptr [edx + 0x20], eax
0047A4B9  8b 89 8c 00 00 00                  mov ecx, dword ptr [ecx + 0x8c]
0047A4BF  89 4a 24                           mov dword ptr [edx + 0x24], ecx
0047A4C2  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A4C5  dd 45 18                           fld qword ptr [ebp + 0x18]
0047A4C8  dc c0                              fadd st(0), st(0)
0047A4CA  dc 42 20                           fadd qword ptr [edx + 0x20]
0047A4CD  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A4D0  dd 98 58 01 00 00                  fstp qword ptr [eax + 0x158]
0047A4D6  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A4D9  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A4DC  8b 81 58 01 00 00                  mov eax, dword ptr [ecx + 0x158]
0047A4E2  89 82 f0 00 00 00                  mov dword ptr [edx + 0xf0], eax
0047A4E8  8b 89 5c 01 00 00                  mov ecx, dword ptr [ecx + 0x15c]
0047A4EE  89 8a f4 00 00 00                  mov dword ptr [edx + 0xf4], ecx
0047A4F4  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A4F7  db 42 3c                           fild dword ptr [edx + 0x3c]
0047A4FA  dc 6d 10                           fsubr qword ptr [ebp + 0x10]
0047A4FD  dc 65 20                           fsub qword ptr [ebp + 0x20]
0047A500  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A503  dd 98 60 01 00 00                  fstp qword ptr [eax + 0x160]
0047A509  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A50C  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A50F  8b 81 60 01 00 00                  mov eax, dword ptr [ecx + 0x160]
0047A515  89 42 28                           mov dword ptr [edx + 0x28], eax
0047A518  8b 89 64 01 00 00                  mov ecx, dword ptr [ecx + 0x164]
0047A51E  89 4a 2c                           mov dword ptr [edx + 0x2c], ecx
0047A521  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A524  dd 45 20                           fld qword ptr [ebp + 0x20]
0047A527  dc c0                              fadd st(0), st(0)
0047A529  dc 42 28                           fadd qword ptr [edx + 0x28]
0047A52C  8b 45 f4                           mov eax, dword ptr [ebp - 0xc]
0047A52F  dd 98 f8 00 00 00                  fstp qword ptr [eax + 0xf8]
0047A535  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A538  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
0047A53B  8b 81 f8 00 00 00                  mov eax, dword ptr [ecx + 0xf8]
0047A541  89 82 90 00 00 00                  mov dword ptr [edx + 0x90], eax
0047A547  8b 89 fc 00 00 00                  mov ecx, dword ptr [ecx + 0xfc]
0047A54D  89 8a 94 00 00 00                  mov dword ptr [edx + 0x94], ecx
0047A553  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A556  52                                 push edx
0047A557  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047A55A  e8 37 06 00 00                     call 0x47ab96
0047A55F  8b e5                              mov esp, ebp
0047A561  5d                                 pop ebp
0047A562  c2 20 00                           ret 0x20

; VA 0047A565 PE offset 501093 bytes 504 SHA256 278b0623c999e4559569ec81a130fe763a64f8cda06ad958dd780b89c9e61fc0; context only, not ported
0047A565  55                                 push ebp
0047A566  8b ec                              mov ebp, esp
0047A568  83 ec 08                           sub esp, 8
0047A56B  89 4d f8                           mov dword ptr [ebp - 8], ecx
0047A56E  8b 45 f8                           mov eax, dword ptr [ebp - 8]
0047A571  8b 88 b8 01 00 00                  mov ecx, dword ptr [eax + 0x1b8]
0047A577  51                                 push ecx
0047A578  8b 55 f8                           mov edx, dword ptr [ebp - 8]
0047A57B  8b 82 cc 01 00 00                  mov eax, dword ptr [edx + 0x1cc]
0047A581  50                                 push eax
0047A582  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047A585  8b 91 c8 01 00 00                  mov edx, dword ptr [ecx + 0x1c8]
0047A58B  52                                 push edx
0047A58C  8b 45 f8                           mov eax, dword ptr [ebp - 8]
0047A58F  8b 88 f8 01 00 00                  mov ecx, dword ptr [eax + 0x1f8]
0047A595  e8 66 e7 fe ff                     call 0x468d00
0047A59A  8b c8                              mov ecx, eax
0047A59C  e8 3f e7 fe ff                     call 0x468ce0
0047A5A1  8b c8                              mov ecx, eax
0047A5A3  e8 18 06 00 00                     call 0x47abc0
0047A5A8  89 45 fc                           mov dword ptr [ebp - 4], eax
0047A5AB  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047A5AE  83 b9 b0 01 00 00 00               cmp dword ptr [ecx + 0x1b0], 0
0047A5B5  75 36                              jne 0x47a5ed
0047A5B7  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A5BA  8b 42 14                           mov eax, dword ptr [edx + 0x14]
0047A5BD  50                                 push eax
0047A5BE  8b 4a 10                           mov ecx, dword ptr [edx + 0x10]
0047A5C1  51                                 push ecx
0047A5C2  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A5C5  8b 42 24                           mov eax, dword ptr [edx + 0x24]
0047A5C8  50                                 push eax
0047A5C9  8b 4a 20                           mov ecx, dword ptr [edx + 0x20]
0047A5CC  51                                 push ecx
0047A5CD  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A5D0  8b 42 1c                           mov eax, dword ptr [edx + 0x1c]
0047A5D3  50                                 push eax
0047A5D4  8b 4a 18                           mov ecx, dword ptr [edx + 0x18]
0047A5D7  51                                 push ecx
0047A5D8  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A5DB  8b 42 2c                           mov eax, dword ptr [edx + 0x2c]
0047A5DE  50                                 push eax
0047A5DF  8b 4a 28                           mov ecx, dword ptr [edx + 0x28]
0047A5E2  51                                 push ecx
0047A5E3  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047A5E6  e8 65 66 fc ff                     call 0x440c50
0047A5EB  eb 34                              jmp 0x47a621
0047A5ED  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A5F0  8b 42 1c                           mov eax, dword ptr [edx + 0x1c]
0047A5F3  50                                 push eax
0047A5F4  8b 4a 18                           mov ecx, dword ptr [edx + 0x18]
0047A5F7  51                                 push ecx
0047A5F8  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A5FB  8b 42 24                           mov eax, dword ptr [edx + 0x24]
0047A5FE  50                                 push eax
0047A5FF  8b 4a 20                           mov ecx, dword ptr [edx + 0x20]
0047A602  51                                 push ecx
0047A603  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A606  8b 42 14                           mov eax, dword ptr [edx + 0x14]
0047A609  50                                 push eax
0047A60A  8b 4a 10                           mov ecx, dword ptr [edx + 0x10]
0047A60D  51                                 push ecx
0047A60E  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A611  8b 42 2c                           mov eax, dword ptr [edx + 0x2c]
0047A614  50                                 push eax
0047A615  8b 4a 28                           mov ecx, dword ptr [edx + 0x28]
0047A618  51                                 push ecx
0047A619  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047A61C  e8 2f 66 fc ff                     call 0x440c50
0047A621  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A624  db 42 30                           fild dword ptr [edx + 0x30]
0047A627  dc 45 08                           fadd qword ptr [ebp + 8]
0047A62A  dd 5d 08                           fstp qword ptr [ebp + 8]
0047A62D  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A630  db 40 34                           fild dword ptr [eax + 0x34]
0047A633  dc 6d 10                           fsubr qword ptr [ebp + 0x10]
0047A636  dd 5d 10                           fstp qword ptr [ebp + 0x10]
0047A639  dd 45 18                           fld qword ptr [ebp + 0x18]
0047A63C  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A642  dc 6d 08                           fsubr qword ptr [ebp + 8]
0047A645  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A648  db 41 38                           fild dword ptr [ecx + 0x38]
0047A64B  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A651  de e9                              fsubp st(1)
0047A653  8b 55 f8                           mov edx, dword ptr [ebp - 8]
0047A656  dd 5a 20                           fstp qword ptr [edx + 0x20]
0047A659  dd 45 20                           fld qword ptr [ebp + 0x20]
0047A65C  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A662  dc 6d 08                           fsubr qword ptr [ebp + 8]
0047A665  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A668  db 40 38                           fild dword ptr [eax + 0x38]
0047A66B  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A671  de e9                              fsubp st(1)
0047A673  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047A676  dd 99 88 00 00 00                  fstp qword ptr [ecx + 0x88]
0047A67C  dd 45 20                           fld qword ptr [ebp + 0x20]
0047A67F  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A685  dc 45 08                           fadd qword ptr [ebp + 8]
0047A688  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A68B  db 42 38                           fild dword ptr [edx + 0x38]
0047A68E  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A694  de e9                              fsubp st(1)
0047A696  8b 45 f8                           mov eax, dword ptr [ebp - 8]
0047A699  dd 98 f0 00 00 00                  fstp qword ptr [eax + 0xf0]
0047A69F  dd 45 18                           fld qword ptr [ebp + 0x18]
0047A6A2  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A6A8  dc 45 08                           fadd qword ptr [ebp + 8]
0047A6AB  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A6AE  db 41 38                           fild dword ptr [ecx + 0x38]
0047A6B1  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A6B7  de e9                              fsubp st(1)
0047A6B9  8b 55 f8                           mov edx, dword ptr [ebp - 8]
0047A6BC  dd 9a 58 01 00 00                  fstp qword ptr [edx + 0x158]
0047A6C2  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A6C5  db 40 3c                           fild dword ptr [eax + 0x3c]
0047A6C8  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A6CE  dc 6d 10                           fsubr qword ptr [ebp + 0x10]
0047A6D1  dd 45 28                           fld qword ptr [ebp + 0x28]
0047A6D4  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A6DA  de e9                              fsubp st(1)
0047A6DC  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047A6DF  dd 59 28                           fstp qword ptr [ecx + 0x28]
0047A6E2  83 3d 3c d5 58 00 00               cmp dword ptr [0x58d53c], 0
0047A6E9  75 18                              jne 0x47a703
0047A6EB  8b 55 f8                           mov edx, dword ptr [ebp - 8]
0047A6EE  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A6F1  db 40 3c                           fild dword ptr [eax + 0x3c]
0047A6F4  dc 35 c8 02 4b 00                  fdiv qword ptr [0x4b02c8]
0047A6FA  dc 42 28                           fadd qword ptr [edx + 0x28]
0047A6FD  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047A700  dd 59 28                           fstp qword ptr [ecx + 0x28]
0047A703  8b 55 f8                           mov edx, dword ptr [ebp - 8]
0047A706  8b 45 f8                           mov eax, dword ptr [ebp - 8]
0047A709  8b 4a 28                           mov ecx, dword ptr [edx + 0x28]
0047A70C  89 88 60 01 00 00                  mov dword ptr [eax + 0x160], ecx
0047A712  8b 52 2c                           mov edx, dword ptr [edx + 0x2c]
0047A715  89 90 64 01 00 00                  mov dword ptr [eax + 0x164], edx
0047A71B  8b 45 f8                           mov eax, dword ptr [ebp - 8]
0047A71E  dd 45 28                           fld qword ptr [ebp + 0x28]
0047A721  dc 40 28                           fadd qword ptr [eax + 0x28]
0047A724  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047A727  dd 99 f8 00 00 00                  fstp qword ptr [ecx + 0xf8]
0047A72D  8b 55 f8                           mov edx, dword ptr [ebp - 8]
0047A730  8b 45 f8                           mov eax, dword ptr [ebp - 8]
0047A733  8b 8a f8 00 00 00                  mov ecx, dword ptr [edx + 0xf8]
0047A739  89 88 90 00 00 00                  mov dword ptr [eax + 0x90], ecx
0047A73F  8b 92 fc 00 00 00                  mov edx, dword ptr [edx + 0xfc]
0047A745  89 90 94 00 00 00                  mov dword ptr [eax + 0x94], edx
0047A74B  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A74E  50                                 push eax
0047A74F  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047A752  e8 3f 04 00 00                     call 0x47ab96
0047A757  8b e5                              mov esp, ebp
0047A759  5d                                 pop ebp
0047A75A  c2 28 00                           ret 0x28

; VA 0047A75D PE offset 501597 bytes 1081 SHA256 d4c121e7ebd9e111cc317f772c36f379b80b955f7cefd3283c28e5988d205e74; context only, not ported
0047A75D  55                                 push ebp
0047A75E  8b ec                              mov ebp, esp
0047A760  83 ec 24                           sub esp, 0x24
0047A763  89 4d ec                           mov dword ptr [ebp - 0x14], ecx
0047A766  83 3d 3c d5 58 00 00               cmp dword ptr [0x58d53c], 0
0047A76D  75 0c                              jne 0x47a77b
0047A76F  6a 00                              push 0
0047A771  b9 80 cd 58 00                     mov ecx, 0x58cd80
0047A776  e8 ae 67 ff ff                     call 0x470f29
0047A77B  8b 45 ec                           mov eax, dword ptr [ebp - 0x14]
0047A77E  8b 88 b8 01 00 00                  mov ecx, dword ptr [eax + 0x1b8]
0047A784  51                                 push ecx
0047A785  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047A788  8b 82 cc 01 00 00                  mov eax, dword ptr [edx + 0x1cc]
0047A78E  50                                 push eax
0047A78F  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047A792  8b 91 c8 01 00 00                  mov edx, dword ptr [ecx + 0x1c8]
0047A798  52                                 push edx
0047A799  8b 45 ec                           mov eax, dword ptr [ebp - 0x14]
0047A79C  8b 88 f8 01 00 00                  mov ecx, dword ptr [eax + 0x1f8]
0047A7A2  e8 59 e5 fe ff                     call 0x468d00
0047A7A7  8b c8                              mov ecx, eax
0047A7A9  e8 32 e5 fe ff                     call 0x468ce0
0047A7AE  8b c8                              mov ecx, eax
0047A7B0  e8 0b 04 00 00                     call 0x47abc0
0047A7B5  89 45 fc                           mov dword ptr [ebp - 4], eax
0047A7B8  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A7BB  db 41 34                           fild dword ptr [ecx + 0x34]
0047A7BE  dc 6d 10                           fsubr qword ptr [ebp + 0x10]
0047A7C1  dd 5d 10                           fstp qword ptr [ebp + 0x10]
0047A7C4  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A7C7  db 42 3c                           fild dword ptr [edx + 0x3c]
0047A7CA  dd 5d f0                           fstp qword ptr [ebp - 0x10]
0047A7CD  a1 3c d5 58 00                     mov eax, dword ptr [0x58d53c]
0047A7D2  2b 05 38 cb 58 00                  sub eax, dword ptr [0x58cb38]
0047A7D8  89 45 f8                           mov dword ptr [ebp - 8], eax
0047A7DB  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047A7DE  33 d2                              xor edx, edx
0047A7E0  8a 91 b4 01 00 00                  mov dl, byte ptr [ecx + 0x1b4]
0047A7E6  85 d2                              test edx, edx
0047A7E8  0f 84 a8 01 00 00                  je 0x47a996
0047A7EE  8b 45 ec                           mov eax, dword ptr [ebp - 0x14]
0047A7F1  83 b8 b0 01 00 00 00               cmp dword ptr [eax + 0x1b0], 0
0047A7F8  75 36                              jne 0x47a830
0047A7FA  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A7FD  8b 51 14                           mov edx, dword ptr [ecx + 0x14]
0047A800  52                                 push edx
0047A801  8b 41 10                           mov eax, dword ptr [ecx + 0x10]
0047A804  50                                 push eax
0047A805  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A808  8b 51 24                           mov edx, dword ptr [ecx + 0x24]
0047A80B  52                                 push edx
0047A80C  8b 41 20                           mov eax, dword ptr [ecx + 0x20]
0047A80F  50                                 push eax
0047A810  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A813  8b 51 1c                           mov edx, dword ptr [ecx + 0x1c]
0047A816  52                                 push edx
0047A817  8b 41 18                           mov eax, dword ptr [ecx + 0x18]
0047A81A  50                                 push eax
0047A81B  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A81E  8b 51 2c                           mov edx, dword ptr [ecx + 0x2c]
0047A821  52                                 push edx
0047A822  8b 41 28                           mov eax, dword ptr [ecx + 0x28]
0047A825  50                                 push eax
0047A826  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047A829  e8 22 64 fc ff                     call 0x440c50
0047A82E  eb 34                              jmp 0x47a864
0047A830  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A833  8b 51 1c                           mov edx, dword ptr [ecx + 0x1c]
0047A836  52                                 push edx
0047A837  8b 41 18                           mov eax, dword ptr [ecx + 0x18]
0047A83A  50                                 push eax
0047A83B  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A83E  8b 51 24                           mov edx, dword ptr [ecx + 0x24]
0047A841  52                                 push edx
0047A842  8b 41 20                           mov eax, dword ptr [ecx + 0x20]
0047A845  50                                 push eax
0047A846  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A849  8b 51 14                           mov edx, dword ptr [ecx + 0x14]
0047A84C  52                                 push edx
0047A84D  8b 41 10                           mov eax, dword ptr [ecx + 0x10]
0047A850  50                                 push eax
0047A851  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A854  8b 51 2c                           mov edx, dword ptr [ecx + 0x2c]
0047A857  52                                 push edx
0047A858  8b 41 28                           mov eax, dword ptr [ecx + 0x28]
0047A85B  50                                 push eax
0047A85C  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047A85F  e8 ec 63 fc ff                     call 0x440c50
0047A864  83 7d f8 00                        cmp dword ptr [ebp - 8], 0
0047A868  75 3b                              jne 0x47a8a5
0047A86A  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A86D  db 41 30                           fild dword ptr [ecx + 0x30]
0047A870  dc 45 08                           fadd qword ptr [ebp + 8]
0047A873  dd 5d 08                           fstp qword ptr [ebp + 8]
0047A876  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A879  db 42 38                           fild dword ptr [edx + 0x38]
0047A87C  dc 6d 08                           fsubr qword ptr [ebp + 8]
0047A87F  8b 45 ec                           mov eax, dword ptr [ebp - 0x14]
0047A882  dd 98 88 00 00 00                  fstp qword ptr [eax + 0x88]
0047A888  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047A88B  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047A88E  8b 81 88 00 00 00                  mov eax, dword ptr [ecx + 0x88]
0047A894  89 42 20                           mov dword ptr [edx + 0x20], eax
0047A897  8b 89 8c 00 00 00                  mov ecx, dword ptr [ecx + 0x8c]
0047A89D  89 4a 24                           mov dword ptr [edx + 0x24], ecx
0047A8A0  e9 c2 00 00 00                     jmp 0x47a967
0047A8A5  83 7d f8 ff                        cmp dword ptr [ebp - 8], -1
0047A8A9  75 62                              jne 0x47a90d
0047A8AB  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A8AE  db 42 34                           fild dword ptr [edx + 0x34]
0047A8B1  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A8B7  dc 45 10                           fadd qword ptr [ebp + 0x10]
0047A8BA  dd 5d 10                           fstp qword ptr [ebp + 0x10]
0047A8BD  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A8C0  db 40 30                           fild dword ptr [eax + 0x30]
0047A8C3  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A8C9  dc 45 08                           fadd qword ptr [ebp + 8]
0047A8CC  dd 5d 08                           fstp qword ptr [ebp + 8]
0047A8CF  dd 45 f0                           fld qword ptr [ebp - 0x10]
0047A8D2  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A8D8  dd 5d f0                           fstp qword ptr [ebp - 0x10]
0047A8DB  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A8DE  db 41 38                           fild dword ptr [ecx + 0x38]
0047A8E1  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047A8E7  dc 6d 08                           fsubr qword ptr [ebp + 8]
0047A8EA  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047A8ED  dd 9a 88 00 00 00                  fstp qword ptr [edx + 0x88]
0047A8F3  8b 45 ec                           mov eax, dword ptr [ebp - 0x14]
0047A8F6  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047A8F9  8b 90 88 00 00 00                  mov edx, dword ptr [eax + 0x88]
0047A8FF  89 51 20                           mov dword ptr [ecx + 0x20], edx
0047A902  8b 80 8c 00 00 00                  mov eax, dword ptr [eax + 0x8c]
0047A908  89 41 24                           mov dword ptr [ecx + 0x24], eax
0047A90B  eb 5a                              jmp 0x47a967
0047A90D  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A910  db 41 34                           fild dword ptr [ecx + 0x34]
0047A913  dc 6d 10                           fsubr qword ptr [ebp + 0x10]
0047A916  dd 5d 10                           fstp qword ptr [ebp + 0x10]
0047A919  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047A91C  8b 42 30                           mov eax, dword ptr [edx + 0x30]
0047A91F  d1 e0                              shl eax, 1
0047A921  89 45 e8                           mov dword ptr [ebp - 0x18], eax
0047A924  db 45 e8                           fild dword ptr [ebp - 0x18]
0047A927  dc 45 08                           fadd qword ptr [ebp + 8]
0047A92A  dd 5d 08                           fstp qword ptr [ebp + 8]
0047A92D  dd 45 f0                           fld qword ptr [ebp - 0x10]
0047A930  dc c0                              fadd st(0), st(0)
0047A932  dd 5d f0                           fstp qword ptr [ebp - 0x10]
0047A935  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047A938  8b 51 38                           mov edx, dword ptr [ecx + 0x38]
0047A93B  d1 e2                              shl edx, 1
0047A93D  89 55 e4                           mov dword ptr [ebp - 0x1c], edx
0047A940  db 45 e4                           fild dword ptr [ebp - 0x1c]
0047A943  dc 6d 08                           fsubr qword ptr [ebp + 8]
0047A946  8b 45 ec                           mov eax, dword ptr [ebp - 0x14]
0047A949  dd 98 88 00 00 00                  fstp qword ptr [eax + 0x88]
0047A94F  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047A952  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047A955  8b 81 88 00 00 00                  mov eax, dword ptr [ecx + 0x88]
0047A95B  89 42 20                           mov dword ptr [edx + 0x20], eax
0047A95E  8b 89 8c 00 00 00                  mov ecx, dword ptr [ecx + 0x8c]
0047A964  89 4a 24                           mov dword ptr [edx + 0x24], ecx
0047A967  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047A96A  8b 45 08                           mov eax, dword ptr [ebp + 8]
0047A96D  89 82 58 01 00 00                  mov dword ptr [edx + 0x158], eax
0047A973  8b 4d 0c                           mov ecx, dword ptr [ebp + 0xc]
0047A976  89 8a 5c 01 00 00                  mov dword ptr [edx + 0x15c], ecx
0047A97C  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047A97F  8b 45 08                           mov eax, dword ptr [ebp + 8]
0047A982  89 82 f0 00 00 00                  mov dword ptr [edx + 0xf0], eax
0047A988  8b 4d 0c                           mov ecx, dword ptr [ebp + 0xc]
0047A98B  89 8a f4 00 00 00                  mov dword ptr [edx + 0xf4], ecx
0047A991  e9 91 01 00 00                     jmp 0x47ab27
0047A996  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047A999  83 ba b0 01 00 00 00               cmp dword ptr [edx + 0x1b0], 0
0047A9A0  75 36                              jne 0x47a9d8
0047A9A2  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A9A5  8b 48 14                           mov ecx, dword ptr [eax + 0x14]
0047A9A8  51                                 push ecx
0047A9A9  8b 50 10                           mov edx, dword ptr [eax + 0x10]
0047A9AC  52                                 push edx
0047A9AD  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A9B0  8b 48 2c                           mov ecx, dword ptr [eax + 0x2c]
0047A9B3  51                                 push ecx
0047A9B4  8b 50 28                           mov edx, dword ptr [eax + 0x28]
0047A9B7  52                                 push edx
0047A9B8  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A9BB  8b 48 1c                           mov ecx, dword ptr [eax + 0x1c]
0047A9BE  51                                 push ecx
0047A9BF  8b 50 18                           mov edx, dword ptr [eax + 0x18]
0047A9C2  52                                 push edx
0047A9C3  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A9C6  8b 48 24                           mov ecx, dword ptr [eax + 0x24]
0047A9C9  51                                 push ecx
0047A9CA  8b 50 20                           mov edx, dword ptr [eax + 0x20]
0047A9CD  52                                 push edx
0047A9CE  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047A9D1  e8 7a 62 fc ff                     call 0x440c50
0047A9D6  eb 34                              jmp 0x47aa0c
0047A9D8  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A9DB  8b 48 1c                           mov ecx, dword ptr [eax + 0x1c]
0047A9DE  51                                 push ecx
0047A9DF  8b 50 18                           mov edx, dword ptr [eax + 0x18]
0047A9E2  52                                 push edx
0047A9E3  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A9E6  8b 48 2c                           mov ecx, dword ptr [eax + 0x2c]
0047A9E9  51                                 push ecx
0047A9EA  8b 50 28                           mov edx, dword ptr [eax + 0x28]
0047A9ED  52                                 push edx
0047A9EE  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A9F1  8b 48 14                           mov ecx, dword ptr [eax + 0x14]
0047A9F4  51                                 push ecx
0047A9F5  8b 50 10                           mov edx, dword ptr [eax + 0x10]
0047A9F8  52                                 push edx
0047A9F9  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047A9FC  8b 48 24                           mov ecx, dword ptr [eax + 0x24]
0047A9FF  51                                 push ecx
0047AA00  8b 50 20                           mov edx, dword ptr [eax + 0x20]
0047AA03  52                                 push edx
0047AA04  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047AA07  e8 44 62 fc ff                     call 0x440c50
0047AA0C  83 7d f8 00                        cmp dword ptr [ebp - 8], 0
0047AA10  75 41                              jne 0x47aa53
0047AA12  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047AA15  db 40 30                           fild dword ptr [eax + 0x30]
0047AA18  dc 6d 08                           fsubr qword ptr [ebp + 8]
0047AA1B  dd 5d 08                           fstp qword ptr [ebp + 8]
0047AA1E  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047AA21  db 41 38                           fild dword ptr [ecx + 0x38]
0047AA24  dc 45 08                           fadd qword ptr [ebp + 8]
0047AA27  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047AA2A  dd 9a 58 01 00 00                  fstp qword ptr [edx + 0x158]
0047AA30  8b 45 ec                           mov eax, dword ptr [ebp - 0x14]
0047AA33  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047AA36  8b 90 58 01 00 00                  mov edx, dword ptr [eax + 0x158]
0047AA3C  89 91 f0 00 00 00                  mov dword ptr [ecx + 0xf0], edx
0047AA42  8b 80 5c 01 00 00                  mov eax, dword ptr [eax + 0x15c]
0047AA48  89 81 f4 00 00 00                  mov dword ptr [ecx + 0xf4], eax
0047AA4E  e9 b0 00 00 00                     jmp 0x47ab03
0047AA53  83 7d f8 ff                        cmp dword ptr [ebp - 8], -1
0047AA57  75 56                              jne 0x47aaaf
0047AA59  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047AA5C  db 41 30                           fild dword ptr [ecx + 0x30]
0047AA5F  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047AA65  dc 6d 08                           fsubr qword ptr [ebp + 8]
0047AA68  dd 5d 08                           fstp qword ptr [ebp + 8]
0047AA6B  dd 45 f0                           fld qword ptr [ebp - 0x10]
0047AA6E  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047AA74  dd 5d f0                           fstp qword ptr [ebp - 0x10]
0047AA77  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047AA7A  db 42 38                           fild dword ptr [edx + 0x38]
0047AA7D  dc 35 f8 03 4b 00                  fdiv qword ptr [0x4b03f8]
0047AA83  dc 45 08                           fadd qword ptr [ebp + 8]
0047AA86  8b 45 ec                           mov eax, dword ptr [ebp - 0x14]
0047AA89  dd 98 58 01 00 00                  fstp qword ptr [eax + 0x158]
0047AA8F  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047AA92  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047AA95  8b 81 58 01 00 00                  mov eax, dword ptr [ecx + 0x158]
0047AA9B  89 82 f0 00 00 00                  mov dword ptr [edx + 0xf0], eax
0047AAA1  8b 89 5c 01 00 00                  mov ecx, dword ptr [ecx + 0x15c]
0047AAA7  89 8a f4 00 00 00                  mov dword ptr [edx + 0xf4], ecx
0047AAAD  eb 54                              jmp 0x47ab03
0047AAAF  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047AAB2  8b 42 30                           mov eax, dword ptr [edx + 0x30]
0047AAB5  d1 e0                              shl eax, 1
0047AAB7  89 45 e0                           mov dword ptr [ebp - 0x20], eax
0047AABA  db 45 e0                           fild dword ptr [ebp - 0x20]
0047AABD  dc 6d 08                           fsubr qword ptr [ebp + 8]
0047AAC0  dd 5d 08                           fstp qword ptr [ebp + 8]
0047AAC3  dd 45 f0                           fld qword ptr [ebp - 0x10]
0047AAC6  dc c0                              fadd st(0), st(0)
0047AAC8  dd 5d f0                           fstp qword ptr [ebp - 0x10]
0047AACB  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047AACE  8b 51 38                           mov edx, dword ptr [ecx + 0x38]
0047AAD1  d1 e2                              shl edx, 1
0047AAD3  89 55 dc                           mov dword ptr [ebp - 0x24], edx
0047AAD6  db 45 dc                           fild dword ptr [ebp - 0x24]
0047AAD9  dc 45 08                           fadd qword ptr [ebp + 8]
0047AADC  8b 45 ec                           mov eax, dword ptr [ebp - 0x14]
0047AADF  dd 98 58 01 00 00                  fstp qword ptr [eax + 0x158]
0047AAE5  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047AAE8  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047AAEB  8b 81 58 01 00 00                  mov eax, dword ptr [ecx + 0x158]
0047AAF1  89 82 f0 00 00 00                  mov dword ptr [edx + 0xf0], eax
0047AAF7  8b 89 5c 01 00 00                  mov ecx, dword ptr [ecx + 0x15c]
0047AAFD  89 8a f4 00 00 00                  mov dword ptr [edx + 0xf4], ecx
0047AB03  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047AB06  8b 45 08                           mov eax, dword ptr [ebp + 8]
0047AB09  89 82 88 00 00 00                  mov dword ptr [edx + 0x88], eax
0047AB0F  8b 4d 0c                           mov ecx, dword ptr [ebp + 0xc]
0047AB12  89 8a 8c 00 00 00                  mov dword ptr [edx + 0x8c], ecx
0047AB18  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047AB1B  8b 45 08                           mov eax, dword ptr [ebp + 8]
0047AB1E  89 42 20                           mov dword ptr [edx + 0x20], eax
0047AB21  8b 4d 0c                           mov ecx, dword ptr [ebp + 0xc]
0047AB24  89 4a 24                           mov dword ptr [edx + 0x24], ecx
0047AB27  dd 45 10                           fld qword ptr [ebp + 0x10]
0047AB2A  dc 65 f0                           fsub qword ptr [ebp - 0x10]
0047AB2D  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047AB30  dd 9a 60 01 00 00                  fstp qword ptr [edx + 0x160]
0047AB36  8b 45 ec                           mov eax, dword ptr [ebp - 0x14]
0047AB39  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047AB3C  8b 90 60 01 00 00                  mov edx, dword ptr [eax + 0x160]
0047AB42  89 51 28                           mov dword ptr [ecx + 0x28], edx
0047AB45  8b 80 64 01 00 00                  mov eax, dword ptr [eax + 0x164]
0047AB4B  89 41 2c                           mov dword ptr [ecx + 0x2c], eax
0047AB4E  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047AB51  8b 55 10                           mov edx, dword ptr [ebp + 0x10]
0047AB54  89 91 f8 00 00 00                  mov dword ptr [ecx + 0xf8], edx
0047AB5A  8b 45 14                           mov eax, dword ptr [ebp + 0x14]
0047AB5D  89 81 fc 00 00 00                  mov dword ptr [ecx + 0xfc], eax
0047AB63  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047AB66  8b 55 10                           mov edx, dword ptr [ebp + 0x10]
0047AB69  89 91 90 00 00 00                  mov dword ptr [ecx + 0x90], edx
0047AB6F  8b 45 14                           mov eax, dword ptr [ebp + 0x14]
0047AB72  89 81 94 00 00 00                  mov dword ptr [ecx + 0x94], eax
0047AB78  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047AB7B  51                                 push ecx
0047AB7C  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047AB7F  e8 12 00 00 00                     call 0x47ab96
0047AB84  6a 01                              push 1
0047AB86  b9 80 cd 58 00                     mov ecx, 0x58cd80
0047AB8B  e8 99 63 ff ff                     call 0x470f29
0047AB90  8b e5                              mov esp, ebp
0047AB92  5d                                 pop ebp
0047AB93  c2 10 00                           ret 0x10

; VA 00440C50 PE offset 265296 bytes 169 SHA256 6d40e3c40cec7c5483f1a91f50831fd0d31b8a6c1fa212bdc8a018b3bc312278; recovered
00440C50  55                                 push ebp
00440C51  8b ec                              mov ebp, esp
00440C53  51                                 push ecx
00440C54  89 4d fc                           mov dword ptr [ebp - 4], ecx
00440C57  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00440C5A  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00440C5D  89 88 98 00 00 00                  mov dword ptr [eax + 0x98], ecx
00440C63  8b 55 0c                           mov edx, dword ptr [ebp + 0xc]
00440C66  89 90 9c 00 00 00                  mov dword ptr [eax + 0x9c], edx
00440C6C  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00440C6F  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00440C72  89 48 30                           mov dword ptr [eax + 0x30], ecx
00440C75  8b 55 0c                           mov edx, dword ptr [ebp + 0xc]
00440C78  89 50 34                           mov dword ptr [eax + 0x34], edx
00440C7B  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00440C7E  8b 4d 18                           mov ecx, dword ptr [ebp + 0x18]
00440C81  89 88 68 01 00 00                  mov dword ptr [eax + 0x168], ecx
00440C87  8b 55 1c                           mov edx, dword ptr [ebp + 0x1c]
00440C8A  89 90 6c 01 00 00                  mov dword ptr [eax + 0x16c], edx
00440C90  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00440C93  8b 4d 18                           mov ecx, dword ptr [ebp + 0x18]
00440C96  89 88 00 01 00 00                  mov dword ptr [eax + 0x100], ecx
00440C9C  8b 55 1c                           mov edx, dword ptr [ebp + 0x1c]
00440C9F  89 90 04 01 00 00                  mov dword ptr [eax + 0x104], edx
00440CA5  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00440CA8  8b 4d 10                           mov ecx, dword ptr [ebp + 0x10]
00440CAB  89 88 70 01 00 00                  mov dword ptr [eax + 0x170], ecx
00440CB1  8b 55 14                           mov edx, dword ptr [ebp + 0x14]
00440CB4  89 90 74 01 00 00                  mov dword ptr [eax + 0x174], edx
00440CBA  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00440CBD  8b 4d 10                           mov ecx, dword ptr [ebp + 0x10]
00440CC0  89 48 38                           mov dword ptr [eax + 0x38], ecx
00440CC3  8b 55 14                           mov edx, dword ptr [ebp + 0x14]
00440CC6  89 50 3c                           mov dword ptr [eax + 0x3c], edx
00440CC9  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00440CCC  8b 4d 20                           mov ecx, dword ptr [ebp + 0x20]
00440CCF  89 88 08 01 00 00                  mov dword ptr [eax + 0x108], ecx
00440CD5  8b 55 24                           mov edx, dword ptr [ebp + 0x24]
00440CD8  89 90 0c 01 00 00                  mov dword ptr [eax + 0x10c], edx
00440CDE  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00440CE1  8b 4d 20                           mov ecx, dword ptr [ebp + 0x20]
00440CE4  89 88 a0 00 00 00                  mov dword ptr [eax + 0xa0], ecx
00440CEA  8b 55 24                           mov edx, dword ptr [ebp + 0x24]
00440CED  89 90 a4 00 00 00                  mov dword ptr [eax + 0xa4], edx
00440CF3  8b e5                              mov esp, ebp
00440CF5  5d                                 pop ebp
00440CF6  c2 20 00                           ret 0x20

; VA 0047AB96 PE offset 502678 bytes 36 SHA256 f9cabb25b6727a0a62e151ef3e1485bc9161f509b5de72d4e50203becc0343bf; recovered
0047AB96  55                                 push ebp
0047AB97  8b ec                              mov ebp, esp
0047AB99  51                                 push ecx
0047AB9A  89 4d fc                           mov dword ptr [ebp - 4], ecx
0047AB9D  8b 45 08                           mov eax, dword ptr [ebp + 8]
0047ABA0  8b 48 40                           mov ecx, dword ptr [eax + 0x40]
0047ABA3  51                                 push ecx
0047ABA4  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047ABA7  e8 84 60 fc ff                     call 0x440c30
0047ABAC  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047ABAF  e8 6c 9d f9 ff                     call 0x414920
0047ABB4  8b e5                              mov esp, ebp
0047ABB6  5d                                 pop ebp
0047ABB7  c2 04 00                           ret 4

; VA 00440C30 PE offset 265264 bytes 25 SHA256 884e31507e541b231d3e767f6d563f0c137594fc226de99558f32b10cc4c10b2; recovered
00440C30  55                                 push ebp
00440C31  8b ec                              mov ebp, esp
00440C33  51                                 push ecx
00440C34  89 4d fc                           mov dword ptr [ebp - 4], ecx
00440C37  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00440C3A  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00440C3D  89 88 a8 01 00 00                  mov dword ptr [eax + 0x1a8], ecx
00440C43  8b e5                              mov esp, ebp
00440C45  5d                                 pop ebp
00440C46  c2 04 00                           ret 4

; VA 00414920 PE offset 84256 bytes 231 SHA256 e0acd79cac30df31f6f2da3b9b9d3377f412ffcb46ebc8bf714f758abaa193d8; context only, not ported
00414920  55                                 push ebp
00414921  8b ec                              mov ebp, esp
00414923  83 ec 08                           sub esp, 8
00414926  56                                 push esi
00414927  89 4d f8                           mov dword ptr [ebp - 8], ecx
0041492A  8b 45 f8                           mov eax, dword ptr [ebp - 8]
0041492D  8b 88 a8 01 00 00                  mov ecx, dword ptr [eax + 0x1a8]
00414933  51                                 push ecx
00414934  b9 48 cb 58 00                     mov ecx, 0x58cb48
00414939  e8 d2 00 00 00                     call 0x414a10
0041493E  89 45 fc                           mov dword ptr [ebp - 4], eax
00414941  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00414944  e8 d6 dc 05 00                     call 0x47261f
00414949  8b 55 f8                           mov edx, dword ptr [ebp - 8]
0041494C  8b 02                              mov eax, dword ptr [edx]
0041494E  50                                 push eax
0041494F  e8 2c 04 00 00                     call 0x414d80
00414954  83 c4 04                           add esp, 4
00414957  b9 78 d5 58 00                     mov ecx, 0x58d578
0041495C  e8 0f 02 00 00                     call 0x414b70
00414961  33 c9                              xor ecx, ecx
00414963  8a 0d 60 61 5d 00                  mov cl, byte ptr [0x5d6160]
00414969  83 e1 01                           and ecx, 1
0041496C  85 c9                              test ecx, ecx
0041496E  75 26                              jne 0x414996
00414970  8a 15 60 61 5d 00                  mov dl, byte ptr [0x5d6160]
00414976  80 ca 01                           or dl, 1
00414979  88 15 60 61 5d 00                  mov byte ptr [0x5d6160], dl
0041497F  b9 70 61 5d 00                     mov ecx, 0x5d6170
00414984  e8 87 05 00 00                     call 0x414f10
00414989  68 a0 50 41 00                     push 0x4150a0
0041498E  e8 96 81 08 00                     call 0x49cb29
00414993  83 c4 04                           add esp, 4
00414996  6a 04                              push 4
00414998  b9 70 61 5d 00                     mov ecx, 0x5d6170
0041499D  e8 be 00 00 00                     call 0x414a60
004149A2  8b 75 f8                           mov esi, dword ptr [ebp - 8]
004149A5  83 c6 08                           add esi, 8
004149A8  6a 00                              push 0
004149AA  b9 70 61 5d 00                     mov ecx, 0x5d6170
004149AF  e8 8c 00 00 00                     call 0x414a40
004149B4  89 30                              mov dword ptr [eax], esi
004149B6  8b 75 f8                           mov esi, dword ptr [ebp - 8]
004149B9  81 c6 40 01 00 00                  add esi, 0x140
004149BF  6a 01                              push 1
004149C1  b9 70 61 5d 00                     mov ecx, 0x5d6170
004149C6  e8 75 00 00 00                     call 0x414a40
004149CB  89 30                              mov dword ptr [eax], esi
004149CD  8b 75 f8                           mov esi, dword ptr [ebp - 8]
004149D0  81 c6 d8 00 00 00                  add esi, 0xd8
004149D6  6a 02                              push 2
004149D8  b9 70 61 5d 00                     mov ecx, 0x5d6170
004149DD  e8 5e 00 00 00                     call 0x414a40
004149E2  89 30                              mov dword ptr [eax], esi
004149E4  8b 75 f8                           mov esi, dword ptr [ebp - 8]
004149E7  83 c6 70                           add esi, 0x70
004149EA  6a 03                              push 3
004149EC  b9 70 61 5d 00                     mov ecx, 0x5d6170
004149F1  e8 4a 00 00 00                     call 0x414a40
004149F6  89 30                              mov dword ptr [eax], esi
004149F8  b9 70 61 5d 00                     mov ecx, 0x5d6170
004149FD  e8 bf 89 05 00                     call 0x46d3c1
00414A02  5e                                 pop esi
00414A03  8b e5                              mov esp, ebp
00414A05  5d                                 pop ebp
00414A06  c3                                 ret

; VA 00468D00 PE offset 429312 bytes 30 SHA256 cd4f5020ae29638cd3347814186c48c1bf65ae888d92c953c19f521cc140d753; recovered
00468D00  55                                 push ebp
00468D01  8b ec                              mov ebp, esp
00468D03  51                                 push ecx
00468D04  89 4d fc                           mov dword ptr [ebp - 4], ecx
00468D07  8b 45 08                           mov eax, dword ptr [ebp + 8]
00468D0A  69 c0 98 00 00 00                  imul eax, eax, 0x98
00468D10  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00468D13  8b 51 0c                           mov edx, dword ptr [ecx + 0xc]
00468D16  03 c2                              add eax, edx
00468D18  8b e5                              mov esp, ebp
00468D1A  5d                                 pop ebp
00468D1B  c2 04 00                           ret 4

; VA 00468CE0 PE offset 429280 bytes 27 SHA256 e041a9b5519845f716b94875e5e31131ee18a69b1a2421a976ee5ecb04a1bbe4; recovered
00468CE0  55                                 push ebp
00468CE1  8b ec                              mov ebp, esp
00468CE3  51                                 push ecx
00468CE4  89 4d fc                           mov dword ptr [ebp - 4], ecx
00468CE7  8b 45 08                           mov eax, dword ptr [ebp + 8]
00468CEA  c1 e0 04                           shl eax, 4
00468CED  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00468CF0  8b 51 0c                           mov edx, dword ptr [ecx + 0xc]
00468CF3  03 c2                              add eax, edx
00468CF5  8b e5                              mov esp, ebp
00468CF7  5d                                 pop ebp
00468CF8  c2 04 00                           ret 4

; VA 0047ABC0 PE offset 502720 bytes 27 SHA256 dec4947d33139e09e25685b4f6aa1ed34b7eec203d39a293f6ee0d76cac06acb; recovered
0047ABC0  55                                 push ebp
0047ABC1  8b ec                              mov ebp, esp
0047ABC3  51                                 push ecx
0047ABC4  89 4d fc                           mov dword ptr [ebp - 4], ecx
0047ABC7  8b 45 08                           mov eax, dword ptr [ebp + 8]
0047ABCA  6b c0 48                           imul eax, eax, 0x48
0047ABCD  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047ABD0  8b 51 0c                           mov edx, dword ptr [ecx + 0xc]
0047ABD3  03 c2                              add eax, edx
0047ABD5  8b e5                              mov esp, ebp
0047ABD7  5d                                 pop ebp
0047ABD8  c2 04 00                           ret 4
