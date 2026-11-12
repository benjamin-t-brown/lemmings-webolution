; Original EXE SHA256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Piece/texture and destruction byte ledgers supply the composed helper spans.

; VA 0x00483EF0 file offset 540400 length 44 SHA256 4085809ff4717a079aaa755c5cc685f3947662685fdc8b39dfbeaea07ffdf74e
00483EF0  55                                 push ebp
00483EF1  8b ec                              mov ebp, esp
00483EF3  51                                 push ecx
00483EF4  89 4d fc                           mov dword ptr [ebp - 4], ecx
00483EF7  6a 00                              push 0
00483EF9  8b 45 08                           mov eax, dword ptr [ebp + 8]
00483EFC  50                                 push eax
00483EFD  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00483F00  e8 5b 02 00 00                     call 0x484160
00483F05  6a 00                              push 0
00483F07  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00483F0A  51                                 push ecx
00483F0B  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00483F0E  83 c1 10                           add ecx, 0x10
00483F11  e8 ca 02 00 00                     call 0x4841e0
00483F16  8b e5                              mov esp, ebp
00483F18  5d                                 pop ebp
00483F19  c2 04 00                           ret 4

; VA 0x00484160 file offset 541024 length 120 SHA256 e86ea86a512f55cfbf60bf5cb30566a23c84f683bcb1871572bf5eb73544f88d
00484160  55                                 push ebp
00484161  8b ec                              mov ebp, esp
00484163  83 ec 0c                           sub esp, 0xc
00484166  89 4d f4                           mov dword ptr [ebp - 0xc], ecx
00484169  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0048416C  e8 0f 04 00 00                     call 0x484580
00484171  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00484174  e8 e7 e3 f7 ff                     call 0x402560
00484179  89 45 f8                           mov dword ptr [ebp - 8], eax
0048417C  8b 45 f8                           mov eax, dword ptr [ebp - 8]
0048417F  03 45 0c                           add eax, dword ptr [ebp + 0xc]
00484182  50                                 push eax
00484183  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00484186  e8 d5 00 00 00                     call 0x484260
0048418B  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0048418E  51                                 push ecx
0048418F  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00484192  e8 c9 00 00 00                     call 0x484260
00484197  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
0048419E  eb 09                              jmp 0x4841a9
004841A0  8b 55 fc                           mov edx, dword ptr [ebp - 4]
004841A3  83 c2 01                           add edx, 1
004841A6  89 55 fc                           mov dword ptr [ebp - 4], edx
004841A9  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
004841AC  e8 9f ff ff ff                     call 0x484150
004841B1  39 45 fc                           cmp dword ptr [ebp - 4], eax
004841B4  7d 1c                              jge 0x4841d2
004841B6  8b 45 08                           mov eax, dword ptr [ebp + 8]
004841B9  50                                 push eax
004841BA  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
004841BD  69 c9 98 00 00 00                  imul ecx, ecx, 0x98
004841C3  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
004841C6  8b 42 0c                           mov eax, dword ptr [edx + 0xc]
004841C9  03 c8                              add ecx, eax
004841CB  e8 30 16 00 00                     call 0x485800
004841D0  eb ce                              jmp 0x4841a0
004841D2  8b e5                              mov esp, ebp
004841D4  5d                                 pop ebp
004841D5  c2 08 00                           ret 8

; VA 0x004841E0 file offset 541152 length 115 SHA256 7d0784e4ebe44406f6a8ba6658eaf70e48780ab0b7b4af04d124f0f269ad3ff9
004841E0  55                                 push ebp
004841E1  8b ec                              mov ebp, esp
004841E3  83 ec 0c                           sub esp, 0xc
004841E6  89 4d f4                           mov dword ptr [ebp - 0xc], ecx
004841E9  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
004841EC  e8 af 05 00 00                     call 0x4847a0
004841F1  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
004841F4  e8 67 e3 f7 ff                     call 0x402560
004841F9  89 45 f8                           mov dword ptr [ebp - 8], eax
004841FC  8b 45 f8                           mov eax, dword ptr [ebp - 8]
004841FF  03 45 0c                           add eax, dword ptr [ebp + 0xc]
00484202  50                                 push eax
00484203  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00484206  e8 e5 03 00 00                     call 0x4845f0
0048420B  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0048420E  51                                 push ecx
0048420F  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00484212  e8 d9 03 00 00                     call 0x4845f0
00484217  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
0048421E  eb 09                              jmp 0x484229
00484220  8b 55 fc                           mov edx, dword ptr [ebp - 4]
00484223  83 c2 01                           add edx, 1
00484226  89 55 fc                           mov dword ptr [ebp - 4], edx
00484229  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0048422C  e8 af 03 00 00                     call 0x4845e0
00484231  39 45 fc                           cmp dword ptr [ebp - 4], eax
00484234  7d 17                              jge 0x48424d
00484236  8b 45 08                           mov eax, dword ptr [ebp + 8]
00484239  50                                 push eax
0048423A  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0048423D  8b 51 0c                           mov edx, dword ptr [ecx + 0xc]
00484240  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00484243  8d 0c c2                           lea ecx, [edx + eax*8]
00484246  e8 f8 83 ff ff                     call 0x47c643
0048424B  eb d3                              jmp 0x484220
0048424D  8b e5                              mov esp, ebp
0048424F  5d                                 pop ebp
00484250  c2 08 00                           ret 8

; VA 0x00485800 file offset 546816 length 56 SHA256 88d296305d52000fa827a6c63aa4e9a3e62f094878698fbab3794f7c3da30a22
00485800  55                                 push ebp
00485801  8b ec                              mov ebp, esp
00485803  51                                 push ecx
00485804  89 4d fc                           mov dword ptr [ebp - 4], ecx
00485807  6a 00                              push 0
00485809  8b 45 08                           mov eax, dword ptr [ebp + 8]
0048580C  50                                 push eax
0048580D  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00485810  e8 2b 00 00 00                     call 0x485840
00485815  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485818  51                                 push ecx
00485819  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0048581C  83 c1 18                           add ecx, 0x18
0048581F  e8 09 cd ff ff                     call 0x48252d
00485824  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485827  e8 54 cd f7 ff                     call 0x402580
0048582C  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0048582F  dd 5a 10                           fstp qword ptr [edx + 0x10]
00485832  8b e5                              mov esp, ebp
00485834  5d                                 pop ebp
00485835  c2 04 00                           ret 4

; VA 0x00485840 file offset 546880 length 117 SHA256 8357651e4a1e9f4c9426e9e18976c2dd4c31ca363eadf879419d7c21792d6df2
00485840  55                                 push ebp
00485841  8b ec                              mov ebp, esp
00485843  83 ec 0c                           sub esp, 0xc
00485846  89 4d f4                           mov dword ptr [ebp - 0xc], ecx
00485849  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0048584C  e8 cf ec ff ff                     call 0x484520
00485851  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485854  e8 07 cd f7 ff                     call 0x402560
00485859  89 45 f8                           mov dword ptr [ebp - 8], eax
0048585C  8b 45 f8                           mov eax, dword ptr [ebp - 8]
0048585F  03 45 0c                           add eax, dword ptr [ebp + 0xc]
00485862  50                                 push eax
00485863  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00485866  e8 55 00 00 00                     call 0x4858c0
0048586B  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0048586E  51                                 push ecx
0048586F  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00485872  e8 49 00 00 00                     call 0x4858c0
00485877  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
0048587E  eb 09                              jmp 0x485889
00485880  8b 55 fc                           mov edx, dword ptr [ebp - 4]
00485883  83 c2 01                           add edx, 1
00485886  89 55 fc                           mov dword ptr [ebp - 4], edx
00485889  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0048588C  e8 5f 60 fd ff                     call 0x45b8f0
00485891  39 45 fc                           cmp dword ptr [ebp - 4], eax
00485894  7d 19                              jge 0x4858af
00485896  8b 45 08                           mov eax, dword ptr [ebp + 8]
00485899  50                                 push eax
0048589A  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0048589D  c1 e1 04                           shl ecx, 4
004858A0  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
004858A3  8b 42 0c                           mov eax, dword ptr [edx + 0xc]
004858A6  03 c8                              add ecx, eax
004858A8  e8 d3 01 00 00                     call 0x485a80
004858AD  eb d1                              jmp 0x485880
004858AF  8b e5                              mov esp, ebp
004858B1  5d                                 pop ebp
004858B2  c2 08 00                           ret 8

; VA 0x00485A80 file offset 547456 length 27 SHA256 b889ac8800cb2d68d5a3ee270635888787a3afdc92b88f27cc172629f14ef9bb
00485A80  55                                 push ebp
00485A81  8b ec                              mov ebp, esp
00485A83  51                                 push ecx
00485A84  89 4d fc                           mov dword ptr [ebp - 4], ecx
00485A87  6a 00                              push 0
00485A89  8b 45 08                           mov eax, dword ptr [ebp + 8]
00485A8C  50                                 push eax
00485A8D  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00485A90  e8 0b 00 00 00                     call 0x485aa0
00485A95  8b e5                              mov esp, ebp
00485A97  5d                                 pop ebp
00485A98  c2 04 00                           ret 4

; VA 0x00485AA0 file offset 547488 length 117 SHA256 9f05a8d9ba38c9a50065bb54b7eb52b9122094aaa0633e8500963d4de5e45044
00485AA0  55                                 push ebp
00485AA1  8b ec                              mov ebp, esp
00485AA3  83 ec 0c                           sub esp, 0xc
00485AA6  89 4d f4                           mov dword ptr [ebp - 0xc], ecx
00485AA9  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00485AAC  e8 7f ee ff ff                     call 0x484930
00485AB1  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485AB4  e8 a7 ca f7 ff                     call 0x402560
00485AB9  89 45 f8                           mov dword ptr [ebp - 8], eax
00485ABC  8b 45 f8                           mov eax, dword ptr [ebp - 8]
00485ABF  03 45 0c                           add eax, dword ptr [ebp + 0xc]
00485AC2  50                                 push eax
00485AC3  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00485AC6  e8 55 00 00 00                     call 0x485b20
00485ACB  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
00485ACE  51                                 push ecx
00485ACF  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00485AD2  e8 49 00 00 00                     call 0x485b20
00485AD7  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
00485ADE  eb 09                              jmp 0x485ae9
00485AE0  8b 55 fc                           mov edx, dword ptr [ebp - 4]
00485AE3  83 c2 01                           add edx, 1
00485AE6  89 55 fc                           mov dword ptr [ebp - 4], edx
00485AE9  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00485AEC  e8 df 31 fe ff                     call 0x468cd0
00485AF1  39 45 fc                           cmp dword ptr [ebp - 4], eax
00485AF4  7d 19                              jge 0x485b0f
00485AF6  8b 45 08                           mov eax, dword ptr [ebp + 8]
00485AF9  50                                 push eax
00485AFA  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00485AFD  6b c9 48                           imul ecx, ecx, 0x48
00485B00  8b 55 f4                           mov edx, dword ptr [ebp - 0xc]
00485B03  8b 42 0c                           mov eax, dword ptr [edx + 0xc]
00485B06  03 c8                              add ecx, eax
00485B08  e8 d3 01 00 00                     call 0x485ce0
00485B0D  eb d1                              jmp 0x485ae0
00485B0F  8b e5                              mov esp, ebp
00485B11  5d                                 pop ebp
00485B12  c2 08 00                           ret 8

; VA 0x0047C6A8 file offset 509608 length 210 SHA256 7e1446a537f4298771470f35a0482f32b4e0f3750eb8b8ffb4b4235317442e72
0047C6A8  55                                 push ebp
0047C6A9  8b ec                              mov ebp, esp
0047C6AB  6a ff                              push -1
0047C6AD  68 c1 e4 4a 00                     push 0x4ae4c1
0047C6B2  64 a1 00 00 00 00                  mov eax, dword ptr fs:[0]
0047C6B8  50                                 push eax
0047C6B9  64 89 25 00 00 00 00               mov dword ptr fs:[0], esp
0047C6C0  83 ec 20                           sub esp, 0x20
0047C6C3  89 4d d8                           mov dword ptr [ebp - 0x28], ecx
0047C6C6  8b 45 08                           mov eax, dword ptr [ebp + 8]
0047C6C9  89 45 f0                           mov dword ptr [ebp - 0x10], eax
0047C6CC  8b 4d d8                           mov ecx, dword ptr [ebp - 0x28]
0047C6CF  83 39 00                           cmp dword ptr [ecx], 0
0047C6D2  74 14                              je 0x47c6e8
0047C6D4  8b 55 d8                           mov edx, dword ptr [ebp - 0x28]
0047C6D7  8b 02                              mov eax, dword ptr [edx]
0047C6D9  89 45 e4                           mov dword ptr [ebp - 0x1c], eax
0047C6DC  8b 4d e4                           mov ecx, dword ptr [ebp - 0x1c]
0047C6DF  51                                 push ecx
0047C6E0  e8 58 fa 01 00                     call 0x49c13d
0047C6E5  83 c4 04                           add esp, 4
0047C6E8  83 7d f0 02                        cmp dword ptr [ebp - 0x10], 2
0047C6EC  75 0b                              jne 0x47c6f9
0047C6EE  8b 55 d8                           mov edx, dword ptr [ebp - 0x28]
0047C6F1  c7 02 00 00 00 00                  mov dword ptr [edx], 0
0047C6F7  eb 71                              jmp 0x47c76a
0047C6F9  c7 45 e8 10 00 00 00               mov dword ptr [ebp - 0x18], 0x10
0047C700  83 7d f0 01                        cmp dword ptr [ebp - 0x10], 1
0047C704  75 07                              jne 0x47c70d
0047C706  c7 45 e8 00 01 00 00               mov dword ptr [ebp - 0x18], 0x100
0047C70D  8b 45 e8                           mov eax, dword ptr [ebp - 0x18]
0047C710  89 45 ec                           mov dword ptr [ebp - 0x14], eax
0047C713  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
0047C716  6b c9 18                           imul ecx, ecx, 0x18
0047C719  51                                 push ecx
0047C71A  e8 89 fa 01 00                     call 0x49c1a8
0047C71F  83 c4 04                           add esp, 4
0047C722  89 45 dc                           mov dword ptr [ebp - 0x24], eax
0047C725  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
0047C72C  83 7d dc 00                        cmp dword ptr [ebp - 0x24], 0
0047C730  74 1c                              je 0x47c74e
0047C732  68 80 1b 40 00                     push 0x401b80
0047C737  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0047C73A  52                                 push edx
0047C73B  6a 18                              push 0x18
0047C73D  8b 45 dc                           mov eax, dword ptr [ebp - 0x24]
0047C740  50                                 push eax
0047C741  e8 5a 4a f8 ff                     call 0x4011a0
0047C746  8b 4d dc                           mov ecx, dword ptr [ebp - 0x24]
0047C749  89 4d d4                           mov dword ptr [ebp - 0x2c], ecx
0047C74C  eb 07                              jmp 0x47c755
0047C74E  c7 45 d4 00 00 00 00               mov dword ptr [ebp - 0x2c], 0
0047C755  8b 55 d4                           mov edx, dword ptr [ebp - 0x2c]
0047C758  89 55 e0                           mov dword ptr [ebp - 0x20], edx
0047C75B  c7 45 fc ff ff ff ff               mov dword ptr [ebp - 4], 0xffffffff
0047C762  8b 45 d8                           mov eax, dword ptr [ebp - 0x28]
0047C765  8b 4d e0                           mov ecx, dword ptr [ebp - 0x20]
0047C768  89 08                              mov dword ptr [eax], ecx
0047C76A  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047C76D  64 89 0d 00 00 00 00               mov dword ptr fs:[0], ecx
0047C774  8b e5                              mov esp, ebp
0047C776  5d                                 pop ebp
0047C777  c2 04 00                           ret 4

; VA 0x0047C643 file offset 509507 length 101 SHA256 6fe386b3451c2143242a73ac9bec30e0c9dacd77ff3e11020b04b595b93e87e3
0047C643  55                                 push ebp
0047C644  8b ec                              mov ebp, esp
0047C646  83 ec 08                           sub esp, 8
0047C649  89 4d f8                           mov dword ptr [ebp - 8], ecx
0047C64C  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
0047C64F  e8 0c 5f f8 ff                     call 0x402560
0047C654  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047C657  89 01                              mov dword ptr [ecx], eax
0047C659  8b 55 f8                           mov edx, dword ptr [ebp - 8]
0047C65C  8b 02                              mov eax, dword ptr [edx]
0047C65E  50                                 push eax
0047C65F  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047C662  83 c1 04                           add ecx, 4
0047C665  e8 3e 00 00 00                     call 0x47c6a8
0047C66A  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
0047C671  eb 09                              jmp 0x47c67c
0047C673  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047C676  83 c1 01                           add ecx, 1
0047C679  89 4d fc                           mov dword ptr [ebp - 4], ecx
0047C67C  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047C67F  e8 8c 01 00 00                     call 0x47c810
0047C684  39 45 fc                           cmp dword ptr [ebp - 4], eax
0047C687  7d 19                              jge 0x47c6a2
0047C689  8b 55 08                           mov edx, dword ptr [ebp + 8]
0047C68C  52                                 push edx
0047C68D  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047C690  6b c0 18                           imul eax, eax, 0x18
0047C693  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
0047C696  8b 49 04                           mov ecx, dword ptr [ecx + 4]
0047C699  03 c8                              add ecx, eax
0047C69B  e8 10 45 00 00                     call 0x480bb0
0047C6A0  eb d1                              jmp 0x47c673
0047C6A2  8b e5                              mov esp, ebp
0047C6A4  5d                                 pop ebp
0047C6A5  c2 04 00                           ret 4

; VA 0x00484260 file offset 541280 length 445 SHA256 55bb31a43963a9fbdf8ee374f0a89b8dac6b1935be173d4a8c2b4e3bc46e9384
00484260  55                                 push ebp
00484261  8b ec                              mov ebp, esp
00484263  6a ff                              push -1
00484265  68 5b e8 4a 00                     push 0x4ae85b
0048426A  64 a1 00 00 00 00                  mov eax, dword ptr fs:[0]
00484270  50                                 push eax
00484271  64 89 25 00 00 00 00               mov dword ptr fs:[0], esp
00484278  83 ec 34                           sub esp, 0x34
0048427B  89 4d cc                           mov dword ptr [ebp - 0x34], ecx
0048427E  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00484281  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00484284  3b 48 04                           cmp ecx, dword ptr [eax + 4]
00484287  0f 8e 78 01 00 00                  jle 0x484405
0048428D  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00484290  83 7a 08 01                        cmp dword ptr [edx + 8], 1
00484294  75 0b                              jne 0x4842a1
00484296  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00484299  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
0048429C  89 48 04                           mov dword ptr [eax + 4], ecx
0048429F  eb 1d                              jmp 0x4842be
004842A1  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
004842A4  8b 42 04                           mov eax, dword ptr [edx + 4]
004842A7  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
004842AA  03 41 08                           add eax, dword ptr [ecx + 8]
004842AD  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
004842B0  89 42 04                           mov dword ptr [edx + 4], eax
004842B3  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
004842B6  8b 48 04                           mov ecx, dword ptr [eax + 4]
004842B9  3b 4d 08                           cmp ecx, dword ptr [ebp + 8]
004842BC  7e e3                              jle 0x4842a1
004842BE  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
004842C1  83 7a 04 00                        cmp dword ptr [edx + 4], 0
004842C5  0f 8e fc 00 00 00                  jle 0x4843c7
004842CB  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
004842CE  8b 48 04                           mov ecx, dword ptr [eax + 4]
004842D1  89 4d f0                           mov dword ptr [ebp - 0x10], ecx
004842D4  8b 55 f0                           mov edx, dword ptr [ebp - 0x10]
004842D7  69 d2 98 00 00 00                  imul edx, edx, 0x98
004842DD  83 c2 04                           add edx, 4
004842E0  52                                 push edx
004842E1  e8 c2 7e 01 00                     call 0x49c1a8
004842E6  83 c4 04                           add esp, 4
004842E9  89 45 e0                           mov dword ptr [ebp - 0x20], eax
004842EC  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
004842F3  83 7d e0 00                        cmp dword ptr [ebp - 0x20], 0
004842F7  74 32                              je 0x48432b
004842F9  68 70 44 48 00                     push 0x484470
004842FE  68 20 44 48 00                     push 0x484420
00484303  8b 45 e0                           mov eax, dword ptr [ebp - 0x20]
00484306  8b 4d f0                           mov ecx, dword ptr [ebp - 0x10]
00484309  89 08                              mov dword ptr [eax], ecx
0048430B  8b 55 f0                           mov edx, dword ptr [ebp - 0x10]
0048430E  52                                 push edx
0048430F  68 98 00 00 00                     push 0x98
00484314  8b 45 e0                           mov eax, dword ptr [ebp - 0x20]
00484317  83 c0 04                           add eax, 4
0048431A  50                                 push eax
0048431B  e8 b3 82 01 00                     call 0x49c5d3
00484320  8b 4d e0                           mov ecx, dword ptr [ebp - 0x20]
00484323  83 c1 04                           add ecx, 4
00484326  89 4d c8                           mov dword ptr [ebp - 0x38], ecx
00484329  eb 07                              jmp 0x484332
0048432B  c7 45 c8 00 00 00 00               mov dword ptr [ebp - 0x38], 0
00484332  8b 55 c8                           mov edx, dword ptr [ebp - 0x38]
00484335  89 55 e4                           mov dword ptr [ebp - 0x1c], edx
00484338  c7 45 fc ff ff ff ff               mov dword ptr [ebp - 4], 0xffffffff
0048433F  8b 45 e4                           mov eax, dword ptr [ebp - 0x1c]
00484342  89 45 ec                           mov dword ptr [ebp - 0x14], eax
00484345  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
00484348  83 79 0c 00                        cmp dword ptr [ecx + 0xc], 0
0048434C  74 6e                              je 0x4843bc
0048434E  c7 45 e8 00 00 00 00               mov dword ptr [ebp - 0x18], 0
00484355  eb 09                              jmp 0x484360
00484357  8b 55 e8                           mov edx, dword ptr [ebp - 0x18]
0048435A  83 c2 01                           add edx, 1
0048435D  89 55 e8                           mov dword ptr [ebp - 0x18], edx
00484360  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00484363  8b 4d e8                           mov ecx, dword ptr [ebp - 0x18]
00484366  3b 08                              cmp ecx, dword ptr [eax]
00484368  7d 27                              jge 0x484391
0048436A  8b 55 e8                           mov edx, dword ptr [ebp - 0x18]
0048436D  69 d2 98 00 00 00                  imul edx, edx, 0x98
00484373  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00484376  8b 48 0c                           mov ecx, dword ptr [eax + 0xc]
00484379  03 ca                              add ecx, edx
0048437B  51                                 push ecx
0048437C  8b 55 e8                           mov edx, dword ptr [ebp - 0x18]
0048437F  69 d2 98 00 00 00                  imul edx, edx, 0x98
00484385  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
00484388  03 ca                              add ecx, edx
0048438A  e8 81 06 00 00                     call 0x484a10
0048438F  eb c6                              jmp 0x484357
00484391  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00484394  8b 48 0c                           mov ecx, dword ptr [eax + 0xc]
00484397  89 4d d8                           mov dword ptr [ebp - 0x28], ecx
0048439A  8b 55 d8                           mov edx, dword ptr [ebp - 0x28]
0048439D  89 55 dc                           mov dword ptr [ebp - 0x24], edx
004843A0  83 7d dc 00                        cmp dword ptr [ebp - 0x24], 0
004843A4  74 0f                              je 0x4843b5
004843A6  6a 03                              push 3
004843A8  8b 4d dc                           mov ecx, dword ptr [ebp - 0x24]
004843AB  e8 e0 05 00 00                     call 0x484990
004843B0  89 45 c4                           mov dword ptr [ebp - 0x3c], eax
004843B3  eb 07                              jmp 0x4843bc
004843B5  c7 45 c4 00 00 00 00               mov dword ptr [ebp - 0x3c], 0
004843BC  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
004843BF  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
004843C2  89 48 0c                           mov dword ptr [eax + 0xc], ecx
004843C5  eb 3e                              jmp 0x484405
004843C7  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
004843CA  83 7a 0c 00                        cmp dword ptr [edx + 0xc], 0
004843CE  74 2b                              je 0x4843fb
004843D0  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
004843D3  8b 48 0c                           mov ecx, dword ptr [eax + 0xc]
004843D6  89 4d d0                           mov dword ptr [ebp - 0x30], ecx
004843D9  8b 55 d0                           mov edx, dword ptr [ebp - 0x30]
004843DC  89 55 d4                           mov dword ptr [ebp - 0x2c], edx
004843DF  83 7d d4 00                        cmp dword ptr [ebp - 0x2c], 0
004843E3  74 0f                              je 0x4843f4
004843E5  6a 03                              push 3
004843E7  8b 4d d4                           mov ecx, dword ptr [ebp - 0x2c]
004843EA  e8 a1 05 00 00                     call 0x484990
004843EF  89 45 c0                           mov dword ptr [ebp - 0x40], eax
004843F2  eb 07                              jmp 0x4843fb
004843F4  c7 45 c0 00 00 00 00               mov dword ptr [ebp - 0x40], 0
004843FB  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
004843FE  c7 40 0c 00 00 00 00               mov dword ptr [eax + 0xc], 0
00484405  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
00484408  8b 55 08                           mov edx, dword ptr [ebp + 8]
0048440B  89 11                              mov dword ptr [ecx], edx
0048440D  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00484410  64 89 0d 00 00 00 00               mov dword ptr fs:[0], ecx
00484417  8b e5                              mov esp, ebp
00484419  5d                                 pop ebp
0048441A  c2 04 00                           ret 4

; VA 0x004845F0 file offset 542192 length 430 SHA256 4fa07c68bb6aaa7342351f546486785390cd8973dff095f68d7309d19ed77dc9
004845F0  55                                 push ebp
004845F1  8b ec                              mov ebp, esp
004845F3  6a ff                              push -1
004845F5  68 bb e8 4a 00                     push 0x4ae8bb
004845FA  64 a1 00 00 00 00                  mov eax, dword ptr fs:[0]
00484600  50                                 push eax
00484601  64 89 25 00 00 00 00               mov dword ptr fs:[0], esp
00484608  83 ec 34                           sub esp, 0x34
0048460B  89 4d cc                           mov dword ptr [ebp - 0x34], ecx
0048460E  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00484611  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00484614  3b 48 04                           cmp ecx, dword ptr [eax + 4]
00484617  0f 8e 69 01 00 00                  jle 0x484786
0048461D  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00484620  83 7a 08 01                        cmp dword ptr [edx + 8], 1
00484624  75 0b                              jne 0x484631
00484626  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00484629  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
0048462C  89 48 04                           mov dword ptr [eax + 4], ecx
0048462F  eb 1d                              jmp 0x48464e
00484631  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00484634  8b 42 04                           mov eax, dword ptr [edx + 4]
00484637  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
0048463A  03 41 08                           add eax, dword ptr [ecx + 8]
0048463D  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00484640  89 42 04                           mov dword ptr [edx + 4], eax
00484643  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00484646  8b 48 04                           mov ecx, dword ptr [eax + 4]
00484649  3b 4d 08                           cmp ecx, dword ptr [ebp + 8]
0048464C  7e e3                              jle 0x484631
0048464E  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00484651  83 7a 04 00                        cmp dword ptr [edx + 4], 0
00484655  0f 8e ed 00 00 00                  jle 0x484748
0048465B  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
0048465E  8b 48 04                           mov ecx, dword ptr [eax + 4]
00484661  89 4d f0                           mov dword ptr [ebp - 0x10], ecx
00484664  8b 55 f0                           mov edx, dword ptr [ebp - 0x10]
00484667  8d 04 d5 04 00 00 00               lea eax, [edx*8 + 4]
0048466E  50                                 push eax
0048466F  e8 34 7b 01 00                     call 0x49c1a8
00484674  83 c4 04                           add esp, 4
00484677  89 45 e0                           mov dword ptr [ebp - 0x20], eax
0048467A  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
00484681  83 7d e0 00                        cmp dword ptr [ebp - 0x20], 0
00484685  74 2f                              je 0x4846b6
00484687  68 d3 c5 47 00                     push 0x47c5d3
0048468C  68 b0 c5 47 00                     push 0x47c5b0
00484691  8b 4d e0                           mov ecx, dword ptr [ebp - 0x20]
00484694  8b 55 f0                           mov edx, dword ptr [ebp - 0x10]
00484697  89 11                              mov dword ptr [ecx], edx
00484699  8b 45 f0                           mov eax, dword ptr [ebp - 0x10]
0048469C  50                                 push eax
0048469D  6a 08                              push 8
0048469F  8b 4d e0                           mov ecx, dword ptr [ebp - 0x20]
004846A2  83 c1 04                           add ecx, 4
004846A5  51                                 push ecx
004846A6  e8 28 7f 01 00                     call 0x49c5d3
004846AB  8b 55 e0                           mov edx, dword ptr [ebp - 0x20]
004846AE  83 c2 04                           add edx, 4
004846B1  89 55 c8                           mov dword ptr [ebp - 0x38], edx
004846B4  eb 07                              jmp 0x4846bd
004846B6  c7 45 c8 00 00 00 00               mov dword ptr [ebp - 0x38], 0
004846BD  8b 45 c8                           mov eax, dword ptr [ebp - 0x38]
004846C0  89 45 e4                           mov dword ptr [ebp - 0x1c], eax
004846C3  c7 45 fc ff ff ff ff               mov dword ptr [ebp - 4], 0xffffffff
004846CA  8b 4d e4                           mov ecx, dword ptr [ebp - 0x1c]
004846CD  89 4d ec                           mov dword ptr [ebp - 0x14], ecx
004846D0  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
004846D3  83 7a 0c 00                        cmp dword ptr [edx + 0xc], 0
004846D7  74 64                              je 0x48473d
004846D9  c7 45 e8 00 00 00 00               mov dword ptr [ebp - 0x18], 0
004846E0  eb 09                              jmp 0x4846eb
004846E2  8b 45 e8                           mov eax, dword ptr [ebp - 0x18]
004846E5  83 c0 01                           add eax, 1
004846E8  89 45 e8                           mov dword ptr [ebp - 0x18], eax
004846EB  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
004846EE  8b 55 e8                           mov edx, dword ptr [ebp - 0x18]
004846F1  3b 11                              cmp edx, dword ptr [ecx]
004846F3  7d 1d                              jge 0x484712
004846F5  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
004846F8  8b 48 0c                           mov ecx, dword ptr [eax + 0xc]
004846FB  8b 55 e8                           mov edx, dword ptr [ebp - 0x18]
004846FE  8d 04 d1                           lea eax, [ecx + edx*8]
00484701  50                                 push eax
00484702  8b 4d e8                           mov ecx, dword ptr [ebp - 0x18]
00484705  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
00484708  8d 0c ca                           lea ecx, [edx + ecx*8]
0048470B  e8 6a 80 ff ff                     call 0x47c77a
00484710  eb d0                              jmp 0x4846e2
00484712  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00484715  8b 48 0c                           mov ecx, dword ptr [eax + 0xc]
00484718  89 4d d8                           mov dword ptr [ebp - 0x28], ecx
0048471B  8b 55 d8                           mov edx, dword ptr [ebp - 0x28]
0048471E  89 55 dc                           mov dword ptr [ebp - 0x24], edx
00484721  83 7d dc 00                        cmp dword ptr [ebp - 0x24], 0
00484725  74 0f                              je 0x484736
00484727  6a 03                              push 3
00484729  8b 4d dc                           mov ecx, dword ptr [ebp - 0x24]
0048472C  e8 1f 03 00 00                     call 0x484a50
00484731  89 45 c4                           mov dword ptr [ebp - 0x3c], eax
00484734  eb 07                              jmp 0x48473d
00484736  c7 45 c4 00 00 00 00               mov dword ptr [ebp - 0x3c], 0
0048473D  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00484740  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
00484743  89 48 0c                           mov dword ptr [eax + 0xc], ecx
00484746  eb 3e                              jmp 0x484786
00484748  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
0048474B  83 7a 0c 00                        cmp dword ptr [edx + 0xc], 0
0048474F  74 2b                              je 0x48477c
00484751  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00484754  8b 48 0c                           mov ecx, dword ptr [eax + 0xc]
00484757  89 4d d0                           mov dword ptr [ebp - 0x30], ecx
0048475A  8b 55 d0                           mov edx, dword ptr [ebp - 0x30]
0048475D  89 55 d4                           mov dword ptr [ebp - 0x2c], edx
00484760  83 7d d4 00                        cmp dword ptr [ebp - 0x2c], 0
00484764  74 0f                              je 0x484775
00484766  6a 03                              push 3
00484768  8b 4d d4                           mov ecx, dword ptr [ebp - 0x2c]
0048476B  e8 e0 02 00 00                     call 0x484a50
00484770  89 45 c0                           mov dword ptr [ebp - 0x40], eax
00484773  eb 07                              jmp 0x48477c
00484775  c7 45 c0 00 00 00 00               mov dword ptr [ebp - 0x40], 0
0048477C  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
0048477F  c7 40 0c 00 00 00 00               mov dword ptr [eax + 0xc], 0
00484786  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
00484789  8b 55 08                           mov edx, dword ptr [ebp + 8]
0048478C  89 11                              mov dword ptr [ecx], edx
0048478E  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00484791  64 89 0d 00 00 00 00               mov dword ptr fs:[0], ecx
00484798  8b e5                              mov esp, ebp
0048479A  5d                                 pop ebp
0048479B  c2 04 00                           ret 4

; VA 0x004858C0 file offset 547008 length 433 SHA256 30bc2cbcb118b2a996f5f36d23b1bb77f5cb8a6e15e48238e61bf078e66a6bd0
004858C0  55                                 push ebp
004858C1  8b ec                              mov ebp, esp
004858C3  6a ff                              push -1
004858C5  68 3b ea 4a 00                     push 0x4aea3b
004858CA  64 a1 00 00 00 00                  mov eax, dword ptr fs:[0]
004858D0  50                                 push eax
004858D1  64 89 25 00 00 00 00               mov dword ptr fs:[0], esp
004858D8  83 ec 34                           sub esp, 0x34
004858DB  89 4d cc                           mov dword ptr [ebp - 0x34], ecx
004858DE  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
004858E1  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
004858E4  3b 48 04                           cmp ecx, dword ptr [eax + 4]
004858E7  0f 8e 6c 01 00 00                  jle 0x485a59
004858ED  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
004858F0  83 7a 08 01                        cmp dword ptr [edx + 8], 1
004858F4  75 0b                              jne 0x485901
004858F6  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
004858F9  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
004858FC  89 48 04                           mov dword ptr [eax + 4], ecx
004858FF  eb 1d                              jmp 0x48591e
00485901  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485904  8b 42 04                           mov eax, dword ptr [edx + 4]
00485907  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
0048590A  03 41 08                           add eax, dword ptr [ecx + 8]
0048590D  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485910  89 42 04                           mov dword ptr [edx + 4], eax
00485913  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00485916  8b 48 04                           mov ecx, dword ptr [eax + 4]
00485919  3b 4d 08                           cmp ecx, dword ptr [ebp + 8]
0048591C  7e e3                              jle 0x485901
0048591E  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485921  83 7a 04 00                        cmp dword ptr [edx + 4], 0
00485925  0f 8e f0 00 00 00                  jle 0x485a1b
0048592B  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
0048592E  8b 48 04                           mov ecx, dword ptr [eax + 4]
00485931  89 4d f0                           mov dword ptr [ebp - 0x10], ecx
00485934  8b 55 f0                           mov edx, dword ptr [ebp - 0x10]
00485937  c1 e2 04                           shl edx, 4
0048593A  83 c2 04                           add edx, 4
0048593D  52                                 push edx
0048593E  e8 65 68 01 00                     call 0x49c1a8
00485943  83 c4 04                           add esp, 4
00485946  89 45 e0                           mov dword ptr [ebp - 0x20], eax
00485949  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
00485950  83 7d e0 00                        cmp dword ptr [ebp - 0x20], 0
00485954  74 2f                              je 0x485985
00485956  68 f0 48 48 00                     push 0x4848f0
0048595B  68 10 4c 48 00                     push 0x484c10
00485960  8b 45 e0                           mov eax, dword ptr [ebp - 0x20]
00485963  8b 4d f0                           mov ecx, dword ptr [ebp - 0x10]
00485966  89 08                              mov dword ptr [eax], ecx
00485968  8b 55 f0                           mov edx, dword ptr [ebp - 0x10]
0048596B  52                                 push edx
0048596C  6a 10                              push 0x10
0048596E  8b 45 e0                           mov eax, dword ptr [ebp - 0x20]
00485971  83 c0 04                           add eax, 4
00485974  50                                 push eax
00485975  e8 59 6c 01 00                     call 0x49c5d3
0048597A  8b 4d e0                           mov ecx, dword ptr [ebp - 0x20]
0048597D  83 c1 04                           add ecx, 4
00485980  89 4d c8                           mov dword ptr [ebp - 0x38], ecx
00485983  eb 07                              jmp 0x48598c
00485985  c7 45 c8 00 00 00 00               mov dword ptr [ebp - 0x38], 0
0048598C  8b 55 c8                           mov edx, dword ptr [ebp - 0x38]
0048598F  89 55 e4                           mov dword ptr [ebp - 0x1c], edx
00485992  c7 45 fc ff ff ff ff               mov dword ptr [ebp - 4], 0xffffffff
00485999  8b 45 e4                           mov eax, dword ptr [ebp - 0x1c]
0048599C  89 45 ec                           mov dword ptr [ebp - 0x14], eax
0048599F  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
004859A2  83 79 0c 00                        cmp dword ptr [ecx + 0xc], 0
004859A6  74 68                              je 0x485a10
004859A8  c7 45 e8 00 00 00 00               mov dword ptr [ebp - 0x18], 0
004859AF  eb 09                              jmp 0x4859ba
004859B1  8b 55 e8                           mov edx, dword ptr [ebp - 0x18]
004859B4  83 c2 01                           add edx, 1
004859B7  89 55 e8                           mov dword ptr [ebp - 0x18], edx
004859BA  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
004859BD  8b 4d e8                           mov ecx, dword ptr [ebp - 0x18]
004859C0  3b 08                              cmp ecx, dword ptr [eax]
004859C2  7d 21                              jge 0x4859e5
004859C4  8b 55 e8                           mov edx, dword ptr [ebp - 0x18]
004859C7  c1 e2 04                           shl edx, 4
004859CA  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
004859CD  8b 48 0c                           mov ecx, dword ptr [eax + 0xc]
004859D0  03 ca                              add ecx, edx
004859D2  51                                 push ecx
004859D3  8b 55 e8                           mov edx, dword ptr [ebp - 0x18]
004859D6  c1 e2 04                           shl edx, 4
004859D9  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
004859DC  03 ca                              add ecx, edx
004859DE  e8 ed f2 ff ff                     call 0x484cd0
004859E3  eb cc                              jmp 0x4859b1
004859E5  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
004859E8  8b 48 0c                           mov ecx, dword ptr [eax + 0xc]
004859EB  89 4d d8                           mov dword ptr [ebp - 0x28], ecx
004859EE  8b 55 d8                           mov edx, dword ptr [ebp - 0x28]
004859F1  89 55 dc                           mov dword ptr [ebp - 0x24], edx
004859F4  83 7d dc 00                        cmp dword ptr [ebp - 0x24], 0
004859F8  74 0f                              je 0x485a09
004859FA  6a 03                              push 3
004859FC  8b 4d dc                           mov ecx, dword ptr [ebp - 0x24]
004859FF  e8 7c ee ff ff                     call 0x484880
00485A04  89 45 c4                           mov dword ptr [ebp - 0x3c], eax
00485A07  eb 07                              jmp 0x485a10
00485A09  c7 45 c4 00 00 00 00               mov dword ptr [ebp - 0x3c], 0
00485A10  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00485A13  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
00485A16  89 48 0c                           mov dword ptr [eax + 0xc], ecx
00485A19  eb 3e                              jmp 0x485a59
00485A1B  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485A1E  83 7a 0c 00                        cmp dword ptr [edx + 0xc], 0
00485A22  74 2b                              je 0x485a4f
00485A24  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00485A27  8b 48 0c                           mov ecx, dword ptr [eax + 0xc]
00485A2A  89 4d d0                           mov dword ptr [ebp - 0x30], ecx
00485A2D  8b 55 d0                           mov edx, dword ptr [ebp - 0x30]
00485A30  89 55 d4                           mov dword ptr [ebp - 0x2c], edx
00485A33  83 7d d4 00                        cmp dword ptr [ebp - 0x2c], 0
00485A37  74 0f                              je 0x485a48
00485A39  6a 03                              push 3
00485A3B  8b 4d d4                           mov ecx, dword ptr [ebp - 0x2c]
00485A3E  e8 3d ee ff ff                     call 0x484880
00485A43  89 45 c0                           mov dword ptr [ebp - 0x40], eax
00485A46  eb 07                              jmp 0x485a4f
00485A48  c7 45 c0 00 00 00 00               mov dword ptr [ebp - 0x40], 0
00485A4F  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00485A52  c7 40 0c 00 00 00 00               mov dword ptr [eax + 0xc], 0
00485A59  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
00485A5C  8b 55 08                           mov edx, dword ptr [ebp + 8]
00485A5F  89 11                              mov dword ptr [ecx], edx
00485A61  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00485A64  64 89 0d 00 00 00 00               mov dword ptr fs:[0], ecx
00485A6B  8b e5                              mov esp, ebp
00485A6D  5d                                 pop ebp
00485A6E  c2 04 00                           ret 4

; VA 0x00485B20 file offset 547616 length 438 SHA256 61ac357ed1b11bbc112db65e5a360a91e2a5e43da1db8a450f236113398c87f6
00485B20  55                                 push ebp
00485B21  8b ec                              mov ebp, esp
00485B23  6a ff                              push -1
00485B25  68 5b ea 4a 00                     push 0x4aea5b
00485B2A  64 a1 00 00 00 00                  mov eax, dword ptr fs:[0]
00485B30  50                                 push eax
00485B31  64 89 25 00 00 00 00               mov dword ptr fs:[0], esp
00485B38  83 ec 34                           sub esp, 0x34
00485B3B  56                                 push esi
00485B3C  57                                 push edi
00485B3D  89 4d cc                           mov dword ptr [ebp - 0x34], ecx
00485B40  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00485B43  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485B46  3b 48 04                           cmp ecx, dword ptr [eax + 4]
00485B49  0f 8e 6d 01 00 00                  jle 0x485cbc
00485B4F  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485B52  83 7a 08 01                        cmp dword ptr [edx + 8], 1
00485B56  75 0b                              jne 0x485b63
00485B58  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00485B5B  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485B5E  89 48 04                           mov dword ptr [eax + 4], ecx
00485B61  eb 1d                              jmp 0x485b80
00485B63  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485B66  8b 42 04                           mov eax, dword ptr [edx + 4]
00485B69  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
00485B6C  03 41 08                           add eax, dword ptr [ecx + 8]
00485B6F  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485B72  89 42 04                           mov dword ptr [edx + 4], eax
00485B75  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00485B78  8b 48 04                           mov ecx, dword ptr [eax + 4]
00485B7B  3b 4d 08                           cmp ecx, dword ptr [ebp + 8]
00485B7E  7e e3                              jle 0x485b63
00485B80  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485B83  83 7a 04 00                        cmp dword ptr [edx + 4], 0
00485B87  0f 8e f1 00 00 00                  jle 0x485c7e
00485B8D  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00485B90  8b 48 04                           mov ecx, dword ptr [eax + 4]
00485B93  89 4d f0                           mov dword ptr [ebp - 0x10], ecx
00485B96  8b 55 f0                           mov edx, dword ptr [ebp - 0x10]
00485B99  6b d2 48                           imul edx, edx, 0x48
00485B9C  83 c2 04                           add edx, 4
00485B9F  52                                 push edx
00485BA0  e8 03 66 01 00                     call 0x49c1a8
00485BA5  83 c4 04                           add esp, 4
00485BA8  89 45 e0                           mov dword ptr [ebp - 0x20], eax
00485BAB  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
00485BB2  83 7d e0 00                        cmp dword ptr [ebp - 0x20], 0
00485BB6  74 2f                              je 0x485be7
00485BB8  68 70 48 48 00                     push 0x484870
00485BBD  68 30 4e 48 00                     push 0x484e30
00485BC2  8b 45 e0                           mov eax, dword ptr [ebp - 0x20]
00485BC5  8b 4d f0                           mov ecx, dword ptr [ebp - 0x10]
00485BC8  89 08                              mov dword ptr [eax], ecx
00485BCA  8b 55 f0                           mov edx, dword ptr [ebp - 0x10]
00485BCD  52                                 push edx
00485BCE  6a 48                              push 0x48
00485BD0  8b 45 e0                           mov eax, dword ptr [ebp - 0x20]
00485BD3  83 c0 04                           add eax, 4
00485BD6  50                                 push eax
00485BD7  e8 f7 69 01 00                     call 0x49c5d3
00485BDC  8b 4d e0                           mov ecx, dword ptr [ebp - 0x20]
00485BDF  83 c1 04                           add ecx, 4
00485BE2  89 4d c8                           mov dword ptr [ebp - 0x38], ecx
00485BE5  eb 07                              jmp 0x485bee
00485BE7  c7 45 c8 00 00 00 00               mov dword ptr [ebp - 0x38], 0
00485BEE  8b 55 c8                           mov edx, dword ptr [ebp - 0x38]
00485BF1  89 55 e4                           mov dword ptr [ebp - 0x1c], edx
00485BF4  c7 45 fc ff ff ff ff               mov dword ptr [ebp - 4], 0xffffffff
00485BFB  8b 45 e4                           mov eax, dword ptr [ebp - 0x1c]
00485BFE  89 45 ec                           mov dword ptr [ebp - 0x14], eax
00485C01  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
00485C04  83 79 0c 00                        cmp dword ptr [ecx + 0xc], 0
00485C08  74 69                              je 0x485c73
00485C0A  c7 45 e8 00 00 00 00               mov dword ptr [ebp - 0x18], 0
00485C11  eb 09                              jmp 0x485c1c
00485C13  8b 55 e8                           mov edx, dword ptr [ebp - 0x18]
00485C16  83 c2 01                           add edx, 1
00485C19  89 55 e8                           mov dword ptr [ebp - 0x18], edx
00485C1C  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00485C1F  8b 4d e8                           mov ecx, dword ptr [ebp - 0x18]
00485C22  3b 08                              cmp ecx, dword ptr [eax]
00485C24  7d 22                              jge 0x485c48
00485C26  8b 55 e8                           mov edx, dword ptr [ebp - 0x18]
00485C29  6b d2 48                           imul edx, edx, 0x48
00485C2C  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00485C2F  8b 70 0c                           mov esi, dword ptr [eax + 0xc]
00485C32  03 f2                              add esi, edx
00485C34  8b 4d e8                           mov ecx, dword ptr [ebp - 0x18]
00485C37  6b c9 48                           imul ecx, ecx, 0x48
00485C3A  8b 7d ec                           mov edi, dword ptr [ebp - 0x14]
00485C3D  03 f9                              add edi, ecx
00485C3F  b9 12 00 00 00                     mov ecx, 0x12
00485C44  f3 a5                              rep movsd dword ptr es:[edi], dword ptr [esi]
00485C46  eb cb                              jmp 0x485c13
00485C48  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485C4B  8b 42 0c                           mov eax, dword ptr [edx + 0xc]
00485C4E  89 45 d8                           mov dword ptr [ebp - 0x28], eax
00485C51  8b 4d d8                           mov ecx, dword ptr [ebp - 0x28]
00485C54  89 4d dc                           mov dword ptr [ebp - 0x24], ecx
00485C57  83 7d dc 00                        cmp dword ptr [ebp - 0x24], 0
00485C5B  74 0f                              je 0x485c6c
00485C5D  6a 03                              push 3
00485C5F  8b 4d dc                           mov ecx, dword ptr [ebp - 0x24]
00485C62  e8 99 eb ff ff                     call 0x484800
00485C67  89 45 c4                           mov dword ptr [ebp - 0x3c], eax
00485C6A  eb 07                              jmp 0x485c73
00485C6C  c7 45 c4 00 00 00 00               mov dword ptr [ebp - 0x3c], 0
00485C73  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485C76  8b 45 ec                           mov eax, dword ptr [ebp - 0x14]
00485C79  89 42 0c                           mov dword ptr [edx + 0xc], eax
00485C7C  eb 3e                              jmp 0x485cbc
00485C7E  8b 4d cc                           mov ecx, dword ptr [ebp - 0x34]
00485C81  83 79 0c 00                        cmp dword ptr [ecx + 0xc], 0
00485C85  74 2b                              je 0x485cb2
00485C87  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485C8A  8b 42 0c                           mov eax, dword ptr [edx + 0xc]
00485C8D  89 45 d0                           mov dword ptr [ebp - 0x30], eax
00485C90  8b 4d d0                           mov ecx, dword ptr [ebp - 0x30]
00485C93  89 4d d4                           mov dword ptr [ebp - 0x2c], ecx
00485C96  83 7d d4 00                        cmp dword ptr [ebp - 0x2c], 0
00485C9A  74 0f                              je 0x485cab
00485C9C  6a 03                              push 3
00485C9E  8b 4d d4                           mov ecx, dword ptr [ebp - 0x2c]
00485CA1  e8 5a eb ff ff                     call 0x484800
00485CA6  89 45 c0                           mov dword ptr [ebp - 0x40], eax
00485CA9  eb 07                              jmp 0x485cb2
00485CAB  c7 45 c0 00 00 00 00               mov dword ptr [ebp - 0x40], 0
00485CB2  8b 55 cc                           mov edx, dword ptr [ebp - 0x34]
00485CB5  c7 42 0c 00 00 00 00               mov dword ptr [edx + 0xc], 0
00485CBC  8b 45 cc                           mov eax, dword ptr [ebp - 0x34]
00485CBF  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485CC2  89 08                              mov dword ptr [eax], ecx
00485CC4  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00485CC7  64 89 0d 00 00 00 00               mov dword ptr fs:[0], ecx
00485CCE  5f                                 pop edi
00485CCF  5e                                 pop esi
00485CD0  8b e5                              mov esp, ebp
00485CD2  5d                                 pop ebp
00485CD3  c2 04 00                           ret 4

; VA 0x00484420 file offset 541728 length 78 SHA256 05ce7cfb94487d1e8b795191e81debf740b2f227c5858cd6bc4717d9644738cc
00484420  55                                 push ebp
00484421  8b ec                              mov ebp, esp
00484423  6a ff                              push -1
00484425  68 79 e8 4a 00                     push 0x4ae879
0048442A  64 a1 00 00 00 00                  mov eax, dword ptr fs:[0]
00484430  50                                 push eax
00484431  64 89 25 00 00 00 00               mov dword ptr fs:[0], esp
00484438  51                                 push ecx
00484439  89 4d f0                           mov dword ptr [ebp - 0x10], ecx
0048443C  8b 4d f0                           mov ecx, dword ptr [ebp - 0x10]
0048443F  e8 7c 00 00 00                     call 0x4844c0
00484444  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
0048444B  8b 4d f0                           mov ecx, dword ptr [ebp - 0x10]
0048444E  83 c1 18                           add ecx, 0x18
00484451  e8 3a 60 f9 ff                     call 0x41a490
00484456  c7 45 fc ff ff ff ff               mov dword ptr [ebp - 4], 0xffffffff
0048445D  8b 45 f0                           mov eax, dword ptr [ebp - 0x10]
00484460  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00484463  64 89 0d 00 00 00 00               mov dword ptr fs:[0], ecx
0048446A  8b e5                              mov esp, ebp
0048446C  5d                                 pop ebp
0048446D  c3                                 ret

; VA 0x00484C10 file offset 543760 length 22 SHA256 90c0aa7faa40cddbc21c573c8301978496fb82ff811a68c2c28dd6a094a55521
00484C10  55                                 push ebp
00484C11  8b ec                              mov ebp, esp
00484C13  51                                 push ecx
00484C14  89 4d fc                           mov dword ptr [ebp - 4], ecx
00484C17  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00484C1A  e8 11 00 00 00                     call 0x484c30
00484C1F  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00484C22  8b e5                              mov esp, ebp
00484C24  5d                                 pop ebp
00484C25  c3                                 ret

; VA 0x0047C5B0 file offset 509360 length 35 SHA256 eb9a915170340852600a26fc3e9885821597850ec73975d7a02a5d4a932d5c0b
0047C5B0  55                                 push ebp
0047C5B1  8b ec                              mov ebp, esp
0047C5B3  51                                 push ecx
0047C5B4  89 4d fc                           mov dword ptr [ebp - 4], ecx
0047C5B7  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047C5BA  83 c1 04                           add ecx, 4
0047C5BD  e8 2e 02 00 00                     call 0x47c7f0
0047C5C2  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047C5C5  c7 40 04 00 00 00 00               mov dword ptr [eax + 4], 0
0047C5CC  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047C5CF  8b e5                              mov esp, ebp
0047C5D1  5d                                 pop ebp
0047C5D2  c3                                 ret

; VA 0x00480BB0 file offset 527280 length 54 SHA256 657dbef004ae2a8d230c1d82eb021ca4dfaedfcf59599a6e9c8d666594c0f454
00480BB0  55                                 push ebp
00480BB1  8b ec                              mov ebp, esp
00480BB3  51                                 push ecx
00480BB4  89 4d fc                           mov dword ptr [ebp - 4], ecx
00480BB7  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00480BBA  e8 c1 19 f8 ff                     call 0x402580
00480BBF  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00480BC2  dd 18                              fstp qword ptr [eax]
00480BC4  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00480BC7  e8 b4 19 f8 ff                     call 0x402580
00480BCC  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00480BCF  dd 59 08                           fstp qword ptr [ecx + 8]
00480BD2  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00480BD5  e8 a6 19 f8 ff                     call 0x402580
00480BDA  8b 55 fc                           mov edx, dword ptr [ebp - 4]
00480BDD  dd 5a 10                           fstp qword ptr [edx + 0x10]
00480BE0  8b e5                              mov esp, ebp
00480BE2  5d                                 pop ebp
00480BE3  c2 04 00                           ret 4

; VA 0x0047C810 file offset 509968 length 43 SHA256 8a0197f8967529dc61c70dbabdbf3d5fda58c65ff481a1b3d974cf37a008fcab
0047C810  55                                 push ebp
0047C811  8b ec                              mov ebp, esp
0047C813  51                                 push ecx
0047C814  89 4d fc                           mov dword ptr [ebp - 4], ecx
0047C817  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047C81A  83 38 00                           cmp dword ptr [eax], 0
0047C81D  75 07                              jne 0x47c826
0047C81F  b8 10 00 00 00                     mov eax, 0x10
0047C824  eb 11                              jmp 0x47c837
0047C826  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047C829  83 39 01                           cmp dword ptr [ecx], 1
0047C82C  75 07                              jne 0x47c835
0047C82E  b8 00 01 00 00                     mov eax, 0x100
0047C833  eb 02                              jmp 0x47c837
0047C835  33 c0                              xor eax, eax
0047C837  8b e5                              mov esp, ebp
0047C839  5d                                 pop ebp
0047C83A  c3                                 ret

; VA 0x004011A0 file offset 4512 length 39 SHA256 c946d75a8e84b4c660af2c6b118aaef8aba4dbcbe0a5a4e6dd820282e646e700
004011A0  55                                 push ebp
004011A1  8b ec                              mov ebp, esp
004011A3  8b 45 10                           mov eax, dword ptr [ebp + 0x10]
004011A6  83 e8 01                           sub eax, 1
004011A9  89 45 10                           mov dword ptr [ebp + 0x10], eax
004011AC  83 7d 10 00                        cmp dword ptr [ebp + 0x10], 0
004011B0  7c 11                              jl 0x4011c3
004011B2  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
004011B5  ff 55 14                           call dword ptr [ebp + 0x14]
004011B8  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
004011BB  03 4d 0c                           add ecx, dword ptr [ebp + 0xc]
004011BE  89 4d 08                           mov dword ptr [ebp + 8], ecx
004011C1  eb e0                              jmp 0x4011a3
004011C3  5d                                 pop ebp
004011C4  c2 10 00                           ret 0x10

; VA 0x004844C0 file offset 541888 length 53 SHA256 1f0030291fa3a68771361e044671fd4c4ed373926b81c41092ccd2071eedc704
004844C0  55                                 push ebp
004844C1  8b ec                              mov ebp, esp
004844C3  51                                 push ecx
004844C4  89 4d fc                           mov dword ptr [ebp - 4], ecx
004844C7  8b 45 fc                           mov eax, dword ptr [ebp - 4]
004844CA  c7 00 00 00 00 00                  mov dword ptr [eax], 0
004844D0  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
004844D3  c7 41 04 00 00 00 00               mov dword ptr [ecx + 4], 0
004844DA  8b 55 fc                           mov edx, dword ptr [ebp - 4]
004844DD  c7 42 08 01 00 00 00               mov dword ptr [edx + 8], 1
004844E4  8b 45 fc                           mov eax, dword ptr [ebp - 4]
004844E7  c7 40 0c 00 00 00 00               mov dword ptr [eax + 0xc], 0
004844EE  8b 45 fc                           mov eax, dword ptr [ebp - 4]
004844F1  8b e5                              mov esp, ebp
004844F3  5d                                 pop ebp
004844F4  c3                                 ret

; VA 0x00484C30 file offset 543792 length 53 SHA256 1f0030291fa3a68771361e044671fd4c4ed373926b81c41092ccd2071eedc704
00484C30  55                                 push ebp
00484C31  8b ec                              mov ebp, esp
00484C33  51                                 push ecx
00484C34  89 4d fc                           mov dword ptr [ebp - 4], ecx
00484C37  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00484C3A  c7 00 00 00 00 00                  mov dword ptr [eax], 0
00484C40  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00484C43  c7 41 04 00 00 00 00               mov dword ptr [ecx + 4], 0
00484C4A  8b 55 fc                           mov edx, dword ptr [ebp - 4]
00484C4D  c7 42 08 01 00 00 00               mov dword ptr [edx + 8], 1
00484C54  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00484C57  c7 40 0c 00 00 00 00               mov dword ptr [eax + 0xc], 0
00484C5E  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00484C61  8b e5                              mov esp, ebp
00484C63  5d                                 pop ebp
00484C64  c3                                 ret

; VA 0x0047C7F0 file offset 509936 length 14 SHA256 befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c
0047C7F0  55                                 push ebp
0047C7F1  8b ec                              mov ebp, esp
0047C7F3  51                                 push ecx
0047C7F4  89 4d fc                           mov dword ptr [ebp - 4], ecx
0047C7F7  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047C7FA  8b e5                              mov esp, ebp
0047C7FC  5d                                 pop ebp
0047C7FD  c3                                 ret

; VA 0x00484150 file offset 541008 length 16 SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936
00484150  55                                 push ebp
00484151  8b ec                              mov ebp, esp
00484153  51                                 push ecx
00484154  89 4d fc                           mov dword ptr [ebp - 4], ecx
00484157  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0048415A  8b 00                              mov eax, dword ptr [eax]
0048415C  8b e5                              mov esp, ebp
0048415E  5d                                 pop ebp
0048415F  c3                                 ret

; VA 0x004845E0 file offset 542176 length 16 SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936
004845E0  55                                 push ebp
004845E1  8b ec                              mov ebp, esp
004845E3  51                                 push ecx
004845E4  89 4d fc                           mov dword ptr [ebp - 4], ecx
004845E7  8b 45 fc                           mov eax, dword ptr [ebp - 4]
004845EA  8b 00                              mov eax, dword ptr [eax]
004845EC  8b e5                              mov esp, ebp
004845EE  5d                                 pop ebp
004845EF  c3                                 ret

; VA 0x0045B8F0 file offset 375024 length 16 SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936
0045B8F0  55                                 push ebp
0045B8F1  8b ec                              mov ebp, esp
0045B8F3  51                                 push ecx
0045B8F4  89 4d fc                           mov dword ptr [ebp - 4], ecx
0045B8F7  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0045B8FA  8b 00                              mov eax, dword ptr [eax]
0045B8FC  8b e5                              mov esp, ebp
0045B8FE  5d                                 pop ebp
0045B8FF  c3                                 ret

; VA 0x00468CD0 file offset 429264 length 16 SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936
00468CD0  55                                 push ebp
00468CD1  8b ec                              mov ebp, esp
00468CD3  51                                 push ecx
00468CD4  89 4d fc                           mov dword ptr [ebp - 4], ecx
00468CD7  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00468CDA  8b 00                              mov eax, dword ptr [eax]
00468CDC  8b e5                              mov esp, ebp
00468CDE  5d                                 pop ebp
00468CDF  c3                                 ret

; VA 0x0048252D file offset 533805 length 30 SHA256 0e251290ebe7640b7655f043e1ad35014e1cfb34afa25376d6927fd91fe15145
0048252D  55                                 push ebp
0048252E  8b ec                              mov ebp, esp
00482530  51                                 push ecx
00482531  89 4d fc                           mov dword ptr [ebp - 4], ecx
00482534  68 80 00 00 00                     push 0x80
00482539  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0048253C  50                                 push eax
0048253D  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00482540  e8 fb b3 ff ff                     call 0x47d940
00482545  8b e5                              mov esp, ebp
00482547  5d                                 pop ebp
00482548  c2 04 00                           ret 4

; VA 0x00401B80 file offset 7040 length 14 SHA256 befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c
00401B80  55                                 push ebp
00401B81  8b ec                              mov ebp, esp
00401B83  51                                 push ecx
00401B84  89 4d fc                           mov dword ptr [ebp - 4], ecx
00401B87  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00401B8A  8b e5                              mov esp, ebp
00401B8C  5d                                 pop ebp
00401B8D  c3                                 ret
