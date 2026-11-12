; VA 0x0047E40D, file offset 517133, length 1039, SHA256 bcd189abe77f85f8b72d15c05f6b9f92f55f71645fd21b8f2663d9501a1b2c7a
0047E40D  55                           push     ebp
0047E40E  8bec                         mov      ebp, esp
0047E410  81ec2c010000                 sub      esp, 0x12c
0047E416  898dd8feffff                 mov      dword ptr [ebp - 0x128], ecx
0047E41C  c645f800                     mov      byte ptr [ebp - 8], 0
0047E420  c645fc00                     mov      byte ptr [ebp - 4], 0
0047E424  6a00                         push     0
0047E426  8b4508                       mov      eax, dword ptr [ebp + 8]
0047E429  50                           push     eax
0047E42A  e825f9ffff                   call     0x47dd54
0047E42F  83c408                       add      esp, 8
0047E432  33c9                         xor      ecx, ecx
0047E434  8a0d04565d00                 mov      cl, byte ptr [0x5d5604]
0047E43A  85c9                         test     ecx, ecx
0047E43C  0f8556010000                 jne      0x47e598
0047E442  837d0801                     cmp      dword ptr [ebp + 8], 1
0047E446  751b                         jne      0x47e463
0047E448  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E44E  83c214                       add      edx, 0x14
0047E451  52                           push     edx
0047E452  e844040000                   call     0x47e89b
0047E457  83c404                       add      esp, 4
0047E45A  25ff000000                   and      eax, 0xff
0047E45F  85c0                         test     eax, eax
0047E461  741a                         je       0x47e47d
0047E463  8b85d8feffff                 mov      eax, dword ptr [ebp - 0x128]
0047E469  c6801d01000000               mov      byte ptr [eax + 0x11d], 0
0047E470  c645f801                     mov      byte ptr [ebp - 8], 1
0047E474  c645fc00                     mov      byte ptr [ebp - 4], 0
0047E478  e9e8000000                   jmp      0x47e565
0047E47D  e8c60a0000                   call     0x47ef48
0047E482  25ff000000                   and      eax, 0xff
0047E487  85c0                         test     eax, eax
0047E489  7505                         jne      0x47e490
0047E48B  e8020b0000                   call     0x47ef92
0047E490  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E496  83c114                       add      ecx, 0x14
0047E499  51                           push     ecx
0047E49A  b908565d00                   mov      ecx, 0x5d5608
0047E49F  e8b5120000                   call     0x47f759
0047E4A4  8945e8                       mov      dword ptr [ebp - 0x18], eax
0047E4A7  837de800                     cmp      dword ptr [ebp - 0x18], 0
0047E4AB  744f                         je       0x47e4fc
0047E4AD  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E4B3  8b45e8                       mov      eax, dword ptr [ebp - 0x18]
0047E4B6  8b8804010000                 mov      ecx, dword ptr [eax + 0x104]
0047E4BC  894a10                       mov      dword ptr [edx + 0x10], ecx
0047E4BF  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E4C5  8b45e8                       mov      eax, dword ptr [ebp - 0x18]
0047E4C8  8b8808010000                 mov      ecx, dword ptr [eax + 0x108]
0047E4CE  898a24010000                 mov      dword ptr [edx + 0x124], ecx
0047E4D4  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E4DA  8b45e8                       mov      eax, dword ptr [ebp - 0x18]
0047E4DD  8b08                         mov      ecx, dword ptr [eax]
0047E4DF  898a28010000                 mov      dword ptr [edx + 0x128], ecx
0047E4E5  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E4EB  c6821d01000001               mov      byte ptr [edx + 0x11d], 1
0047E4F2  c645f801                     mov      byte ptr [ebp - 8], 1
0047E4F6  c645fc00                     mov      byte ptr [ebp - 4], 0
0047E4FA  eb69                         jmp      0x47e565
0047E4FC  8b450c                       mov      eax, dword ptr [ebp + 0xc]
0047E4FF  25ff000000                   and      eax, 0xff
0047E504  85c0                         test     eax, eax
0047E506  7448                         je       0x47e550
0047E508  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E50E  83c114                       add      ecx, 0x14
0047E511  51                           push     ecx
0047E512  68842a4c00                   push     0x4c2a84
0047E517  8d95e0feffff                 lea      edx, [ebp - 0x120]
0047E51D  52                           push     edx
0047E51E  e847e50100                   call     0x49ca6a
0047E523  83c40c                       add      esp, 0xc
0047E526  6831210000                   push     0x2131
0047E52B  68d02a4c00                   push     0x4c2ad0
0047E530  8d85e0feffff                 lea      eax, [ebp - 0x120]
0047E536  50                           push     eax
0047E537  6a00                         push     0
0047E539  ff15cc014b00                 call     dword ptr [0x4b01cc]
0047E53F  8945e4                       mov      dword ptr [ebp - 0x1c], eax
0047E542  33c9                         xor      ecx, ecx
0047E544  837de401                     cmp      dword ptr [ebp - 0x1c], 1
0047E548  0f94c1                       sete     cl
0047E54B  884dfc                       mov      byte ptr [ebp - 4], cl
0047E54E  eb04                         jmp      0x47e554
0047E550  c645fc00                     mov      byte ptr [ebp - 4], 0
0047E554  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E55A  c6821d01000000               mov      byte ptr [edx + 0x11d], 0
0047E561  c645f800                     mov      byte ptr [ebp - 8], 0
0047E565  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047E568  25ff000000                   and      eax, 0xff
0047E56D  85c0                         test     eax, eax
0047E56F  7414                         je       0x47e585
0047E571  e87dfdffff                   call     0x47e2f3
0047E576  e8c0fcffff                   call     0x47e23b
0047E57B  e8660a0000                   call     0x47efe6
0047E580  e80d0a0000                   call     0x47ef92
0047E585  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0047E588  81e1ff000000                 and      ecx, 0xff
0047E58E  85c9                         test     ecx, ecx
0047E590  0f85acfeffff                 jne      0x47e442
0047E596  eb11                         jmp      0x47e5a9
0047E598  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E59E  c6821d01000000               mov      byte ptr [edx + 0x11d], 0
0047E5A5  c645f801                     mov      byte ptr [ebp - 8], 1
0047E5A9  8b45f8                       mov      eax, dword ptr [ebp - 8]
0047E5AC  25ff000000                   and      eax, 0xff
0047E5B1  85c0                         test     eax, eax
0047E5B3  7507                         jne      0x47e5bc
0047E5B5  32c0                         xor      al, al
0047E5B7  e95a020000                   jmp      0x47e816
0047E5BC  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E5C2  8b5508                       mov      edx, dword ptr [ebp + 8]
0047E5C5  899130010000                 mov      dword ptr [ecx + 0x130], edx
0047E5CB  8b85d8feffff                 mov      eax, dword ptr [ebp - 0x128]
0047E5D1  8b8830010000                 mov      ecx, dword ptr [eax + 0x130]
0047E5D7  898dd4feffff                 mov      dword ptr [ebp - 0x12c], ecx
0047E5DD  8b95d4feffff                 mov      edx, dword ptr [ebp - 0x12c]
0047E5E3  83ea01                       sub      edx, 1
0047E5E6  8995d4feffff                 mov      dword ptr [ebp - 0x12c], edx
0047E5EC  83bdd4feffff09               cmp      dword ptr [ebp - 0x12c], 9
0047E5F3  777d                         ja       0x47e672
0047E5F5  8b85d4feffff                 mov      eax, dword ptr [ebp - 0x12c]
0047E5FB  ff24851ce84700               jmp      dword ptr [eax*4 + 0x47e81c]
0047E602  68e42a4c00                   push     0x4c2ae4
0047E607  8d4dec                       lea      ecx, [ebp - 0x14]
0047E60A  51                           push     ecx
0047E60B  e840e10100                   call     0x49c750
0047E610  83c408                       add      esp, 8
0047E613  eb5d                         jmp      0x47e672
0047E615  68e82a4c00                   push     0x4c2ae8
0047E61A  8d55ec                       lea      edx, [ebp - 0x14]
0047E61D  52                           push     edx
0047E61E  e82de10100                   call     0x49c750
0047E623  83c408                       add      esp, 8
0047E626  eb4a                         jmp      0x47e672
0047E628  68ec2a4c00                   push     0x4c2aec
0047E62D  8d45ec                       lea      eax, [ebp - 0x14]
0047E630  50                           push     eax
0047E631  e81ae10100                   call     0x49c750
0047E636  83c408                       add      esp, 8
0047E639  eb37                         jmp      0x47e672
0047E63B  68f02a4c00                   push     0x4c2af0
0047E640  8d4dec                       lea      ecx, [ebp - 0x14]
0047E643  51                           push     ecx
0047E644  e807e10100                   call     0x49c750
0047E649  83c408                       add      esp, 8
0047E64C  eb24                         jmp      0x47e672
0047E64E  68f42a4c00                   push     0x4c2af4
0047E653  8d55ec                       lea      edx, [ebp - 0x14]
0047E656  52                           push     edx
0047E657  e8f4e00100                   call     0x49c750
0047E65C  83c408                       add      esp, 8
0047E65F  eb11                         jmp      0x47e672
0047E661  68f82a4c00                   push     0x4c2af8
0047E666  8d45ec                       lea      eax, [ebp - 0x14]
0047E669  50                           push     eax
0047E66A  e8e1e00100                   call     0x49c750
0047E66F  83c408                       add      esp, 8
0047E672  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E678  33d2                         xor      edx, edx
0047E67A  8a911d010000                 mov      dl, byte ptr [ecx + 0x11d]
0047E680  85d2                         test     edx, edx
0047E682  7471                         je       0x47e6f5
0047E684  8b85d8feffff                 mov      eax, dword ptr [ebp - 0x128]
0047E68A  83b81801000000               cmp      dword ptr [eax + 0x118], 0
0047E691  7539                         jne      0x47e6cc
0047E693  6a20                         push     0x20
0047E695  8d4dec                       lea      ecx, [ebp - 0x14]
0047E698  51                           push     ecx
0047E699  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E69F  8b8228010000                 mov      eax, dword ptr [edx + 0x128]
0047E6A5  50                           push     eax
0047E6A6  b908565d00                   mov      ecx, 0x5d5608
0047E6AB  e8600b0000                   call     0x47f210
0047E6B0  8b08                         mov      ecx, dword ptr [eax]
0047E6B2  e8f9090000                   call     0x47f0b0
0047E6B7  50                           push     eax
0047E6B8  e8adf40100                   call     0x49db6a
0047E6BD  83c40c                       add      esp, 0xc
0047E6C0  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E6C6  898118010000                 mov      dword ptr [ecx + 0x118], eax
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
0047E6F5  8b85d8feffff                 mov      eax, dword ptr [ebp - 0x128]
0047E6FB  83b81801000000               cmp      dword ptr [eax + 0x118], 0
0047E702  7425                         je       0x47e729
0047E704  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E70A  8b9118010000                 mov      edx, dword ptr [ecx + 0x118]
0047E710  52                           push     edx
0047E711  e816f30100                   call     0x49da2c
0047E716  83c404                       add      esp, 4
0047E719  8b85d8feffff                 mov      eax, dword ptr [ebp - 0x128]
0047E71F  c7801801000000000000         mov      dword ptr [eax + 0x118], 0
0047E729  6a20                         push     0x20
0047E72B  8d4dec                       lea      ecx, [ebp - 0x14]
0047E72E  51                           push     ecx
0047E72F  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E735  83c214                       add      edx, 0x14
0047E738  52                           push     edx
0047E739  e82cf40100                   call     0x49db6a
0047E73E  83c40c                       add      esp, 0xc
0047E741  8b8dd8feffff                 mov      ecx, dword ptr [ebp - 0x128]
0047E747  898118010000                 mov      dword ptr [ecx + 0x118], eax
0047E74D  8b95d8feffff                 mov      edx, dword ptr [ebp - 0x128]
0047E753  83ba1801000000               cmp      dword ptr [edx + 0x118], 0
0047E75A  7514                         jne      0x47e770
0047E75C  8b85d8feffff                 mov      eax, dword ptr [ebp - 0x128]
0047E762  c6801c01000001               mov      byte ptr [eax + 0x11c], 1
0047E769  32c0                         xor      al, al
0047E76B  e9a6000000                   jmp      0x47e816
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

; VA 0x0047F0B0, file offset 520368, length 22, SHA256 6800f39676c99dc7ae1c648bed624d161bdc6dcdb1c69632b6163e4852328140
0047F0B0  55                           push     ebp
0047F0B1  8bec                         mov      ebp, esp
0047F0B3  51                           push     ecx
0047F0B4  894dfc                       mov      dword ptr [ebp - 4], ecx
0047F0B7  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0047F0BA  83c138                       add      ecx, 0x38
0047F0BD  e83e74f8ff                   call     0x406500
0047F0C2  8be5                         mov      esp, ebp
0047F0C4  5d                           pop      ebp
0047F0C5  c3                           ret

; DATA VA 0x0047E81C, file offset 518172, length 40, SHA256 2b9a9e9ed3b35715798c084d70af86f56208c851b4eb096b4e8e5329341bfedf
02e6470015e6470028e6470072e6470072e647003be647004ee6470072e6470072e6470061e64700

; DATA VA 0x004C2A84, file offset 797316, length 120, SHA256 c8133acb8a7695fa688e980a230ed7a3ce77d0e0620b790a3803d4794ce5fada
43616e6e6f742066696e642072657175697265642066696c6520272573272e2020436c69636b204f4b20746f2074727920616761696e2c206f722063616e63656c20746f20657869742e00004c656d6d696e6773205265766f6c7574696f6e00726200007762000077622b006162000061622b0077740000
