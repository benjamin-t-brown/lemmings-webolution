; VA 0x00446040, file offset 286784, length 312, SHA256 74cf544a3840fb929bfc3ab58fea77a73847fb8e67ffa03b689c331d3767435f
00446040  55                           push     ebp
00446041  8bec                         mov      ebp, esp
00446043  6aff                         push     -1
00446045  6848c44a00                   push     0x4ac448
0044604A  64a100000000                 mov      eax, dword ptr fs:[0]
00446050  50                           push     eax
00446051  64892500000000               mov      dword ptr fs:[0], esp
00446058  81ec28010000                 sub      esp, 0x128
0044605E  898ddcfeffff                 mov      dword ptr [ebp - 0x124], ecx
00446064  c745f000000000               mov      dword ptr [ebp - 0x10], 0
0044606B  8b8ddcfeffff                 mov      ecx, dword ptr [ebp - 0x124]
00446071  83c130                       add      ecx, 0x30
00446074  e8c705fcff                   call     0x406640
00446079  3945f0                       cmp      dword ptr [ebp - 0x10], eax
0044607C  7d29                         jge      0x4460a7
0044607E  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
00446081  50                           push     eax
00446082  8b8ddcfeffff                 mov      ecx, dword ptr [ebp - 0x124]
00446088  83c130                       add      ecx, 0x30
0044608B  e810a7feff                   call     0x4307a0
00446090  33c9                         xor      ecx, ecx
00446092  8a8884000000                 mov      cl, byte ptr [eax + 0x84]
00446098  85c9                         test     ecx, ecx
0044609A  740b                         je       0x4460a7
0044609C  8b55f0                       mov      edx, dword ptr [ebp - 0x10]
0044609F  83c201                       add      edx, 1
004460A2  8955f0                       mov      dword ptr [ebp - 0x10], edx
004460A5  ebc4                         jmp      0x44606b
004460A7  8b8ddcfeffff                 mov      ecx, dword ptr [ebp - 0x124]
004460AD  83c130                       add      ecx, 0x30
004460B0  e88b05fcff                   call     0x406640
004460B5  3945f0                       cmp      dword ptr [ebp - 0x10], eax
004460B8  7551                         jne      0x44610b
004460BA  8b4508                       mov      eax, dword ptr [ebp + 8]
004460BD  50                           push     eax
004460BE  8d8d68ffffff                 lea      ecx, [ebp - 0x98]
004460C4  e8a7300000                   call     0x449170
004460C9  8985d8feffff                 mov      dword ptr [ebp - 0x128], eax
004460CF  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
004460D5  898dd4feffff                 mov      dword ptr [ebp - 0x12c], ecx
004460DB  c745fc00000000               mov      dword ptr [ebp - 4], 0
004460E2  8b95d4feffff                 mov      edx, dword ptr [ebp - 0x12c]
004460E8  52                           push     edx
004460E9  8b8ddcfeffff                 mov      ecx, dword ptr [ebp - 0x124]
004460EF  83c130                       add      ecx, 0x30
004460F2  e8b93c0000                   call     0x449db0
004460F7  c745fcffffffff               mov      dword ptr [ebp - 4], 0xffffffff
004460FE  8d8d68ffffff                 lea      ecx, [ebp - 0x98]
00446104  e8a7300000                   call     0x4491b0
00446109  eb5a                         jmp      0x446165
0044610B  8b4508                       mov      eax, dword ptr [ebp + 8]
0044610E  50                           push     eax
0044610F  8d8de0feffff                 lea      ecx, [ebp - 0x120]
00446115  e856300000                   call     0x449170
0044611A  8985d0feffff                 mov      dword ptr [ebp - 0x130], eax
00446120  8b8dd0feffff                 mov      ecx, dword ptr [ebp - 0x130]
00446126  898dccfeffff                 mov      dword ptr [ebp - 0x134], ecx
0044612C  c745fc01000000               mov      dword ptr [ebp - 4], 1
00446133  8b95ccfeffff                 mov      edx, dword ptr [ebp - 0x134]
00446139  52                           push     edx
0044613A  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
0044613D  50                           push     eax
0044613E  8b8ddcfeffff                 mov      ecx, dword ptr [ebp - 0x124]
00446144  83c130                       add      ecx, 0x30
00446147  e854a6feff                   call     0x4307a0
0044614C  8bc8                         mov      ecx, eax
0044614E  e8fd2d0000                   call     0x448f50
00446153  c745fcffffffff               mov      dword ptr [ebp - 4], 0xffffffff
0044615A  8d8de0feffff                 lea      ecx, [ebp - 0x120]
00446160  e84b300000                   call     0x4491b0
00446165  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
00446168  8b4df4                       mov      ecx, dword ptr [ebp - 0xc]
0044616B  64890d00000000               mov      dword ptr fs:[0], ecx
00446172  8be5                         mov      esp, ebp
00446174  5d                           pop      ebp
00446175  c20400                       ret      4

; VA 0x00449170, file offset 299376, length 51, SHA256 a3a639d2e2b30828ddefe7022bd134452b827c4b38a59d6ae9d2d49e8352db70
00449170  55                           push     ebp
00449171  8bec                         mov      ebp, esp
00449173  51                           push     ecx
00449174  894dfc                       mov      dword ptr [ebp - 4], ecx
00449177  8b4508                       mov      eax, dword ptr [ebp + 8]
0044917A  50                           push     eax
0044917B  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0044917E  e80d48fdff                   call     0x41d990
00449183  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
00449186  c7818000000000000000         mov      dword ptr [ecx + 0x80], 0
00449190  8b55fc                       mov      edx, dword ptr [ebp - 4]
00449193  c6828400000001               mov      byte ptr [edx + 0x84], 1
0044919A  8b45fc                       mov      eax, dword ptr [ebp - 4]
0044919D  8be5                         mov      esp, ebp
0044919F  5d                           pop      ebp
004491A0  c20400                       ret      4

; VA 0x00449DB0, file offset 302512, length 363, SHA256 1dd4eaf471b7356df2af50e9e6bb1b88df4b441504fcca1ddae49b44fa5ce8ca
00449DB0  55                           push     ebp
00449DB1  8bec                         mov      ebp, esp
00449DB3  6aff                         push     -1
00449DB5  689bc74a00                   push     0x4ac79b
00449DBA  64a100000000                 mov      eax, dword ptr fs:[0]
00449DC0  50                           push     eax
00449DC1  64892500000000               mov      dword ptr fs:[0], esp
00449DC8  83ec28                       sub      esp, 0x28
00449DCB  894dd4                       mov      dword ptr [ebp - 0x2c], ecx
00449DCE  8b45d4                       mov      eax, dword ptr [ebp - 0x2c]
00449DD1  8b4dd4                       mov      ecx, dword ptr [ebp - 0x2c]
00449DD4  8b10                         mov      edx, dword ptr [eax]
00449DD6  3b5104                       cmp      edx, dword ptr [ecx + 4]
00449DD9  0f8c03010000                 jl       0x449ee2
00449DDF  8b45d4                       mov      eax, dword ptr [ebp - 0x2c]
00449DE2  8b4804                       mov      ecx, dword ptr [eax + 4]
00449DE5  8b55d4                       mov      edx, dword ptr [ebp - 0x2c]
00449DE8  034a08                       add      ecx, dword ptr [edx + 8]
00449DEB  8b45d4                       mov      eax, dword ptr [ebp - 0x2c]
00449DEE  894804                       mov      dword ptr [eax + 4], ecx
00449DF1  8b4dd4                       mov      ecx, dword ptr [ebp - 0x2c]
00449DF4  8b5104                       mov      edx, dword ptr [ecx + 4]
00449DF7  8955f0                       mov      dword ptr [ebp - 0x10], edx
00449DFA  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
00449DFD  69c088000000                 imul     eax, eax, 0x88
00449E03  83c004                       add      eax, 4
00449E06  50                           push     eax
00449E07  e89c230500                   call     0x49c1a8
00449E0C  83c404                       add      esp, 4
00449E0F  8945e0                       mov      dword ptr [ebp - 0x20], eax
00449E12  c745fc00000000               mov      dword ptr [ebp - 4], 0
00449E19  837de000                     cmp      dword ptr [ebp - 0x20], 0
00449E1D  7432                         je       0x449e51
00449E1F  68b0914400                   push     0x4491b0
00449E24  6860be4400                   push     0x44be60
00449E29  8b4de0                       mov      ecx, dword ptr [ebp - 0x20]
00449E2C  8b55f0                       mov      edx, dword ptr [ebp - 0x10]
00449E2F  8911                         mov      dword ptr [ecx], edx
00449E31  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
00449E34  50                           push     eax
00449E35  6888000000                   push     0x88
00449E3A  8b4de0                       mov      ecx, dword ptr [ebp - 0x20]
00449E3D  83c104                       add      ecx, 4
00449E40  51                           push     ecx
00449E41  e88d270500                   call     0x49c5d3
00449E46  8b55e0                       mov      edx, dword ptr [ebp - 0x20]
00449E49  83c204                       add      edx, 4
00449E4C  8955d0                       mov      dword ptr [ebp - 0x30], edx
00449E4F  eb07                         jmp      0x449e58
00449E51  c745d000000000               mov      dword ptr [ebp - 0x30], 0
00449E58  8b45d0                       mov      eax, dword ptr [ebp - 0x30]
00449E5B  8945e4                       mov      dword ptr [ebp - 0x1c], eax
00449E5E  c745fcffffffff               mov      dword ptr [ebp - 4], 0xffffffff
00449E65  8b4de4                       mov      ecx, dword ptr [ebp - 0x1c]
00449E68  894de8                       mov      dword ptr [ebp - 0x18], ecx
00449E6B  c745ec00000000               mov      dword ptr [ebp - 0x14], 0
00449E72  eb09                         jmp      0x449e7d
00449E74  8b55ec                       mov      edx, dword ptr [ebp - 0x14]
00449E77  83c201                       add      edx, 1
00449E7A  8955ec                       mov      dword ptr [ebp - 0x14], edx
00449E7D  8b45d4                       mov      eax, dword ptr [ebp - 0x2c]
00449E80  8b4dec                       mov      ecx, dword ptr [ebp - 0x14]
00449E83  3b08                         cmp      ecx, dword ptr [eax]
00449E85  7d27                         jge      0x449eae
00449E87  8b55ec                       mov      edx, dword ptr [ebp - 0x14]
00449E8A  69d288000000                 imul     edx, edx, 0x88
00449E90  8b45d4                       mov      eax, dword ptr [ebp - 0x2c]
00449E93  8b480c                       mov      ecx, dword ptr [eax + 0xc]
00449E96  03ca                         add      ecx, edx
00449E98  51                           push     ecx
00449E99  8b55ec                       mov      edx, dword ptr [ebp - 0x14]
00449E9C  69d288000000                 imul     edx, edx, 0x88
00449EA2  8b4de8                       mov      ecx, dword ptr [ebp - 0x18]
00449EA5  03ca                         add      ecx, edx
00449EA7  e8a4f0ffff                   call     0x448f50
00449EAC  ebc6                         jmp      0x449e74
00449EAE  8b45d4                       mov      eax, dword ptr [ebp - 0x2c]
00449EB1  8b480c                       mov      ecx, dword ptr [eax + 0xc]
00449EB4  894dd8                       mov      dword ptr [ebp - 0x28], ecx
00449EB7  8b55d8                       mov      edx, dword ptr [ebp - 0x28]
00449EBA  8955dc                       mov      dword ptr [ebp - 0x24], edx
00449EBD  837ddc00                     cmp      dword ptr [ebp - 0x24], 0
00449EC1  740f                         je       0x449ed2
00449EC3  6a03                         push     3
00449EC5  8b4ddc                       mov      ecx, dword ptr [ebp - 0x24]
00449EC8  e8631b0000                   call     0x44ba30
00449ECD  8945cc                       mov      dword ptr [ebp - 0x34], eax
00449ED0  eb07                         jmp      0x449ed9
00449ED2  c745cc00000000               mov      dword ptr [ebp - 0x34], 0
00449ED9  8b45d4                       mov      eax, dword ptr [ebp - 0x2c]
00449EDC  8b4de8                       mov      ecx, dword ptr [ebp - 0x18]
00449EDF  89480c                       mov      dword ptr [eax + 0xc], ecx
00449EE2  8b5508                       mov      edx, dword ptr [ebp + 8]
00449EE5  52                           push     edx
00449EE6  8b45d4                       mov      eax, dword ptr [ebp - 0x2c]
00449EE9  8b08                         mov      ecx, dword ptr [eax]
00449EEB  69c988000000                 imul     ecx, ecx, 0x88
00449EF1  8b55d4                       mov      edx, dword ptr [ebp - 0x2c]
00449EF4  8b420c                       mov      eax, dword ptr [edx + 0xc]
00449EF7  03c8                         add      ecx, eax
00449EF9  e852f0ffff                   call     0x448f50
00449EFE  8b4dd4                       mov      ecx, dword ptr [ebp - 0x2c]
00449F01  8b11                         mov      edx, dword ptr [ecx]
00449F03  83c201                       add      edx, 1
00449F06  8b45d4                       mov      eax, dword ptr [ebp - 0x2c]
00449F09  8910                         mov      dword ptr [eax], edx
00449F0B  8b4df4                       mov      ecx, dword ptr [ebp - 0xc]
00449F0E  64890d00000000               mov      dword ptr fs:[0], ecx
00449F15  8be5                         mov      esp, ebp
00449F17  5d                           pop      ebp
00449F18  c20400                       ret      4

; VA 0x00448F50, file offset 298832, length 64, SHA256 a11452cb9eb4b8606f13b3de42a75166cc3725424bda3421088dc248db8443ef
00448F50  55                           push     ebp
00448F51  8bec                         mov      ebp, esp
00448F53  51                           push     ecx
00448F54  894dfc                       mov      dword ptr [ebp - 4], ecx
00448F57  8b4508                       mov      eax, dword ptr [ebp + 8]
00448F5A  50                           push     eax
00448F5B  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
00448F5E  e88da7fbff                   call     0x4036f0
00448F63  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
00448F66  8b5508                       mov      edx, dword ptr [ebp + 8]
00448F69  8b8280000000                 mov      eax, dword ptr [edx + 0x80]
00448F6F  898180000000                 mov      dword ptr [ecx + 0x80], eax
00448F75  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
00448F78  8b5508                       mov      edx, dword ptr [ebp + 8]
00448F7B  8a8284000000                 mov      al, byte ptr [edx + 0x84]
00448F81  888184000000                 mov      byte ptr [ecx + 0x84], al
00448F87  8b45fc                       mov      eax, dword ptr [ebp - 4]
00448F8A  8be5                         mov      esp, ebp
00448F8C  5d                           pop      ebp
00448F8D  c20400                       ret      4

; VA 0x0041D990, file offset 121232, length 34, SHA256 601f9b14fddb7b8835707d6e64f8286deed8105db40677f59e03f2d7469e2f1e
0041D990  55                           push     ebp
0041D991  8bec                         mov      ebp, esp
0041D993  51                           push     ecx
0041D994  894dfc                       mov      dword ptr [ebp - 4], ecx
0041D997  8b45fc                       mov      eax, dword ptr [ebp - 4]
0041D99A  c60000                       mov      byte ptr [eax], 0
0041D99D  8b4d08                       mov      ecx, dword ptr [ebp + 8]
0041D9A0  51                           push     ecx
0041D9A1  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0041D9A4  e8875dfeff                   call     0x403730
0041D9A9  8b45fc                       mov      eax, dword ptr [ebp - 4]
0041D9AC  8be5                         mov      esp, ebp
0041D9AE  5d                           pop      ebp
0041D9AF  c20400                       ret      4

; VA 0x00403730, file offset 14128, length 29, SHA256 8e80e9e25c15e694e4d6d739cf16c6fc36a6602d56a2280629b070498887476c
00403730  55                           push     ebp
00403731  8bec                         mov      ebp, esp
00403733  51                           push     ecx
00403734  894dfc                       mov      dword ptr [ebp - 4], ecx
00403737  8b4508                       mov      eax, dword ptr [ebp + 8]
0040373A  50                           push     eax
0040373B  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0040373E  51                           push     ecx
0040373F  e80c900900                   call     0x49c750
00403744  83c408                       add      esp, 8
00403747  8be5                         mov      esp, ebp
00403749  5d                           pop      ebp
0040374A  c20400                       ret      4

; VA 0x004036F0, file offset 14064, length 33, SHA256 d521bf6126450a43521d6cd9e20dee9184fa73048b0363c5196086931fcbf8e2
004036F0  55                           push     ebp
004036F1  8bec                         mov      ebp, esp
004036F3  51                           push     ecx
004036F4  894dfc                       mov      dword ptr [ebp - 4], ecx
004036F7  8b4d08                       mov      ecx, dword ptr [ebp + 8]
004036FA  e821000000                   call     0x403720
004036FF  50                           push     eax
00403700  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
00403703  e828000000                   call     0x403730
00403708  8b45fc                       mov      eax, dword ptr [ebp - 4]
0040370B  8be5                         mov      esp, ebp
0040370D  5d                           pop      ebp
0040370E  c20400                       ret      4

; VA 0x0044BA30, file offset 309808, length 115, SHA256 5827cf1987a02e0cb1ea9bcc7387a551f88db5acdd97c04aa573d5fb21e89b91
0044BA30  55                           push     ebp
0044BA31  8bec                         mov      ebp, esp
0044BA33  51                           push     ecx
0044BA34  894dfc                       mov      dword ptr [ebp - 4], ecx
0044BA37  8b4508                       mov      eax, dword ptr [ebp + 8]
0044BA3A  83e002                       and      eax, 2
0044BA3D  85c0                         test     eax, eax
0044BA3F  743b                         je       0x44ba7c
0044BA41  68b0914400                   push     0x4491b0
0044BA46  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0044BA49  8b51fc                       mov      edx, dword ptr [ecx - 4]
0044BA4C  52                           push     edx
0044BA4D  6888000000                   push     0x88
0044BA52  8b45fc                       mov      eax, dword ptr [ebp - 4]
0044BA55  50                           push     eax
0044BA56  e8fa0b0500                   call     0x49c655
0044BA5B  8b4d08                       mov      ecx, dword ptr [ebp + 8]
0044BA5E  83e101                       and      ecx, 1
0044BA61  85c9                         test     ecx, ecx
0044BA63  740f                         je       0x44ba74
0044BA65  8b55fc                       mov      edx, dword ptr [ebp - 4]
0044BA68  83ea04                       sub      edx, 4
0044BA6B  52                           push     edx
0044BA6C  e8cc060500                   call     0x49c13d
0044BA71  83c404                       add      esp, 4
0044BA74  8b45fc                       mov      eax, dword ptr [ebp - 4]
0044BA77  83e804                       sub      eax, 4
0044BA7A  eb21                         jmp      0x44ba9d
0044BA7C  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0044BA7F  e82cd7ffff                   call     0x4491b0
0044BA84  8b4508                       mov      eax, dword ptr [ebp + 8]
0044BA87  83e001                       and      eax, 1
0044BA8A  85c0                         test     eax, eax
0044BA8C  740c                         je       0x44ba9a
0044BA8E  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0044BA91  51                           push     ecx
0044BA92  e8a6060500                   call     0x49c13d
0044BA97  83c404                       add      esp, 4
0044BA9A  8b45fc                       mov      eax, dword ptr [ebp - 4]
0044BA9D  8be5                         mov      esp, ebp
0044BA9F  5d                           pop      ebp
0044BAA0  c20400                       ret      4

; VA 0x00403720, file offset 14112, length 14, SHA256 befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c
00403720  55                           push     ebp
00403721  8bec                         mov      ebp, esp
00403723  51                           push     ecx
00403724  894dfc                       mov      dword ptr [ebp - 4], ecx
00403727  8b45fc                       mov      eax, dword ptr [ebp - 4]
0040372A  8be5                         mov      esp, ebp
0040372C  5d                           pop      ebp
0040372D  c3                           ret

; VA 0x0049C750, file offset 640848, length 240, SHA256 de277c53bfccd0f688de14be2f3a7258615d8c770e4bd3f7cc88a8e29afb91b9
0049C750  57                           push     edi
0049C751  8b7c2408                     mov      edi, dword ptr [esp + 8]
0049C755  eb6a                         jmp      0x49c7c1
0049C757  8da42400000000               lea      esp, [esp]
0049C75E  8bff                         mov      edi, edi
0049C760  8b4c2404                     mov      ecx, dword ptr [esp + 4]
0049C764  57                           push     edi
0049C765  f7c103000000                 test     ecx, 3
0049C76B  740f                         je       0x49c77c
0049C76D  8a01                         mov      al, byte ptr [ecx]
0049C76F  41                           inc      ecx
0049C770  84c0                         test     al, al
0049C772  743b                         je       0x49c7af
0049C774  f7c103000000                 test     ecx, 3
0049C77A  75f1                         jne      0x49c76d
0049C77C  8b01                         mov      eax, dword ptr [ecx]
0049C77E  bafffefe7e                   mov      edx, 0x7efefeff
0049C783  03d0                         add      edx, eax
0049C785  83f0ff                       xor      eax, 0xffffffff
0049C788  33c2                         xor      eax, edx
0049C78A  83c104                       add      ecx, 4
0049C78D  a900010181                   test     eax, 0x81010100
0049C792  74e8                         je       0x49c77c
0049C794  8b41fc                       mov      eax, dword ptr [ecx - 4]
0049C797  84c0                         test     al, al
0049C799  7423                         je       0x49c7be
0049C79B  84e4                         test     ah, ah
0049C79D  741a                         je       0x49c7b9
0049C79F  a90000ff00                   test     eax, 0xff0000
0049C7A4  740e                         je       0x49c7b4
0049C7A6  a9000000ff                   test     eax, 0xff000000
0049C7AB  7402                         je       0x49c7af
0049C7AD  ebcd                         jmp      0x49c77c
0049C7AF  8d79ff                       lea      edi, [ecx - 1]
0049C7B2  eb0d                         jmp      0x49c7c1
0049C7B4  8d79fe                       lea      edi, [ecx - 2]
0049C7B7  eb08                         jmp      0x49c7c1
0049C7B9  8d79fd                       lea      edi, [ecx - 3]
0049C7BC  eb03                         jmp      0x49c7c1
0049C7BE  8d79fc                       lea      edi, [ecx - 4]
0049C7C1  8b4c240c                     mov      ecx, dword ptr [esp + 0xc]
0049C7C5  f7c103000000                 test     ecx, 3
0049C7CB  7419                         je       0x49c7e6
0049C7CD  8a11                         mov      dl, byte ptr [ecx]
0049C7CF  41                           inc      ecx
0049C7D0  84d2                         test     dl, dl
0049C7D2  7464                         je       0x49c838
0049C7D4  8817                         mov      byte ptr [edi], dl
0049C7D6  47                           inc      edi
0049C7D7  f7c103000000                 test     ecx, 3
0049C7DD  75ee                         jne      0x49c7cd
0049C7DF  eb05                         jmp      0x49c7e6
0049C7E1  8917                         mov      dword ptr [edi], edx
0049C7E3  83c704                       add      edi, 4
0049C7E6  bafffefe7e                   mov      edx, 0x7efefeff
0049C7EB  8b01                         mov      eax, dword ptr [ecx]
0049C7ED  03d0                         add      edx, eax
0049C7EF  83f0ff                       xor      eax, 0xffffffff
0049C7F2  33c2                         xor      eax, edx
0049C7F4  8b11                         mov      edx, dword ptr [ecx]
0049C7F6  83c104                       add      ecx, 4
0049C7F9  a900010181                   test     eax, 0x81010100
0049C7FE  74e1                         je       0x49c7e1
0049C800  84d2                         test     dl, dl
0049C802  7434                         je       0x49c838
0049C804  84f6                         test     dh, dh
0049C806  7427                         je       0x49c82f
0049C808  f7c20000ff00                 test     edx, 0xff0000
0049C80E  7412                         je       0x49c822
0049C810  f7c2000000ff                 test     edx, 0xff000000
0049C816  7402                         je       0x49c81a
0049C818  ebc7                         jmp      0x49c7e1
0049C81A  8917                         mov      dword ptr [edi], edx
0049C81C  8b442408                     mov      eax, dword ptr [esp + 8]
0049C820  5f                           pop      edi
0049C821  c3                           ret
0049C822  668917                       mov      word ptr [edi], dx
0049C825  8b442408                     mov      eax, dword ptr [esp + 8]
0049C829  c6470200                     mov      byte ptr [edi + 2], 0
0049C82D  5f                           pop      edi
0049C82E  c3                           ret
0049C82F  668917                       mov      word ptr [edi], dx
0049C832  8b442408                     mov      eax, dword ptr [esp + 8]
0049C836  5f                           pop      edi
0049C837  c3                           ret
0049C838  8817                         mov      byte ptr [edi], dl
0049C83A  8b442408                     mov      eax, dword ptr [esp + 8]
0049C83E  5f                           pop      edi
0049C83F  c3                           ret

; VA 0x0044BE60, file offset 310880, length 32, SHA256 b396891f78951760185388c076c55d644a0b91531e86d08d9e53bd8713f25701
0044BE60  55                           push     ebp
0044BE61  8bec                         mov      ebp, esp
0044BE63  51                           push     ecx
0044BE64  894dfc                       mov      dword ptr [ebp - 4], ecx
0044BE67  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0044BE6A  e821e6fcff                   call     0x41a490
0044BE6F  8b45fc                       mov      eax, dword ptr [ebp - 4]
0044BE72  c6808400000000               mov      byte ptr [eax + 0x84], 0
0044BE79  8b45fc                       mov      eax, dword ptr [ebp - 4]
0044BE7C  8be5                         mov      esp, ebp
0044BE7E  5d                           pop      ebp
0044BE7F  c3                           ret

; VA 0x004491B0, file offset 299440, length 19, SHA256 57b58fd6b21542d7a72f5cffde310144904f79a92bb46025d7650143a817bc13
004491B0  55                           push     ebp
004491B1  8bec                         mov      ebp, esp
004491B3  51                           push     ecx
004491B4  894dfc                       mov      dword ptr [ebp - 4], ecx
004491B7  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
004491BA  e8f112fdff                   call     0x41a4b0
004491BF  8be5                         mov      esp, ebp
004491C1  5d                           pop      ebp
004491C2  c3                           ret

; VA 0x00406640, file offset 26176, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936
00406640  55                           push     ebp
00406641  8bec                         mov      ebp, esp
00406643  51                           push     ecx
00406644  894dfc                       mov      dword ptr [ebp - 4], ecx
00406647  8b45fc                       mov      eax, dword ptr [ebp - 4]
0040664A  8b00                         mov      eax, dword ptr [eax]
0040664C  8be5                         mov      esp, ebp
0040664E  5d                           pop      ebp
0040664F  c3                           ret
