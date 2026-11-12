; Original EXE SHA256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Full byte spans; strcmp includes three alignment bytes between analyzed blocks.

; VA 0x00474519 file offset 476441 length 68 SHA256 c61b03952da7744188c3fbbd46abde58bf908baab536c708024531d3a14584b8
00474519  55                                 push ebp
0047451A  8b ec                              mov ebp, esp
0047451C  8b 45 08                           mov eax, dword ptr [ebp + 8]
0047451F  50                                 push eax
00474520  b9 48 cb 58 00                     mov ecx, 0x58cb48
00474525  e8 06 56 fd ff                     call 0x449b30
0047452A  25 ff 00 00 00                     and eax, 0xff
0047452F  85 c0                              test eax, eax
00474531  75 15                              jne 0x474548
00474533  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00474536  51                                 push ecx
00474537  b9 48 cb 58 00                     mov ecx, 0x58cb48
0047453C  e8 cf b8 fe ff                     call 0x45fe10
00474541  8b 55 0c                           mov edx, dword ptr [ebp + 0xc]
00474544  89 02                              mov dword ptr [edx], eax
00474546  eb 13                              jmp 0x47455b
00474548  8b 45 08                           mov eax, dword ptr [ebp + 8]
0047454B  50                                 push eax
0047454C  b9 48 cb 58 00                     mov ecx, 0x58cb48
00474551  e8 7a c2 fe ff                     call 0x4607d0
00474556  8b 4d 0c                           mov ecx, dword ptr [ebp + 0xc]
00474559  89 01                              mov dword ptr [ecx], eax
0047455B  5d                                 pop ebp
0047455C  c3                                 ret

; VA 0x00484E30 file offset 544304 length 14 SHA256 befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c
00484E30  55                                 push ebp
00484E31  8b ec                              mov ebp, esp
00484E33  51                                 push ecx
00484E34  89 4d fc                           mov dword ptr [ebp - 4], ecx
00484E37  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00484E3A  8b e5                              mov esp, ebp
00484E3C  5d                                 pop ebp
00484E3D  c3                                 ret

; VA 0x00485CE0 file offset 548064 length 269 SHA256 f823e4183fb5cf6dddcddf56827596e577eb4923ac6739de17c92a45db59e887
00485CE0  55                                 push ebp
00485CE1  8b ec                              mov ebp, esp
00485CE3  81 ec 08 01 00 00                  sub esp, 0x108
00485CE9  89 8d f8 fe ff ff                  mov dword ptr [ebp - 0x108], ecx
00485CEF  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485CF2  e8 69 c8 f7 ff                     call 0x402560
00485CF7  8b 8d f8 fe ff ff                  mov ecx, dword ptr [ebp - 0x108]
00485CFD  89 01                              mov dword ptr [ecx], eax
00485CFF  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485D02  e8 59 c8 f7 ff                     call 0x402560
00485D07  8b 95 f8 fe ff ff                  mov edx, dword ptr [ebp - 0x108]
00485D0D  89 42 04                           mov dword ptr [edx + 4], eax
00485D10  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485D13  e8 48 c8 f7 ff                     call 0x402560
00485D18  8b 8d f8 fe ff ff                  mov ecx, dword ptr [ebp - 0x108]
00485D1E  89 41 08                           mov dword ptr [ecx + 8], eax
00485D21  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485D24  e8 37 c8 f7 ff                     call 0x402560
00485D29  8b 95 f8 fe ff ff                  mov edx, dword ptr [ebp - 0x108]
00485D2F  89 42 0c                           mov dword ptr [edx + 0xc], eax
00485D32  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485D35  e8 46 c8 f7 ff                     call 0x402580
00485D3A  8b 85 f8 fe ff ff                  mov eax, dword ptr [ebp - 0x108]
00485D40  dd 58 10                           fstp qword ptr [eax + 0x10]
00485D43  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485D46  e8 35 c8 f7 ff                     call 0x402580
00485D4B  8b 8d f8 fe ff ff                  mov ecx, dword ptr [ebp - 0x108]
00485D51  dd 59 18                           fstp qword ptr [ecx + 0x18]
00485D54  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485D57  e8 24 c8 f7 ff                     call 0x402580
00485D5C  8b 95 f8 fe ff ff                  mov edx, dword ptr [ebp - 0x108]
00485D62  dd 5a 20                           fstp qword ptr [edx + 0x20]
00485D65  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485D68  e8 13 c8 f7 ff                     call 0x402580
00485D6D  8b 85 f8 fe ff ff                  mov eax, dword ptr [ebp - 0x108]
00485D73  dd 58 28                           fstp qword ptr [eax + 0x28]
00485D76  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485D79  e8 e2 c7 f7 ff                     call 0x402560
00485D7E  8b 8d f8 fe ff ff                  mov ecx, dword ptr [ebp - 0x108]
00485D84  89 41 30                           mov dword ptr [ecx + 0x30], eax
00485D87  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485D8A  e8 d1 c7 f7 ff                     call 0x402560
00485D8F  8b 95 f8 fe ff ff                  mov edx, dword ptr [ebp - 0x108]
00485D95  89 42 34                           mov dword ptr [edx + 0x34], eax
00485D98  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485D9B  e8 c0 c7 f7 ff                     call 0x402560
00485DA0  8b 8d f8 fe ff ff                  mov ecx, dword ptr [ebp - 0x108]
00485DA6  89 41 38                           mov dword ptr [ecx + 0x38], eax
00485DA9  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485DAC  e8 af c7 f7 ff                     call 0x402560
00485DB1  8b 95 f8 fe ff ff                  mov edx, dword ptr [ebp - 0x108]
00485DB7  89 42 3c                           mov dword ptr [edx + 0x3c], eax
00485DBA  68 04 01 00 00                     push 0x104
00485DBF  8d 85 fc fe ff ff                  lea eax, [ebp - 0x104]
00485DC5  50                                 push eax
00485DC6  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
00485DC9  e8 72 7b ff ff                     call 0x47d940
00485DCE  8b 8d f8 fe ff ff                  mov ecx, dword ptr [ebp - 0x108]
00485DD4  83 c1 40                           add ecx, 0x40
00485DD7  51                                 push ecx
00485DD8  8d 95 fc fe ff ff                  lea edx, [ebp - 0x104]
00485DDE  52                                 push edx
00485DDF  e8 35 e7 fe ff                     call 0x474519
00485DE4  83 c4 08                           add esp, 8
00485DE7  8b e5                              mov esp, ebp
00485DE9  5d                                 pop ebp
00485DEA  c2 04 00                           ret 4

; VA 0x00449B30 file offset 301872 length 136 SHA256 480c2760a29e67ec213fd7b522bd545da387cd837878b5eb8c0ea48a0ef92b45
00449B30  55                                 push ebp
00449B31  8b ec                              mov ebp, esp
00449B33  83 ec 08                           sub esp, 8
00449B36  89 4d f8                           mov dword ptr [ebp - 8], ecx
00449B39  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
00449B40  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
00449B43  83 c1 30                           add ecx, 0x30
00449B46  e8 e5 fd ff ff                     call 0x449930
00449B4B  39 45 fc                           cmp dword ptr [ebp - 4], eax
00449B4E  74 4d                              je 0x449b9d
00449B50  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00449B53  50                                 push eax
00449B54  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
00449B57  83 c1 30                           add ecx, 0x30
00449B5A  e8 a1 25 00 00                     call 0x44c100
00449B5F  33 c9                              xor ecx, ecx
00449B61  8a 88 84 00 00 00                  mov cl, byte ptr [eax + 0x84]
00449B67  85 c9                              test ecx, ecx
00449B69  74 27                              je 0x449b92
00449B6B  8b 55 fc                           mov edx, dword ptr [ebp - 4]
00449B6E  52                                 push edx
00449B6F  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
00449B72  83 c1 30                           add ecx, 0x30
00449B75  e8 86 25 00 00                     call 0x44c100
00449B7A  8b c8                              mov ecx, eax
00449B7C  e8 9f 9b fb ff                     call 0x403720
00449B81  50                                 push eax
00449B82  8b 45 08                           mov eax, dword ptr [ebp + 8]
00449B85  50                                 push eax
00449B86  e8 e5 2c 05 00                     call 0x49c870
00449B8B  83 c4 08                           add esp, 8
00449B8E  85 c0                              test eax, eax
00449B90  74 0b                              je 0x449b9d
00449B92  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00449B95  83 c1 01                           add ecx, 1
00449B98  89 4d fc                           mov dword ptr [ebp - 4], ecx
00449B9B  eb a3                              jmp 0x449b40
00449B9D  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
00449BA0  83 c1 30                           add ecx, 0x30
00449BA3  e8 88 fd ff ff                     call 0x449930
00449BA8  33 d2                              xor edx, edx
00449BAA  39 45 fc                           cmp dword ptr [ebp - 4], eax
00449BAD  0f 95 c2                           setne dl
00449BB0  8a c2                              mov al, dl
00449BB2  8b e5                              mov esp, ebp
00449BB4  5d                                 pop ebp
00449BB5  c2 04 00                           ret 4

; VA 0x004607D0 file offset 395216 length 118 SHA256 6bc543a99f7303cd60e45602087f8fcb13e5f9ec69c5b2ea38e3b9680d2f8835
004607D0  55                                 push ebp
004607D1  8b ec                              mov ebp, esp
004607D3  83 ec 08                           sub esp, 8
004607D6  89 4d f8                           mov dword ptr [ebp - 8], ecx
004607D9  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
004607E0  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
004607E3  83 c1 30                           add ecx, 0x30
004607E6  e8 45 91 fe ff                     call 0x449930
004607EB  39 45 fc                           cmp dword ptr [ebp - 4], eax
004607EE  74 4d                              je 0x46083d
004607F0  8b 45 fc                           mov eax, dword ptr [ebp - 4]
004607F3  50                                 push eax
004607F4  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
004607F7  83 c1 30                           add ecx, 0x30
004607FA  e8 01 b9 fe ff                     call 0x44c100
004607FF  33 c9                              xor ecx, ecx
00460801  8a 88 84 00 00 00                  mov cl, byte ptr [eax + 0x84]
00460807  85 c9                              test ecx, ecx
00460809  74 27                              je 0x460832
0046080B  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0046080E  52                                 push edx
0046080F  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
00460812  83 c1 30                           add ecx, 0x30
00460815  e8 e6 b8 fe ff                     call 0x44c100
0046081A  8b c8                              mov ecx, eax
0046081C  e8 ff 2e fa ff                     call 0x403720
00460821  50                                 push eax
00460822  8b 45 08                           mov eax, dword ptr [ebp + 8]
00460825  50                                 push eax
00460826  e8 45 c0 03 00                     call 0x49c870
0046082B  83 c4 08                           add esp, 8
0046082E  85 c0                              test eax, eax
00460830  74 0b                              je 0x46083d
00460832  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00460835  83 c1 01                           add ecx, 1
00460838  89 4d fc                           mov dword ptr [ebp - 4], ecx
0046083B  eb a3                              jmp 0x4607e0
0046083D  8b 45 fc                           mov eax, dword ptr [ebp - 4]
00460840  8b e5                              mov esp, ebp
00460842  5d                                 pop ebp
00460843  c2 04 00                           ret 4

; VA 0x0045FE10 file offset 392720 length 312 SHA256 9ba048bb4d6cf932957d766706bf389187dd625177162edd8d9909064c89c393
0045FE10  55                                 push ebp
0045FE11  8b ec                              mov ebp, esp
0045FE13  6a ff                              push -1
0045FE15  68 88 d6 4a 00                     push 0x4ad688
0045FE1A  64 a1 00 00 00 00                  mov eax, dword ptr fs:[0]
0045FE20  50                                 push eax
0045FE21  64 89 25 00 00 00 00               mov dword ptr fs:[0], esp
0045FE28  81 ec 28 01 00 00                  sub esp, 0x128
0045FE2E  89 8d dc fe ff ff                  mov dword ptr [ebp - 0x124], ecx
0045FE34  c7 45 f0 00 00 00 00               mov dword ptr [ebp - 0x10], 0
0045FE3B  8b 8d dc fe ff ff                  mov ecx, dword ptr [ebp - 0x124]
0045FE41  83 c1 30                           add ecx, 0x30
0045FE44  e8 e7 9a fe ff                     call 0x449930
0045FE49  39 45 f0                           cmp dword ptr [ebp - 0x10], eax
0045FE4C  7d 29                              jge 0x45fe77
0045FE4E  8b 45 f0                           mov eax, dword ptr [ebp - 0x10]
0045FE51  50                                 push eax
0045FE52  8b 8d dc fe ff ff                  mov ecx, dword ptr [ebp - 0x124]
0045FE58  83 c1 30                           add ecx, 0x30
0045FE5B  e8 a0 4e fb ff                     call 0x414d00
0045FE60  33 c9                              xor ecx, ecx
0045FE62  8a 88 84 00 00 00                  mov cl, byte ptr [eax + 0x84]
0045FE68  85 c9                              test ecx, ecx
0045FE6A  74 0b                              je 0x45fe77
0045FE6C  8b 55 f0                           mov edx, dword ptr [ebp - 0x10]
0045FE6F  83 c2 01                           add edx, 1
0045FE72  89 55 f0                           mov dword ptr [ebp - 0x10], edx
0045FE75  eb c4                              jmp 0x45fe3b
0045FE77  8b 8d dc fe ff ff                  mov ecx, dword ptr [ebp - 0x124]
0045FE7D  83 c1 30                           add ecx, 0x30
0045FE80  e8 ab 9a fe ff                     call 0x449930
0045FE85  39 45 f0                           cmp dword ptr [ebp - 0x10], eax
0045FE88  75 51                              jne 0x45fedb
0045FE8A  8b 45 08                           mov eax, dword ptr [ebp + 8]
0045FE8D  50                                 push eax
0045FE8E  8d 8d 68 ff ff ff                  lea ecx, [ebp - 0x98]
0045FE94  e8 37 04 00 00                     call 0x4602d0
0045FE99  89 85 d8 fe ff ff                  mov dword ptr [ebp - 0x128], eax
0045FE9F  8b 8d d8 fe ff ff                  mov ecx, dword ptr [ebp - 0x128]
0045FEA5  89 8d d4 fe ff ff                  mov dword ptr [ebp - 0x12c], ecx
0045FEAB  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
0045FEB2  8b 95 d4 fe ff ff                  mov edx, dword ptr [ebp - 0x12c]
0045FEB8  52                                 push edx
0045FEB9  8b 8d dc fe ff ff                  mov ecx, dword ptr [ebp - 0x124]
0045FEBF  83 c1 30                           add ecx, 0x30
0045FEC2  e8 69 04 00 00                     call 0x460330
0045FEC7  c7 45 fc ff ff ff ff               mov dword ptr [ebp - 4], 0xffffffff
0045FECE  8d 8d 68 ff ff ff                  lea ecx, [ebp - 0x98]
0045FED4  e8 07 bf fe ff                     call 0x44bde0
0045FED9  eb 5a                              jmp 0x45ff35
0045FEDB  8b 45 08                           mov eax, dword ptr [ebp + 8]
0045FEDE  50                                 push eax
0045FEDF  8d 8d e0 fe ff ff                  lea ecx, [ebp - 0x120]
0045FEE5  e8 e6 03 00 00                     call 0x4602d0
0045FEEA  89 85 d0 fe ff ff                  mov dword ptr [ebp - 0x130], eax
0045FEF0  8b 8d d0 fe ff ff                  mov ecx, dword ptr [ebp - 0x130]
0045FEF6  89 8d cc fe ff ff                  mov dword ptr [ebp - 0x134], ecx
0045FEFC  c7 45 fc 01 00 00 00               mov dword ptr [ebp - 4], 1
0045FF03  8b 95 cc fe ff ff                  mov edx, dword ptr [ebp - 0x134]
0045FF09  52                                 push edx
0045FF0A  8b 45 f0                           mov eax, dword ptr [ebp - 0x10]
0045FF0D  50                                 push eax
0045FF0E  8b 8d dc fe ff ff                  mov ecx, dword ptr [ebp - 0x124]
0045FF14  83 c1 30                           add ecx, 0x30
0045FF17  e8 e4 4d fb ff                     call 0x414d00
0045FF1C  8b c8                              mov ecx, eax
0045FF1E  e8 cd ba fe ff                     call 0x44b9f0
0045FF23  c7 45 fc ff ff ff ff               mov dword ptr [ebp - 4], 0xffffffff
0045FF2A  8d 8d e0 fe ff ff                  lea ecx, [ebp - 0x120]
0045FF30  e8 ab be fe ff                     call 0x44bde0
0045FF35  8b 45 f0                           mov eax, dword ptr [ebp - 0x10]
0045FF38  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0045FF3B  64 89 0d 00 00 00 00               mov dword ptr fs:[0], ecx
0045FF42  8b e5                              mov esp, ebp
0045FF44  5d                                 pop ebp
0045FF45  c2 04 00                           ret 4

; VA 0x004602D0 file offset 393936 length 51 SHA256 bdca2e6b53db9a25fcb765bd7c8e30e18024274776ddccd448b386b9a2fe24fd
004602D0  55                                 push ebp
004602D1  8b ec                              mov ebp, esp
004602D3  51                                 push ecx
004602D4  89 4d fc                           mov dword ptr [ebp - 4], ecx
004602D7  8b 45 08                           mov eax, dword ptr [ebp + 8]
004602DA  50                                 push eax
004602DB  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
004602DE  e8 ad d6 fb ff                     call 0x41d990
004602E3  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
004602E6  c7 81 80 00 00 00 00 00 00 00      mov dword ptr [ecx + 0x80], 0
004602F0  8b 55 fc                           mov edx, dword ptr [ebp - 4]
004602F3  c6 82 84 00 00 00 01               mov byte ptr [edx + 0x84], 1
004602FA  8b 45 fc                           mov eax, dword ptr [ebp - 4]
004602FD  8b e5                              mov esp, ebp
004602FF  5d                                 pop ebp
00460300  c2 04 00                           ret 4

; VA 0x00460330 file offset 394032 length 363 SHA256 48b945acf9ae996eeef85350e99d082dbe0929ebb56d8da6f6488411f340c622
00460330  55                                 push ebp
00460331  8b ec                              mov ebp, esp
00460333  6a ff                              push -1
00460335  68 6b d7 4a 00                     push 0x4ad76b
0046033A  64 a1 00 00 00 00                  mov eax, dword ptr fs:[0]
00460340  50                                 push eax
00460341  64 89 25 00 00 00 00               mov dword ptr fs:[0], esp
00460348  83 ec 28                           sub esp, 0x28
0046034B  89 4d d4                           mov dword ptr [ebp - 0x2c], ecx
0046034E  8b 45 d4                           mov eax, dword ptr [ebp - 0x2c]
00460351  8b 4d d4                           mov ecx, dword ptr [ebp - 0x2c]
00460354  8b 10                              mov edx, dword ptr [eax]
00460356  3b 51 04                           cmp edx, dword ptr [ecx + 4]
00460359  0f 8c 03 01 00 00                  jl 0x460462
0046035F  8b 45 d4                           mov eax, dword ptr [ebp - 0x2c]
00460362  8b 48 04                           mov ecx, dword ptr [eax + 4]
00460365  8b 55 d4                           mov edx, dword ptr [ebp - 0x2c]
00460368  03 4a 08                           add ecx, dword ptr [edx + 8]
0046036B  8b 45 d4                           mov eax, dword ptr [ebp - 0x2c]
0046036E  89 48 04                           mov dword ptr [eax + 4], ecx
00460371  8b 4d d4                           mov ecx, dword ptr [ebp - 0x2c]
00460374  8b 51 04                           mov edx, dword ptr [ecx + 4]
00460377  89 55 f0                           mov dword ptr [ebp - 0x10], edx
0046037A  8b 45 f0                           mov eax, dword ptr [ebp - 0x10]
0046037D  69 c0 88 00 00 00                  imul eax, eax, 0x88
00460383  83 c0 04                           add eax, 4
00460386  50                                 push eax
00460387  e8 1c be 03 00                     call 0x49c1a8
0046038C  83 c4 04                           add esp, 4
0046038F  89 45 e0                           mov dword ptr [ebp - 0x20], eax
00460392  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
00460399  83 7d e0 00                        cmp dword ptr [ebp - 0x20], 0
0046039D  74 32                              je 0x4603d1
0046039F  68 e0 bd 44 00                     push 0x44bde0
004603A4  68 c0 bd 44 00                     push 0x44bdc0
004603A9  8b 4d e0                           mov ecx, dword ptr [ebp - 0x20]
004603AC  8b 55 f0                           mov edx, dword ptr [ebp - 0x10]
004603AF  89 11                              mov dword ptr [ecx], edx
004603B1  8b 45 f0                           mov eax, dword ptr [ebp - 0x10]
004603B4  50                                 push eax
004603B5  68 88 00 00 00                     push 0x88
004603BA  8b 4d e0                           mov ecx, dword ptr [ebp - 0x20]
004603BD  83 c1 04                           add ecx, 4
004603C0  51                                 push ecx
004603C1  e8 0d c2 03 00                     call 0x49c5d3
004603C6  8b 55 e0                           mov edx, dword ptr [ebp - 0x20]
004603C9  83 c2 04                           add edx, 4
004603CC  89 55 d0                           mov dword ptr [ebp - 0x30], edx
004603CF  eb 07                              jmp 0x4603d8
004603D1  c7 45 d0 00 00 00 00               mov dword ptr [ebp - 0x30], 0
004603D8  8b 45 d0                           mov eax, dword ptr [ebp - 0x30]
004603DB  89 45 e4                           mov dword ptr [ebp - 0x1c], eax
004603DE  c7 45 fc ff ff ff ff               mov dword ptr [ebp - 4], 0xffffffff
004603E5  8b 4d e4                           mov ecx, dword ptr [ebp - 0x1c]
004603E8  89 4d e8                           mov dword ptr [ebp - 0x18], ecx
004603EB  c7 45 ec 00 00 00 00               mov dword ptr [ebp - 0x14], 0
004603F2  eb 09                              jmp 0x4603fd
004603F4  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
004603F7  83 c2 01                           add edx, 1
004603FA  89 55 ec                           mov dword ptr [ebp - 0x14], edx
004603FD  8b 45 d4                           mov eax, dword ptr [ebp - 0x2c]
00460400  8b 4d ec                           mov ecx, dword ptr [ebp - 0x14]
00460403  3b 08                              cmp ecx, dword ptr [eax]
00460405  7d 27                              jge 0x46042e
00460407  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0046040A  69 d2 88 00 00 00                  imul edx, edx, 0x88
00460410  8b 45 d4                           mov eax, dword ptr [ebp - 0x2c]
00460413  8b 48 0c                           mov ecx, dword ptr [eax + 0xc]
00460416  03 ca                              add ecx, edx
00460418  51                                 push ecx
00460419  8b 55 ec                           mov edx, dword ptr [ebp - 0x14]
0046041C  69 d2 88 00 00 00                  imul edx, edx, 0x88
00460422  8b 4d e8                           mov ecx, dword ptr [ebp - 0x18]
00460425  03 ca                              add ecx, edx
00460427  e8 c4 b5 fe ff                     call 0x44b9f0
0046042C  eb c6                              jmp 0x4603f4
0046042E  8b 45 d4                           mov eax, dword ptr [ebp - 0x2c]
00460431  8b 48 0c                           mov ecx, dword ptr [eax + 0xc]
00460434  89 4d d8                           mov dword ptr [ebp - 0x28], ecx
00460437  8b 55 d8                           mov edx, dword ptr [ebp - 0x28]
0046043A  89 55 dc                           mov dword ptr [ebp - 0x24], edx
0046043D  83 7d dc 00                        cmp dword ptr [ebp - 0x24], 0
00460441  74 0f                              je 0x460452
00460443  6a 03                              push 3
00460445  8b 4d dc                           mov ecx, dword ptr [ebp - 0x24]
00460448  e8 23 b5 fe ff                     call 0x44b970
0046044D  89 45 cc                           mov dword ptr [ebp - 0x34], eax
00460450  eb 07                              jmp 0x460459
00460452  c7 45 cc 00 00 00 00               mov dword ptr [ebp - 0x34], 0
00460459  8b 45 d4                           mov eax, dword ptr [ebp - 0x2c]
0046045C  8b 4d e8                           mov ecx, dword ptr [ebp - 0x18]
0046045F  89 48 0c                           mov dword ptr [eax + 0xc], ecx
00460462  8b 55 08                           mov edx, dword ptr [ebp + 8]
00460465  52                                 push edx
00460466  8b 45 d4                           mov eax, dword ptr [ebp - 0x2c]
00460469  8b 08                              mov ecx, dword ptr [eax]
0046046B  69 c9 88 00 00 00                  imul ecx, ecx, 0x88
00460471  8b 55 d4                           mov edx, dword ptr [ebp - 0x2c]
00460474  8b 42 0c                           mov eax, dword ptr [edx + 0xc]
00460477  03 c8                              add ecx, eax
00460479  e8 72 b5 fe ff                     call 0x44b9f0
0046047E  8b 4d d4                           mov ecx, dword ptr [ebp - 0x2c]
00460481  8b 11                              mov edx, dword ptr [ecx]
00460483  83 c2 01                           add edx, 1
00460486  8b 45 d4                           mov eax, dword ptr [ebp - 0x2c]
00460489  89 10                              mov dword ptr [eax], edx
0046048B  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0046048E  64 89 0d 00 00 00 00               mov dword ptr fs:[0], ecx
00460495  8b e5                              mov esp, ebp
00460497  5d                                 pop ebp
00460498  c2 04 00                           ret 4

; VA 0x0049C870 file offset 641136 length 132 SHA256 0d2e7ba2802652e4517aec0af8ad2ea5809f9077c7a301a4adb42c2e6d3c789d
0049C870  8b 54 24 04                        mov edx, dword ptr [esp + 4]
0049C874  8b 4c 24 08                        mov ecx, dword ptr [esp + 8]
0049C878  f7 c2 03 00 00 00                  test edx, 3
0049C87E  75 3c                              jne 0x49c8bc
0049C880  8b 02                              mov eax, dword ptr [edx]
0049C882  3a 01                              cmp al, byte ptr [ecx]
0049C884  75 2e                              jne 0x49c8b4
0049C886  0a c0                              or al, al
0049C888  74 26                              je 0x49c8b0
0049C88A  3a 61 01                           cmp ah, byte ptr [ecx + 1]
0049C88D  75 25                              jne 0x49c8b4
0049C88F  0a e4                              or ah, ah
0049C891  74 1d                              je 0x49c8b0
0049C893  c1 e8 10                           shr eax, 0x10
0049C896  3a 41 02                           cmp al, byte ptr [ecx + 2]
0049C899  75 19                              jne 0x49c8b4
0049C89B  0a c0                              or al, al
0049C89D  74 11                              je 0x49c8b0
0049C89F  3a 61 03                           cmp ah, byte ptr [ecx + 3]
0049C8A2  75 10                              jne 0x49c8b4
0049C8A4  83 c1 04                           add ecx, 4
0049C8A7  83 c2 04                           add edx, 4
0049C8AA  0a e4                              or ah, ah
0049C8AC  75 d2                              jne 0x49c880
0049C8AE  8b ff                              mov edi, edi
0049C8B0  33 c0                              xor eax, eax
0049C8B2  c3                                 ret
0049C8B3  90                                 nop
0049C8B4  1b c0                              sbb eax, eax
0049C8B6  d1 e0                              shl eax, 1
0049C8B8  40                                 inc eax
0049C8B9  c3                                 ret
0049C8BA  8b ff                              mov edi, edi
0049C8BC  f7 c2 01 00 00 00                  test edx, 1
0049C8C2  74 14                              je 0x49c8d8
0049C8C4  8a 02                              mov al, byte ptr [edx]
0049C8C6  42                                 inc edx
0049C8C7  3a 01                              cmp al, byte ptr [ecx]
0049C8C9  75 e9                              jne 0x49c8b4
0049C8CB  41                                 inc ecx
0049C8CC  0a c0                              or al, al
0049C8CE  74 e0                              je 0x49c8b0
0049C8D0  f7 c2 02 00 00 00                  test edx, 2
0049C8D6  74 a8                              je 0x49c880
0049C8D8  66 8b 02                           mov ax, word ptr [edx]
0049C8DB  83 c2 02                           add edx, 2
0049C8DE  3a 01                              cmp al, byte ptr [ecx]
0049C8E0  75 d2                              jne 0x49c8b4
0049C8E2  0a c0                              or al, al
0049C8E4  74 ca                              je 0x49c8b0
0049C8E6  3a 61 01                           cmp ah, byte ptr [ecx + 1]
0049C8E9  75 c9                              jne 0x49c8b4
0049C8EB  0a e4                              or ah, ah
0049C8ED  74 c1                              je 0x49c8b0
0049C8EF  83 c1 02                           add ecx, 2
0049C8F2  eb 8c                              jmp 0x49c880

; VA 0x0044B9F0 file offset 309744 length 64 SHA256 e57abbe5406382be5458096218c99e0e7b3d7c5230cc0bc24aa34d513369dba9
0044B9F0  55                                 push ebp
0044B9F1  8b ec                              mov ebp, esp
0044B9F3  51                                 push ecx
0044B9F4  89 4d fc                           mov dword ptr [ebp - 4], ecx
0044B9F7  8b 45 08                           mov eax, dword ptr [ebp + 8]
0044B9FA  50                                 push eax
0044B9FB  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0044B9FE  e8 ed 7c fb ff                     call 0x4036f0
0044BA03  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0044BA06  8b 55 08                           mov edx, dword ptr [ebp + 8]
0044BA09  8b 82 80 00 00 00                  mov eax, dword ptr [edx + 0x80]
0044BA0F  89 81 80 00 00 00                  mov dword ptr [ecx + 0x80], eax
0044BA15  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0044BA18  8b 55 08                           mov edx, dword ptr [ebp + 8]
0044BA1B  8a 82 84 00 00 00                  mov al, byte ptr [edx + 0x84]
0044BA21  88 81 84 00 00 00                  mov byte ptr [ecx + 0x84], al
0044BA27  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0044BA2A  8b e5                              mov esp, ebp
0044BA2C  5d                                 pop ebp
0044BA2D  c2 04 00                           ret 4

; VA 0x0044BDC0 file offset 310720 length 32 SHA256 61b30415985362599c2e0f4a8dfc90ca3b41d0c6ca492414a2e169fb8c25bb40
0044BDC0  55                                 push ebp
0044BDC1  8b ec                              mov ebp, esp
0044BDC3  51                                 push ecx
0044BDC4  89 4d fc                           mov dword ptr [ebp - 4], ecx
0044BDC7  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0044BDCA  e8 c1 e6 fc ff                     call 0x41a490
0044BDCF  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0044BDD2  c6 80 84 00 00 00 00               mov byte ptr [eax + 0x84], 0
0044BDD9  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0044BDDC  8b e5                              mov esp, ebp
0044BDDE  5d                                 pop ebp
0044BDDF  c3                                 ret

; VA 0x0041D990 file offset 121232 length 34 SHA256 601f9b14fddb7b8835707d6e64f8286deed8105db40677f59e03f2d7469e2f1e
0041D990  55                                 push ebp
0041D991  8b ec                              mov ebp, esp
0041D993  51                                 push ecx
0041D994  89 4d fc                           mov dword ptr [ebp - 4], ecx
0041D997  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0041D99A  c6 00 00                           mov byte ptr [eax], 0
0041D99D  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
0041D9A0  51                                 push ecx
0041D9A1  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0041D9A4  e8 87 5d fe ff                     call 0x403730
0041D9A9  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0041D9AC  8b e5                              mov esp, ebp
0041D9AE  5d                                 pop ebp
0041D9AF  c2 04 00                           ret 4

; VA 0x00414D00 file offset 85248 length 30 SHA256 3f6d2d5c7711f92761fa78394636a1ea22836c94e313ce6fc2378ef7e44d1c34
00414D00  55                                 push ebp
00414D01  8b ec                              mov ebp, esp
00414D03  51                                 push ecx
00414D04  89 4d fc                           mov dword ptr [ebp - 4], ecx
00414D07  8b 45 08                           mov eax, dword ptr [ebp + 8]
00414D0A  69 c0 88 00 00 00                  imul eax, eax, 0x88
00414D10  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00414D13  8b 51 0c                           mov edx, dword ptr [ecx + 0xc]
00414D16  03 c2                              add eax, edx
00414D18  8b e5                              mov esp, ebp
00414D1A  5d                                 pop ebp
00414D1B  c2 04 00                           ret 4

; VA 0x0044B970 file offset 309616 length 115 SHA256 0f9f262a02f2c3eec264fc0e5f1e8dbbe7fd7ccce5952208dc6f81a8c425af3c
0044B970  55                                 push ebp
0044B971  8b ec                              mov ebp, esp
0044B973  51                                 push ecx
0044B974  89 4d fc                           mov dword ptr [ebp - 4], ecx
0044B977  8b 45 08                           mov eax, dword ptr [ebp + 8]
0044B97A  83 e0 02                           and eax, 2
0044B97D  85 c0                              test eax, eax
0044B97F  74 3b                              je 0x44b9bc
0044B981  68 e0 bd 44 00                     push 0x44bde0
0044B986  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0044B989  8b 51 fc                           mov edx, dword ptr [ecx - 4]
0044B98C  52                                 push edx
0044B98D  68 88 00 00 00                     push 0x88
0044B992  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0044B995  50                                 push eax
0044B996  e8 ba 0c 05 00                     call 0x49c655
0044B99B  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
0044B99E  83 e1 01                           and ecx, 1
0044B9A1  85 c9                              test ecx, ecx
0044B9A3  74 0f                              je 0x44b9b4
0044B9A5  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0044B9A8  83 ea 04                           sub edx, 4
0044B9AB  52                                 push edx
0044B9AC  e8 8c 07 05 00                     call 0x49c13d
0044B9B1  83 c4 04                           add esp, 4
0044B9B4  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0044B9B7  83 e8 04                           sub eax, 4
0044B9BA  eb 21                              jmp 0x44b9dd
0044B9BC  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0044B9BF  e8 1c 04 00 00                     call 0x44bde0
0044B9C4  8b 45 08                           mov eax, dword ptr [ebp + 8]
0044B9C7  83 e0 01                           and eax, 1
0044B9CA  85 c0                              test eax, eax
0044B9CC  74 0c                              je 0x44b9da
0044B9CE  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0044B9D1  51                                 push ecx
0044B9D2  e8 66 07 05 00                     call 0x49c13d
0044B9D7  83 c4 04                           add esp, 4
0044B9DA  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0044B9DD  8b e5                              mov esp, ebp
0044B9DF  5d                                 pop ebp
0044B9E0  c2 04 00                           ret 4

; VA 0x0044BDE0 file offset 310752 length 19 SHA256 a2e1ed5b46ccb13f634c4d68bd0c91c8f24eaacedd51b8fbf408cf87ddeb9da6
0044BDE0  55                                 push ebp
0044BDE1  8b ec                              mov ebp, esp
0044BDE3  51                                 push ecx
0044BDE4  89 4d fc                           mov dword ptr [ebp - 4], ecx
0044BDE7  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0044BDEA  e8 c1 e6 fc ff                     call 0x41a4b0
0044BDEF  8b e5                              mov esp, ebp
0044BDF1  5d                                 pop ebp
0044BDF2  c3                                 ret

; VA 0x0044C100 file offset 311552 length 30 SHA256 3f6d2d5c7711f92761fa78394636a1ea22836c94e313ce6fc2378ef7e44d1c34
0044C100  55                                 push ebp
0044C101  8b ec                              mov ebp, esp
0044C103  51                                 push ecx
0044C104  89 4d fc                           mov dword ptr [ebp - 4], ecx
0044C107  8b 45 08                           mov eax, dword ptr [ebp + 8]
0044C10A  69 c0 88 00 00 00                  imul eax, eax, 0x88
0044C110  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0044C113  8b 51 0c                           mov edx, dword ptr [ecx + 0xc]
0044C116  03 c2                              add eax, edx
0044C118  8b e5                              mov esp, ebp
0044C11A  5d                                 pop ebp
0044C11B  c2 04 00                           ret 4

; VA 0x00449930 file offset 301360 length 16 SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936
00449930  55                                 push ebp
00449931  8b ec                              mov ebp, esp
00449933  51                                 push ecx
00449934  89 4d fc                           mov dword ptr [ebp - 4], ecx
00449937  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0044993A  8b 00                              mov eax, dword ptr [eax]
0044993C  8b e5                              mov esp, ebp
0044993E  5d                                 pop ebp
0044993F  c3                                 ret

; VA 0x00403720 file offset 14112 length 14 SHA256 befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c
00403720  55                                 push ebp
00403721  8b ec                              mov ebp, esp
00403723  51                                 push ecx
00403724  89 4d fc                           mov dword ptr [ebp - 4], ecx
00403727  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0040372A  8b e5                              mov esp, ebp
0040372C  5d                                 pop ebp
0040372D  c3                                 ret

; VA 0x00403730 file offset 14128 length 29 SHA256 8e80e9e25c15e694e4d6d739cf16c6fc36a6602d56a2280629b070498887476c
00403730  55                                 push ebp
00403731  8b ec                              mov ebp, esp
00403733  51                                 push ecx
00403734  89 4d fc                           mov dword ptr [ebp - 4], ecx
00403737  8b 45 08                           mov eax, dword ptr [ebp + 8]
0040373A  50                                 push eax
0040373B  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0040373E  51                                 push ecx
0040373F  e8 0c 90 09 00                     call 0x49c750
00403744  83 c4 08                           add esp, 8
00403747  8b e5                              mov esp, ebp
00403749  5d                                 pop ebp
0040374A  c2 04 00                           ret 4

; VA 0x004036F0 file offset 14064 length 33 SHA256 d521bf6126450a43521d6cd9e20dee9184fa73048b0363c5196086931fcbf8e2
004036F0  55                                 push ebp
004036F1  8b ec                              mov ebp, esp
004036F3  51                                 push ecx
004036F4  89 4d fc                           mov dword ptr [ebp - 4], ecx
004036F7  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
004036FA  e8 21 00 00 00                     call 0x403720
004036FF  50                                 push eax
00403700  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
00403703  e8 28 00 00 00                     call 0x403730
00403708  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0040370B  8b e5                              mov esp, ebp
0040370D  5d                                 pop ebp
0040370E  c2 04 00                           ret 4

; VA 0x0049C750 file offset 640848 length 240 SHA256 de277c53bfccd0f688de14be2f3a7258615d8c770e4bd3f7cc88a8e29afb91b9
0049C750  57                                 push edi
0049C751  8b 7c 24 08                        mov edi, dword ptr [esp + 8]
0049C755  eb 6a                              jmp 0x49c7c1
0049C757  8d a4 24 00 00 00 00               lea esp, [esp]
0049C75E  8b ff                              mov edi, edi
0049C760  8b 4c 24 04                        mov ecx, dword ptr [esp + 4]
0049C764  57                                 push edi
0049C765  f7 c1 03 00 00 00                  test ecx, 3
0049C76B  74 0f                              je 0x49c77c
0049C76D  8a 01                              mov al, byte ptr [ecx]
0049C76F  41                                 inc ecx
0049C770  84 c0                              test al, al
0049C772  74 3b                              je 0x49c7af
0049C774  f7 c1 03 00 00 00                  test ecx, 3
0049C77A  75 f1                              jne 0x49c76d
0049C77C  8b 01                              mov eax, dword ptr [ecx]
0049C77E  ba ff fe fe 7e                     mov edx, 0x7efefeff
0049C783  03 d0                              add edx, eax
0049C785  83 f0 ff                           xor eax, 0xffffffff
0049C788  33 c2                              xor eax, edx
0049C78A  83 c1 04                           add ecx, 4
0049C78D  a9 00 01 01 81                     test eax, 0x81010100
0049C792  74 e8                              je 0x49c77c
0049C794  8b 41 fc                           mov eax, dword ptr [ecx - 4]
0049C797  84 c0                              test al, al
0049C799  74 23                              je 0x49c7be
0049C79B  84 e4                              test ah, ah
0049C79D  74 1a                              je 0x49c7b9
0049C79F  a9 00 00 ff 00                     test eax, 0xff0000
0049C7A4  74 0e                              je 0x49c7b4
0049C7A6  a9 00 00 00 ff                     test eax, 0xff000000
0049C7AB  74 02                              je 0x49c7af
0049C7AD  eb cd                              jmp 0x49c77c
0049C7AF  8d 79 ff                           lea edi, [ecx - 1]
0049C7B2  eb 0d                              jmp 0x49c7c1
0049C7B4  8d 79 fe                           lea edi, [ecx - 2]
0049C7B7  eb 08                              jmp 0x49c7c1
0049C7B9  8d 79 fd                           lea edi, [ecx - 3]
0049C7BC  eb 03                              jmp 0x49c7c1
0049C7BE  8d 79 fc                           lea edi, [ecx - 4]
0049C7C1  8b 4c 24 0c                        mov ecx, dword ptr [esp + 0xc]
0049C7C5  f7 c1 03 00 00 00                  test ecx, 3
0049C7CB  74 19                              je 0x49c7e6
0049C7CD  8a 11                              mov dl, byte ptr [ecx]
0049C7CF  41                                 inc ecx
0049C7D0  84 d2                              test dl, dl
0049C7D2  74 64                              je 0x49c838
0049C7D4  88 17                              mov byte ptr [edi], dl
0049C7D6  47                                 inc edi
0049C7D7  f7 c1 03 00 00 00                  test ecx, 3
0049C7DD  75 ee                              jne 0x49c7cd
0049C7DF  eb 05                              jmp 0x49c7e6
0049C7E1  89 17                              mov dword ptr [edi], edx
0049C7E3  83 c7 04                           add edi, 4
0049C7E6  ba ff fe fe 7e                     mov edx, 0x7efefeff
0049C7EB  8b 01                              mov eax, dword ptr [ecx]
0049C7ED  03 d0                              add edx, eax
0049C7EF  83 f0 ff                           xor eax, 0xffffffff
0049C7F2  33 c2                              xor eax, edx
0049C7F4  8b 11                              mov edx, dword ptr [ecx]
0049C7F6  83 c1 04                           add ecx, 4
0049C7F9  a9 00 01 01 81                     test eax, 0x81010100
0049C7FE  74 e1                              je 0x49c7e1
0049C800  84 d2                              test dl, dl
0049C802  74 34                              je 0x49c838
0049C804  84 f6                              test dh, dh
0049C806  74 27                              je 0x49c82f
0049C808  f7 c2 00 00 ff 00                  test edx, 0xff0000
0049C80E  74 12                              je 0x49c822
0049C810  f7 c2 00 00 00 ff                  test edx, 0xff000000
0049C816  74 02                              je 0x49c81a
0049C818  eb c7                              jmp 0x49c7e1
0049C81A  89 17                              mov dword ptr [edi], edx
0049C81C  8b 44 24 08                        mov eax, dword ptr [esp + 8]
0049C820  5f                                 pop edi
0049C821  c3                                 ret
0049C822  66 89 17                           mov word ptr [edi], dx
0049C825  8b 44 24 08                        mov eax, dword ptr [esp + 8]
0049C829  c6 47 02 00                        mov byte ptr [edi + 2], 0
0049C82D  5f                                 pop edi
0049C82E  c3                                 ret
0049C82F  66 89 17                           mov word ptr [edi], dx
0049C832  8b 44 24 08                        mov eax, dword ptr [esp + 8]
0049C836  5f                                 pop edi
0049C837  c3                                 ret
0049C838  88 17                              mov byte ptr [edi], dl
0049C83A  8b 44 24 08                        mov eax, dword ptr [esp + 8]
0049C83E  5f                                 pop edi
0049C83F  c3                                 ret

; VA 0x0041A490 file offset 107664 length 20 SHA256 8604081cbfaca3bd2be59e14d7329270bd8c943ec93d07607b4d57e4713e05c1
0041A490  55                                 push ebp
0041A491  8b ec                              mov ebp, esp
0041A493  51                                 push ecx
0041A494  89 4d fc                           mov dword ptr [ebp - 4], ecx
0041A497  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0041A49A  c6 00 00                           mov byte ptr [eax], 0
0041A49D  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0041A4A0  8b e5                              mov esp, ebp
0041A4A2  5d                                 pop ebp
0041A4A3  c3                                 ret

; VA 0x0041A4B0 file offset 107696 length 11 SHA256 012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5
0041A4B0  55                                 push ebp
0041A4B1  8b ec                              mov ebp, esp
0041A4B3  51                                 push ecx
0041A4B4  89 4d fc                           mov dword ptr [ebp - 4], ecx
0041A4B7  8b e5                              mov esp, ebp
0041A4B9  5d                                 pop ebp
0041A4BA  c3                                 ret

; VA 0x0049C5D3 file offset 640467 length 106 SHA256 b80af3235f4307f2af0c9cb3c24267eb2cba55ad9a856b8d070ddbb17f7483d5
0049C5D3  55                                 push ebp
0049C5D4  8b ec                              mov ebp, esp
0049C5D6  6a ff                              push -1
0049C5D8  68 b8 3f 4b 00                     push 0x4b3fb8
0049C5DD  68 00 f2 49 00                     push 0x49f200
0049C5E2  64 a1 00 00 00 00                  mov eax, dword ptr fs:[0]
0049C5E8  50                                 push eax
0049C5E9  64 89 25 00 00 00 00               mov dword ptr fs:[0], esp
0049C5F0  83 ec 10                           sub esp, 0x10
0049C5F3  53                                 push ebx
0049C5F4  56                                 push esi
0049C5F5  57                                 push edi
0049C5F6  33 c0                              xor eax, eax
0049C5F8  89 45 e0                           mov dword ptr [ebp - 0x20], eax
0049C5FB  89 45 fc                           mov dword ptr [ebp - 4], eax
0049C5FE  89 45 e4                           mov dword ptr [ebp - 0x1c], eax
0049C601  8b 45 e4                           mov eax, dword ptr [ebp - 0x1c]
0049C604  3b 45 10                           cmp eax, dword ptr [ebp + 0x10]
0049C607  7d 13                              jge 0x49c61c
0049C609  8b 75 08                           mov esi, dword ptr [ebp + 8]
0049C60C  8b ce                              mov ecx, esi
0049C60E  ff 55 14                           call dword ptr [ebp + 0x14]
0049C611  03 75 0c                           add esi, dword ptr [ebp + 0xc]
0049C614  89 75 08                           mov dword ptr [ebp + 8], esi
0049C617  ff 45 e4                           inc dword ptr [ebp - 0x1c]
0049C61A  eb e5                              jmp 0x49c601
0049C61C  c7 45 e0 01 00 00 00               mov dword ptr [ebp - 0x20], 1
0049C623  83 4d fc ff                        or dword ptr [ebp - 4], 0xffffffff
0049C627  e8 11 00 00 00                     call 0x49c63d
0049C62C  8b 4d f0                           mov ecx, dword ptr [ebp - 0x10]
0049C62F  64 89 0d 00 00 00 00               mov dword ptr fs:[0], ecx
0049C636  5f                                 pop edi
0049C637  5e                                 pop esi
0049C638  5b                                 pop ebx
0049C639  c9                                 leave
0049C63A  c2 14 00                           ret 0x14

; VA 0x0049C63D file offset 640573 length 24 SHA256 3f6add3a0b9e7cae0d44ae4404274273921e43355dbfca561cc807194b6fba45
0049C63D  83 7d e0 00                        cmp dword ptr [ebp - 0x20], 0
0049C641  75 11                              jne 0x49c654
0049C643  ff 75 18                           push dword ptr [ebp + 0x18]
0049C646  ff 75 e4                           push dword ptr [ebp - 0x1c]
0049C649  ff 75 0c                           push dword ptr [ebp + 0xc]
0049C64C  ff 75 08                           push dword ptr [ebp + 8]
0049C64F  e8 81 00 00 00                     call 0x49c6d5
0049C654  c3                                 ret

; VA 0x0049C655 file offset 640597 length 104 SHA256 e7a5b601ea8f3f9000f5f6c522c22bfc271fe8c8645a1ea1a270ef4b5a7ae5f7
0049C655  55                                 push ebp
0049C656  8b ec                              mov ebp, esp
0049C658  6a ff                              push -1
0049C65A  68 c8 3f 4b 00                     push 0x4b3fc8
0049C65F  68 00 f2 49 00                     push 0x49f200
0049C664  64 a1 00 00 00 00                  mov eax, dword ptr fs:[0]
0049C66A  50                                 push eax
0049C66B  64 89 25 00 00 00 00               mov dword ptr fs:[0], esp
0049C672  83 ec 0c                           sub esp, 0xc
0049C675  53                                 push ebx
0049C676  56                                 push esi
0049C677  57                                 push edi
0049C678  83 65 e4 00                        and dword ptr [ebp - 0x1c], 0
0049C67C  8b 75 0c                           mov esi, dword ptr [ebp + 0xc]
0049C67F  8b c6                              mov eax, esi
0049C681  0f af 45 10                        imul eax, dword ptr [ebp + 0x10]
0049C685  01 45 08                           add dword ptr [ebp + 8], eax
0049C688  83 65 fc 00                        and dword ptr [ebp - 4], 0
0049C68C  ff 4d 10                           dec dword ptr [ebp + 0x10]
0049C68F  78 0b                              js 0x49c69c
0049C691  29 75 08                           sub dword ptr [ebp + 8], esi
0049C694  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
0049C697  ff 55 14                           call dword ptr [ebp + 0x14]
0049C69A  eb f0                              jmp 0x49c68c
0049C69C  c7 45 e4 01 00 00 00               mov dword ptr [ebp - 0x1c], 1
0049C6A3  83 4d fc ff                        or dword ptr [ebp - 4], 0xffffffff
0049C6A7  e8 11 00 00 00                     call 0x49c6bd
0049C6AC  8b 4d f0                           mov ecx, dword ptr [ebp - 0x10]
0049C6AF  64 89 0d 00 00 00 00               mov dword ptr fs:[0], ecx
0049C6B6  5f                                 pop edi
0049C6B7  5e                                 pop esi
0049C6B8  5b                                 pop ebx
0049C6B9  c9                                 leave
0049C6BA  c2 10 00                           ret 0x10

; VA 0x0049C6BD file offset 640701 length 24 SHA256 2c7fc6efbbb1db82eb61aeff1e738b410fed12f9b658526a8357e8fde26cce90
0049C6BD  83 7d e4 00                        cmp dword ptr [ebp - 0x1c], 0
0049C6C1  75 11                              jne 0x49c6d4
0049C6C3  ff 75 14                           push dword ptr [ebp + 0x14]
0049C6C6  ff 75 10                           push dword ptr [ebp + 0x10]
0049C6C9  ff 75 0c                           push dword ptr [ebp + 0xc]
0049C6CC  ff 75 08                           push dword ptr [ebp + 8]
0049C6CF  e8 01 00 00 00                     call 0x49c6d5
0049C6D4  c3                                 ret

; VA 0x00402540 file offset 9536 length 30 SHA256 414bda19110faad8da3ab5f3ee7533991a0f1b24d0a139ed7e5d15e056da8e0d
00402540  55                                 push ebp
00402541  8b ec                              mov ebp, esp
00402543  83 ec 08                           sub esp, 8
00402546  89 4d f8                           mov dword ptr [ebp - 8], ecx
00402549  8d 45 fc                           lea eax, [ebp - 4]
0040254C  50                                 push eax
0040254D  6a 01                              push 1
0040254F  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
00402552  e8 71 b5 07 00                     call 0x47dac8
00402557  8a 45 fc                           mov al, byte ptr [ebp - 4]
0040255A  8b e5                              mov esp, ebp
0040255C  5d                                 pop ebp
0040255D  c3                                 ret

; VA 0x00402560 file offset 9568 length 30 SHA256 531b623d06732f119fb38d980a47b04527d771c46b30e7de21eb0aa7e8205dd2
00402560  55                                 push ebp
00402561  8b ec                              mov ebp, esp
00402563  83 ec 08                           sub esp, 8
00402566  89 4d f8                           mov dword ptr [ebp - 8], ecx
00402569  8d 45 fc                           lea eax, [ebp - 4]
0040256C  50                                 push eax
0040256D  6a 04                              push 4
0040256F  8b 4d f8                           mov ecx, dword ptr [ebp - 8]
00402572  e8 51 b5 07 00                     call 0x47dac8
00402577  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0040257A  8b e5                              mov esp, ebp
0040257C  5d                                 pop ebp
0040257D  c3                                 ret

; VA 0x00402580 file offset 9600 length 30 SHA256 408e982b32f871e875d64540f47bb6e5fedb62f6c04bdaf3fcc96a8c67fadd9b
00402580  55                                 push ebp
00402581  8b ec                              mov ebp, esp
00402583  83 ec 0c                           sub esp, 0xc
00402586  89 4d f4                           mov dword ptr [ebp - 0xc], ecx
00402589  8d 45 f8                           lea eax, [ebp - 8]
0040258C  50                                 push eax
0040258D  6a 08                              push 8
0040258F  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
00402592  e8 31 b5 07 00                     call 0x47dac8
00402597  dd 45 f8                           fld qword ptr [ebp - 8]
0040259A  8b e5                              mov esp, ebp
0040259C  5d                                 pop ebp
0040259D  c3                                 ret

; VA 0x0047D940 file offset 514368 length 135 SHA256 61ec9413f6432987d089c747b92e5f5c3f18db8cd70d262b02e0e103a349fdd5
0047D940  55                                 push ebp
0047D941  8b ec                              mov ebp, esp
0047D943  83 ec 0c                           sub esp, 0xc
0047D946  89 4d f4                           mov dword ptr [ebp - 0xc], ecx
0047D949  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047D94C  e8 0f 4c f8 ff                     call 0x402560
0047D951  89 45 f8                           mov dword ptr [ebp - 8], eax
0047D954  8b 45 f8                           mov eax, dword ptr [ebp - 8]
0047D957  3b 45 0c                           cmp eax, dword ptr [ebp + 0xc]
0047D95A  7e 08                              jle 0x47d964
0047D95C  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
0047D95F  c6 01 00                           mov byte ptr [ecx], 0
0047D962  eb 5d                              jmp 0x47d9c1
0047D964  c7 45 fc 00 00 00 00               mov dword ptr [ebp - 4], 0
0047D96B  eb 09                              jmp 0x47d976
0047D96D  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047D970  83 c2 01                           add edx, 1
0047D973  89 55 fc                           mov dword ptr [ebp - 4], edx
0047D976  8b 45 fc                           mov eax, dword ptr [ebp - 4]
0047D979  3b 45 f8                           cmp eax, dword ptr [ebp - 8]
0047D97C  7d 1d                              jge 0x47d99b
0047D97E  8b 4d 0c                           mov ecx, dword ptr [ebp + 0xc]
0047D981  83 e9 01                           sub ecx, 1
0047D984  39 4d fc                           cmp dword ptr [ebp - 4], ecx
0047D987  7d 12                              jge 0x47d99b
0047D989  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047D98C  e8 af 4b f8 ff                     call 0x402540
0047D991  8b 55 08                           mov edx, dword ptr [ebp + 8]
0047D994  03 55 fc                           add edx, dword ptr [ebp - 4]
0047D997  88 02                              mov byte ptr [edx], al
0047D999  eb d2                              jmp 0x47d96d
0047D99B  8b 45 08                           mov eax, dword ptr [ebp + 8]
0047D99E  03 45 fc                           add eax, dword ptr [ebp - 4]
0047D9A1  c6 00 00                           mov byte ptr [eax], 0
0047D9A4  eb 09                              jmp 0x47d9af
0047D9A6  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047D9A9  83 c1 01                           add ecx, 1
0047D9AC  89 4d fc                           mov dword ptr [ebp - 4], ecx
0047D9AF  8b 55 fc                           mov edx, dword ptr [ebp - 4]
0047D9B2  3b 55 f8                           cmp edx, dword ptr [ebp - 8]
0047D9B5  7d 0a                              jge 0x47d9c1
0047D9B7  8b 4d f4                           mov ecx, dword ptr [ebp - 0xc]
0047D9BA  e8 81 4b f8 ff                     call 0x402540
0047D9BF  eb e5                              jmp 0x47d9a6
0047D9C1  8b e5                              mov esp, ebp
0047D9C3  5d                                 pop ebp
0047D9C4  c2 08 00                           ret 8

; VA 0x0047DAC8 file offset 514760 length 29 SHA256 aec40f1e210009a794b263010b78bce2e4aa490c9c5560c2cd8d25330b64dfc7
0047DAC8  55                                 push ebp
0047DAC9  8b ec                              mov ebp, esp
0047DACB  51                                 push ecx
0047DACC  89 4d fc                           mov dword ptr [ebp - 4], ecx
0047DACF  8b 45 0c                           mov eax, dword ptr [ebp + 0xc]
0047DAD2  50                                 push eax
0047DAD3  8b 4d 08                           mov ecx, dword ptr [ebp + 8]
0047DAD6  51                                 push ecx
0047DAD7  8b 4d fc                           mov ecx, dword ptr [ebp - 4]
0047DADA  e8 18 14 00 00                     call 0x47eef7
0047DADF  8b e5                              mov esp, ebp
0047DAE1  5d                                 pop ebp
0047DAE2  c2 08 00                           ret 8
