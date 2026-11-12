; EXE SHA256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; VA 004306F0, PE file offset 198384, length 106, SHA256 c9ac76eb0fdf7ee30c39be22275c04a317e44ba8e990c4aa6d4030d792b31b72

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004306f0 <.text+0x2f6f0>:
  4306f0:	55                   	push   ebp
  4306f1:	8b ec                	mov    ebp,esp
  4306f3:	83 ec 08             	sub    esp,0x8
  4306f6:	89 4d f8             	mov    DWORD PTR [ebp-0x8],ecx
  4306f9:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  4306fc:	83 c1 04             	add    ecx,0x4
  4306ff:	e8 5c 00 00 00       	call   0x430760
  430704:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
  430707:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  43070a:	50                   	push   eax
  43070b:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  43070e:	83 c1 04             	add    ecx,0x4
  430711:	e8 6a 00 00 00       	call   0x430780
  430716:	25 ff 00 00 00       	and    eax,0xff
  43071b:	85 c0                	test   eax,eax
  43071d:	75 16                	jne    0x430735
  43071f:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  430722:	8b 51 04             	mov    edx,DWORD PTR [ecx+0x4]
  430725:	3b 55 08             	cmp    edx,DWORD PTR [ebp+0x8]
  430728:	74 0b                	je     0x430735
  43072a:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  43072d:	8b 48 0c             	mov    ecx,DWORD PTR [eax+0xc]
  430730:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  430733:	eb d2                	jmp    0x430707
  430735:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  430738:	52                   	push   edx
  430739:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  43073c:	83 c1 04             	add    ecx,0x4
  43073f:	e8 3c 00 00 00       	call   0x430780
  430744:	25 ff 00 00 00       	and    eax,0xff
  430749:	85 c0                	test   eax,eax
  43074b:	74 04                	je     0x430751
  43074d:	33 c0                	xor    eax,eax
  43074f:	eb 03                	jmp    0x430754
  430751:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  430754:	8b e5                	mov    esp,ebp
  430756:	5d                   	pop    ebp
  430757:	c2 04 00             	ret    0x4
; VA 00430760, PE file offset 198496, length 17, SHA256 a26cd3064709e57ee483cfbbb2753f8da501718ce4ced1e834a5123dd2fcaa3d

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00430760 <.text+0x2f760>:
  430760:	55                   	push   ebp
  430761:	8b ec                	mov    ebp,esp
  430763:	51                   	push   ecx
  430764:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  430767:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  43076a:	8b 40 0c             	mov    eax,DWORD PTR [eax+0xc]
  43076d:	8b e5                	mov    esp,ebp
  43076f:	5d                   	pop    ebp
  430770:	c3                   	ret
; VA 00430780, PE file offset 198528, length 29, SHA256 279bb61281a416389bf3313e5358634b5d927b0d349db8573597cecd1ad504d0

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00430780 <.text+0x2f780>:
  430780:	55                   	push   ebp
  430781:	8b ec                	mov    ebp,esp
  430783:	51                   	push   ecx
  430784:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  430787:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  43078a:	83 c0 14             	add    eax,0x14
  43078d:	33 c9                	xor    ecx,ecx
  43078f:	39 45 08             	cmp    DWORD PTR [ebp+0x8],eax
  430792:	0f 94 c1             	sete   cl
  430795:	8a c1                	mov    al,cl
  430797:	8b e5                	mov    esp,ebp
  430799:	5d                   	pop    ebp
  43079a:	c2 04 00             	ret    0x4
