; Reference SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Static disassembly and original bytes; Capstone 5.0.7 x86 32-bit.

; VA 0x0042E7F3, file offset 0x2E7F3, length 212, SHA-256 05d08169889a56bf462db385c68e81f287fd2cd5a11d61ee0021aa72206f9f40
0042E7F3  55                              push ebp
0042E7F4  8b ec                           mov ebp, esp
0042E7F6  83 ec 24                        sub esp, 0x24
0042E7F9  89 4d e4                        mov dword ptr [ebp - 0x1c], ecx
0042E7FC  8b 45 08                        mov eax, dword ptr [ebp + 8]
0042E7FF  50                              push eax
0042E800  8b 4d e4                        mov ecx, dword ptr [ebp - 0x1c]
0042E803  e8 45 05 00 00                  call 0x42ed4d
0042E808  89 45 fc                        mov dword ptr [ebp - 4], eax
0042E80B  83 7d fc 00                     cmp dword ptr [ebp - 4], 0
0042E80F  75 07                           jne 0x42e818
0042E811  32 c0                           xor al, al
0042E813  e9 a9 00 00 00                  jmp 0x42e8c1
0042E818  8b 4d e4                        mov ecx, dword ptr [ebp - 0x1c]
0042E81B  51                              push ecx
0042E81C  8b 55 fc                        mov edx, dword ptr [ebp - 4]
0042E81F  8b 02                           mov eax, dword ptr [edx]
0042E821  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
0042E824  ff 50 0c                        call dword ptr [eax + 0xc]
0042E827  8b 4d e4                        mov ecx, dword ptr [ebp - 0x1c]
0042E82A  8b 55 fc                        mov edx, dword ptr [ebp - 4]
0042E82D  3b 91 40 03 00 00               cmp edx, dword ptr [ecx + 0x340]
0042E833  75 41                           jne 0x42e876
0042E835  8b 45 e4                        mov eax, dword ptr [ebp - 0x1c]
0042E838  8b 88 40 03 00 00               mov ecx, dword ptr [eax + 0x340]
0042E83E  89 4d f0                        mov dword ptr [ebp - 0x10], ecx
0042E841  8b 55 f0                        mov edx, dword ptr [ebp - 0x10]
0042E844  89 55 f4                        mov dword ptr [ebp - 0xc], edx
0042E847  83 7d f4 00                     cmp dword ptr [ebp - 0xc], 0
0042E84B  74 11                           je 0x42e85e
0042E84D  6a 01                           push 1
0042E84F  8b 45 f4                        mov eax, dword ptr [ebp - 0xc]
0042E852  8b 10                           mov edx, dword ptr [eax]
0042E854  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0042E857  ff 12                           call dword ptr [edx]
0042E859  89 45 e0                        mov dword ptr [ebp - 0x20], eax
0042E85C  eb 07                           jmp 0x42e865
0042E85E  c7 45 e0 00 00 00 00            mov dword ptr [ebp - 0x20], 0
0042E865  8b 45 e4                        mov eax, dword ptr [ebp - 0x1c]
0042E868  c7 80 40 03 00 00 00 00 00 00   mov dword ptr [eax + 0x340], 0
0042E872  b0 01                           mov al, 1
0042E874  eb 4b                           jmp 0x42e8c1
0042E876  8d 4d fc                        lea ecx, [ebp - 4]
0042E879  51                              push ecx
0042E87A  8b 4d e4                        mov ecx, dword ptr [ebp - 0x1c]
0042E87D  81 c1 74 03 00 00               add ecx, 0x374
0042E883  e8 28 19 00 00                  call 0x4301b0
0042E888  89 45 f8                        mov dword ptr [ebp - 8], eax
0042E88B  8b 55 f8                        mov edx, dword ptr [ebp - 8]
0042E88E  8b 02                           mov eax, dword ptr [edx]
0042E890  89 45 e8                        mov dword ptr [ebp - 0x18], eax
0042E893  8b 4d e8                        mov ecx, dword ptr [ebp - 0x18]
0042E896  89 4d ec                        mov dword ptr [ebp - 0x14], ecx
0042E899  83 7d ec 00                     cmp dword ptr [ebp - 0x14], 0
0042E89D  74 11                           je 0x42e8b0
0042E89F  6a 01                           push 1
0042E8A1  8b 55 ec                        mov edx, dword ptr [ebp - 0x14]
0042E8A4  8b 02                           mov eax, dword ptr [edx]
0042E8A6  8b 4d ec                        mov ecx, dword ptr [ebp - 0x14]
0042E8A9  ff 10                           call dword ptr [eax]
0042E8AB  89 45 dc                        mov dword ptr [ebp - 0x24], eax
0042E8AE  eb 07                           jmp 0x42e8b7
0042E8B0  c7 45 dc 00 00 00 00            mov dword ptr [ebp - 0x24], 0
0042E8B7  8b 4d f8                        mov ecx, dword ptr [ebp - 8]
0042E8BA  e8 31 16 00 00                  call 0x42fef0
0042E8BF  b0 01                           mov al, 1
0042E8C1  8b e5                           mov esp, ebp
0042E8C3  5d                              pop ebp
0042E8C4  c2 04 00                        ret 4

; VA 0x0042ED4D, file offset 0x2ED4D, length 128, SHA-256 7489b69dcd94fe9bb072ea93b9ef01d0e38db74713ae7fa5d0301095ff94354b
0042ED4D  55                              push ebp
0042ED4E  8b ec                           mov ebp, esp
0042ED50  83 ec 0c                        sub esp, 0xc
0042ED53  89 4d f4                        mov dword ptr [ebp - 0xc], ecx
0042ED56  8b 45 f4                        mov eax, dword ptr [ebp - 0xc]
0042ED59  8b 88 40 03 00 00               mov ecx, dword ptr [eax + 0x340]
0042ED5F  e8 0c 1d 00 00                  call 0x430a70
0042ED64  3b 45 08                        cmp eax, dword ptr [ebp + 8]
0042ED67  75 0b                           jne 0x42ed74
0042ED69  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0042ED6C  8b 81 40 03 00 00               mov eax, dword ptr [ecx + 0x340]
0042ED72  eb 53                           jmp 0x42edc7
0042ED74  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0042ED77  81 c1 74 03 00 00               add ecx, 0x374
0042ED7D  e8 ee 13 00 00                  call 0x430170
0042ED82  89 45 fc                        mov dword ptr [ebp - 4], eax
0042ED85  8b 55 fc                        mov edx, dword ptr [ebp - 4]
0042ED88  52                              push edx
0042ED89  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0042ED8C  81 c1 74 03 00 00               add ecx, 0x374
0042ED92  e8 f9 13 00 00                  call 0x430190
0042ED97  25 ff 00 00 00                  and eax, 0xff
0042ED9C  85 c0                           test eax, eax
0042ED9E  75 25                           jne 0x42edc5
0042EDA0  8b 45 fc                        mov eax, dword ptr [ebp - 4]
0042EDA3  8b 08                           mov ecx, dword ptr [eax]
0042EDA5  89 4d f8                        mov dword ptr [ebp - 8], ecx
0042EDA8  8b 4d f8                        mov ecx, dword ptr [ebp - 8]
0042EDAB  e8 c0 1c 00 00                  call 0x430a70
0042EDB0  3b 45 08                        cmp eax, dword ptr [ebp + 8]
0042EDB3  75 05                           jne 0x42edba
0042EDB5  8b 45 f8                        mov eax, dword ptr [ebp - 8]
0042EDB8  eb 0d                           jmp 0x42edc7
0042EDBA  8b 55 fc                        mov edx, dword ptr [ebp - 4]
0042EDBD  8b 42 04                        mov eax, dword ptr [edx + 4]
0042EDC0  89 45 fc                        mov dword ptr [ebp - 4], eax
0042EDC3  eb c0                           jmp 0x42ed85
0042EDC5  33 c0                           xor eax, eax
0042EDC7  8b e5                           mov esp, ebp
0042EDC9  5d                              pop ebp
0042EDCA  c2 04 00                        ret 4

; VA 0x004301B0, file offset 0x301B0, length 77, SHA-256 4135f72767e6100f8a64e843517ff962f260dd139ebc4932b910fc50a8d94535
004301B0  55                              push ebp
004301B1  8b ec                           mov ebp, esp
004301B3  83 ec 08                        sub esp, 8
004301B6  89 4d f8                        mov dword ptr [ebp - 8], ecx
004301B9  8b 4d f8                        mov ecx, dword ptr [ebp - 8]
004301BC  e8 af ff ff ff                  call 0x430170
004301C1  89 45 fc                        mov dword ptr [ebp - 4], eax
004301C4  8b 45 fc                        mov eax, dword ptr [ebp - 4]
004301C7  50                              push eax
004301C8  8b 4d f8                        mov ecx, dword ptr [ebp - 8]
004301CB  e8 c0 ff ff ff                  call 0x430190
004301D0  25 ff 00 00 00                  and eax, 0xff
004301D5  85 c0                           test eax, eax
004301D7  75 1c                           jne 0x4301f5
004301D9  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
004301DC  8b 55 08                        mov edx, dword ptr [ebp + 8]
004301DF  8b 01                           mov eax, dword ptr [ecx]
004301E1  3b 02                           cmp eax, dword ptr [edx]
004301E3  75 05                           jne 0x4301ea
004301E5  8b 45 fc                        mov eax, dword ptr [ebp - 4]
004301E8  eb 0d                           jmp 0x4301f7
004301EA  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
004301ED  8b 51 04                        mov edx, dword ptr [ecx + 4]
004301F0  89 55 fc                        mov dword ptr [ebp - 4], edx
004301F3  eb cf                           jmp 0x4301c4
004301F5  33 c0                           xor eax, eax
004301F7  8b e5                           mov esp, ebp
004301F9  5d                              pop ebp
004301FA  c2 04 00                        ret 4

; VA 0x0042FEF0, file offset 0x2FEF0, length 83, SHA-256 7bc9223189ff9857ef8bcb775ae6a6aec1dffe2f66e3631cb95f3c7d32d675d5
0042FEF0  55                              push ebp
0042FEF1  8b ec                           mov ebp, esp
0042FEF3  83 ec 10                        sub esp, 0x10
0042FEF6  89 4d f4                        mov dword ptr [ebp - 0xc], ecx
0042FEF9  8b 45 f4                        mov eax, dword ptr [ebp - 0xc]
0042FEFC  8b 48 08                        mov ecx, dword ptr [eax + 8]
0042FEFF  8b 55 f4                        mov edx, dword ptr [ebp - 0xc]
0042FF02  8b 42 04                        mov eax, dword ptr [edx + 4]
0042FF05  89 41 04                        mov dword ptr [ecx + 4], eax
0042FF08  8b 4d f4                        mov ecx, dword ptr [ebp - 0xc]
0042FF0B  8b 51 04                        mov edx, dword ptr [ecx + 4]
0042FF0E  8b 45 f4                        mov eax, dword ptr [ebp - 0xc]
0042FF11  8b 48 08                        mov ecx, dword ptr [eax + 8]
0042FF14  89 4a 08                        mov dword ptr [edx + 8], ecx
0042FF17  8b 55 f4                        mov edx, dword ptr [ebp - 0xc]
0042FF1A  89 55 f8                        mov dword ptr [ebp - 8], edx
0042FF1D  8b 45 f8                        mov eax, dword ptr [ebp - 8]
0042FF20  89 45 fc                        mov dword ptr [ebp - 4], eax
0042FF23  83 7d fc 00                     cmp dword ptr [ebp - 4], 0
0042FF27  74 0f                           je 0x42ff38
0042FF29  6a 01                           push 1
0042FF2B  8b 4d fc                        mov ecx, dword ptr [ebp - 4]
0042FF2E  e8 1d 00 00 00                  call 0x42ff50
0042FF33  89 45 f0                        mov dword ptr [ebp - 0x10], eax
0042FF36  eb 07                           jmp 0x42ff3f
0042FF38  c7 45 f0 00 00 00 00            mov dword ptr [ebp - 0x10], 0
0042FF3F  8b e5                           mov esp, ebp
0042FF41  5d                              pop ebp
0042FF42  c3                              ret


; Clear primary: VA 0042E725, offset 190245, size 101, SHA-256 c5cf8bc9b8e06c427119c28fe34ca99f56383d36896ff33105f0297a01841942
0042E725  55                               push ebp
0042E726  8bec                             mov ebp, esp
0042E728  83ec10                           sub esp, 0x10
0042E72B  894df4                           mov dword ptr [ebp - 0xc], ecx
0042E72E  8b45f4                           mov eax, dword ptr [ebp - 0xc]
0042E731  50                               push eax
0042E732  8b4df4                           mov ecx, dword ptr [ebp - 0xc]
0042E735  8b8940030000                     mov ecx, dword ptr [ecx + 0x340]
0042E73B  8b55f4                           mov edx, dword ptr [ebp - 0xc]
0042E73E  8b8240030000                     mov eax, dword ptr [edx + 0x340]
0042E744  8b10                             mov edx, dword ptr [eax]
0042E746  ff520c                           call dword ptr [edx + 0xc]
0042E749  8b45f4                           mov eax, dword ptr [ebp - 0xc]
0042E74C  8b8840030000                     mov ecx, dword ptr [eax + 0x340]
0042E752  894df8                           mov dword ptr [ebp - 8], ecx
0042E755  8b55f8                           mov edx, dword ptr [ebp - 8]
0042E758  8955fc                           mov dword ptr [ebp - 4], edx
0042E75B  837dfc00                         cmp dword ptr [ebp - 4], 0
0042E75F  7411                             je 0x42e772
0042E761  6a01                             push 1
0042E763  8b45fc                           mov eax, dword ptr [ebp - 4]
0042E766  8b10                             mov edx, dword ptr [eax]
0042E768  8b4dfc                           mov ecx, dword ptr [ebp - 4]
0042E76B  ff12                             call dword ptr [edx]
0042E76D  8945f0                           mov dword ptr [ebp - 0x10], eax
0042E770  eb07                             jmp 0x42e779
0042E772  c745f000000000                   mov dword ptr [ebp - 0x10], 0
0042E779  8b45f4                           mov eax, dword ptr [ebp - 0xc]
0042E77C  c7804003000000000000             mov dword ptr [eax + 0x340], 0
0042E786  8be5                             mov esp, ebp
0042E788  5d                               pop ebp
0042E789  c3                               ret
