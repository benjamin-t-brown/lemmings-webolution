; VA 0x00484E40, file offset 544320, length 31, SHA256 abc654c126fc53484988c6d82c1b27bd769cfac8fe24f09aac83e92549a14ee1
00484E40  55                           push     ebp
00484E41  8bec                         mov      ebp, esp
00484E43  51                           push     ecx
00484E44  894dfc                       mov      dword ptr [ebp - 4], ecx
00484E47  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
00484E4A  e8d1020000                   call     0x485120
00484E4F  8b45fc                       mov      eax, dword ptr [ebp - 4]
00484E52  c700a81b4b00                 mov      dword ptr [eax], 0x4b1ba8
00484E58  8b45fc                       mov      eax, dword ptr [ebp - 4]
00484E5B  8be5                         mov      esp, ebp
00484E5D  5d                           pop      ebp
00484E5E  c3                           ret

; VA 0x00485120, file offset 545056, length 87, SHA256 e2c88a9d6d75312e95dfe18ca4f3458937bf4f21db6f2591f9d729e5ad7899dd
00485120  55                           push     ebp
00485121  8bec                         mov      ebp, esp
00485123  6aff                         push     -1
00485125  6899e94a00                   push     0x4ae999
0048512A  64a100000000                 mov      eax, dword ptr fs:[0]
00485130  50                           push     eax
00485131  64892500000000               mov      dword ptr fs:[0], esp
00485138  51                           push     ecx
00485139  894df0                       mov      dword ptr [ebp - 0x10], ecx
0048513C  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
0048513F  e88c010000                   call     0x4852d0
00485144  c745fc00000000               mov      dword ptr [ebp - 4], 0
0048514B  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
0048514E  83c130                       add      ecx, 0x30
00485151  e81a030000                   call     0x485470
00485156  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
00485159  c700c01b4b00                 mov      dword ptr [eax], 0x4b1bc0
0048515F  c745fcffffffff               mov      dword ptr [ebp - 4], 0xffffffff
00485166  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
00485169  8b4df4                       mov      ecx, dword ptr [ebp - 0xc]
0048516C  64890d00000000               mov      dword ptr fs:[0], ecx
00485173  8be5                         mov      esp, ebp
00485175  5d                           pop      ebp
00485176  c3                           ret

; VA 0x004852D0, file offset 545488, length 44, SHA256 f96b6ded611f158614ece584fd9815320401773e881c7483472a6d0f6d90ec02
004852D0  55                           push     ebp
004852D1  8bec                         mov      ebp, esp
004852D3  51                           push     ecx
004852D4  894dfc                       mov      dword ptr [ebp - 4], ecx
004852D7  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
004852DA  83c104                       add      ecx, 4
004852DD  e80e030000                   call     0x4855f0
004852E2  8b45fc                       mov      eax, dword ptr [ebp - 4]
004852E5  c700d81b4b00                 mov      dword ptr [eax], 0x4b1bd8
004852EB  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
004852EE  c7412c00000000               mov      dword ptr [ecx + 0x2c], 0
004852F5  8b45fc                       mov      eax, dword ptr [ebp - 4]
004852F8  8be5                         mov      esp, ebp
004852FA  5d                           pop      ebp
004852FB  c3                           ret

; VA 0x004855F0, file offset 546288, length 119, SHA256 637f730d8f47f10710df0a739f47f5dcff74504268cd8f24f1b2458afb229b81
004855F0  55                           push     ebp
004855F1  8bec                         mov      ebp, esp
004855F3  6aff                         push     -1
004855F5  68f9e94a00                   push     0x4ae9f9
004855FA  64a100000000                 mov      eax, dword ptr fs:[0]
00485600  50                           push     eax
00485601  64892500000000               mov      dword ptr fs:[0], esp
00485608  51                           push     ecx
00485609  894df0                       mov      dword ptr [ebp - 0x10], ecx
0048560C  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
0048560F  e81c010000                   call     0x485730
00485614  c745fc00000000               mov      dword ptr [ebp - 4], 0
0048561B  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
0048561E  83c114                       add      ecx, 0x14
00485621  e80a010000                   call     0x485730
00485626  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
00485629  83c014                       add      eax, 0x14
0048562C  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
0048562F  89410c                       mov      dword ptr [ecx + 0xc], eax
00485632  8b55f0                       mov      edx, dword ptr [ebp - 0x10]
00485635  c7421000000000               mov      dword ptr [edx + 0x10], 0
0048563C  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
0048563F  c7402000000000               mov      dword ptr [eax + 0x20], 0
00485646  8b4df0                       mov      ecx, dword ptr [ebp - 0x10]
00485649  8b55f0                       mov      edx, dword ptr [ebp - 0x10]
0048564C  895124                       mov      dword ptr [ecx + 0x24], edx
0048564F  c745fcffffffff               mov      dword ptr [ebp - 4], 0xffffffff
00485656  8b45f0                       mov      eax, dword ptr [ebp - 0x10]
00485659  8b4df4                       mov      ecx, dword ptr [ebp - 0xc]
0048565C  64890d00000000               mov      dword ptr fs:[0], ecx
00485663  8be5                         mov      esp, ebp
00485665  5d                           pop      ebp
00485666  c3                           ret

; VA 0x00485730, file offset 546608, length 22, SHA256 7153939bf56a7e8321d61a27f4b79a78b95b23606a048e8b3faa8027df24a98e
00485730  55                           push     ebp
00485731  8bec                         mov      ebp, esp
00485733  51                           push     ecx
00485734  894dfc                       mov      dword ptr [ebp - 4], ecx
00485737  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
0048573A  e8e1b1faff                   call     0x430920
0048573F  8b45fc                       mov      eax, dword ptr [ebp - 4]
00485742  8be5                         mov      esp, ebp
00485744  5d                           pop      ebp
00485745  c3                           ret

; VA 0x00485470, file offset 545904, length 53, SHA256 1f0030291fa3a68771361e044671fd4c4ed373926b81c41092ccd2071eedc704
00485470  55                           push     ebp
00485471  8bec                         mov      ebp, esp
00485473  51                           push     ecx
00485474  894dfc                       mov      dword ptr [ebp - 4], ecx
00485477  8b45fc                       mov      eax, dword ptr [ebp - 4]
0048547A  c70000000000                 mov      dword ptr [eax], 0
00485480  8b4dfc                       mov      ecx, dword ptr [ebp - 4]
00485483  c7410400000000               mov      dword ptr [ecx + 4], 0
0048548A  8b55fc                       mov      edx, dword ptr [ebp - 4]
0048548D  c7420801000000               mov      dword ptr [edx + 8], 1
00485494  8b45fc                       mov      eax, dword ptr [ebp - 4]
00485497  c7400c00000000               mov      dword ptr [eax + 0xc], 0
0048549E  8b45fc                       mov      eax, dword ptr [ebp - 4]
004854A1  8be5                         mov      esp, ebp
004854A3  5d                           pop      ebp
004854A4  c3                           ret
