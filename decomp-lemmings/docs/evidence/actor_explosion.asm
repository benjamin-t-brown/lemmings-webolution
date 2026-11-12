# Reference SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f

# ev_62510352342841a062909b05180622c7137a87f3910246f04eb92a8f38831265; offset 33200; length 48
004081B0 55                             push ebp
004081B1 8bec                           mov ebp, esp
004081B3 51                             push ecx
004081B4 894dfc                         mov dword ptr [ebp - 4], ecx
004081B7 8b4dfc                         mov ecx, dword ptr [ebp - 4]
004081BA e8d1010100                     call 0x418390
004081BF 8b45fc                         mov eax, dword ptr [ebp - 4]
004081C2 c70050054b00                   mov dword ptr [eax], 0x4b0550
004081C8 8b4dfc                         mov ecx, dword ptr [ebp - 4]
004081CB c7411402000000                 mov dword ptr [ecx + 0x14], 2
004081D2 8b55fc                         mov edx, dword ptr [ebp - 4]
004081D5 c6421100                       mov byte ptr [edx + 0x11], 0
004081D9 8b45fc                         mov eax, dword ptr [ebp - 4]
004081DC 8be5                           mov esp, ebp
004081DE 5d                             pop ebp
004081DF c3                             ret

# ev_1eb60836ec9ac047eed8a1456a53d8b0bf246a664eff5c1887227abaa0cbee74; offset 33276; length 43
004081FC 55                             push ebp
004081FD 8bec                           mov ebp, esp
004081FF 51                             push ecx
00408200 894dfc                         mov dword ptr [ebp - 4], ecx
00408203 6a00                           push 0
00408205 a1c0324e00                     mov eax, dword ptr [0x4e32c0]
0040820A 50                             push eax
0040820B 8b4d08                         mov ecx, dword ptr [ebp + 8]
0040820E e8ede7ffff                     call 0x406a00
00408213 0fbfc8                         movsx ecx, ax
00408216 51                             push ecx
00408217 b9086a4c00                     mov ecx, 0x4c6a08
0040821C e83ad10300                     call 0x44535b
00408221 8be5                           mov esp, ebp
00408223 5d                             pop ebp
00408224 c20400                         ret 4

# ev_363f91922ae50e4763946b6bbd486e8e98c9412a082f9aaab97c17f188de3516; offset 33319; length 1193
00408227 55                             push ebp
00408228 8bec                           mov ebp, esp
0040822A 6aff                           push -1
0040822C 684a964a00                     push 0x4a964a
00408231 64a100000000                   mov eax, dword ptr fs:[0]
00408237 50                             push eax
00408238 64892500000000                 mov dword ptr fs:[0], esp
0040823F 81ec40010000                   sub esp, 0x140
00408245 56                             push esi
00408246 57                             push edi
00408247 898dc4feffff                   mov dword ptr [ebp - 0x13c], ecx
0040824D 8d45dc                         lea eax, [ebp - 0x24]
00408250 50                             push eax
00408251 8b4d08                         mov ecx, dword ptr [ebp + 8]
00408254 e82a6f0200                     call 0x42f183
00408259 c745fc00000000                 mov dword ptr [ebp - 4], 0
00408260 6800000840                     push 0x40080000
00408265 6a00                           push 0
00408267 8d4ddc                         lea ecx, [ebp - 0x24]
0040826A 51                             push ecx
0040826B e8165b0100                     call 0x41dd86
00408270 83c40c                         add esp, 0xc
00408273 8b7508                         mov esi, dword ptr [ebp + 8]
00408276 83c608                         add esi, 8
00408279 b906000000                     mov ecx, 6
0040827E 8d7dc4                         lea edi, [ebp - 0x3c]
00408281 f3a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
00408283 c645fc01                       mov byte ptr [ebp - 4], 1
00408287 db05bc104b00                   fild dword ptr [0x4b10bc]
0040828D dc35f8034b00                   fdiv qword ptr [0x4b03f8]
00408293 dc45c4                         fadd qword ptr [ebp - 0x3c]
00408296 dd5dc4                         fstp qword ptr [ebp - 0x3c]
00408299 db05b8104b00                   fild dword ptr [0x4b10b8]
0040829F dc35f8034b00                   fdiv qword ptr [0x4b03f8]
004082A5 dc45cc                         fadd qword ptr [ebp - 0x34]
004082A8 dd5dcc                         fstp qword ptr [ebp - 0x34]
004082AB 8d55c4                         lea edx, [ebp - 0x3c]
004082AE 52                             push edx
004082AF 6a00                           push 0
004082B1 b9106a4c00                     mov ecx, 0x4c6a10
004082B6 e863300300                     call 0x43b31e
004082BB dd45c4                         fld qword ptr [ebp - 0x3c]
004082BE dc25d0024b00                   fsub qword ptr [0x4b02d0]
004082C4 dd5dc4                         fstp qword ptr [ebp - 0x3c]
004082C7 8b450c                         mov eax, dword ptr [ebp + 0xc]
004082CA 8b4804                         mov ecx, dword ptr [eax + 4]
004082CD 51                             push ecx
004082CE 8b10                           mov edx, dword ptr [eax]
004082D0 52                             push edx
004082D1 6a00                           push 0
004082D3 8d45c4                         lea eax, [ebp - 0x3c]
004082D6 50                             push eax
004082D7 e834720300                     call 0x43f510
004082DC 83c410                         add esp, 0x10
004082DF 8b4d08                         mov ecx, dword ptr [ebp + 8]
004082E2 83794c03                       cmp dword ptr [ecx + 0x4c], 3
004082E6 0f84cb010000                   je 0x4084b7
004082EC 6828010000                     push 0x128
004082F1 e8b23e0900                     call 0x49c1a8
004082F6 83c404                         add esp, 4
004082F9 89857cffffff                   mov dword ptr [ebp - 0x84], eax
004082FF c645fc02                       mov byte ptr [ebp - 4], 2
00408303 83bd7cffffff00                 cmp dword ptr [ebp - 0x84], 0
0040830A 7413                           je 0x40831f
0040830C 8b8d7cffffff                   mov ecx, dword ptr [ebp - 0x84]
00408312 e8c995ffff                     call 0x4018e0
00408317 8985c0feffff                   mov dword ptr [ebp - 0x140], eax
0040831D eb0a                           jmp 0x408329
0040831F c785c0feffff00000000           mov dword ptr [ebp - 0x140], 0
00408329 8b95c0feffff                   mov edx, dword ptr [ebp - 0x140]
0040832F 895580                         mov dword ptr [ebp - 0x80], edx
00408332 c645fc01                       mov byte ptr [ebp - 4], 1
00408336 8b4580                         mov eax, dword ptr [ebp - 0x80]
00408339 8945bc                         mov dword ptr [ebp - 0x44], eax
0040833C 6a06                           push 6
0040833E 8b4dbc                         mov ecx, dword ptr [ebp - 0x44]
00408341 e86a98ffff                     call 0x401bb0
00408346 8d4dc4                         lea ecx, [ebp - 0x3c]
00408349 51                             push ecx
0040834A 8b55bc                         mov edx, dword ptr [ebp - 0x44]
0040834D 8b02                           mov eax, dword ptr [edx]
0040834F 8b4dbc                         mov ecx, dword ptr [ebp - 0x44]
00408352 ff5008                         call dword ptr [eax + 8]
00408355 6a00                           push 0
00408357 6a00                           push 0
00408359 6800001040                     push 0x40100000
0040835E 6a00                           push 0
00408360 8b4dbc                         mov ecx, dword ptr [ebp - 0x44]
00408363 e8889bffff                     call 0x401ef0
00408368 6a06                           push 6
0040836A 8b4dbc                         mov ecx, dword ptr [ebp - 0x44]
0040836D e8de9bffff                     call 0x401f50
00408372 8b4d08                         mov ecx, dword ptr [ebp + 8]
00408375 83c108                         add ecx, 8
00408378 51                             push ecx
00408379 8d954cffffff                   lea edx, [ebp - 0xb4]
0040837F 52                             push edx
00408380 b9106a4c00                     mov ecx, 0x4c6a10
00408385 e8e2510300                     call 0x43d56c
0040838A 50                             push eax
0040838B 8d8534ffffff                   lea eax, [ebp - 0xcc]
00408391 50                             push eax
00408392 8d4dc4                         lea ecx, [ebp - 0x3c]
00408395 51                             push ecx
00408396 8d9564ffffff                   lea edx, [ebp - 0x9c]
0040839C 52                             push edx
0040839D b9106a4c00                     mov ecx, 0x4c6a10
004083A2 e8c5510300                     call 0x43d56c
004083A7 8bc8                           mov ecx, eax
004083A9 e88294ffff                     call 0x401830
004083AE 50                             push eax
004083AF 8b45bc                         mov eax, dword ptr [ebp - 0x44]
004083B2 8b10                           mov edx, dword ptr [eax]
004083B4 8b4dbc                         mov ecx, dword ptr [ebp - 0x44]
004083B7 ff5254                         call dword ptr [edx + 0x54]
004083BA 8b450c                         mov eax, dword ptr [ebp + 0xc]
004083BD 8b4804                         mov ecx, dword ptr [eax + 4]
004083C0 51                             push ecx
004083C1 8b10                           mov edx, dword ptr [eax]
004083C3 52                             push edx
004083C4 8b45bc                         mov eax, dword ptr [ebp - 0x44]
004083C7 8b10                           mov edx, dword ptr [eax]
004083C9 8b4dbc                         mov ecx, dword ptr [ebp - 0x44]
004083CC ff522c                         call dword ptr [edx + 0x2c]
004083CF dd45cc                         fld qword ptr [ebp - 0x34]
004083D2 e8d5410900                     call 0x49c5ac
004083D7 50                             push eax
004083D8 dd45c4                         fld qword ptr [ebp - 0x3c]
004083DB e8cc410900                     call 0x49c5ac
004083E0 50                             push eax
004083E1 b9106a4c00                     mov ecx, 0x4c6a10
004083E6 e855bbffff                     call 0x403f40
004083EB 8bf0                           mov esi, eax
004083ED b906000000                     mov ecx, 6
004083F2 8d7da4                         lea edi, [ebp - 0x5c]
004083F5 f3a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
004083F7 c745c000000000                 mov dword ptr [ebp - 0x40], 0
004083FE eb09                           jmp 0x408409
00408400 8b45c0                         mov eax, dword ptr [ebp - 0x40]
00408403 83c001                         add eax, 1
00408406 8945c0                         mov dword ptr [ebp - 0x40], eax
00408409 8b4dbc                         mov ecx, dword ptr [ebp - 0x44]
0040840C 81c1b0000000                   add ecx, 0xb0
00408412 e8b994ffff                     call 0x4018d0
00408417 3945c0                         cmp dword ptr [ebp - 0x40], eax
0040841A 7d25                           jge 0x408441
0040841C 8d4da4                         lea ecx, [ebp - 0x5c]
0040841F 51                             push ecx
00408420 8b55c0                         mov edx, dword ptr [ebp - 0x40]
00408423 52                             push edx
00408424 8b4dbc                         mov ecx, dword ptr [ebp - 0x44]
00408427 81c1b0000000                   add ecx, 0xb0
0040842D e87e94ffff                     call 0x4018b0
00408432 8bc8                           mov ecx, eax
00408434 81c1b0000000                   add ecx, 0xb0
0040843A e8e1b3ffff                     call 0x403820
0040843F ebbf                           jmp 0x408400
00408441 6a48                           push 0x48
00408443 e8603d0900                     call 0x49c1a8
00408448 83c404                         add esp, 4
0040844B 898528ffffff                   mov dword ptr [ebp - 0xd8], eax
00408451 c645fc03                       mov byte ptr [ebp - 4], 3
00408455 83bd28ffffff00                 cmp dword ptr [ebp - 0xd8], 0
0040845C 741d                           je 0x40847b
0040845E 6a01                           push 1
00408460 8b45bc                         mov eax, dword ptr [ebp - 0x44]
00408463 50                             push eax
00408464 8d4dc4                         lea ecx, [ebp - 0x3c]
00408467 51                             push ecx
00408468 8b8d28ffffff                   mov ecx, dword ptr [ebp - 0xd8]
0040846E e80d9c0200                     call 0x432080
00408473 8985bcfeffff                   mov dword ptr [ebp - 0x144], eax
00408479 eb0a                           jmp 0x408485
0040847B c785bcfeffff00000000           mov dword ptr [ebp - 0x144], 0
00408485 8b95bcfeffff                   mov edx, dword ptr [ebp - 0x144]
0040848B 89952cffffff                   mov dword ptr [ebp - 0xd4], edx
00408491 c645fc01                       mov byte ptr [ebp - 4], 1
00408495 8b852cffffff                   mov eax, dword ptr [ebp - 0xd4]
0040849B 898530ffffff                   mov dword ptr [ebp - 0xd0], eax
004084A1 8d8d30ffffff                   lea ecx, [ebp - 0xd0]
004084A7 51                             push ecx
004084A8 b9a8884c00                     mov ecx, 0x4c88a8
004084AD e8de93ffff                     call 0x401890
004084B2 e9cc010000                     jmp 0x408683
004084B7 6828010000                     push 0x128
004084BC e8e73c0900                     call 0x49c1a8
004084C1 83c404                         add esp, 4
004084C4 898520ffffff                   mov dword ptr [ebp - 0xe0], eax
004084CA c645fc04                       mov byte ptr [ebp - 4], 4
004084CE 83bd20ffffff00                 cmp dword ptr [ebp - 0xe0], 0
004084D5 7413                           je 0x4084ea
004084D7 8b8d20ffffff                   mov ecx, dword ptr [ebp - 0xe0]
004084DD e8beedffff                     call 0x4072a0
004084E2 8985b8feffff                   mov dword ptr [ebp - 0x148], eax
004084E8 eb0a                           jmp 0x4084f4
004084EA c785b8feffff00000000           mov dword ptr [ebp - 0x148], 0
004084F4 8b95b8feffff                   mov edx, dword ptr [ebp - 0x148]
004084FA 899524ffffff                   mov dword ptr [ebp - 0xdc], edx
00408500 c645fc01                       mov byte ptr [ebp - 4], 1
00408504 8b8524ffffff                   mov eax, dword ptr [ebp - 0xdc]
0040850A 89459c                         mov dword ptr [ebp - 0x64], eax
0040850D 6a06                           push 6
0040850F 8b4d9c                         mov ecx, dword ptr [ebp - 0x64]
00408512 e849f0ffff                     call 0x407560
00408517 8d4dc4                         lea ecx, [ebp - 0x3c]
0040851A 51                             push ecx
0040851B 8b559c                         mov edx, dword ptr [ebp - 0x64]
0040851E 8b02                           mov eax, dword ptr [edx]
00408520 8b4d9c                         mov ecx, dword ptr [ebp - 0x64]
00408523 ff5008                         call dword ptr [eax + 8]
00408526 6a00                           push 0
00408528 6a00                           push 0
0040852A 6800001040                     push 0x40100000
0040852F 6a00                           push 0
00408531 8b4d9c                         mov ecx, dword ptr [ebp - 0x64]
00408534 e897f1ffff                     call 0x4076d0
00408539 6a06                           push 6
0040853B 8b4d9c                         mov ecx, dword ptr [ebp - 0x64]
0040853E e8edf1ffff                     call 0x407730
00408543 8b4d08                         mov ecx, dword ptr [ebp + 8]
00408546 83c108                         add ecx, 8
00408549 51                             push ecx
0040854A 8d95f0feffff                   lea edx, [ebp - 0x110]
00408550 52                             push edx
00408551 b9106a4c00                     mov ecx, 0x4c6a10
00408556 e811500300                     call 0x43d56c
0040855B 50                             push eax
0040855C 8d85d8feffff                   lea eax, [ebp - 0x128]
00408562 50                             push eax
00408563 8d4dc4                         lea ecx, [ebp - 0x3c]
00408566 51                             push ecx
00408567 8d9508ffffff                   lea edx, [ebp - 0xf8]
0040856D 52                             push edx
0040856E b9106a4c00                     mov ecx, 0x4c6a10
00408573 e8f44f0300                     call 0x43d56c
00408578 8bc8                           mov ecx, eax
0040857A e8b192ffff                     call 0x401830
0040857F 50                             push eax
00408580 8b459c                         mov eax, dword ptr [ebp - 0x64]
00408583 8b10                           mov edx, dword ptr [eax]
00408585 8b4d9c                         mov ecx, dword ptr [ebp - 0x64]
00408588 ff5254                         call dword ptr [edx + 0x54]
0040858B 8b450c                         mov eax, dword ptr [ebp + 0xc]
0040858E 8b4804                         mov ecx, dword ptr [eax + 4]
00408591 51                             push ecx
00408592 8b10                           mov edx, dword ptr [eax]
00408594 52                             push edx
00408595 8b459c                         mov eax, dword ptr [ebp - 0x64]
00408598 8b10                           mov edx, dword ptr [eax]
0040859A 8b4d9c                         mov ecx, dword ptr [ebp - 0x64]
0040859D ff522c                         call dword ptr [edx + 0x2c]
004085A0 dd45cc                         fld qword ptr [ebp - 0x34]
004085A3 e804400900                     call 0x49c5ac
004085A8 50                             push eax
004085A9 dd45c4                         fld qword ptr [ebp - 0x3c]
004085AC e8fb3f0900                     call 0x49c5ac
004085B1 50                             push eax
004085B2 b9106a4c00                     mov ecx, 0x4c6a10
004085B7 e884b9ffff                     call 0x403f40
004085BC 8bf0                           mov esi, eax
004085BE b906000000                     mov ecx, 6
004085C3 8d7d84                         lea edi, [ebp - 0x7c]
004085C6 f3a5                           rep movsd dword ptr es:[edi], dword ptr [esi]
004085C8 c745a000000000                 mov dword ptr [ebp - 0x60], 0
004085CF eb09                           jmp 0x4085da
004085D1 8b45a0                         mov eax, dword ptr [ebp - 0x60]
004085D4 83c001                         add eax, 1
004085D7 8945a0                         mov dword ptr [ebp - 0x60], eax
004085DA 8b4d9c                         mov ecx, dword ptr [ebp - 0x64]
004085DD 81c1b0000000                   add ecx, 0xb0
004085E3 e8e892ffff                     call 0x4018d0
004085E8 3945a0                         cmp dword ptr [ebp - 0x60], eax
004085EB 7d25                           jge 0x408612
004085ED 8d4d84                         lea ecx, [ebp - 0x7c]
004085F0 51                             push ecx
004085F1 8b55a0                         mov edx, dword ptr [ebp - 0x60]
004085F4 52                             push edx
004085F5 8b4d9c                         mov ecx, dword ptr [ebp - 0x64]
004085F8 81c1b0000000                   add ecx, 0xb0
004085FE e8ad92ffff                     call 0x4018b0
00408603 8bc8                           mov ecx, eax
00408605 81c1b0000000                   add ecx, 0xb0
0040860B e810b2ffff                     call 0x403820
00408610 ebbf                           jmp 0x4085d1
00408612 6a48                           push 0x48
00408614 e88f3b0900                     call 0x49c1a8
00408619 83c404                         add esp, 4
0040861C 8985ccfeffff                   mov dword ptr [ebp - 0x134], eax
00408622 c645fc05                       mov byte ptr [ebp - 4], 5
00408626 83bdccfeffff00                 cmp dword ptr [ebp - 0x134], 0
0040862D 741d                           je 0x40864c
0040862F 6a01                           push 1
00408631 8b459c                         mov eax, dword ptr [ebp - 0x64]
00408634 50                             push eax
00408635 8d4dc4                         lea ecx, [ebp - 0x3c]
00408638 51                             push ecx
00408639 8b8dccfeffff                   mov ecx, dword ptr [ebp - 0x134]
0040863F e83c9a0200                     call 0x432080
00408644 8985b4feffff                   mov dword ptr [ebp - 0x14c], eax
0040864A eb0a                           jmp 0x408656
0040864C c785b4feffff00000000           mov dword ptr [ebp - 0x14c], 0
00408656 8b95b4feffff                   mov edx, dword ptr [ebp - 0x14c]
0040865C 8995d0feffff                   mov dword ptr [ebp - 0x130], edx
00408662 c645fc01                       mov byte ptr [ebp - 4], 1
00408666 8b85d0feffff                   mov eax, dword ptr [ebp - 0x130]
0040866C 8985d4feffff                   mov dword ptr [ebp - 0x12c], eax
00408672 8d8dd4feffff                   lea ecx, [ebp - 0x12c]
00408678 51                             push ecx
00408679 b9a8884c00                     mov ecx, 0x4c88a8
0040867E e80d92ffff                     call 0x401890
00408683 8b550c                         mov edx, dword ptr [ebp + 0xc]
00408686 c70200000000                   mov dword ptr [edx], 0
0040868C c7420400000000                 mov dword ptr [edx + 4], 0
00408693 c785c8feffff02000000           mov dword ptr [ebp - 0x138], 2
0040869D c645fc00                       mov byte ptr [ebp - 4], 0
004086A1 8d4dc4                         lea ecx, [ebp - 0x3c]
004086A4 e807b8ffff                     call 0x403eb0
004086A9 c745fcffffffff                 mov dword ptr [ebp - 4], 0xffffffff
004086B0 8d4ddc                         lea ecx, [ebp - 0x24]
004086B3 e8f8b7ffff                     call 0x403eb0
004086B8 8b85c8feffff                   mov eax, dword ptr [ebp - 0x138]
004086BE 8b4df4                         mov ecx, dword ptr [ebp - 0xc]
004086C1 64890d00000000                 mov dword ptr fs:[0], ecx
004086C8 5f                             pop edi
004086C9 5e                             pop esi
004086CA 8be5                           mov esp, ebp
004086CC 5d                             pop ebp
004086CD c20800                         ret 8
