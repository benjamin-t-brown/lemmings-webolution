; VA 0x0047E6CC, file offset 517836, length 41, SHA256 5788559ea557c7e7290a940732e627d1333c3eed890f4d8edd3bf1d80e008f58
0047E6CC  6a00                         push     0
0047E6CE  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E6D4  8b8224010000                 mov      eax, dword ptr [edx + 0x124]
0047E6DA  50                           push     eax
0047E6DB  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E6E1  8b9118010000                 mov      edx, dword ptr [ecx + 0x118]
0047E6E7  52                           push     edx
0047E6E8  e808f60100                   call     0x49dcf5
0047E6ED  83c40c                       add      esp, 0xc
0047E6F0  e9eb000000                   jmp      0x47e7e0

; VA 0x0047E770, file offset 518000, length 172, SHA256 3a69b43387be5f2f815de749e31a157c4b8628068e511caf2bb73a54a8110b49
0047E770  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E776  8b9118010000                 mov      edx, dword ptr [ecx + 0x118]
0047E77C  52                           push     edx
0047E77D  e81bf40100                   call     0x49db9d
0047E782  83c404                       add      esp, 4
0047E785  8985dcfeffff                 mov      dword ptr [ebp - 0x124], eax
0047E78B  6a02                         push     2
0047E78D  6a00                         push     0
0047E78F  8b85d8feffff                 mov      eax, dword ptr [ebp - 0x128]
0047E795  8b8818010000                 mov      ecx, dword ptr [eax + 0x118]
0047E79B  51                           push     ecx
0047E79C  e854f50100                   call     0x49dcf5
0047E7A1  83c40c                       add      esp, 0xc
0047E7A4  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E7AA  8b8218010000                 mov      eax, dword ptr [edx + 0x118]
0047E7B0  50                           push     eax
0047E7B1  e8e7f30100                   call     0x49db9d
0047E7B6  83c404                       add      esp, 4
0047E7B9  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E7BF  894110                       mov      dword ptr [ecx + 0x10], eax
0047E7C2  6a00                         push     0
0047E7C4  8b95dcfeffff                 mov      edx, dword ptr [ebp - 0x124]
0047E7CA  52                           push     edx
0047E7CB  8b85d8feffff                 mov      eax, dword ptr [ebp - 0x128]
0047E7D1  8b8818010000                 mov      ecx, dword ptr [eax + 0x118]
0047E7D7  51                           push     ecx
0047E7D8  e818f50100                   call     0x49dcf5
0047E7DD  83c40c                       add      esp, 0xc
0047E7E0  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E7E6  c6821c01000000               mov      byte ptr [edx + 0x11c], 0
0047E7ED  8b85d8feffff                 mov      eax, dword ptr [ebp - 0x128]
0047E7F3  83b83001000001               cmp      dword ptr [eax + 0x130], 1
0047E7FA  750b                         jne      0x47e807
0047E7FC  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E802  e8a2f2ffff                   call     0x47daa9
0047E807  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E80D  c6812c01000001               mov      byte ptr [ecx + 0x12c], 1
0047E814  b001                         mov      al, 1
0047E816  8be5                         mov      esp, ebp
0047E818  5d                           pop      ebp
0047E819  c20800                       ret      8
