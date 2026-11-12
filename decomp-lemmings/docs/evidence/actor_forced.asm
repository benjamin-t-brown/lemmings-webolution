# Original instruction bytes; executable SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f

# VA 0x416BF0; mapped offset 93168; length 48
00416BF0  55                             push ebp
00416BF1  8bec                           mov ebp, esp
00416BF3  51                             push ecx
00416BF4  894dfc                         mov dword ptr [ebp - 4], ecx
00416BF7  8b4dfc                         mov ecx, dword ptr [ebp - 4]
00416BFA  e891170000                     call 0x418390
00416BFF  8b45fc                         mov eax, dword ptr [ebp - 4]
00416C02  c700780a4b00                   mov dword ptr [eax], 0x4b0a78
00416C08  8b4dfc                         mov ecx, dword ptr [ebp - 4]
00416C0B  c7411425000000                 mov dword ptr [ecx + 0x14], 0x25
00416C12  8b55fc                         mov edx, dword ptr [ebp - 4]
00416C15  c6421100                       mov byte ptr [edx + 0x11], 0
00416C19  8b45fc                         mov eax, dword ptr [ebp - 4]
00416C1C  8be5                           mov esp, ebp
00416C1E  5d                             pop ebp
00416C1F  c3                             ret

# VA 0x416C3C; mapped offset 93244; length 13
00416C3C  55                             push ebp
00416C3D  8bec                           mov ebp, esp
00416C3F  51                             push ecx
00416C40  894dfc                         mov dword ptr [ebp - 4], ecx
00416C43  8be5                           mov esp, ebp
00416C45  5d                             pop ebp
00416C46  c20400                         ret 4

# VA 0x416C49; mapped offset 93257; length 587
00416C49  55                             push ebp
00416C4A  8bec                           mov ebp, esp
00416C4C  6aff                           push -1
00416C4E  68a2a04a00                     push 0x4aa0a2
00416C53  64a100000000                   mov eax, dword ptr fs:[0]
00416C59  50                             push eax
00416C5A  64892500000000                 mov dword ptr fs:[0], esp
00416C61  81eca8000000                   sub esp, 0xa8
00416C67  56                             push esi
00416C68  57                             push edi
00416C69  898d5cffffff                   mov dword ptr [ebp - 0xa4], ecx
00416C6F  8b7508                         mov esi, dword ptr [ebp + 8]
00416C72  83c608                         add esi, 8
00416C75  b906000000                     mov ecx, 6
00416C7A  8d7dd8                         lea edi, [ebp - 0x28]
00416C7D  f3a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00416C7F  c745fc00000000                 mov dword ptr [ebp - 4], 0
00416C86  e8f5d3feff                     call 0x404080
00416C8B  dc05f8034b00                   fadd qword ptr [0x4b03f8]
00416C91  dc45e8                         fadd qword ptr [ebp - 0x18]
00416C94  dd5de8                         fstp qword ptr [ebp - 0x18]
00416C97  db05b8104b00                   fild dword ptr [0x4b10b8]
00416C9D  dc35f8034b00                   fdiv qword ptr [0x4b03f8]
00416CA3  dd9d54ffffff                   fstp qword ptr [ebp - 0xac]
00416CA9  e8d2d3feff                     call 0x404080
00416CAE  dc8554ffffff                   fadd qword ptr [ebp - 0xac]
00416CB4  dc45e0                         fadd qword ptr [ebp - 0x20]
00416CB7  dd5de0                         fstp qword ptr [ebp - 0x20]
00416CBA  6848010000                     push 0x148
00416CBF  e8e4540800                     call 0x49c1a8
00416CC4  83c404                         add esp, 4
00416CC7  8945b8                         mov dword ptr [ebp - 0x48], eax
00416CCA  c645fc01                       mov byte ptr [ebp - 4], 1
00416CCE  837db800                       cmp dword ptr [ebp - 0x48], 0
00416CD2  7410                           je 0x416ce4
00416CD4  8b4db8                         mov ecx, dword ptr [ebp - 0x48]
00416CD7  e8f4010000                     call 0x416ed0
00416CDC  898550ffffff                   mov dword ptr [ebp - 0xb0], eax
00416CE2  eb0a                           jmp 0x416cee
00416CE4  c78550ffffff00000000           mov dword ptr [ebp - 0xb0], 0
00416CEE  8b8550ffffff                   mov eax, dword ptr [ebp - 0xb0]
00416CF4  8945bc                         mov dword ptr [ebp - 0x44], eax
00416CF7  c645fc00                       mov byte ptr [ebp - 4], 0
00416CFB  8b4dbc                         mov ecx, dword ptr [ebp - 0x44]
00416CFE  894df0                         mov dword ptr [ebp - 0x10], ecx
00416D01  8b55f0                         mov edx, dword ptr [ebp - 0x10]
00416D04  8b4508                         mov eax, dword ptr [ebp + 8]
00416D07  8b484c                         mov ecx, dword ptr [eax + 0x4c]
00416D0A  898ad8000000                   mov dword ptr [edx + 0xd8], ecx
00416D10  6a01                           push 1
00416D12  8b4df0                         mov ecx, dword ptr [ebp - 0x10]
00416D15  e836040000                     call 0x417150
00416D1A  8b5508                         mov edx, dword ptr [ebp + 8]
00416D1D  83c208                         add edx, 8
00416D20  52                             push edx
00416D21  8d4588                         lea eax, [ebp - 0x78]
00416D24  50                             push eax
00416D25  b9106a4c00                     mov ecx, 0x4c6a10
00416D2A  e83d680200                     call 0x43d56c
00416D2F  50                             push eax
00416D30  8d8d70ffffff                   lea ecx, [ebp - 0x90]
00416D36  51                             push ecx
00416D37  8d55d8                         lea edx, [ebp - 0x28]
00416D3A  52                             push edx
00416D3B  8d45a0                         lea eax, [ebp - 0x60]
00416D3E  50                             push eax
00416D3F  b9106a4c00                     mov ecx, 0x4c6a10
00416D44  e823680200                     call 0x43d56c
00416D49  8bc8                           mov ecx, eax
00416D4B  e8e0aafeff                     call 0x401830
00416D50  50                             push eax
00416D51  8b4df0                         mov ecx, dword ptr [ebp - 0x10]
00416D54  8b11                           mov edx, dword ptr [ecx]
00416D56  8b4df0                         mov ecx, dword ptr [ebp - 0x10]
00416D59  ff5254                         call dword ptr [edx + 0x54]
00416D5C  8b4508                         mov eax, dword ptr [ebp + 8]
00416D5F  83c008                         add eax, 8
00416D62  50                             push eax
00416D63  8b4df0                         mov ecx, dword ptr [ebp - 0x10]
00416D66  8b11                           mov edx, dword ptr [ecx]
00416D68  8b4df0                         mov ecx, dword ptr [ebp - 0x10]
00416D6B  ff5208                         call dword ptr [edx + 8]
00416D6E  680000e03f                     push 0x3fe00000
00416D73  6a00                           push 0
00416D75  6800001040                     push 0x40100000
00416D7A  6a00                           push 0
00416D7C  8b4df0                         mov ecx, dword ptr [ebp - 0x10]
00416D7F  e86c040000                     call 0x4171f0
00416D84  6a08                           push 8
00416D86  8b4df0                         mov ecx, dword ptr [ebp - 0x10]
00416D89  e8a2040000                     call 0x417230
00416D8E  8b450c                         mov eax, dword ptr [ebp + 0xc]
00416D91  8b4804                         mov ecx, dword ptr [eax + 4]
00416D94  51                             push ecx
00416D95  8b10                           mov edx, dword ptr [eax]
00416D97  52                             push edx
00416D98  8b45f0                         mov eax, dword ptr [ebp - 0x10]
00416D9B  8b10                           mov edx, dword ptr [eax]
00416D9D  8b4df0                         mov ecx, dword ptr [ebp - 0x10]
00416DA0  ff522c                         call dword ptr [edx + 0x2c]
00416DA3  8b4508                         mov eax, dword ptr [ebp + 8]
00416DA6  dd4010                         fld qword ptr [eax + 0x10]
00416DA9  e8fe570800                     call 0x49c5ac
00416DAE  50                             push eax
00416DAF  8b4d08                         mov ecx, dword ptr [ebp + 8]
00416DB2  dd4108                         fld qword ptr [ecx + 8]
00416DB5  e8f2570800                     call 0x49c5ac
00416DBA  50                             push eax
00416DBB  b9106a4c00                     mov ecx, 0x4c6a10
00416DC0  e87bd1feff                     call 0x403f40
00416DC5  8bf0                           mov esi, eax
00416DC7  b906000000                     mov ecx, 6
00416DCC  8d7dc0                         lea edi, [ebp - 0x40]
00416DCF  f3a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00416DD1  8d55c0                         lea edx, [ebp - 0x40]
00416DD4  52                             push edx
00416DD5  6a00                           push 0
00416DD7  8b4df0                         mov ecx, dword ptr [ebp - 0x10]
00416DDA  81c1b0000000                   add ecx, 0xb0
00416DE0  e8cbaafeff                     call 0x4018b0
00416DE5  8bc8                           mov ecx, eax
00416DE7  81c1b0000000                   add ecx, 0xb0
00416DED  e82ecafeff                     call 0x403820
00416DF2  6a48                           push 0x48
00416DF4  e8af530800                     call 0x49c1a8
00416DF9  83c404                         add esp, 4
00416DFC  898564ffffff                   mov dword ptr [ebp - 0x9c], eax
00416E02  c645fc02                       mov byte ptr [ebp - 4], 2
00416E06  83bd64ffffff00                 cmp dword ptr [ebp - 0x9c], 0
00416E0D  741d                           je 0x416e2c
00416E0F  6a01                           push 1
00416E11  8b45f0                         mov eax, dword ptr [ebp - 0x10]
00416E14  50                             push eax
00416E15  8d4dd8                         lea ecx, [ebp - 0x28]
00416E18  51                             push ecx
00416E19  8b8d64ffffff                   mov ecx, dword ptr [ebp - 0x9c]
00416E1F  e85cb20100                     call 0x432080
00416E24  89854cffffff                   mov dword ptr [ebp - 0xb4], eax
00416E2A  eb0a                           jmp 0x416e36
00416E2C  c7854cffffff00000000           mov dword ptr [ebp - 0xb4], 0
00416E36  8b954cffffff                   mov edx, dword ptr [ebp - 0xb4]
00416E3C  899568ffffff                   mov dword ptr [ebp - 0x98], edx
00416E42  c645fc00                       mov byte ptr [ebp - 4], 0
00416E46  8b8568ffffff                   mov eax, dword ptr [ebp - 0x98]
00416E4C  89856cffffff                   mov dword ptr [ebp - 0x94], eax
00416E52  8d8d6cffffff                   lea ecx, [ebp - 0x94]
00416E58  51                             push ecx
00416E59  b9a8884c00                     mov ecx, 0x4c88a8
00416E5E  e82daafeff                     call 0x401890
00416E63  c78560ffffff02000000           mov dword ptr [ebp - 0xa0], 2
00416E6D  c745fcffffffff                 mov dword ptr [ebp - 4], 0xffffffff
00416E74  8d4dd8                         lea ecx, [ebp - 0x28]
00416E77  e834d0feff                     call 0x403eb0
00416E7C  8b8560ffffff                   mov eax, dword ptr [ebp - 0xa0]
00416E82  8b4df4                         mov ecx, dword ptr [ebp - 0xc]
00416E85  64890d00000000                 mov dword ptr fs:[0], ecx
00416E8C  5f                             pop edi
00416E8D  5e                             pop esi
00416E8E  8be5                           mov esp, ebp
00416E90  5d                             pop ebp
00416E91  c20800                         ret 8

# VA 0x401320; mapped offset 4896; length 13
00401320  55                             push ebp
00401321  8bec                           mov ebp, esp
00401323  51                             push ecx
00401324  894dfc                         mov dword ptr [ebp - 4], ecx
00401327  8be5                           mov esp, ebp
00401329  5d                             pop ebp
0040132A  c20400                         ret 4
