; VA 0046D3C1 file offset 6D3C1 length 159 SHA256 a1e2cde7267f1dc6f46a3d05e968261300cc344e0eb84a303116eff440a1db6e
0046D3C1 55                         push ebp
0046D3C2 8bec                       mov ebp, esp
0046D3C4 51                         push ecx
0046D3C5 894dfc                     mov dword ptr [ebp - 4], ecx
0046D3C8 db0588d25800               fild dword ptr [0x58d288]
0046D3CE 83ec08                     sub esp, 8
0046D3D1 dd1c24                     fstp qword ptr [esp]
0046D3D4 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046D3D7 e806f9ffff                 call 0x46cce2
0046D3DC b958d55800                 mov ecx, 0x58d558
0046D3E1 e8da0a0000                 call 0x46dec0
0046D3E6 85c0                       test eax, eax
0046D3E8 7502                       jne 0x46d3ec
0046D3EA eb70                       jmp 0x46d45c
0046D3EC db058cd25800               fild dword ptr [0x58d28c]
0046D3F2 83ec08                     sub esp, 8
0046D3F5 dd1c24                     fstp qword ptr [esp]
0046D3F8 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046D3FB e860faffff                 call 0x46ce60
0046D400 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046D403 e8b80a0000                 call 0x46dec0
0046D408 85c0                       test eax, eax
0046D40A 7502                       jne 0x46d40e
0046D40C eb4e                       jmp 0x46d45c
0046D40E db0584d25800               fild dword ptr [0x58d284]
0046D414 83ec08                     sub esp, 8
0046D417 dd1c24                     fstp qword ptr [esp]
0046D41A 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046D41D e8bcfbffff                 call 0x46cfde
0046D422 b958d55800                 mov ecx, 0x58d558
0046D427 e8940a0000                 call 0x46dec0
0046D42C 85c0                       test eax, eax
0046D42E 7502                       jne 0x46d432
0046D430 eb2a                       jmp 0x46d45c
0046D432 db0580d25800               fild dword ptr [0x58d280]
0046D438 83ec08                     sub esp, 8
0046D43B dd1c24                     fstp qword ptr [esp]
0046D43E 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046D441 e816fdffff                 call 0x46d15c
0046D446 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046D449 e8720a0000                 call 0x46dec0
0046D44E 85c0                       test eax, eax
0046D450 7502                       jne 0x46d454
0046D452 eb08                       jmp 0x46d45c
0046D454 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046D457 e804000000                 call 0x46d460
0046D45C 8be5                       mov esp, ebp
0046D45E 5d                         pop ebp
0046D45F c3                         ret

; VA 0046CCE2 file offset 6CCE2 length 382 SHA256 c69a5ce8d74dddbf7c8315c3509ace323d7b51806e44f99a76a75c449c084610
0046CCE2 55                         push ebp
0046CCE3 8bec                       mov ebp, esp
0046CCE5 81ecf0000000               sub esp, 0xf0
0046CCEB 56                         push esi
0046CCEC 57                         push edi
0046CCED 898d18ffffff               mov dword ptr [ebp - 0xe8], ecx
0046CCF3 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046CCF9 e8c2110000                 call 0x46dec0
0046CCFE 83e801                     sub eax, 1
0046CD01 50                         push eax
0046CD02 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046CD08 e8337dfaff                 call 0x414a40
0046CD0D 8945fc                     mov dword ptr [ebp - 4], eax
0046CD10 6a00                       push 0
0046CD12 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046CD18 e8237dfaff                 call 0x414a40
0046CD1D 8945f8                     mov dword ptr [ebp - 8], eax
0046CD20 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046CD26 e895110000                 call 0x46dec0
0046CD2B 8945f4                     mov dword ptr [ebp - 0xc], eax
0046CD2E b958d55800                 mov ecx, 0x58d558
0046CD33 e8b881faff                 call 0x414ef0
0046CD38 8b45f8                     mov eax, dword ptr [ebp - 8]
0046CD3B 8b08                       mov ecx, dword ptr [eax]
0046CD3D dd4120                     fld qword ptr [ecx + 0x20]
0046CD40 dc5d08                     fcomp qword ptr [ebp + 8]
0046CD43 dfe0                       fnstsw ax
0046CD45 f6c441                     test ah, 0x41
0046CD48 747b                       je 0x46cdc5
0046CD4A 8b55fc                     mov edx, dword ptr [ebp - 4]
0046CD4D 8b02                       mov eax, dword ptr [edx]
0046CD4F dd4020                     fld qword ptr [eax + 0x20]
0046CD52 dc5d08                     fcomp qword ptr [ebp + 8]
0046CD55 dfe0                       fnstsw ax
0046CD57 f6c441                     test ah, 0x41
0046CD5A 7559                       jne 0x46cdb5
0046CD5C b978d55800                 mov ecx, 0x58d578
0046CD61 e82a120000                 call 0x46df90
0046CD66 8945f0                     mov dword ptr [ebp - 0x10], eax
0046CD69 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046CD6C 8b11                       mov edx, dword ptr [ecx]
0046CD6E 52                         push edx
0046CD6F 8b45fc                     mov eax, dword ptr [ebp - 4]
0046CD72 8b08                       mov ecx, dword ptr [eax]
0046CD74 51                         push ecx
0046CD75 8b550c                     mov edx, dword ptr [ebp + 0xc]
0046CD78 52                         push edx
0046CD79 8b4508                     mov eax, dword ptr [ebp + 8]
0046CD7C 50                         push eax
0046CD7D 8d4d84                     lea ecx, [ebp - 0x7c]
0046CD80 51                         push ecx
0046CD81 e88a0b0000                 call 0x46d910
0046CD86 83c414                     add esp, 0x14
0046CD89 898514ffffff               mov dword ptr [ebp - 0xec], eax
0046CD8F 8bb514ffffff               mov esi, dword ptr [ebp - 0xec]
0046CD95 b91a000000                 mov ecx, 0x1a
0046CD9A 8b7df0                     mov edi, dword ptr [ebp - 0x10]
0046CD9D f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046CD9F 8d4d84                     lea ecx, [ebp - 0x7c]
0046CDA2 e8f93ffbff                 call 0x420da0
0046CDA7 8d55f0                     lea edx, [ebp - 0x10]
0046CDAA 52                         push edx
0046CDAB b958d55800                 mov ecx, 0x58d558
0046CDB0 e81b110000                 call 0x46ded0
0046CDB5 8b45f8                     mov eax, dword ptr [ebp - 8]
0046CDB8 50                         push eax
0046CDB9 b958d55800                 mov ecx, 0x58d558
0046CDBE e80d110000                 call 0x46ded0
0046CDC3 eb71                       jmp 0x46ce36
0046CDC5 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046CDC8 8b11                       mov edx, dword ptr [ecx]
0046CDCA dd4220                     fld qword ptr [edx + 0x20]
0046CDCD dc5d08                     fcomp qword ptr [ebp + 8]
0046CDD0 dfe0                       fnstsw ax
0046CDD2 f6c441                     test ah, 0x41
0046CDD5 745f                       je 0x46ce36
0046CDD7 b978d55800                 mov ecx, 0x58d578
0046CDDC e8af110000                 call 0x46df90
0046CDE1 8945ec                     mov dword ptr [ebp - 0x14], eax
0046CDE4 8b45f8                     mov eax, dword ptr [ebp - 8]
0046CDE7 8b08                       mov ecx, dword ptr [eax]
0046CDE9 51                         push ecx
0046CDEA 8b55fc                     mov edx, dword ptr [ebp - 4]
0046CDED 8b02                       mov eax, dword ptr [edx]
0046CDEF 50                         push eax
0046CDF0 8b4d0c                     mov ecx, dword ptr [ebp + 0xc]
0046CDF3 51                         push ecx
0046CDF4 8b5508                     mov edx, dword ptr [ebp + 8]
0046CDF7 52                         push edx
0046CDF8 8d851cffffff               lea eax, [ebp - 0xe4]
0046CDFE 50                         push eax
0046CDFF e80c0b0000                 call 0x46d910
0046CE04 83c414                     add esp, 0x14
0046CE07 898510ffffff               mov dword ptr [ebp - 0xf0], eax
0046CE0D 8bb510ffffff               mov esi, dword ptr [ebp - 0xf0]
0046CE13 b91a000000                 mov ecx, 0x1a
0046CE18 8b7dec                     mov edi, dword ptr [ebp - 0x14]
0046CE1B f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046CE1D 8d8d1cffffff               lea ecx, [ebp - 0xe4]
0046CE23 e8783ffbff                 call 0x420da0
0046CE28 8d4dec                     lea ecx, [ebp - 0x14]
0046CE2B 51                         push ecx
0046CE2C b958d55800                 mov ecx, 0x58d558
0046CE31 e89a100000                 call 0x46ded0
0046CE36 8b55f8                     mov edx, dword ptr [ebp - 8]
0046CE39 8955fc                     mov dword ptr [ebp - 4], edx
0046CE3C 8b45f8                     mov eax, dword ptr [ebp - 8]
0046CE3F 83c004                     add eax, 4
0046CE42 8945f8                     mov dword ptr [ebp - 8], eax
0046CE45 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046CE48 83e901                     sub ecx, 1
0046CE4B 894df4                     mov dword ptr [ebp - 0xc], ecx
0046CE4E 837df400                   cmp dword ptr [ebp - 0xc], 0
0046CE52 0f8fe0feffff               jg 0x46cd38
0046CE58 5f                         pop edi
0046CE59 5e                         pop esi
0046CE5A 8be5                       mov esp, ebp
0046CE5C 5d                         pop ebp
0046CE5D c20800                     ret 8

; VA 0046CE60 file offset 6CE60 length 382 SHA256 f8baf6417cb7afacb4b4ed0d2c421cd65e0d089f197cd0d1c975c265d5c025ed
0046CE60 55                         push ebp
0046CE61 8bec                       mov ebp, esp
0046CE63 81ecf0000000               sub esp, 0xf0
0046CE69 56                         push esi
0046CE6A 57                         push edi
0046CE6B 898d18ffffff               mov dword ptr [ebp - 0xe8], ecx
0046CE71 b958d55800                 mov ecx, 0x58d558
0046CE76 e845100000                 call 0x46dec0
0046CE7B 83e801                     sub eax, 1
0046CE7E 50                         push eax
0046CE7F b958d55800                 mov ecx, 0x58d558
0046CE84 e8b77bfaff                 call 0x414a40
0046CE89 8945fc                     mov dword ptr [ebp - 4], eax
0046CE8C 6a00                       push 0
0046CE8E b958d55800                 mov ecx, 0x58d558
0046CE93 e8a87bfaff                 call 0x414a40
0046CE98 8945f8                     mov dword ptr [ebp - 8], eax
0046CE9B b958d55800                 mov ecx, 0x58d558
0046CEA0 e81b100000                 call 0x46dec0
0046CEA5 8945f4                     mov dword ptr [ebp - 0xc], eax
0046CEA8 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046CEAE e83d80faff                 call 0x414ef0
0046CEB3 8b45f8                     mov eax, dword ptr [ebp - 8]
0046CEB6 8b08                       mov ecx, dword ptr [eax]
0046CEB8 dd4120                     fld qword ptr [ecx + 0x20]
0046CEBB dc5d08                     fcomp qword ptr [ebp + 8]
0046CEBE dfe0                       fnstsw ax
0046CEC0 f6c401                     test ah, 1
0046CEC3 757d                       jne 0x46cf42
0046CEC5 8b55fc                     mov edx, dword ptr [ebp - 4]
0046CEC8 8b02                       mov eax, dword ptr [edx]
0046CECA dd4020                     fld qword ptr [eax + 0x20]
0046CECD dc5d08                     fcomp qword ptr [ebp + 8]
0046CED0 dfe0                       fnstsw ax
0046CED2 f6c401                     test ah, 1
0046CED5 745a                       je 0x46cf31
0046CED7 b978d55800                 mov ecx, 0x58d578
0046CEDC e8af100000                 call 0x46df90
0046CEE1 8945f0                     mov dword ptr [ebp - 0x10], eax
0046CEE4 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046CEE7 8b11                       mov edx, dword ptr [ecx]
0046CEE9 52                         push edx
0046CEEA 8b45fc                     mov eax, dword ptr [ebp - 4]
0046CEED 8b08                       mov ecx, dword ptr [eax]
0046CEEF 51                         push ecx
0046CEF0 8b550c                     mov edx, dword ptr [ebp + 0xc]
0046CEF3 52                         push edx
0046CEF4 8b4508                     mov eax, dword ptr [ebp + 8]
0046CEF7 50                         push eax
0046CEF8 8d4d84                     lea ecx, [ebp - 0x7c]
0046CEFB 51                         push ecx
0046CEFC e80f0a0000                 call 0x46d910
0046CF01 83c414                     add esp, 0x14
0046CF04 898514ffffff               mov dword ptr [ebp - 0xec], eax
0046CF0A 8bb514ffffff               mov esi, dword ptr [ebp - 0xec]
0046CF10 b91a000000                 mov ecx, 0x1a
0046CF15 8b7df0                     mov edi, dword ptr [ebp - 0x10]
0046CF18 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046CF1A 8d4d84                     lea ecx, [ebp - 0x7c]
0046CF1D e87e3efbff                 call 0x420da0
0046CF22 8d55f0                     lea edx, [ebp - 0x10]
0046CF25 52                         push edx
0046CF26 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046CF2C e89f0f0000                 call 0x46ded0
0046CF31 8b45f8                     mov eax, dword ptr [ebp - 8]
0046CF34 50                         push eax
0046CF35 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046CF3B e8900f0000                 call 0x46ded0
0046CF40 eb72                       jmp 0x46cfb4
0046CF42 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046CF45 8b11                       mov edx, dword ptr [ecx]
0046CF47 dd4220                     fld qword ptr [edx + 0x20]
0046CF4A dc5d08                     fcomp qword ptr [ebp + 8]
0046CF4D dfe0                       fnstsw ax
0046CF4F f6c401                     test ah, 1
0046CF52 7560                       jne 0x46cfb4
0046CF54 b978d55800                 mov ecx, 0x58d578
0046CF59 e832100000                 call 0x46df90
0046CF5E 8945ec                     mov dword ptr [ebp - 0x14], eax
0046CF61 8b45f8                     mov eax, dword ptr [ebp - 8]
0046CF64 8b08                       mov ecx, dword ptr [eax]
0046CF66 51                         push ecx
0046CF67 8b55fc                     mov edx, dword ptr [ebp - 4]
0046CF6A 8b02                       mov eax, dword ptr [edx]
0046CF6C 50                         push eax
0046CF6D 8b4d0c                     mov ecx, dword ptr [ebp + 0xc]
0046CF70 51                         push ecx
0046CF71 8b5508                     mov edx, dword ptr [ebp + 8]
0046CF74 52                         push edx
0046CF75 8d851cffffff               lea eax, [ebp - 0xe4]
0046CF7B 50                         push eax
0046CF7C e88f090000                 call 0x46d910
0046CF81 83c414                     add esp, 0x14
0046CF84 898510ffffff               mov dword ptr [ebp - 0xf0], eax
0046CF8A 8bb510ffffff               mov esi, dword ptr [ebp - 0xf0]
0046CF90 b91a000000                 mov ecx, 0x1a
0046CF95 8b7dec                     mov edi, dword ptr [ebp - 0x14]
0046CF98 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046CF9A 8d8d1cffffff               lea ecx, [ebp - 0xe4]
0046CFA0 e8fb3dfbff                 call 0x420da0
0046CFA5 8d4dec                     lea ecx, [ebp - 0x14]
0046CFA8 51                         push ecx
0046CFA9 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046CFAF e81c0f0000                 call 0x46ded0
0046CFB4 8b55f8                     mov edx, dword ptr [ebp - 8]
0046CFB7 8955fc                     mov dword ptr [ebp - 4], edx
0046CFBA 8b45f8                     mov eax, dword ptr [ebp - 8]
0046CFBD 83c004                     add eax, 4
0046CFC0 8945f8                     mov dword ptr [ebp - 8], eax
0046CFC3 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046CFC6 83e901                     sub ecx, 1
0046CFC9 894df4                     mov dword ptr [ebp - 0xc], ecx
0046CFCC 837df400                   cmp dword ptr [ebp - 0xc], 0
0046CFD0 0f8fddfeffff               jg 0x46ceb3
0046CFD6 5f                         pop edi
0046CFD7 5e                         pop esi
0046CFD8 8be5                       mov esp, ebp
0046CFDA 5d                         pop ebp
0046CFDB c20800                     ret 8

; VA 0046CFDE file offset 6CFDE length 382 SHA256 0431cd5069bf0d221306b91216abac2d39e9e1614ba8824dd9df3a79feba1c81
0046CFDE 55                         push ebp
0046CFDF 8bec                       mov ebp, esp
0046CFE1 81ecf0000000               sub esp, 0xf0
0046CFE7 56                         push esi
0046CFE8 57                         push edi
0046CFE9 898d18ffffff               mov dword ptr [ebp - 0xe8], ecx
0046CFEF 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046CFF5 e8c60e0000                 call 0x46dec0
0046CFFA 83e801                     sub eax, 1
0046CFFD 50                         push eax
0046CFFE 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046D004 e8377afaff                 call 0x414a40
0046D009 8945fc                     mov dword ptr [ebp - 4], eax
0046D00C 6a00                       push 0
0046D00E 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046D014 e8277afaff                 call 0x414a40
0046D019 8945f8                     mov dword ptr [ebp - 8], eax
0046D01C 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046D022 e8990e0000                 call 0x46dec0
0046D027 8945f4                     mov dword ptr [ebp - 0xc], eax
0046D02A b958d55800                 mov ecx, 0x58d558
0046D02F e8bc7efaff                 call 0x414ef0
0046D034 8b45f8                     mov eax, dword ptr [ebp - 8]
0046D037 8b08                       mov ecx, dword ptr [eax]
0046D039 dd4118                     fld qword ptr [ecx + 0x18]
0046D03C dc5d08                     fcomp qword ptr [ebp + 8]
0046D03F dfe0                       fnstsw ax
0046D041 f6c441                     test ah, 0x41
0046D044 747b                       je 0x46d0c1
0046D046 8b55fc                     mov edx, dword ptr [ebp - 4]
0046D049 8b02                       mov eax, dword ptr [edx]
0046D04B dd4018                     fld qword ptr [eax + 0x18]
0046D04E dc5d08                     fcomp qword ptr [ebp + 8]
0046D051 dfe0                       fnstsw ax
0046D053 f6c441                     test ah, 0x41
0046D056 7559                       jne 0x46d0b1
0046D058 b978d55800                 mov ecx, 0x58d578
0046D05D e82e0f0000                 call 0x46df90
0046D062 8945f0                     mov dword ptr [ebp - 0x10], eax
0046D065 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046D068 8b11                       mov edx, dword ptr [ecx]
0046D06A 52                         push edx
0046D06B 8b45fc                     mov eax, dword ptr [ebp - 4]
0046D06E 8b08                       mov ecx, dword ptr [eax]
0046D070 51                         push ecx
0046D071 8b550c                     mov edx, dword ptr [ebp + 0xc]
0046D074 52                         push edx
0046D075 8b4508                     mov eax, dword ptr [ebp + 8]
0046D078 50                         push eax
0046D079 8d4d84                     lea ecx, [ebp - 0x7c]
0046D07C 51                         push ecx
0046D07D e8fe0a0000                 call 0x46db80
0046D082 83c414                     add esp, 0x14
0046D085 898514ffffff               mov dword ptr [ebp - 0xec], eax
0046D08B 8bb514ffffff               mov esi, dword ptr [ebp - 0xec]
0046D091 b91a000000                 mov ecx, 0x1a
0046D096 8b7df0                     mov edi, dword ptr [ebp - 0x10]
0046D099 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046D09B 8d4d84                     lea ecx, [ebp - 0x7c]
0046D09E e8fd3cfbff                 call 0x420da0
0046D0A3 8d55f0                     lea edx, [ebp - 0x10]
0046D0A6 52                         push edx
0046D0A7 b958d55800                 mov ecx, 0x58d558
0046D0AC e81f0e0000                 call 0x46ded0
0046D0B1 8b45f8                     mov eax, dword ptr [ebp - 8]
0046D0B4 50                         push eax
0046D0B5 b958d55800                 mov ecx, 0x58d558
0046D0BA e8110e0000                 call 0x46ded0
0046D0BF eb71                       jmp 0x46d132
0046D0C1 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046D0C4 8b11                       mov edx, dword ptr [ecx]
0046D0C6 dd4218                     fld qword ptr [edx + 0x18]
0046D0C9 dc5d08                     fcomp qword ptr [ebp + 8]
0046D0CC dfe0                       fnstsw ax
0046D0CE f6c441                     test ah, 0x41
0046D0D1 745f                       je 0x46d132
0046D0D3 b978d55800                 mov ecx, 0x58d578
0046D0D8 e8b30e0000                 call 0x46df90
0046D0DD 8945ec                     mov dword ptr [ebp - 0x14], eax
0046D0E0 8b45f8                     mov eax, dword ptr [ebp - 8]
0046D0E3 8b08                       mov ecx, dword ptr [eax]
0046D0E5 51                         push ecx
0046D0E6 8b55fc                     mov edx, dword ptr [ebp - 4]
0046D0E9 8b02                       mov eax, dword ptr [edx]
0046D0EB 50                         push eax
0046D0EC 8b4d0c                     mov ecx, dword ptr [ebp + 0xc]
0046D0EF 51                         push ecx
0046D0F0 8b5508                     mov edx, dword ptr [ebp + 8]
0046D0F3 52                         push edx
0046D0F4 8d851cffffff               lea eax, [ebp - 0xe4]
0046D0FA 50                         push eax
0046D0FB e8800a0000                 call 0x46db80
0046D100 83c414                     add esp, 0x14
0046D103 898510ffffff               mov dword ptr [ebp - 0xf0], eax
0046D109 8bb510ffffff               mov esi, dword ptr [ebp - 0xf0]
0046D10F b91a000000                 mov ecx, 0x1a
0046D114 8b7dec                     mov edi, dword ptr [ebp - 0x14]
0046D117 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046D119 8d8d1cffffff               lea ecx, [ebp - 0xe4]
0046D11F e87c3cfbff                 call 0x420da0
0046D124 8d4dec                     lea ecx, [ebp - 0x14]
0046D127 51                         push ecx
0046D128 b958d55800                 mov ecx, 0x58d558
0046D12D e89e0d0000                 call 0x46ded0
0046D132 8b55f8                     mov edx, dword ptr [ebp - 8]
0046D135 8955fc                     mov dword ptr [ebp - 4], edx
0046D138 8b45f8                     mov eax, dword ptr [ebp - 8]
0046D13B 83c004                     add eax, 4
0046D13E 8945f8                     mov dword ptr [ebp - 8], eax
0046D141 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046D144 83e901                     sub ecx, 1
0046D147 894df4                     mov dword ptr [ebp - 0xc], ecx
0046D14A 837df400                   cmp dword ptr [ebp - 0xc], 0
0046D14E 0f8fe0feffff               jg 0x46d034
0046D154 5f                         pop edi
0046D155 5e                         pop esi
0046D156 8be5                       mov esp, ebp
0046D158 5d                         pop ebp
0046D159 c20800                     ret 8

; VA 0046D15C file offset 6D15C length 382 SHA256 d08c8417b7d8c4a382cc93b3c32e309ffb566a45d2e1dcece42657bf4fb22d58
0046D15C 55                         push ebp
0046D15D 8bec                       mov ebp, esp
0046D15F 81ecf0000000               sub esp, 0xf0
0046D165 56                         push esi
0046D166 57                         push edi
0046D167 898d18ffffff               mov dword ptr [ebp - 0xe8], ecx
0046D16D b958d55800                 mov ecx, 0x58d558
0046D172 e8490d0000                 call 0x46dec0
0046D177 83e801                     sub eax, 1
0046D17A 50                         push eax
0046D17B b958d55800                 mov ecx, 0x58d558
0046D180 e8bb78faff                 call 0x414a40
0046D185 8945fc                     mov dword ptr [ebp - 4], eax
0046D188 6a00                       push 0
0046D18A b958d55800                 mov ecx, 0x58d558
0046D18F e8ac78faff                 call 0x414a40
0046D194 8945f8                     mov dword ptr [ebp - 8], eax
0046D197 b958d55800                 mov ecx, 0x58d558
0046D19C e81f0d0000                 call 0x46dec0
0046D1A1 8945f4                     mov dword ptr [ebp - 0xc], eax
0046D1A4 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046D1AA e8417dfaff                 call 0x414ef0
0046D1AF 8b45f8                     mov eax, dword ptr [ebp - 8]
0046D1B2 8b08                       mov ecx, dword ptr [eax]
0046D1B4 dd4118                     fld qword ptr [ecx + 0x18]
0046D1B7 dc5d08                     fcomp qword ptr [ebp + 8]
0046D1BA dfe0                       fnstsw ax
0046D1BC f6c401                     test ah, 1
0046D1BF 757d                       jne 0x46d23e
0046D1C1 8b55fc                     mov edx, dword ptr [ebp - 4]
0046D1C4 8b02                       mov eax, dword ptr [edx]
0046D1C6 dd4018                     fld qword ptr [eax + 0x18]
0046D1C9 dc5d08                     fcomp qword ptr [ebp + 8]
0046D1CC dfe0                       fnstsw ax
0046D1CE f6c401                     test ah, 1
0046D1D1 745a                       je 0x46d22d
0046D1D3 b978d55800                 mov ecx, 0x58d578
0046D1D8 e8b30d0000                 call 0x46df90
0046D1DD 8945f0                     mov dword ptr [ebp - 0x10], eax
0046D1E0 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046D1E3 8b11                       mov edx, dword ptr [ecx]
0046D1E5 52                         push edx
0046D1E6 8b45fc                     mov eax, dword ptr [ebp - 4]
0046D1E9 8b08                       mov ecx, dword ptr [eax]
0046D1EB 51                         push ecx
0046D1EC 8b550c                     mov edx, dword ptr [ebp + 0xc]
0046D1EF 52                         push edx
0046D1F0 8b4508                     mov eax, dword ptr [ebp + 8]
0046D1F3 50                         push eax
0046D1F4 8d4d84                     lea ecx, [ebp - 0x7c]
0046D1F7 51                         push ecx
0046D1F8 e883090000                 call 0x46db80
0046D1FD 83c414                     add esp, 0x14
0046D200 898514ffffff               mov dword ptr [ebp - 0xec], eax
0046D206 8bb514ffffff               mov esi, dword ptr [ebp - 0xec]
0046D20C b91a000000                 mov ecx, 0x1a
0046D211 8b7df0                     mov edi, dword ptr [ebp - 0x10]
0046D214 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046D216 8d4d84                     lea ecx, [ebp - 0x7c]
0046D219 e8823bfbff                 call 0x420da0
0046D21E 8d55f0                     lea edx, [ebp - 0x10]
0046D221 52                         push edx
0046D222 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046D228 e8a30c0000                 call 0x46ded0
0046D22D 8b45f8                     mov eax, dword ptr [ebp - 8]
0046D230 50                         push eax
0046D231 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046D237 e8940c0000                 call 0x46ded0
0046D23C eb72                       jmp 0x46d2b0
0046D23E 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046D241 8b11                       mov edx, dword ptr [ecx]
0046D243 dd4218                     fld qword ptr [edx + 0x18]
0046D246 dc5d08                     fcomp qword ptr [ebp + 8]
0046D249 dfe0                       fnstsw ax
0046D24B f6c401                     test ah, 1
0046D24E 7560                       jne 0x46d2b0
0046D250 b978d55800                 mov ecx, 0x58d578
0046D255 e8360d0000                 call 0x46df90
0046D25A 8945ec                     mov dword ptr [ebp - 0x14], eax
0046D25D 8b45f8                     mov eax, dword ptr [ebp - 8]
0046D260 8b08                       mov ecx, dword ptr [eax]
0046D262 51                         push ecx
0046D263 8b55fc                     mov edx, dword ptr [ebp - 4]
0046D266 8b02                       mov eax, dword ptr [edx]
0046D268 50                         push eax
0046D269 8b4d0c                     mov ecx, dword ptr [ebp + 0xc]
0046D26C 51                         push ecx
0046D26D 8b5508                     mov edx, dword ptr [ebp + 8]
0046D270 52                         push edx
0046D271 8d851cffffff               lea eax, [ebp - 0xe4]
0046D277 50                         push eax
0046D278 e803090000                 call 0x46db80
0046D27D 83c414                     add esp, 0x14
0046D280 898510ffffff               mov dword ptr [ebp - 0xf0], eax
0046D286 8bb510ffffff               mov esi, dword ptr [ebp - 0xf0]
0046D28C b91a000000                 mov ecx, 0x1a
0046D291 8b7dec                     mov edi, dword ptr [ebp - 0x14]
0046D294 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046D296 8d8d1cffffff               lea ecx, [ebp - 0xe4]
0046D29C e8ff3afbff                 call 0x420da0
0046D2A1 8d4dec                     lea ecx, [ebp - 0x14]
0046D2A4 51                         push ecx
0046D2A5 8b8d18ffffff               mov ecx, dword ptr [ebp - 0xe8]
0046D2AB e8200c0000                 call 0x46ded0
0046D2B0 8b55f8                     mov edx, dword ptr [ebp - 8]
0046D2B3 8955fc                     mov dword ptr [ebp - 4], edx
0046D2B6 8b45f8                     mov eax, dword ptr [ebp - 8]
0046D2B9 83c004                     add eax, 4
0046D2BC 8945f8                     mov dword ptr [ebp - 8], eax
0046D2BF 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046D2C2 83e901                     sub ecx, 1
0046D2C5 894df4                     mov dword ptr [ebp - 0xc], ecx
0046D2C8 837df400                   cmp dword ptr [ebp - 0xc], 0
0046D2CC 0f8fddfeffff               jg 0x46d1af
0046D2D2 5f                         pop edi
0046D2D3 5e                         pop esi
0046D2D4 8be5                       mov esp, ebp
0046D2D6 5d                         pop ebp
0046D2D7 c20800                     ret 8

; VA 0046D910 file offset 6D910 length 615 SHA256 3c88e21018cfa6ab5dd722d5e22b92031b71d68616ed2840e1f0b27c302f653e
0046D910 55                         push ebp
0046D911 8bec                       mov ebp, esp
0046D913 6aff                       push -1
0046D915 6853dc4a00                 push 0x4adc53
0046D91A 64a100000000               mov eax, dword ptr fs:[0]
0046D920 50                         push eax
0046D921 64892500000000             mov dword ptr fs:[0], esp
0046D928 81ec3c010000               sub esp, 0x13c
0046D92E 56                         push esi
0046D92F 57                         push edi
0046D930 c785b8feffff00000000       mov dword ptr [ebp - 0x148], 0
0046D93A 8b4514                     mov eax, dword ptr [ebp + 0x14]
0046D93D 8b4d18                     mov ecx, dword ptr [ebp + 0x18]
0046D940 dd4020                     fld qword ptr [eax + 0x20]
0046D943 dc5920                     fcomp qword ptr [ecx + 0x20]
0046D946 dfe0                       fnstsw ax
0046D948 f6c441                     test ah, 0x41
0046D94B 7511                       jne 0x46d95e
0046D94D 8b5514                     mov edx, dword ptr [ebp + 0x14]
0046D950 89957cffffff               mov dword ptr [ebp - 0x84], edx
0046D956 8b4518                     mov eax, dword ptr [ebp + 0x18]
0046D959 894580                     mov dword ptr [ebp - 0x80], eax
0046D95C eb0f                       jmp 0x46d96d
0046D95E 8b4d18                     mov ecx, dword ptr [ebp + 0x18]
0046D961 898d7cffffff               mov dword ptr [ebp - 0x84], ecx
0046D967 8b5514                     mov edx, dword ptr [ebp + 0x14]
0046D96A 895580                     mov dword ptr [ebp - 0x80], edx
0046D96D 8d4d84                     lea ecx, [ebp - 0x7c]
0046D970 e8eb33fbff                 call 0x420d60
0046D975 c745fc01000000             mov dword ptr [ebp - 4], 1
0046D97C 8b857cffffff               mov eax, dword ptr [ebp - 0x84]
0046D982 dd450c                     fld qword ptr [ebp + 0xc]
0046D985 dc6020                     fsub qword ptr [eax + 0x20]
0046D988 8b4d80                     mov ecx, dword ptr [ebp - 0x80]
0046D98B 8b957cffffff               mov edx, dword ptr [ebp - 0x84]
0046D991 dd4120                     fld qword ptr [ecx + 0x20]
0046D994 dc6220                     fsub qword ptr [edx + 0x20]
0046D997 def9                       fdivp st(1)
0046D999 dd5dec                     fstp qword ptr [ebp - 0x14]
0046D99C 8b857cffffff               mov eax, dword ptr [ebp - 0x84]
0046D9A2 8b4d80                     mov ecx, dword ptr [ebp - 0x80]
0046D9A5 8b957cffffff               mov edx, dword ptr [ebp - 0x84]
0046D9AB dd4118                     fld qword ptr [ecx + 0x18]
0046D9AE dc6218                     fsub qword ptr [edx + 0x18]
0046D9B1 dc4dec                     fmul qword ptr [ebp - 0x14]
0046D9B4 dc4018                     fadd qword ptr [eax + 0x18]
0046D9B7 dd5d9c                     fstp qword ptr [ebp - 0x64]
0046D9BA 8b450c                     mov eax, dword ptr [ebp + 0xc]
0046D9BD 8945a4                     mov dword ptr [ebp - 0x5c], eax
0046D9C0 8b4d10                     mov ecx, dword ptr [ebp + 0x10]
0046D9C3 894da8                     mov dword ptr [ebp - 0x58], ecx
0046D9C6 8b5580                     mov edx, dword ptr [ebp - 0x80]
0046D9C9 8b4580                     mov eax, dword ptr [ebp - 0x80]
0046D9CC dd4020                     fld qword ptr [eax + 0x20]
0046D9CF dc650c                     fsub qword ptr [ebp + 0xc]
0046D9D2 dc4a10                     fmul qword ptr [edx + 0x10]
0046D9D5 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046D9DB 8b957cffffff               mov edx, dword ptr [ebp - 0x84]
0046D9E1 dd450c                     fld qword ptr [ebp + 0xc]
0046D9E4 dc6220                     fsub qword ptr [edx + 0x20]
0046D9E7 dc4910                     fmul qword ptr [ecx + 0x10]
0046D9EA dec1                       faddp st(1)
0046D9EC dd5dec                     fstp qword ptr [ebp - 0x14]
0046D9EF dd45ec                     fld qword ptr [ebp - 0x14]
0046D9F2 dc1d38034b00               fcomp qword ptr [0x4b0338]
0046D9F8 dfe0                       fnstsw ax
0046D9FA f6c440                     test ah, 0x40
0046D9FD 751b                       jne 0x46da1a
0046D9FF 8b857cffffff               mov eax, dword ptr [ebp - 0x84]
0046DA05 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DA0B dd450c                     fld qword ptr [ebp + 0xc]
0046DA0E dc6120                     fsub qword ptr [ecx + 0x20]
0046DA11 dc4810                     fmul qword ptr [eax + 0x10]
0046DA14 dc75ec                     fdiv qword ptr [ebp - 0x14]
0046DA17 dd5dec                     fstp qword ptr [ebp - 0x14]
0046DA1A 8b55f0                     mov edx, dword ptr [ebp - 0x10]
0046DA1D 52                         push edx
0046DA1E 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0046DA21 50                         push eax
0046DA22 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DA28 51                         push ecx
0046DA29 8d9564ffffff               lea edx, [ebp - 0x9c]
0046DA2F 52                         push edx
0046DA30 8b4d80                     mov ecx, dword ptr [ebp - 0x80]
0046DA33 e8f83df9ff                 call 0x401830
0046DA38 50                         push eax
0046DA39 8d854cffffff               lea eax, [ebp - 0xb4]
0046DA3F 50                         push eax
0046DA40 e87b4df9ff                 call 0x4027c0
0046DA45 83c410                     add esp, 0x10
0046DA48 50                         push eax
0046DA49 8d8d34ffffff               lea ecx, [ebp - 0xcc]
0046DA4F 51                         push ecx
0046DA50 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DA56 e8254cf9ff                 call 0x402680
0046DA5B 8bf0                       mov esi, eax
0046DA5D b906000000                 mov ecx, 6
0046DA62 8d7d84                     lea edi, [ebp - 0x7c]
0046DA65 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046DA67 8b55f0                     mov edx, dword ptr [ebp - 0x10]
0046DA6A 52                         push edx
0046DA6B 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0046DA6E 50                         push eax
0046DA6F 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DA75 83c128                     add ecx, 0x28
0046DA78 51                         push ecx
0046DA79 8d9524ffffff               lea edx, [ebp - 0xdc]
0046DA7F 52                         push edx
0046DA80 8b4d80                     mov ecx, dword ptr [ebp - 0x80]
0046DA83 83c128                     add ecx, 0x28
0046DA86 e8e54cfeff                 call 0x452770
0046DA8B 50                         push eax
0046DA8C 8d8514ffffff               lea eax, [ebp - 0xec]
0046DA92 50                         push eax
0046DA93 e8384dfeff                 call 0x4527d0
0046DA98 83c410                     add esp, 0x10
0046DA9B 50                         push eax
0046DA9C 8d8d04ffffff               lea ecx, [ebp - 0xfc]
0046DAA2 51                         push ecx
0046DAA3 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DAA9 83c128                     add ecx, 0x28
0046DAAC e8fffdffff                 call 0x46d8b0
0046DAB1 8b10                       mov edx, dword ptr [eax]
0046DAB3 8955ac                     mov dword ptr [ebp - 0x54], edx
0046DAB6 8b4804                     mov ecx, dword ptr [eax + 4]
0046DAB9 894db0                     mov dword ptr [ebp - 0x50], ecx
0046DABC 8b5008                     mov edx, dword ptr [eax + 8]
0046DABF 8955b4                     mov dword ptr [ebp - 0x4c], edx
0046DAC2 8b400c                     mov eax, dword ptr [eax + 0xc]
0046DAC5 8945b8                     mov dword ptr [ebp - 0x48], eax
0046DAC8 8b4df0                     mov ecx, dword ptr [ebp - 0x10]
0046DACB 51                         push ecx
0046DACC 8b55ec                     mov edx, dword ptr [ebp - 0x14]
0046DACF 52                         push edx
0046DAD0 8b857cffffff               mov eax, dword ptr [ebp - 0x84]
0046DAD6 83c038                     add eax, 0x38
0046DAD9 50                         push eax
0046DADA 8d8decfeffff               lea ecx, [ebp - 0x114]
0046DAE0 51                         push ecx
0046DAE1 8b4d80                     mov ecx, dword ptr [ebp - 0x80]
0046DAE4 83c138                     add ecx, 0x38
0046DAE7 e8443df9ff                 call 0x401830
0046DAEC 50                         push eax
0046DAED 8d95d4feffff               lea edx, [ebp - 0x12c]
0046DAF3 52                         push edx
0046DAF4 e8c74cf9ff                 call 0x4027c0
0046DAF9 83c410                     add esp, 0x10
0046DAFC 50                         push eax
0046DAFD 8d85bcfeffff               lea eax, [ebp - 0x144]
0046DB03 50                         push eax
0046DB04 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DB0A 83c138                     add ecx, 0x38
0046DB0D e86e4bf9ff                 call 0x402680
0046DB12 8bf0                       mov esi, eax
0046DB14 b906000000                 mov ecx, 6
0046DB19 8d7dbc                     lea edi, [ebp - 0x44]
0046DB1C f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046DB1E 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DB24 8b5580                     mov edx, dword ptr [ebp - 0x80]
0046DB27 8b857cffffff               mov eax, dword ptr [ebp - 0x84]
0046DB2D dd4250                     fld qword ptr [edx + 0x50]
0046DB30 dc6050                     fsub qword ptr [eax + 0x50]
0046DB33 dc4dec                     fmul qword ptr [ebp - 0x14]
0046DB36 dc4150                     fadd qword ptr [ecx + 0x50]
0046DB39 dd5dd4                     fstp qword ptr [ebp - 0x2c]
0046DB3C b91a000000                 mov ecx, 0x1a
0046DB41 8d7584                     lea esi, [ebp - 0x7c]
0046DB44 8b7d08                     mov edi, dword ptr [ebp + 8]
0046DB47 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046DB49 8b8db8feffff               mov ecx, dword ptr [ebp - 0x148]
0046DB4F 83c901                     or ecx, 1
0046DB52 898db8feffff               mov dword ptr [ebp - 0x148], ecx
0046DB58 c645fc00                   mov byte ptr [ebp - 4], 0
0046DB5C 8d4d84                     lea ecx, [ebp - 0x7c]
0046DB5F e83c32fbff                 call 0x420da0
0046DB64 8b4508                     mov eax, dword ptr [ebp + 8]
0046DB67 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046DB6A 64890d00000000             mov dword ptr fs:[0], ecx
0046DB71 5f                         pop edi
0046DB72 5e                         pop esi
0046DB73 8be5                       mov esp, ebp
0046DB75 5d                         pop ebp
0046DB76 c3                         ret

; VA 0046DB80 file offset 6DB80 length 615 SHA256 b6d870e26d32d35bdfc0d4d1dbc3253c48b5128437c38257772a11f40fd56199
0046DB80 55                         push ebp
0046DB81 8bec                       mov ebp, esp
0046DB83 6aff                       push -1
0046DB85 6883dc4a00                 push 0x4adc83
0046DB8A 64a100000000               mov eax, dword ptr fs:[0]
0046DB90 50                         push eax
0046DB91 64892500000000             mov dword ptr fs:[0], esp
0046DB98 81ec3c010000               sub esp, 0x13c
0046DB9E 56                         push esi
0046DB9F 57                         push edi
0046DBA0 c785b8feffff00000000       mov dword ptr [ebp - 0x148], 0
0046DBAA 8b4514                     mov eax, dword ptr [ebp + 0x14]
0046DBAD 8b4d18                     mov ecx, dword ptr [ebp + 0x18]
0046DBB0 dd4018                     fld qword ptr [eax + 0x18]
0046DBB3 dc5918                     fcomp qword ptr [ecx + 0x18]
0046DBB6 dfe0                       fnstsw ax
0046DBB8 f6c441                     test ah, 0x41
0046DBBB 7511                       jne 0x46dbce
0046DBBD 8b5514                     mov edx, dword ptr [ebp + 0x14]
0046DBC0 89957cffffff               mov dword ptr [ebp - 0x84], edx
0046DBC6 8b4518                     mov eax, dword ptr [ebp + 0x18]
0046DBC9 894580                     mov dword ptr [ebp - 0x80], eax
0046DBCC eb0f                       jmp 0x46dbdd
0046DBCE 8b4d18                     mov ecx, dword ptr [ebp + 0x18]
0046DBD1 898d7cffffff               mov dword ptr [ebp - 0x84], ecx
0046DBD7 8b5514                     mov edx, dword ptr [ebp + 0x14]
0046DBDA 895580                     mov dword ptr [ebp - 0x80], edx
0046DBDD 8d4d84                     lea ecx, [ebp - 0x7c]
0046DBE0 e87b31fbff                 call 0x420d60
0046DBE5 c745fc01000000             mov dword ptr [ebp - 4], 1
0046DBEC 8b857cffffff               mov eax, dword ptr [ebp - 0x84]
0046DBF2 dd450c                     fld qword ptr [ebp + 0xc]
0046DBF5 dc6018                     fsub qword ptr [eax + 0x18]
0046DBF8 8b4d80                     mov ecx, dword ptr [ebp - 0x80]
0046DBFB 8b957cffffff               mov edx, dword ptr [ebp - 0x84]
0046DC01 dd4118                     fld qword ptr [ecx + 0x18]
0046DC04 dc6218                     fsub qword ptr [edx + 0x18]
0046DC07 def9                       fdivp st(1)
0046DC09 dd5dec                     fstp qword ptr [ebp - 0x14]
0046DC0C 8b450c                     mov eax, dword ptr [ebp + 0xc]
0046DC0F 89459c                     mov dword ptr [ebp - 0x64], eax
0046DC12 8b4d10                     mov ecx, dword ptr [ebp + 0x10]
0046DC15 894da0                     mov dword ptr [ebp - 0x60], ecx
0046DC18 8b957cffffff               mov edx, dword ptr [ebp - 0x84]
0046DC1E 8b4580                     mov eax, dword ptr [ebp - 0x80]
0046DC21 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DC27 dd4020                     fld qword ptr [eax + 0x20]
0046DC2A dc6120                     fsub qword ptr [ecx + 0x20]
0046DC2D dc4dec                     fmul qword ptr [ebp - 0x14]
0046DC30 dc4220                     fadd qword ptr [edx + 0x20]
0046DC33 dd5da4                     fstp qword ptr [ebp - 0x5c]
0046DC36 8b5580                     mov edx, dword ptr [ebp - 0x80]
0046DC39 8b4580                     mov eax, dword ptr [ebp - 0x80]
0046DC3C dd4018                     fld qword ptr [eax + 0x18]
0046DC3F dc650c                     fsub qword ptr [ebp + 0xc]
0046DC42 dc4a10                     fmul qword ptr [edx + 0x10]
0046DC45 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DC4B 8b957cffffff               mov edx, dword ptr [ebp - 0x84]
0046DC51 dd450c                     fld qword ptr [ebp + 0xc]
0046DC54 dc6218                     fsub qword ptr [edx + 0x18]
0046DC57 dc4910                     fmul qword ptr [ecx + 0x10]
0046DC5A dec1                       faddp st(1)
0046DC5C dd5dec                     fstp qword ptr [ebp - 0x14]
0046DC5F dd45ec                     fld qword ptr [ebp - 0x14]
0046DC62 dc1d38034b00               fcomp qword ptr [0x4b0338]
0046DC68 dfe0                       fnstsw ax
0046DC6A f6c440                     test ah, 0x40
0046DC6D 751b                       jne 0x46dc8a
0046DC6F 8b857cffffff               mov eax, dword ptr [ebp - 0x84]
0046DC75 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DC7B dd450c                     fld qword ptr [ebp + 0xc]
0046DC7E dc6118                     fsub qword ptr [ecx + 0x18]
0046DC81 dc4810                     fmul qword ptr [eax + 0x10]
0046DC84 dc75ec                     fdiv qword ptr [ebp - 0x14]
0046DC87 dd5dec                     fstp qword ptr [ebp - 0x14]
0046DC8A 8b55f0                     mov edx, dword ptr [ebp - 0x10]
0046DC8D 52                         push edx
0046DC8E 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0046DC91 50                         push eax
0046DC92 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DC98 51                         push ecx
0046DC99 8d9564ffffff               lea edx, [ebp - 0x9c]
0046DC9F 52                         push edx
0046DCA0 8b4d80                     mov ecx, dword ptr [ebp - 0x80]
0046DCA3 e8883bf9ff                 call 0x401830
0046DCA8 50                         push eax
0046DCA9 8d854cffffff               lea eax, [ebp - 0xb4]
0046DCAF 50                         push eax
0046DCB0 e80b4bf9ff                 call 0x4027c0
0046DCB5 83c410                     add esp, 0x10
0046DCB8 50                         push eax
0046DCB9 8d8d34ffffff               lea ecx, [ebp - 0xcc]
0046DCBF 51                         push ecx
0046DCC0 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DCC6 e8b549f9ff                 call 0x402680
0046DCCB 8bf0                       mov esi, eax
0046DCCD b906000000                 mov ecx, 6
0046DCD2 8d7d84                     lea edi, [ebp - 0x7c]
0046DCD5 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046DCD7 8b55f0                     mov edx, dword ptr [ebp - 0x10]
0046DCDA 52                         push edx
0046DCDB 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0046DCDE 50                         push eax
0046DCDF 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DCE5 83c128                     add ecx, 0x28
0046DCE8 51                         push ecx
0046DCE9 8d9524ffffff               lea edx, [ebp - 0xdc]
0046DCEF 52                         push edx
0046DCF0 8b4d80                     mov ecx, dword ptr [ebp - 0x80]
0046DCF3 83c128                     add ecx, 0x28
0046DCF6 e8754afeff                 call 0x452770
0046DCFB 50                         push eax
0046DCFC 8d8514ffffff               lea eax, [ebp - 0xec]
0046DD02 50                         push eax
0046DD03 e8c84afeff                 call 0x4527d0
0046DD08 83c410                     add esp, 0x10
0046DD0B 50                         push eax
0046DD0C 8d8d04ffffff               lea ecx, [ebp - 0xfc]
0046DD12 51                         push ecx
0046DD13 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DD19 83c128                     add ecx, 0x28
0046DD1C e88ffbffff                 call 0x46d8b0
0046DD21 8b10                       mov edx, dword ptr [eax]
0046DD23 8955ac                     mov dword ptr [ebp - 0x54], edx
0046DD26 8b4804                     mov ecx, dword ptr [eax + 4]
0046DD29 894db0                     mov dword ptr [ebp - 0x50], ecx
0046DD2C 8b5008                     mov edx, dword ptr [eax + 8]
0046DD2F 8955b4                     mov dword ptr [ebp - 0x4c], edx
0046DD32 8b400c                     mov eax, dword ptr [eax + 0xc]
0046DD35 8945b8                     mov dword ptr [ebp - 0x48], eax
0046DD38 8b4df0                     mov ecx, dword ptr [ebp - 0x10]
0046DD3B 51                         push ecx
0046DD3C 8b55ec                     mov edx, dword ptr [ebp - 0x14]
0046DD3F 52                         push edx
0046DD40 8b857cffffff               mov eax, dword ptr [ebp - 0x84]
0046DD46 83c038                     add eax, 0x38
0046DD49 50                         push eax
0046DD4A 8d8decfeffff               lea ecx, [ebp - 0x114]
0046DD50 51                         push ecx
0046DD51 8b4d80                     mov ecx, dword ptr [ebp - 0x80]
0046DD54 83c138                     add ecx, 0x38
0046DD57 e8d43af9ff                 call 0x401830
0046DD5C 50                         push eax
0046DD5D 8d95d4feffff               lea edx, [ebp - 0x12c]
0046DD63 52                         push edx
0046DD64 e8574af9ff                 call 0x4027c0
0046DD69 83c410                     add esp, 0x10
0046DD6C 50                         push eax
0046DD6D 8d85bcfeffff               lea eax, [ebp - 0x144]
0046DD73 50                         push eax
0046DD74 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DD7A 83c138                     add ecx, 0x38
0046DD7D e8fe48f9ff                 call 0x402680
0046DD82 8bf0                       mov esi, eax
0046DD84 b906000000                 mov ecx, 6
0046DD89 8d7dbc                     lea edi, [ebp - 0x44]
0046DD8C f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046DD8E 8b8d7cffffff               mov ecx, dword ptr [ebp - 0x84]
0046DD94 8b5580                     mov edx, dword ptr [ebp - 0x80]
0046DD97 8b857cffffff               mov eax, dword ptr [ebp - 0x84]
0046DD9D dd4250                     fld qword ptr [edx + 0x50]
0046DDA0 dc6050                     fsub qword ptr [eax + 0x50]
0046DDA3 dc4dec                     fmul qword ptr [ebp - 0x14]
0046DDA6 dc4150                     fadd qword ptr [ecx + 0x50]
0046DDA9 dd5dd4                     fstp qword ptr [ebp - 0x2c]
0046DDAC b91a000000                 mov ecx, 0x1a
0046DDB1 8d7584                     lea esi, [ebp - 0x7c]
0046DDB4 8b7d08                     mov edi, dword ptr [ebp + 8]
0046DDB7 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
0046DDB9 8b8db8feffff               mov ecx, dword ptr [ebp - 0x148]
0046DDBF 83c901                     or ecx, 1
0046DDC2 898db8feffff               mov dword ptr [ebp - 0x148], ecx
0046DDC8 c645fc00                   mov byte ptr [ebp - 4], 0
0046DDCC 8d4d84                     lea ecx, [ebp - 0x7c]
0046DDCF e8cc2ffbff                 call 0x420da0
0046DDD4 8b4508                     mov eax, dword ptr [ebp + 8]
0046DDD7 8b4df4                     mov ecx, dword ptr [ebp - 0xc]
0046DDDA 64890d00000000             mov dword ptr fs:[0], ecx
0046DDE1 5f                         pop edi
0046DDE2 5e                         pop esi
0046DDE3 8be5                       mov esp, ebp
0046DDE5 5d                         pop ebp
0046DDE6 c3                         ret

; VA 0046DED0 file offset 6DED0 length 188 SHA256 3d9e007f2e279e2d4f30abcc83cf557d2eba347c58a6b6c2890562922eae4db6
0046DED0 55                         push ebp
0046DED1 8bec                       mov ebp, esp
0046DED3 83ec14                     sub esp, 0x14
0046DED6 56                         push esi
0046DED7 894dec                     mov dword ptr [ebp - 0x14], ecx
0046DEDA 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0046DEDD 8b4dec                     mov ecx, dword ptr [ebp - 0x14]
0046DEE0 8b10                       mov edx, dword ptr [eax]
0046DEE2 3b5104                     cmp edx, dword ptr [ecx + 4]
0046DEE5 7c7e                       jl 0x46df65
0046DEE7 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0046DEEA 8b4804                     mov ecx, dword ptr [eax + 4]
0046DEED 8b55ec                     mov edx, dword ptr [ebp - 0x14]
0046DEF0 034a08                     add ecx, dword ptr [edx + 8]
0046DEF3 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0046DEF6 894804                     mov dword ptr [eax + 4], ecx
0046DEF9 8b4dec                     mov ecx, dword ptr [ebp - 0x14]
0046DEFC 8b5104                     mov edx, dword ptr [ecx + 4]
0046DEFF c1e202                     shl edx, 2
0046DF02 52                         push edx
0046DF03 e8a0e20200                 call 0x49c1a8
0046DF08 83c404                     add esp, 4
0046DF0B 8945f4                     mov dword ptr [ebp - 0xc], eax
0046DF0E 8b45f4                     mov eax, dword ptr [ebp - 0xc]
0046DF11 8945f8                     mov dword ptr [ebp - 8], eax
0046DF14 c745fc00000000             mov dword ptr [ebp - 4], 0
0046DF1B eb09                       jmp 0x46df26
0046DF1D 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046DF20 83c101                     add ecx, 1
0046DF23 894dfc                     mov dword ptr [ebp - 4], ecx
0046DF26 8b55ec                     mov edx, dword ptr [ebp - 0x14]
0046DF29 8b45fc                     mov eax, dword ptr [ebp - 4]
0046DF2C 3b02                       cmp eax, dword ptr [edx]
0046DF2E 7d17                       jge 0x46df47
0046DF30 8b4dec                     mov ecx, dword ptr [ebp - 0x14]
0046DF33 8b510c                     mov edx, dword ptr [ecx + 0xc]
0046DF36 8b45fc                     mov eax, dword ptr [ebp - 4]
0046DF39 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046DF3C 8b75fc                     mov esi, dword ptr [ebp - 4]
0046DF3F 8b14b2                     mov edx, dword ptr [edx + esi*4]
0046DF42 891481                     mov dword ptr [ecx + eax*4], edx
0046DF45 ebd6                       jmp 0x46df1d
0046DF47 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0046DF4A 8b480c                     mov ecx, dword ptr [eax + 0xc]
0046DF4D 894df0                     mov dword ptr [ebp - 0x10], ecx
0046DF50 8b55f0                     mov edx, dword ptr [ebp - 0x10]
0046DF53 52                         push edx
0046DF54 e8e4e10200                 call 0x49c13d
0046DF59 83c404                     add esp, 4
0046DF5C 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0046DF5F 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046DF62 89480c                     mov dword ptr [eax + 0xc], ecx
0046DF65 8b55ec                     mov edx, dword ptr [ebp - 0x14]
0046DF68 8b02                       mov eax, dword ptr [edx]
0046DF6A 8b4dec                     mov ecx, dword ptr [ebp - 0x14]
0046DF6D 8b510c                     mov edx, dword ptr [ecx + 0xc]
0046DF70 8b4d08                     mov ecx, dword ptr [ebp + 8]
0046DF73 8b09                       mov ecx, dword ptr [ecx]
0046DF75 890c82                     mov dword ptr [edx + eax*4], ecx
0046DF78 8b55ec                     mov edx, dword ptr [ebp - 0x14]
0046DF7B 8b02                       mov eax, dword ptr [edx]
0046DF7D 83c001                     add eax, 1
0046DF80 8b4dec                     mov ecx, dword ptr [ebp - 0x14]
0046DF83 8901                       mov dword ptr [ecx], eax
0046DF85 5e                         pop esi
0046DF86 8be5                       mov esp, ebp
0046DF88 5d                         pop ebp
0046DF89 c20400                     ret 4

; VA 0046DF90 file offset 6DF90 length 101 SHA256 14e0d305d4ce2e3fa2cb7c7d53ca582d697cfb03a7fa6475d8fe335ebb32a806
0046DF90 55                         push ebp
0046DF91 8bec                       mov ebp, esp
0046DF93 83ec0c                     sub esp, 0xc
0046DF96 894df8                     mov dword ptr [ebp - 8], ecx
0046DF99 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046DF9C 83c110                     add ecx, 0x10
0046DF9F e8cc010000                 call 0x46e170
0046DFA4 25ff000000                 and eax, 0xff
0046DFA9 85c0                       test eax, eax
0046DFAB 7520                       jne 0x46dfcd
0046DFAD 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046DFB0 83c110                     add ecx, 0x10
0046DFB3 e898010000                 call 0x46e150
0046DFB8 8b00                       mov eax, dword ptr [eax]
0046DFBA 8945fc                     mov dword ptr [ebp - 4], eax
0046DFBD 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046DFC0 83c110                     add ecx, 0x10
0046DFC3 e8c8010000                 call 0x46e190
0046DFC8 8b45fc                     mov eax, dword ptr [ebp - 4]
0046DFCB eb24                       jmp 0x46dff1
0046DFCD 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046DFD0 8b5120                     mov edx, dword ptr [ecx + 0x20]
0046DFD3 8955f4                     mov dword ptr [ebp - 0xc], edx
0046DFD6 8b45f4                     mov eax, dword ptr [ebp - 0xc]
0046DFD9 50                         push eax
0046DFDA 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046DFDD 8b5120                     mov edx, dword ptr [ecx + 0x20]
0046DFE0 83c201                     add edx, 1
0046DFE3 8b45f8                     mov eax, dword ptr [ebp - 8]
0046DFE6 895020                     mov dword ptr [eax + 0x20], edx
0046DFE9 8b4df8                     mov ecx, dword ptr [ebp - 8]
0046DFEC e88f2bfbff                 call 0x420b80
0046DFF1 8be5                       mov esp, ebp
0046DFF3 5d                         pop ebp
0046DFF4 c3                         ret

; VA 00420D60 file offset 20D60 length 55 SHA256 ebc365e89b779b7dcb472eec894880fd1194961acea6a94f9d6734d02d8f37f5
00420D60 55                         push ebp
00420D61 8bec                       mov ebp, esp
00420D63 51                         push ecx
00420D64 894dfc                     mov dword ptr [ebp - 4], ecx
00420D67 8b4dfc                     mov ecx, dword ptr [ebp - 4]
00420D6A e8110efeff                 call 0x401b80
00420D6F 8b4dfc                     mov ecx, dword ptr [ebp - 4]
00420D72 83c118                     add ecx, 0x18
00420D75 e856140600                 call 0x4821d0
00420D7A 8b4dfc                     mov ecx, dword ptr [ebp - 4]
00420D7D 83c128                     add ecx, 0x28
00420D80 e84b140600                 call 0x4821d0
00420D85 8b4dfc                     mov ecx, dword ptr [ebp - 4]
00420D88 83c138                     add ecx, 0x38
00420D8B e8f00dfeff                 call 0x401b80
00420D90 8b45fc                     mov eax, dword ptr [ebp - 4]
00420D93 8be5                       mov esp, ebp
00420D95 5d                         pop ebp
00420D96 c3                         ret

; VA 00420DA0 file offset 20DA0 length 11 SHA256 012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5
00420DA0 55                         push ebp
00420DA1 8bec                       mov ebp, esp
00420DA3 51                         push ecx
00420DA4 894dfc                     mov dword ptr [ebp - 4], ecx
00420DA7 8be5                       mov esp, ebp
00420DA9 5d                         pop ebp
00420DAA c3                         ret

; VA 00401830 file offset 1830 length 94 SHA256 6def4c8304bc663043e2b7b8fa17dd87218c10f868015bb5694902da9d90c3c4
00401830 55                         push ebp
00401831 8bec                       mov ebp, esp
00401833 83ec1c                     sub esp, 0x1c
00401836 56                         push esi
00401837 57                         push edi
00401838 894de4                     mov dword ptr [ebp - 0x1c], ecx
0040183B 8b45e4                     mov eax, dword ptr [ebp - 0x1c]
0040183E 8b4d0c                     mov ecx, dword ptr [ebp + 0xc]
00401841 dd4010                     fld qword ptr [eax + 0x10]
00401844 dc6110                     fsub qword ptr [ecx + 0x10]
00401847 83ec08                     sub esp, 8
0040184A dd1c24                     fstp qword ptr [esp]
0040184D 8b55e4                     mov edx, dword ptr [ebp - 0x1c]
00401850 8b450c                     mov eax, dword ptr [ebp + 0xc]
00401853 dd4208                     fld qword ptr [edx + 8]
00401856 dc6008                     fsub qword ptr [eax + 8]
00401859 83ec08                     sub esp, 8
0040185C dd1c24                     fstp qword ptr [esp]
0040185F 8b4de4                     mov ecx, dword ptr [ebp - 0x1c]
00401862 8b550c                     mov edx, dword ptr [ebp + 0xc]
00401865 dd01                       fld qword ptr [ecx]
00401867 dc22                       fsub qword ptr [edx]
00401869 83ec08                     sub esp, 8
0040186C dd1c24                     fstp qword ptr [esp]
0040186F 8d4de8                     lea ecx, [ebp - 0x18]
00401872 e879ffffff                 call 0x4017f0
00401877 8bf0                       mov esi, eax
00401879 b906000000                 mov ecx, 6
0040187E 8b7d08                     mov edi, dword ptr [ebp + 8]
00401881 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
00401883 8b4508                     mov eax, dword ptr [ebp + 8]
00401886 5f                         pop edi
00401887 5e                         pop esi
00401888 8be5                       mov esp, ebp
0040188A 5d                         pop ebp
0040188B c20800                     ret 8

; VA 00402680 file offset 2680 length 94 SHA256 85020102726f8d637f9bdb677895e7452219e9d639e16baf8437261ed4d27493
00402680 55                         push ebp
00402681 8bec                       mov ebp, esp
00402683 83ec1c                     sub esp, 0x1c
00402686 56                         push esi
00402687 57                         push edi
00402688 894de4                     mov dword ptr [ebp - 0x1c], ecx
0040268B 8b45e4                     mov eax, dword ptr [ebp - 0x1c]
0040268E 8b4d0c                     mov ecx, dword ptr [ebp + 0xc]
00402691 dd4010                     fld qword ptr [eax + 0x10]
00402694 dc4110                     fadd qword ptr [ecx + 0x10]
00402697 83ec08                     sub esp, 8
0040269A dd1c24                     fstp qword ptr [esp]
0040269D 8b55e4                     mov edx, dword ptr [ebp - 0x1c]
004026A0 8b450c                     mov eax, dword ptr [ebp + 0xc]
004026A3 dd4208                     fld qword ptr [edx + 8]
004026A6 dc4008                     fadd qword ptr [eax + 8]
004026A9 83ec08                     sub esp, 8
004026AC dd1c24                     fstp qword ptr [esp]
004026AF 8b4de4                     mov ecx, dword ptr [ebp - 0x1c]
004026B2 8b550c                     mov edx, dword ptr [ebp + 0xc]
004026B5 dd01                       fld qword ptr [ecx]
004026B7 dc02                       fadd qword ptr [edx]
004026B9 83ec08                     sub esp, 8
004026BC dd1c24                     fstp qword ptr [esp]
004026BF 8d4de8                     lea ecx, [ebp - 0x18]
004026C2 e829f1ffff                 call 0x4017f0
004026C7 8bf0                       mov esi, eax
004026C9 b906000000                 mov ecx, 6
004026CE 8b7d08                     mov edi, dword ptr [ebp + 8]
004026D1 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
004026D3 8b4508                     mov eax, dword ptr [ebp + 8]
004026D6 5f                         pop edi
004026D7 5e                         pop esi
004026D8 8be5                       mov esp, ebp
004026DA 5d                         pop ebp
004026DB c20800                     ret 8

; VA 004027C0 file offset 27C0 length 81 SHA256 3a77e9ce86051a9f7c1ecb7e8a986b80d6a221afd04c2d65d4331e09247aae4a
004027C0 55                         push ebp
004027C1 8bec                       mov ebp, esp
004027C3 83ec18                     sub esp, 0x18
004027C6 56                         push esi
004027C7 57                         push edi
004027C8 8b450c                     mov eax, dword ptr [ebp + 0xc]
004027CB dd4510                     fld qword ptr [ebp + 0x10]
004027CE dc4810                     fmul qword ptr [eax + 0x10]
004027D1 83ec08                     sub esp, 8
004027D4 dd1c24                     fstp qword ptr [esp]
004027D7 8b4d0c                     mov ecx, dword ptr [ebp + 0xc]
004027DA dd4510                     fld qword ptr [ebp + 0x10]
004027DD dc4908                     fmul qword ptr [ecx + 8]
004027E0 83ec08                     sub esp, 8
004027E3 dd1c24                     fstp qword ptr [esp]
004027E6 8b550c                     mov edx, dword ptr [ebp + 0xc]
004027E9 dd4510                     fld qword ptr [ebp + 0x10]
004027EC dc0a                       fmul qword ptr [edx]
004027EE 83ec08                     sub esp, 8
004027F1 dd1c24                     fstp qword ptr [esp]
004027F4 8d4de8                     lea ecx, [ebp - 0x18]
004027F7 e8f4efffff                 call 0x4017f0
004027FC 8bf0                       mov esi, eax
004027FE b906000000                 mov ecx, 6
00402803 8b7d08                     mov edi, dword ptr [ebp + 8]
00402806 f3a5                       rep movsd dword ptr es:[edi], dword ptr [esi]
00402808 8b4508                     mov eax, dword ptr [ebp + 8]
0040280B 5f                         pop edi
0040280C 5e                         pop esi
0040280D 8be5                       mov esp, ebp
0040280F 5d                         pop ebp
00402810 c3                         ret

; VA 00452770 file offset 52770 length 85 SHA256 320b95f74dfdf773874bf933435e47c4958b6a1d5b7d469925072bd811040ab7
00452770 55                         push ebp
00452771 8bec                       mov ebp, esp
00452773 83ec14                     sub esp, 0x14
00452776 894dec                     mov dword ptr [ebp - 0x14], ecx
00452779 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0045277C 8b4d0c                     mov ecx, dword ptr [ebp + 0xc]
0045277F dd4008                     fld qword ptr [eax + 8]
00452782 dc6108                     fsub qword ptr [ecx + 8]
00452785 83ec08                     sub esp, 8
00452788 dd1c24                     fstp qword ptr [esp]
0045278B 8b55ec                     mov edx, dword ptr [ebp - 0x14]
0045278E 8b450c                     mov eax, dword ptr [ebp + 0xc]
00452791 dd02                       fld qword ptr [edx]
00452793 dc20                       fsub qword ptr [eax]
00452795 83ec08                     sub esp, 8
00452798 dd1c24                     fstp qword ptr [esp]
0045279B 8d4df0                     lea ecx, [ebp - 0x10]
0045279E e89d25fbff                 call 0x404d40
004527A3 8b4d08                     mov ecx, dword ptr [ebp + 8]
004527A6 8b10                       mov edx, dword ptr [eax]
004527A8 8911                       mov dword ptr [ecx], edx
004527AA 8b5004                     mov edx, dword ptr [eax + 4]
004527AD 895104                     mov dword ptr [ecx + 4], edx
004527B0 8b5008                     mov edx, dword ptr [eax + 8]
004527B3 895108                     mov dword ptr [ecx + 8], edx
004527B6 8b400c                     mov eax, dword ptr [eax + 0xc]
004527B9 89410c                     mov dword ptr [ecx + 0xc], eax
004527BC 8b4508                     mov eax, dword ptr [ebp + 8]
004527BF 8be5                       mov esp, ebp
004527C1 5d                         pop ebp
004527C2 c20800                     ret 8

; VA 004527D0 file offset 527D0 length 84 SHA256 99f8aa072cf71af83f9fc1af69067f331d39f52fd53e84515d95a8acc9eecf03
004527D0 55                         push ebp
004527D1 8bec                       mov ebp, esp
004527D3 83ec10                     sub esp, 0x10
004527D6 6a01                       push 1
004527D8 8b4d0c                     mov ecx, dword ptr [ebp + 0xc]
004527DB e850000000                 call 0x452830
004527E0 dc4d10                     fmul qword ptr [ebp + 0x10]
004527E3 83ec08                     sub esp, 8
004527E6 dd1c24                     fstp qword ptr [esp]
004527E9 6a00                       push 0
004527EB 8b4d0c                     mov ecx, dword ptr [ebp + 0xc]
004527EE e83d000000                 call 0x452830
004527F3 dc4d10                     fmul qword ptr [ebp + 0x10]
004527F6 83ec08                     sub esp, 8
004527F9 dd1c24                     fstp qword ptr [esp]
004527FC 8d4df0                     lea ecx, [ebp - 0x10]
004527FF e83c25fbff                 call 0x404d40
00452804 8b4d08                     mov ecx, dword ptr [ebp + 8]
00452807 8b10                       mov edx, dword ptr [eax]
00452809 8911                       mov dword ptr [ecx], edx
0045280B 8b5004                     mov edx, dword ptr [eax + 4]
0045280E 895104                     mov dword ptr [ecx + 4], edx
00452811 8b5008                     mov edx, dword ptr [eax + 8]
00452814 895108                     mov dword ptr [ecx + 8], edx
00452817 8b400c                     mov eax, dword ptr [eax + 0xc]
0045281A 89410c                     mov dword ptr [ecx + 0xc], eax
0045281D 8b4508                     mov eax, dword ptr [ebp + 8]
00452820 8be5                       mov esp, ebp
00452822 5d                         pop ebp
00452823 c3                         ret

; VA 0046D8B0 file offset 6D8B0 length 85 SHA256 c135e2ae09ca2257289ece9a2df35a5f0d241624424905291aa3c4b6391e21b8
0046D8B0 55                         push ebp
0046D8B1 8bec                       mov ebp, esp
0046D8B3 83ec14                     sub esp, 0x14
0046D8B6 894dec                     mov dword ptr [ebp - 0x14], ecx
0046D8B9 8b45ec                     mov eax, dword ptr [ebp - 0x14]
0046D8BC 8b4d0c                     mov ecx, dword ptr [ebp + 0xc]
0046D8BF dd4008                     fld qword ptr [eax + 8]
0046D8C2 dc4108                     fadd qword ptr [ecx + 8]
0046D8C5 83ec08                     sub esp, 8
0046D8C8 dd1c24                     fstp qword ptr [esp]
0046D8CB 8b55ec                     mov edx, dword ptr [ebp - 0x14]
0046D8CE 8b450c                     mov eax, dword ptr [ebp + 0xc]
0046D8D1 dd02                       fld qword ptr [edx]
0046D8D3 dc00                       fadd qword ptr [eax]
0046D8D5 83ec08                     sub esp, 8
0046D8D8 dd1c24                     fstp qword ptr [esp]
0046D8DB 8d4df0                     lea ecx, [ebp - 0x10]
0046D8DE e85d74f9ff                 call 0x404d40
0046D8E3 8b4d08                     mov ecx, dword ptr [ebp + 8]
0046D8E6 8b10                       mov edx, dword ptr [eax]
0046D8E8 8911                       mov dword ptr [ecx], edx
0046D8EA 8b5004                     mov edx, dword ptr [eax + 4]
0046D8ED 895104                     mov dword ptr [ecx + 4], edx
0046D8F0 8b5008                     mov edx, dword ptr [eax + 8]
0046D8F3 895108                     mov dword ptr [ecx + 8], edx
0046D8F6 8b400c                     mov eax, dword ptr [eax + 0xc]
0046D8F9 89410c                     mov dword ptr [ecx + 0xc], eax
0046D8FC 8b4508                     mov eax, dword ptr [ebp + 8]
0046D8FF 8be5                       mov esp, ebp
0046D901 5d                         pop ebp
0046D902 c20800                     ret 8

; VA 00420B80 file offset 20B80 length 27 SHA256 576b55cba35dd615c181ac03d3e090472e137d2157682f55ce5e6a195138e16b
00420B80 55                         push ebp
00420B81 8bec                       mov ebp, esp
00420B83 51                         push ecx
00420B84 894dfc                     mov dword ptr [ebp - 4], ecx
00420B87 8b4508                     mov eax, dword ptr [ebp + 8]
00420B8A 6bc068                     imul eax, eax, 0x68
00420B8D 8b4dfc                     mov ecx, dword ptr [ebp - 4]
00420B90 8b510c                     mov edx, dword ptr [ecx + 0xc]
00420B93 03c2                       add eax, edx
00420B95 8be5                       mov esp, ebp
00420B97 5d                         pop ebp
00420B98 c20400                     ret 4

; VA 0046E150 file offset 6E150 length 31 SHA256 8fe7e8a96ea10c3c27c0d1abea24d79334f6c08f29992a70283253edb787a02c
0046E150 55                         push ebp
0046E151 8bec                       mov ebp, esp
0046E153 51                         push ecx
0046E154 894dfc                     mov dword ptr [ebp - 4], ecx
0046E157 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046E15A e861fdffff                 call 0x46dec0
0046E15F 83e801                     sub eax, 1
0046E162 50                         push eax
0046E163 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046E166 e845000000                 call 0x46e1b0
0046E16B 8be5                       mov esp, ebp
0046E16D 5d                         pop ebp
0046E16E c3                         ret

; VA 0046E170 file offset 6E170 length 24 SHA256 9cdf1527a43f47327016700858a307268675914b232e308bda6290e089cebf10
0046E170 55                         push ebp
0046E171 8bec                       mov ebp, esp
0046E173 51                         push ecx
0046E174 894dfc                     mov dword ptr [ebp - 4], ecx
0046E177 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046E17A e871000000                 call 0x46e1f0
0046E17F f7d8                       neg eax
0046E181 1bc0                       sbb eax, eax
0046E183 40                         inc eax
0046E184 8be5                       mov esp, ebp
0046E186 5d                         pop ebp
0046E187 c3                         ret

; VA 0046E190 file offset 6E190 length 31 SHA256 4b9e64eda0de9592a12c8ba584c943d1fbc774ea2f90b46c95b5152ee1f0058a
0046E190 55                         push ebp
0046E191 8bec                       mov ebp, esp
0046E193 51                         push ecx
0046E194 894dfc                     mov dword ptr [ebp - 4], ecx
0046E197 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046E19A e851000000                 call 0x46e1f0
0046E19F 83e801                     sub eax, 1
0046E1A2 50                         push eax
0046E1A3 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046E1A6 e825000000                 call 0x46e1d0
0046E1AB 8be5                       mov esp, ebp
0046E1AD 5d                         pop ebp
0046E1AE c3                         ret

; VA 00401B80 file offset 1B80 length 14 SHA256 befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c
00401B80 55                         push ebp
00401B81 8bec                       mov ebp, esp
00401B83 51                         push ecx
00401B84 894dfc                     mov dword ptr [ebp - 4], ecx
00401B87 8b45fc                     mov eax, dword ptr [ebp - 4]
00401B8A 8be5                       mov esp, ebp
00401B8C 5d                         pop ebp
00401B8D c3                         ret

; VA 004821D0 file offset 821D0 length 14 SHA256 befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c
004821D0 55                         push ebp
004821D1 8bec                       mov ebp, esp
004821D3 51                         push ecx
004821D4 894dfc                     mov dword ptr [ebp - 4], ecx
004821D7 8b45fc                     mov eax, dword ptr [ebp - 4]
004821DA 8be5                       mov esp, ebp
004821DC 5d                         pop ebp
004821DD c3                         ret

; VA 004017F0 file offset 17F0 length 60 SHA256 9a8c820a68a43ee2211d82f4ef1737664908d5bafae0ecc4e37856b948998034
004017F0 55                         push ebp
004017F1 8bec                       mov ebp, esp
004017F3 51                         push ecx
004017F4 894dfc                     mov dword ptr [ebp - 4], ecx
004017F7 8b45fc                     mov eax, dword ptr [ebp - 4]
004017FA 8b4d08                     mov ecx, dword ptr [ebp + 8]
004017FD 8908                       mov dword ptr [eax], ecx
004017FF 8b550c                     mov edx, dword ptr [ebp + 0xc]
00401802 895004                     mov dword ptr [eax + 4], edx
00401805 8b45fc                     mov eax, dword ptr [ebp - 4]
00401808 8b4d10                     mov ecx, dword ptr [ebp + 0x10]
0040180B 894808                     mov dword ptr [eax + 8], ecx
0040180E 8b5514                     mov edx, dword ptr [ebp + 0x14]
00401811 89500c                     mov dword ptr [eax + 0xc], edx
00401814 8b45fc                     mov eax, dword ptr [ebp - 4]
00401817 8b4d18                     mov ecx, dword ptr [ebp + 0x18]
0040181A 894810                     mov dword ptr [eax + 0x10], ecx
0040181D 8b551c                     mov edx, dword ptr [ebp + 0x1c]
00401820 895014                     mov dword ptr [eax + 0x14], edx
00401823 8b45fc                     mov eax, dword ptr [ebp - 4]
00401826 8be5                       mov esp, ebp
00401828 5d                         pop ebp
00401829 c21800                     ret 0x18

; VA 00404D40 file offset 4D40 length 45 SHA256 3ebc9d00a9d2a61f993f1a29129630d9b6b9f86c193fd84945747265e7021535
00404D40 55                         push ebp
00404D41 8bec                       mov ebp, esp
00404D43 51                         push ecx
00404D44 894dfc                     mov dword ptr [ebp - 4], ecx
00404D47 8b45fc                     mov eax, dword ptr [ebp - 4]
00404D4A 8b4d08                     mov ecx, dword ptr [ebp + 8]
00404D4D 8908                       mov dword ptr [eax], ecx
00404D4F 8b550c                     mov edx, dword ptr [ebp + 0xc]
00404D52 895004                     mov dword ptr [eax + 4], edx
00404D55 8b45fc                     mov eax, dword ptr [ebp - 4]
00404D58 8b4d10                     mov ecx, dword ptr [ebp + 0x10]
00404D5B 894808                     mov dword ptr [eax + 8], ecx
00404D5E 8b5514                     mov edx, dword ptr [ebp + 0x14]
00404D61 89500c                     mov dword ptr [eax + 0xc], edx
00404D64 8b45fc                     mov eax, dword ptr [ebp - 4]
00404D67 8be5                       mov esp, ebp
00404D69 5d                         pop ebp
00404D6A c21000                     ret 0x10

; VA 00452830 file offset 52830 length 22 SHA256 55caa3b3318c713a8ec0a0f9bb73a8e210229282b5e904814321c3235a817c54
00452830 55                         push ebp
00452831 8bec                       mov ebp, esp
00452833 51                         push ecx
00452834 894dfc                     mov dword ptr [ebp - 4], ecx
00452837 8b4508                     mov eax, dword ptr [ebp + 8]
0045283A 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0045283D dd04c1                     fld qword ptr [ecx + eax*8]
00452840 8be5                       mov esp, ebp
00452842 5d                         pop ebp
00452843 c20400                     ret 4

; VA 0046E1B0 file offset 6E1B0 length 25 SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be
0046E1B0 55                         push ebp
0046E1B1 8bec                       mov ebp, esp
0046E1B3 51                         push ecx
0046E1B4 894dfc                     mov dword ptr [ebp - 4], ecx
0046E1B7 8b45fc                     mov eax, dword ptr [ebp - 4]
0046E1BA 8b480c                     mov ecx, dword ptr [eax + 0xc]
0046E1BD 8b5508                     mov edx, dword ptr [ebp + 8]
0046E1C0 8d0491                     lea eax, [ecx + edx*4]
0046E1C3 8be5                       mov esp, ebp
0046E1C5 5d                         pop ebp
0046E1C6 c20400                     ret 4

; VA 0046E1D0 file offset 6E1D0 length 25 SHA256 622081feaced521a63d97414098085d07d16a38f470a22315bbd1beb9ae3ce2a
0046E1D0 55                         push ebp
0046E1D1 8bec                       mov ebp, esp
0046E1D3 51                         push ecx
0046E1D4 894dfc                     mov dword ptr [ebp - 4], ecx
0046E1D7 8b4508                     mov eax, dword ptr [ebp + 8]
0046E1DA 50                         push eax
0046E1DB 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046E1DE e87d68faff                 call 0x414a60
0046E1E3 8be5                       mov esp, ebp
0046E1E5 5d                         pop ebp
0046E1E6 c20400                     ret 4

; VA 0046E1F0 file offset 6E1F0 length 19 SHA256 189f02ee356dfbeadece015fedb0b63d6b90363d531395e5c80383750c33aa98
0046E1F0 55                         push ebp
0046E1F1 8bec                       mov ebp, esp
0046E1F3 51                         push ecx
0046E1F4 894dfc                     mov dword ptr [ebp - 4], ecx
0046E1F7 8b4dfc                     mov ecx, dword ptr [ebp - 4]
0046E1FA e8c1fcffff                 call 0x46dec0
0046E1FF 8be5                       mov esp, ebp
0046E201 5d                         pop ebp
0046E202 c3                         ret

; VA 0046DEC0 file offset 6DEC0 length 16 SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936
0046DEC0 55                         push ebp
0046DEC1 8bec                       mov ebp, esp
0046DEC3 51                         push ecx
0046DEC4 894dfc                     mov dword ptr [ebp - 4], ecx
0046DEC7 8b45fc                     mov eax, dword ptr [ebp - 4]
0046DECA 8b00                       mov eax, dword ptr [eax]
0046DECC 8be5                       mov esp, ebp
0046DECE 5d                         pop ebp
0046DECF c3                         ret

; VA 00414A40 file offset 14A40 length 25 SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be
00414A40 55                         push ebp
00414A41 8bec                       mov ebp, esp
00414A43 51                         push ecx
00414A44 894dfc                     mov dword ptr [ebp - 4], ecx
00414A47 8b45fc                     mov eax, dword ptr [ebp - 4]
00414A4A 8b480c                     mov ecx, dword ptr [eax + 0xc]
00414A4D 8b5508                     mov edx, dword ptr [ebp + 8]
00414A50 8d0491                     lea eax, [ecx + edx*4]
00414A53 8be5                       mov esp, ebp
00414A55 5d                         pop ebp
00414A56 c20400                     ret 4

; VA 00414A60 file offset 14A60 length 257 SHA256 070dad4c3f50908563eb41018a37e766d7bb2b7f46377759535e47332d86edfa
00414A60 55                         push ebp
00414A61 8bec                       mov ebp, esp
00414A63 83ec18                     sub esp, 0x18
00414A66 56                         push esi
00414A67 894de8                     mov dword ptr [ebp - 0x18], ecx
00414A6A 8b45e8                     mov eax, dword ptr [ebp - 0x18]
00414A6D 8b4d08                     mov ecx, dword ptr [ebp + 8]
00414A70 3b4804                     cmp ecx, dword ptr [eax + 4]
00414A73 0f8ed9000000               jle 0x414b52
00414A79 8b55e8                     mov edx, dword ptr [ebp - 0x18]
00414A7C 837a0801                   cmp dword ptr [edx + 8], 1
00414A80 750b                       jne 0x414a8d
00414A82 8b45e8                     mov eax, dword ptr [ebp - 0x18]
00414A85 8b4d08                     mov ecx, dword ptr [ebp + 8]
00414A88 894804                     mov dword ptr [eax + 4], ecx
00414A8B eb1d                       jmp 0x414aaa
00414A8D 8b55e8                     mov edx, dword ptr [ebp - 0x18]
00414A90 8b4204                     mov eax, dword ptr [edx + 4]
00414A93 8b4de8                     mov ecx, dword ptr [ebp - 0x18]
00414A96 034108                     add eax, dword ptr [ecx + 8]
00414A99 8b55e8                     mov edx, dword ptr [ebp - 0x18]
00414A9C 894204                     mov dword ptr [edx + 4], eax
00414A9F 8b45e8                     mov eax, dword ptr [ebp - 0x18]
00414AA2 8b4804                     mov ecx, dword ptr [eax + 4]
00414AA5 3b4d08                     cmp ecx, dword ptr [ebp + 8]
00414AA8 7ee3                       jle 0x414a8d
00414AAA 8b55e8                     mov edx, dword ptr [ebp - 0x18]
00414AAD 837a0400                   cmp dword ptr [edx + 4], 0
00414AB1 7e77                       jle 0x414b2a
00414AB3 8b45e8                     mov eax, dword ptr [ebp - 0x18]
00414AB6 8b4804                     mov ecx, dword ptr [eax + 4]
00414AB9 c1e102                     shl ecx, 2
00414ABC 51                         push ecx
00414ABD e8e6760800                 call 0x49c1a8
00414AC2 83c404                     add esp, 4
00414AC5 8945f4                     mov dword ptr [ebp - 0xc], eax
00414AC8 8b55f4                     mov edx, dword ptr [ebp - 0xc]
00414ACB 8955fc                     mov dword ptr [ebp - 4], edx
00414ACE 8b45e8                     mov eax, dword ptr [ebp - 0x18]
00414AD1 83780c00                   cmp dword ptr [eax + 0xc], 0
00414AD5 7448                       je 0x414b1f
00414AD7 c745f800000000             mov dword ptr [ebp - 8], 0
00414ADE eb09                       jmp 0x414ae9
00414AE0 8b4df8                     mov ecx, dword ptr [ebp - 8]
00414AE3 83c101                     add ecx, 1
00414AE6 894df8                     mov dword ptr [ebp - 8], ecx
00414AE9 8b55e8                     mov edx, dword ptr [ebp - 0x18]
00414AEC 8b45f8                     mov eax, dword ptr [ebp - 8]
00414AEF 3b02                       cmp eax, dword ptr [edx]
00414AF1 7d17                       jge 0x414b0a
00414AF3 8b4de8                     mov ecx, dword ptr [ebp - 0x18]
00414AF6 8b510c                     mov edx, dword ptr [ecx + 0xc]
00414AF9 8b45f8                     mov eax, dword ptr [ebp - 8]
00414AFC 8b4dfc                     mov ecx, dword ptr [ebp - 4]
00414AFF 8b75f8                     mov esi, dword ptr [ebp - 8]
00414B02 8b14b2                     mov edx, dword ptr [edx + esi*4]
00414B05 891481                     mov dword ptr [ecx + eax*4], edx
00414B08 ebd6                       jmp 0x414ae0
00414B0A 8b45e8                     mov eax, dword ptr [ebp - 0x18]
00414B0D 8b480c                     mov ecx, dword ptr [eax + 0xc]
00414B10 894df0                     mov dword ptr [ebp - 0x10], ecx
00414B13 8b55f0                     mov edx, dword ptr [ebp - 0x10]
00414B16 52                         push edx
00414B17 e821760800                 call 0x49c13d
00414B1C 83c404                     add esp, 4
00414B1F 8b45e8                     mov eax, dword ptr [ebp - 0x18]
00414B22 8b4dfc                     mov ecx, dword ptr [ebp - 4]
00414B25 89480c                     mov dword ptr [eax + 0xc], ecx
00414B28 eb28                       jmp 0x414b52
00414B2A 8b55e8                     mov edx, dword ptr [ebp - 0x18]
00414B2D 837a0c00                   cmp dword ptr [edx + 0xc], 0
00414B31 7415                       je 0x414b48
00414B33 8b45e8                     mov eax, dword ptr [ebp - 0x18]
00414B36 8b480c                     mov ecx, dword ptr [eax + 0xc]
00414B39 894dec                     mov dword ptr [ebp - 0x14], ecx
00414B3C 8b55ec                     mov edx, dword ptr [ebp - 0x14]
00414B3F 52                         push edx
00414B40 e8f8750800                 call 0x49c13d
00414B45 83c404                     add esp, 4
00414B48 8b45e8                     mov eax, dword ptr [ebp - 0x18]
00414B4B c7400c00000000             mov dword ptr [eax + 0xc], 0
00414B52 8b4de8                     mov ecx, dword ptr [ebp - 0x18]
00414B55 8b5508                     mov edx, dword ptr [ebp + 8]
00414B58 8911                       mov dword ptr [ecx], edx
00414B5A 5e                         pop esi
00414B5B 8be5                       mov esp, ebp
00414B5D 5d                         pop ebp
00414B5E c20400                     ret 4

; VA 00414EF0 file offset 14EF0 length 20 SHA256 a068869c343013a89039a5c55be7cdb6c83ff8952b8a0522ce59771b0170b434
00414EF0 55                         push ebp
00414EF1 8bec                       mov ebp, esp
00414EF3 51                         push ecx
00414EF4 894dfc                     mov dword ptr [ebp - 4], ecx
00414EF7 8b45fc                     mov eax, dword ptr [ebp - 4]
00414EFA c70000000000               mov dword ptr [eax], 0
00414F00 8be5                       mov esp, ebp
00414F02 5d                         pop ebp
00414F03 c3                         ret
