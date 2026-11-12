; 0x42e17c
0042E17C  55                        push ebp
0042E17D  8bec                      mov ebp, esp
0042E17F  6aff                      push -1
0042E181  6875af4a00                push 0x4aaf75
0042E186  64a100000000              mov eax, dword ptr fs:[0]
0042E18C  50                        push eax
0042E18D  64892500000000            mov dword ptr fs:[0], esp
0042E194  83ec10                    sub esp, 0x10
0042E197  894de8                    mov dword ptr [ebp - 0x18], ecx
0042E19A  8b45e8                    mov eax, dword ptr [ebp - 0x18]
0042E19D  c700e8104b00              mov dword ptr [eax], 0x4b10e8
0042E1A3  c745fc02000000            mov dword ptr [ebp - 4], 2
0042E1AA  8b4de8                    mov ecx, dword ptr [ebp - 0x18]
0042E1AD  83b94403000000            cmp dword ptr [ecx + 0x344], 0
0042E1B4  742e                      je 0x42e1e4
0042E1B6  8b55e8                    mov edx, dword ptr [ebp - 0x18]
0042E1B9  8b8244030000              mov eax, dword ptr [edx + 0x344]
0042E1BF  8945ec                    mov dword ptr [ebp - 0x14], eax
0042E1C2  8b4dec                    mov ecx, dword ptr [ebp - 0x14]
0042E1C5  894df0                    mov dword ptr [ebp - 0x10], ecx
0042E1C8  837df000                  cmp dword ptr [ebp - 0x10], 0
0042E1CC  740f                      je 0x42e1dd
0042E1CE  6a01                      push 1
0042E1D0  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
0042E1D3  e8e81c0000                call 0x42fec0
0042E1D8  8945e4                    mov dword ptr [ebp - 0x1c], eax
0042E1DB  eb07                      jmp 0x42e1e4
0042E1DD  c745e400000000            mov dword ptr [ebp - 0x1c], 0
0042E1E4  8b4de8                    mov ecx, dword ptr [ebp - 0x18]
0042E1E7  e81e050000                call 0x42e70a
0042E1EC  8b55e8                    mov edx, dword ptr [ebp - 0x18]
0042E1EF  8b424c                    mov eax, dword ptr [edx + 0x4c]
0042E1F2  668b0c4578ba4c00          mov cx, word ptr [eax*2 + 0x4cba78]
0042E1FA  6683e901                  sub cx, 1
0042E1FE  8b55e8                    mov edx, dword ptr [ebp - 0x18]
0042E201  8b424c                    mov eax, dword ptr [edx + 0x4c]
0042E204  66890c4578ba4c00          mov word ptr [eax*2 + 0x4cba78], cx
0042E20C  b9086a4c00                mov ecx, 0x4c6a08
0042E211  e8a6680100                call 0x444abc
0042E216  c645fc01                  mov byte ptr [ebp - 4], 1
0042E21A  8b4de8                    mov ecx, dword ptr [ebp - 0x18]
0042E21D  81c174030000              add ecx, 0x374
0042E223  e8e81e0000                call 0x430110
0042E228  c645fc00                  mov byte ptr [ebp - 4], 0
0042E22C  8b4de8                    mov ecx, dword ptr [ebp - 0x18]
0042E22F  83c178                    add ecx, 0x78
0042E232  e851ec0000                call 0x43ce88
0042E237  c745fcffffffff            mov dword ptr [ebp - 4], 0xffffffff
0042E23E  8b4de8                    mov ecx, dword ptr [ebp - 0x18]
0042E241  e8264fffff                call 0x42316c
0042E246  8b4df4                    mov ecx, dword ptr [ebp - 0xc]
0042E249  64890d00000000            mov dword ptr fs:[0], ecx
0042E250  8be5                      mov esp, ebp
0042E252  5d                        pop ebp
0042E253  c3                        ret

; 0x42316c
0042316C  55                        push ebp
0042316D  8bec                      mov ebp, esp
0042316F  6aff                      push -1
00423171  6892a94a00                push 0x4aa992
00423176  64a100000000              mov eax, dword ptr fs:[0]
0042317C  50                        push eax
0042317D  64892500000000            mov dword ptr fs:[0], esp
00423184  51                        push ecx
00423185  894df0                    mov dword ptr [ebp - 0x10], ecx
00423188  8b45f0                    mov eax, dword ptr [ebp - 0x10]
0042318B  c700900e4b00              mov dword ptr [eax], 0x4b0e90
00423191  c745fc00000000            mov dword ptr [ebp - 4], 0
00423198  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
0042319B  0fbf513a                  movsx edx, word ptr [ecx + 0x3a]
0042319F  52                        push edx
004231A0  b918bb4c00                mov ecx, 0x4cbb18
004231A5  e8d6010000                call 0x423380
004231AA  c70000000000              mov dword ptr [eax], 0
004231B0  8b45f0                    mov eax, dword ptr [ebp - 0x10]
004231B3  50                        push eax
004231B4  b9106a4c00                mov ecx, 0x4c6a10
004231B9  e878830100                call 0x43b536
004231BE  c745fcffffffff            mov dword ptr [ebp - 4], 0xffffffff
004231C5  8b4df0                    mov ecx, dword ptr [ebp - 0x10]
004231C8  83c108                    add ecx, 8
004231CB  e8e00cfeff                call 0x403eb0
004231D0  8b4df4                    mov ecx, dword ptr [ebp - 0xc]
004231D3  64890d00000000            mov dword ptr fs:[0], ecx
004231DA  8be5                      mov esp, ebp
004231DC  5d                        pop ebp
004231DD  c3                        ret

; 0x42e70a
0042E70A  55                        push ebp
0042E70B  8bec                      mov ebp, esp
0042E70D  51                        push ecx
0042E70E  894dfc                    mov dword ptr [ebp - 4], ecx
0042E711  8b4dfc                    mov ecx, dword ptr [ebp - 4]
0042E714  e871000000                call 0x42e78a
0042E719  8b4dfc                    mov ecx, dword ptr [ebp - 4]
0042E71C  e804000000                call 0x42e725
0042E721  8be5                      mov esp, ebp
0042E723  5d                        pop ebp
0042E724  c3                        ret
