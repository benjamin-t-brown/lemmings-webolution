
; 0x42de51
0042DE51  55                        push ebp
0042DE52  8bec                      mov ebp, esp
0042DE54  6aff                      push -1
0042DE56  6847af4a00                push 0x4aaf47
0042DE5B  64a100000000              mov eax, dword ptr fs:[0]
0042DE61  50                        push eax
0042DE62  64892500000000            mov dword ptr fs:[0], esp
0042DE69  83ec60                    sub esp, 0x60
0042DE6C  894da0                    mov dword ptr [ebp - 0x60], ecx
0042DE6F  680000e03f                push 0x3fe00000
0042DE74  6a00                      push 0
0042DE76  8b4514                    mov eax, dword ptr [ebp + 0x14]
0042DE79  50                        push eax
0042DE7A  8b4d10                    mov ecx, dword ptr [ebp + 0x10]
0042DE7D  51                        push ecx
0042DE7E  8b550c                    mov edx, dword ptr [ebp + 0xc]
0042DE81  52                        push edx
0042DE82  8b4508                    mov eax, dword ptr [ebp + 8]
0042DE85  50                        push eax
0042DE86  8d4dbc                    lea ecx, [ebp - 0x44]
0042DE89  e86239fdff                call 0x4017f0
0042DE8E  50                        push eax
0042DE8F  8d4dd4                    lea ecx, [ebp - 0x2c]
0042DE92  e83988fdff                call 0x4066d0
0042DE97  c745fc00000000            mov dword ptr [ebp - 4], 0
0042DE9E  8d4dd4                    lea ecx, [ebp - 0x2c]
0042DEA1  51                        push ecx
0042DEA2  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DEA5  e8f851ffff                call 0x4230a2
0042DEAA  c645fc02                  mov byte ptr [ebp - 4], 2
0042DEAE  8d4dd4                    lea ecx, [ebp - 0x2c]
0042DEB1  e8fa5ffdff                call 0x403eb0
0042DEB6  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DEB9  83c168                    add ecx, 0x68
0042DEBC  e80f430500                call 0x4821d0
0042DEC1  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DEC4  83c178                    add ecx, 0x78
0042DEC7  e834ef0000                call 0x43ce00
0042DECC  c645fc03                  mov byte ptr [ebp - 4], 3
0042DED0  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DED3  81c148030000              add ecx, 0x348
0042DED9  e8b22b0000                call 0x430a90
0042DEDE  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DEE1  81c150030000              add ecx, 0x350
0042DEE7  e8e4420500                call 0x4821d0
0042DEEC  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DEEF  81c160030000              add ecx, 0x360
0042DEF5  e8d6420500                call 0x4821d0
0042DEFA  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DEFD  81c174030000              add ecx, 0x374
0042DF03  e888210000                call 0x430090
0042DF08  c645fc04                  mov byte ptr [ebp - 4], 4
0042DF0C  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DF0F  81c190030000              add ecx, 0x390
0042DF15  e8b6420500                call 0x4821d0
0042DF1A  8b55a0                    mov edx, dword ptr [ebp - 0x60]
0042DF1D  c702e8104b00              mov dword ptr [edx], 0x4b10e8
0042DF23  68d0030000                push 0x3d0
0042DF28  e87be20600                call 0x49c1a8
0042DF2D  83c404                    add esp, 4
0042DF30  8945b4                    mov dword ptr [ebp - 0x4c], eax
0042DF33  c645fc05                  mov byte ptr [ebp - 4], 5
0042DF37  837db400                  cmp dword ptr [ebp - 0x4c], 0
0042DF3B  7411                      je 0x42df4e
0042DF3D  8b45a0                    mov eax, dword ptr [ebp - 0x60]
0042DF40  50                        push eax
0042DF41  8b4db4                    mov ecx, dword ptr [ebp - 0x4c]
0042DF44  e8f79f0200                call 0x457f40
0042DF49  89459c                    mov dword ptr [ebp - 0x64], eax
0042DF4C  eb07                      jmp 0x42df55
0042DF4E  c7459c00000000            mov dword ptr [ebp - 0x64], 0
0042DF55  8b4d9c                    mov ecx, dword ptr [ebp - 0x64]
0042DF58  894db8                    mov dword ptr [ebp - 0x48], ecx
0042DF5B  c645fc04                  mov byte ptr [ebp - 4], 4
0042DF5F  8b55a0                    mov edx, dword ptr [ebp - 0x60]
0042DF62  8b45b8                    mov eax, dword ptr [ebp - 0x48]
0042DF65  898244030000              mov dword ptr [edx + 0x344], eax
0042DF6B  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DF6E  c6818c03000000            mov byte ptr [ecx + 0x38c], 0
0042DF75  8b5518                    mov edx, dword ptr [ebp + 0x18]
0042DF78  8b04955c334e00            mov eax, dword ptr [edx*4 + 0x4e335c]
0042DF7F  50                        push eax
0042DF80  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DF83  81c128010000              add ecx, 0x128
0042DF89  e8dfa30300                call 0x46836d
0042DF8E  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DF91  c6416000                  mov byte ptr [ecx + 0x60], 0
0042DF95  8b55a0                    mov edx, dword ptr [ebp - 0x60]
0042DF98  c6425c00                  mov byte ptr [edx + 0x5c], 0
0042DF9C  8b45a0                    mov eax, dword ptr [ebp - 0x60]
0042DF9F  c6405f00                  mov byte ptr [eax + 0x5f], 0
0042DFA3  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DFA6  c6414a00                  mov byte ptr [ecx + 0x4a], 0
0042DFAA  8b55a0                    mov edx, dword ptr [ebp - 0x60]
0042DFAD  c6425e01                  mov byte ptr [edx + 0x5e], 1
0042DFB1  8b45a0                    mov eax, dword ptr [ebp - 0x60]
0042DFB4  c6405d00                  mov byte ptr [eax + 0x5d], 0
0042DFB8  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DFBB  c7416800000000            mov dword ptr [ecx + 0x68], 0
0042DFC2  c7416c00000000            mov dword ptr [ecx + 0x6c], 0
0042DFC9  8b55a0                    mov edx, dword ptr [ebp - 0x60]
0042DFCC  c7427000000000            mov dword ptr [edx + 0x70], 0
0042DFD3  c7427400000000            mov dword ptr [edx + 0x74], 0
0042DFDA  8b45a0                    mov eax, dword ptr [ebp - 0x60]
0042DFDD  c7404001000000            mov dword ptr [eax + 0x40], 1
0042DFE4  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042DFE7  c7414401000000            mov dword ptr [ecx + 0x44], 1
0042DFEE  8b5518                    mov edx, dword ptr [ebp + 0x18]
0042DFF1  668b045578ba4c00          mov ax, word ptr [edx*2 + 0x4cba78]
0042DFF9  66050100                  add ax, 1
0042DFFD  8b4d18                    mov ecx, dword ptr [ebp + 0x18]
0042E000  6689044d78ba4c00          mov word ptr [ecx*2 + 0x4cba78], ax
0042E008  b9086a4c00                mov ecx, 0x4c6a08
0042E00D  e8aa6a0100                call 0x444abc
0042E012  6a28                      push 0x28
0042E014  e88fe10600                call 0x49c1a8
0042E019  83c404                    add esp, 4
0042E01C  8945ac                    mov dword ptr [ebp - 0x54], eax
0042E01F  c645fc06                  mov byte ptr [ebp - 4], 6
0042E023  837dac00                  cmp dword ptr [ebp - 0x54], 0
0042E027  740d                      je 0x42e036
0042E029  8b4dac                    mov ecx, dword ptr [ebp - 0x54]
0042E02C  e8bf71feff                call 0x4151f0
0042E031  894598                    mov dword ptr [ebp - 0x68], eax
0042E034  eb07                      jmp 0x42e03d
0042E036  c7459800000000            mov dword ptr [ebp - 0x68], 0
0042E03D  8b5598                    mov edx, dword ptr [ebp - 0x68]
0042E040  8955b0                    mov dword ptr [ebp - 0x50], edx
0042E043  c645fc04                  mov byte ptr [ebp - 4], 4
0042E047  8b45b0                    mov eax, dword ptr [ebp - 0x50]
0042E04A  8945ec                    mov dword ptr [ebp - 0x14], eax
0042E04D  8b4dec                    mov ecx, dword ptr [ebp - 0x14]
0042E050  51                        push ecx
0042E051  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042E054  e83d060000                call 0x42e696
0042E059  8b55a0                    mov edx, dword ptr [ebp - 0x60]
0042E05C  8b4518                    mov eax, dword ptr [ebp + 0x18]
0042E05F  89424c                    mov dword ptr [edx + 0x4c], eax
0042E062  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042E065  c7415400000000            mov dword ptr [ecx + 0x54], 0
0042E06C  8b55a0                    mov edx, dword ptr [ebp - 0x60]
0042E06F  c6827003000001            mov byte ptr [edx + 0x370], 1
0042E076  8b45a0                    mov eax, dword ptr [ebp - 0x60]
0042E079  c7804003000000000000      mov dword ptr [eax + 0x340], 0
0042E083  6a28                      push 0x28
0042E085  e81ee10600                call 0x49c1a8
0042E08A  83c404                    add esp, 4
0042E08D  8945a4                    mov dword ptr [ebp - 0x5c], eax
0042E090  c645fc07                  mov byte ptr [ebp - 4], 7
0042E094  837da400                  cmp dword ptr [ebp - 0x5c], 0
0042E098  740d                      je 0x42e0a7
0042E09A  8b4da4                    mov ecx, dword ptr [ebp - 0x5c]
0042E09D  e8fe7efeff                call 0x415fa0
0042E0A2  894594                    mov dword ptr [ebp - 0x6c], eax
0042E0A5  eb07                      jmp 0x42e0ae
0042E0A7  c7459400000000            mov dword ptr [ebp - 0x6c], 0
0042E0AE  8b4d94                    mov ecx, dword ptr [ebp - 0x6c]
0042E0B1  894da8                    mov dword ptr [ebp - 0x58], ecx
0042E0B4  c645fc04                  mov byte ptr [ebp - 4], 4
0042E0B8  8b55a8                    mov edx, dword ptr [ebp - 0x58]
0042E0BB  8955f0                    mov dword ptr [ebp - 0x10], edx
0042E0BE  8b45f0                    mov eax, dword ptr [ebp - 0x10]
0042E0C1  50                        push eax
0042E0C2  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042E0C5  e8cc050000                call 0x42e696
0042E0CA  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042E0CD  c6414800                  mov byte ptr [ecx + 0x48], 0
0042E0D1  8b55a0                    mov edx, dword ptr [ebp - 0x60]
0042E0D4  c6424900                  mov byte ptr [edx + 0x49], 0
0042E0D8  8b45a0                    mov eax, dword ptr [ebp - 0x60]
0042E0DB  c7805003000000000000      mov dword ptr [eax + 0x350], 0
0042E0E5  c7805403000000000000      mov dword ptr [eax + 0x354], 0
0042E0EF  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042E0F2  c7815803000000000000      mov dword ptr [ecx + 0x358], 0
0042E0FC  c7815c03000000000840      mov dword ptr [ecx + 0x35c], 0x40080000
0042E106  8b55a0                    mov edx, dword ptr [ebp - 0x60]
0042E109  c7826003000000000000      mov dword ptr [edx + 0x360], 0
0042E113  c7826403000000000000      mov dword ptr [edx + 0x364], 0
0042E11D  8b45a0                    mov eax, dword ptr [ebp - 0x60]
0042E120  c7806803000000000000      mov dword ptr [eax + 0x368], 0
0042E12A  c7806c03000000000000      mov dword ptr [eax + 0x36c], 0
0042E134  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042E137  83c108                    add ecx, 8
0042E13A  51                        push ecx
0042E13B  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042E13E  83c178                    add ecx, 0x78
0042E141  8b55a0                    mov edx, dword ptr [ebp - 0x60]
0042E144  8b4278                    mov eax, dword ptr [edx + 0x78]
0042E147  ff5008                    call dword ptr [eax + 8]
0042E14A  8b4da0                    mov ecx, dword ptr [ebp - 0x60]
0042E14D  51                        push ecx
0042E14E  b9106a4c00                mov ecx, 0x4c6a10
0042E153  e8bfd30000                call 0x43b517
0042E158  8b55a0                    mov edx, dword ptr [ebp - 0x60]
0042E15B  c7425001000000            mov dword ptr [edx + 0x50], 1
0042E162  c745fcffffffff            mov dword ptr [ebp - 4], 0xffffffff
0042E169  8b45a0                    mov eax, dword ptr [ebp - 0x60]
0042E16C  8b4df4                    mov ecx, dword ptr [ebp - 0xc]
0042E16F  64890d00000000            mov dword ptr fs:[0], ecx
0042E176  8be5                      mov esp, ebp
0042E178  5d                        pop ebp
0042E179  c21400                    ret 0x14

; 0x430a90
00430A90  55                        push ebp
00430A91  8bec                      mov ebp, esp
00430A93  51                        push ecx
00430A94  894dfc                    mov dword ptr [ebp - 4], ecx
00430A97  8b45fc                    mov eax, dword ptr [ebp - 4]
00430A9A  8be5                      mov esp, ebp
00430A9C  5d                        pop ebp
00430A9D  c3                        ret
