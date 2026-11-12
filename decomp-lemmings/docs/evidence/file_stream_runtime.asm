; VA 0x0047DB2F, file offset 514863, length 68, SHA256 58a31fa3db446f8568be5b3f9c4a825de8b90add559be12f6a30d7c89efb4a58
0047DB2F  55                           push     ebp
0047DB30  8bec                         mov      ebp, esp
0047DB32  51                           push     ecx
0047DB33  894dfc                       mov      dword ptr [ebp - 4], ecx
0047DB36  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0047DB39  e802150000                   call     0x47f040
0047DB3E  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047DB41  c7801801000000000000         mov      dword ptr [eax + 0x118], 0
0047DB4B  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0047DB4E  c7812001000000000000         mov      dword ptr [ecx + 0x120], 0
0047DB58  8b55fc                       mov      edx, dword ptr [ebp - 4]
0047DB5B  c6821c01000001               mov      byte ptr [edx + 0x11c], 1
0047DB62  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047DB65  c6802c01000000               mov      byte ptr [eax + 0x12c], 0
0047DB6C  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047DB6F  8be5                         mov      esp, ebp
0047DB71  5d                           pop      ebp
0047DB72  c3                           ret

; VA 0x0047DB73, file offset 514931, length 127, SHA256 07fa7b1c6b1312d93bd37eafda9db96b451f01d33f272d3bc0624b9f64233232
0047DB73  55                           push     ebp
0047DB74  8bec                         mov      ebp, esp
0047DB76  6aff                         push     -1
0047DB78  68c9e54a00                   push     0x4ae5c9
0047DB7D  64a100000000                 mov      eax, dword ptr fs:[0]
0047DB83  50                           push     eax
0047DB84  64892500000000               mov      dword ptr fs:[0], esp
0047DB8B  51                           push     ecx
0047DB8C  894df0                       mov      dword ptr [ebp - 0x10], ecx
0047DB8F  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
0047DB92  e8a9140000                   call     0x47f040
0047DB97  c745fc00000000               mov      dword ptr [ebp - 4], 0
0047DB9E  8b4508                       mov      eax, dword ptr [ebp + 8]
0047DBA1  50                           push     eax
0047DBA2  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
0047DBA5  e8eb070000                   call     0x47e395
0047DBAA  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
0047DBAD  c7811801000000000000         mov      dword ptr [ecx + 0x118], 0
0047DBB7  8b55f0                       mov      edx, dword ptr [ebp - 0x10]
0047DBBA  c7822001000000000000         mov      dword ptr [edx + 0x120], 0
0047DBC4  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
0047DBC7  c6801c01000001               mov      byte ptr [eax + 0x11c], 1
0047DBCE  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
0047DBD1  c6812c01000000               mov      byte ptr [ecx + 0x12c], 0
0047DBD8  c745fcffffffff               mov      dword ptr [ebp - 4], 0xffffffff
0047DBDF  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
0047DBE2  8b4df4                       mov      ecx, dword ptr [ebp - 0xc]
0047DBE5  64890d00000000               mov      dword ptr fs:[0], ecx
0047DBEC  8be5                         mov      esp, ebp
0047DBEE  5d                           pop      ebp
0047DBEF  c20400                       ret      4

; VA 0x0047E395, file offset 517013, length 120, SHA256 efb411804ff8b80768f70b471282eb9849d919cb709b4ef9ab03166393c204a5
0047E395  55                           push     ebp
0047E396  8bec                         mov      ebp, esp
0047E398  81ec08010000                 sub      esp, 0x108
0047E39E  898df8feffff                 mov      dword ptr [ebp - 0x108], ecx
0047E3A4  33c0                         xor      eax, eax
0047E3A6  a004565d00                   mov      al, byte ptr [0x5d5604]
0047E3AB  85c0                         test     eax, eax
0047E3AD  7442                         je       0x47e3f1
0047E3AF  6828565d00                   push     0x5d5628
0047E3B4  8d8dfcfeffff                 lea      ecx, [ebp - 0x104]
0047E3BA  51                           push     ecx
0047E3BB  e890e30100                   call     0x49c750
0047E3C0  83c408                       add      esp, 8
0047E3C3  8b5508                       mov      edx, dword ptr [ebp + 8]
0047E3C6  52                           push     edx
0047E3C7  8d85fcfeffff                 lea      eax, [ebp - 0x104]
0047E3CD  50                           push     eax
0047E3CE  e88de30100                   call     0x49c760
0047E3D3  83c408                       add      esp, 8
0047E3D6  8d8dfcfeffff                 lea      ecx, [ebp - 0x104]
0047E3DC  51                           push     ecx
0047E3DD  8b95f8feffff                 mov      edx, dword ptr [ebp - 0x108]
0047E3E3  83c214                       add      edx, 0x14
0047E3E6  52                           push     edx
0047E3E7  e864e30100                   call     0x49c750
0047E3EC  83c408                       add      esp, 8
0047E3EF  eb16                         jmp      0x47e407
0047E3F1  8b4508                       mov      eax, dword ptr [ebp + 8]
0047E3F4  50                           push     eax
0047E3F5  8b8df8feffff                 mov      ecx, dword ptr [ebp - 0x108]
0047E3FB  83c114                       add      ecx, 0x14
0047E3FE  51                           push     ecx
0047E3FF  e84ce30100                   call     0x49c750
0047E404  83c408                       add      esp, 8
0047E407  8be5                         mov      esp, ebp
0047E409  5d                           pop      ebp
0047E40A  c20400                       ret      4

; VA 0x0047F040, file offset 520256, length 14, SHA256 befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c
0047F040  55                           push     ebp
0047F041  8bec                         mov      ebp, esp
0047F043  51                           push     ecx
0047F044  894dfc                       mov      dword ptr [ebp - 4], ecx
0047F047  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047F04A  8be5                         mov      esp, ebp
0047F04C  5d                           pop      ebp
0047F04D  c3                           ret

; VA 0x0047E844, file offset 518212, length 87, SHA256 d0f2429aa3e193d964dc67437a108b2de4dbbb541e2134eea74c8c11dd503446
0047E844  55                           push     ebp
0047E845  8bec                         mov      ebp, esp
0047E847  51                           push     ecx
0047E848  894dfc                       mov      dword ptr [ebp - 4], ecx
0047E84B  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047E84E  83b81801000000               cmp      dword ptr [eax + 0x118], 0
0047E855  7440                         je       0x47e897
0047E857  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0047E85A  8b9118010000                 mov      edx, dword ptr [ecx + 0x118]
0047E860  52                           push     edx
0047E861  e8c6f10100                   call     0x49da2c
0047E866  83c404                       add      esp, 4
0047E869  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047E86C  c7801801000000000000         mov      dword ptr [eax + 0x118], 0
0047E876  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0047E879  c6811c01000001               mov      byte ptr [ecx + 0x11c], 1
0047E880  8b55fc                       mov      edx, dword ptr [ebp - 4]
0047E883  c7822001000000000000         mov      dword ptr [edx + 0x120], 0
0047E88D  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047E890  c6802c01000000               mov      byte ptr [eax + 0x12c], 0
0047E897  8be5                         mov      esp, ebp
0047E899  5d                           pop      ebp
0047E89A  c3                           ret

; VA 0x0047EEF7, file offset 519927, length 70, SHA256 70601e5ef5678f380b7f37bac8241e2962339b38963c2e1c97408c26823acd39
0047EEF7  55                           push     ebp
0047EEF8  8bec                         mov      ebp, esp
0047EEFA  83ec08                       sub      esp, 8
0047EEFD  894df8                       mov      dword ptr [ebp - 8], ecx
0047EF00  8b45f8                       mov      eax, dword ptr [ebp - 8]
0047EF03  8b8818010000                 mov      ecx, dword ptr [eax + 0x118]
0047EF09  51                           push     ecx
0047EF0A  8b5508                       mov      edx, dword ptr [ebp + 8]
0047EF0D  52                           push     edx
0047EF0E  6a01                         push     1
0047EF10  8b450c                       mov      eax, dword ptr [ebp + 0xc]
0047EF13  50                           push     eax
0047EF14  e869eb0100                   call     0x49da82
0047EF19  83c410                       add      esp, 0x10
0047EF1C  8945fc                       mov      dword ptr [ebp - 4], eax
0047EF1F  8b4df8                       mov      ecx, dword ptr [ebp - 8]
0047EF22  8b9120010000                 mov      edx, dword ptr [ecx + 0x120]
0047EF28  0355fc                       add      edx, dword ptr [ebp - 4]
0047EF2B  8b45f8                       mov      eax, dword ptr [ebp - 8]
0047EF2E  899020010000                 mov      dword ptr [eax + 0x120], edx
0047EF34  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047EF37  8be5                         mov      esp, ebp
0047EF39  5d                           pop      ebp
0047EF3A  c20800                       ret      8

; VA 0x0047DAC8, file offset 514760, length 29, SHA256 aec40f1e210009a794b263010b78bce2e4aa490c9c5560c2cd8d25330b64dfc7
0047DAC8  55                           push     ebp
0047DAC9  8bec                         mov      ebp, esp
0047DACB  51                           push     ecx
0047DACC  894dfc                       mov      dword ptr [ebp - 4], ecx
0047DACF  8b450c                       mov      eax, dword ptr [ebp + 0xc]
0047DAD2  50                           push     eax
0047DAD3  8b4d08                       mov      ecx, dword ptr [ebp + 8]
0047DAD6  51                           push     ecx
0047DAD7  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0047DADA  e818140000                   call     0x47eef7
0047DADF  8be5                         mov      esp, ebp
0047DAE1  5d                           pop      ebp
0047DAE2  c20800                       ret      8

; VA 0x0047DC28, file offset 515112, length 72, SHA256 aff14242f418c9727edf3f555fd0daf4b70029be83de56454d342b2f4a47394a
0047DC28  55                           push     ebp
0047DC29  8bec                         mov      ebp, esp
0047DC2B  6aff                         push     -1
0047DC2D  68dce54a00                   push     0x4ae5dc
0047DC32  64a100000000                 mov      eax, dword ptr fs:[0]
0047DC38  50                           push     eax
0047DC39  64892500000000               mov      dword ptr fs:[0], esp
0047DC40  51                           push     ecx
0047DC41  894df0                       mov      dword ptr [ebp - 0x10], ecx
0047DC44  c745fc00000000               mov      dword ptr [ebp - 4], 0
0047DC4B  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
0047DC4E  e8f10b0000                   call     0x47e844
0047DC53  c745fcffffffff               mov      dword ptr [ebp - 4], 0xffffffff
0047DC5A  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
0047DC5D  e8db120000                   call     0x47ef3d
0047DC62  8b4df4                       mov      ecx, dword ptr [ebp - 0xc]
0047DC65  64890d00000000               mov      dword ptr fs:[0], ecx
0047DC6C  8be5                         mov      esp, ebp
0047DC6E  5d                           pop      ebp
0047DC6F  c3                           ret

; VA 0x0047EF3D, file offset 519997, length 11, SHA256 012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5
0047EF3D  55                           push     ebp
0047EF3E  8bec                         mov      ebp, esp
0047EF40  51                           push     ecx
0047EF41  894dfc                       mov      dword ptr [ebp - 4], ecx
0047EF44  8be5                         mov      esp, ebp
0047EF46  5d                           pop      ebp
0047EF47  c3                           ret

; VA 0x0047EE27, file offset 519719, length 17, SHA256 d500a64daa2531474397cfd9958b6e1b88fc9b0b86d04aa2ea4015b50431bc91
0047EE27  55                           push     ebp
0047EE28  8bec                         mov      ebp, esp
0047EE2A  51                           push     ecx
0047EE2B  894dfc                       mov      dword ptr [ebp - 4], ecx
0047EE2E  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047EE31  8b4010                       mov      eax, dword ptr [eax + 0x10]
0047EE34  8be5                         mov      esp, ebp
0047EE36  5d                           pop      ebp
0047EE37  c3                           ret

; VA 0x0047EE38, file offset 519736, length 84, SHA256 f001d4cc9ee000cc449d69d658a34d67bdcc2fe4216db4707487f96ac5c6e44f
0047EE38  55                           push     ebp
0047EE39  8bec                         mov      ebp, esp
0047EE3B  83ec08                       sub      esp, 8
0047EE3E  894dfc                       mov      dword ptr [ebp - 4], ecx
0047EE41  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047EE44  83b83001000001               cmp      dword ptr [eax + 0x130], 1
0047EE4B  7532                         jne      0x47ee7f
0047EE4D  8b0dfc555d00                 mov      ecx, dword ptr [0x5d55fc]
0047EE53  3b0d00565d00                 cmp      ecx, dword ptr [0x5d5600]
0047EE59  7518                         jne      0x47ee73
0047EE5B  8b55fc                       mov      edx, dword ptr [ebp - 4]
0047EE5E  33c0                         xor      eax, eax
0047EE60  8a821c010000                 mov      al, byte ptr [edx + 0x11c]
0047EE66  85c0                         test     eax, eax
0047EE68  7409                         je       0x47ee73
0047EE6A  c745f801000000               mov      dword ptr [ebp - 8], 1
0047EE71  eb07                         jmp      0x47ee7a
0047EE73  c745f800000000               mov      dword ptr [ebp - 8], 0
0047EE7A  8a45f8                       mov      al, byte ptr [ebp - 8]
0047EE7D  eb09                         jmp      0x47ee88
0047EE7F  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0047EE82  8a811c010000                 mov      al, byte ptr [ecx + 0x11c]
0047EE88  8be5                         mov      esp, ebp
0047EE8A  5d                           pop      ebp
0047EE8B  c3                           ret

; VA 0x0047EE8C, file offset 519820, length 20, SHA256 9a0a6ccfb070e91f4254c606398f78e0d8299c783663935500cb20bd7ba7afd7
0047EE8C  55                           push     ebp
0047EE8D  8bec                         mov      ebp, esp
0047EE8F  51                           push     ecx
0047EE90  894dfc                       mov      dword ptr [ebp - 4], ecx
0047EE93  8b45fc                       mov      eax, dword ptr [ebp - 4]
0047EE96  8a802c010000                 mov      al, byte ptr [eax + 0x12c]
0047EE9C  8be5                         mov      esp, ebp
0047EE9E  5d                           pop      ebp
0047EE9F  c3                           ret

; VA 0x0047DAA9, file offset 514729, length 31, SHA256 3911d4ea905910cfff6f69dae9b45866a66d81e313790b703e961dd6100daf17
0047DAA9  55                           push     ebp
0047DAAA  8bec                         mov      ebp, esp
0047DAAC  51                           push     ecx
0047DAAD  894dfc                       mov      dword ptr [ebp - 4], ecx
0047DAB0  c70500565d00fcd55c00         mov      dword ptr [0x5d5600], 0x5cd5fc
0047DABA  a100565d00                   mov      eax, dword ptr [0x5d5600]
0047DABF  a3fc555d00                   mov      dword ptr [0x5d55fc], eax
0047DAC4  8be5                         mov      esp, ebp
0047DAC6  5d                           pop      ebp
0047DAC7  c3                           ret
