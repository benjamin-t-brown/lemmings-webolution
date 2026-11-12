; Drowning behavior original code evidence
; Executable SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00408700 <.text+0x7700>:
  408700:	55                   	push   ebp
  408701:	8b ec                	mov    ebp,esp
  408703:	51                   	push   ecx
  408704:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  408707:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  40870a:	e8 81 fc 00 00       	call   0x418390
  40870f:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  408712:	c7 00 70 05 4b 00    	mov    DWORD PTR [eax],0x4b0570
  408718:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  40871b:	c7 41 14 22 00 00 00 	mov    DWORD PTR [ecx+0x14],0x22
  408722:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  408725:	c6 42 11 00          	mov    BYTE PTR [edx+0x11],0x0
  408729:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  40872c:	8b e5                	mov    esp,ebp
  40872e:	5d                   	pop    ebp
  40872f:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0040874c <.text+0x774c>:
  40874c:	55                   	push   ebp
  40874d:	8b ec                	mov    ebp,esp
  40874f:	51                   	push   ecx
  408750:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  408753:	68 30 f1 4b 00       	push   0x4bf130
  408758:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  40875b:	e8 c5 75 02 00       	call   0x42fd25
  408760:	8b e5                	mov    esp,ebp
  408762:	5d                   	pop    ebp
  408763:	c2 04 00             	ret    0x4


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00408766 <.text+0x7766>:
  408766:	55                   	push   ebp
  408767:	8b ec                	mov    ebp,esp
  408769:	51                   	push   ecx
  40876a:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  40876d:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  408770:	8b 48 04             	mov    ecx,DWORD PTR [eax+0x4]
  408773:	51                   	push   ecx
  408774:	8b 10                	mov    edx,DWORD PTR [eax]
  408776:	52                   	push   edx
  408777:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  40877a:	83 c1 78             	add    ecx,0x78
  40877d:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  408780:	8b 50 78             	mov    edx,DWORD PTR [eax+0x78]
  408783:	ff 52 2c             	call   DWORD PTR [edx+0x2c]
  408786:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  408789:	8b 4d 0c             	mov    ecx,DWORD PTR [ebp+0xc]
  40878c:	dd 40 08             	fld    QWORD PTR [eax+0x8]
  40878f:	dc 01                	fadd   QWORD PTR [ecx]
  408791:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  408794:	dd 5a 08             	fstp   QWORD PTR [edx+0x8]
  408797:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  40879a:	c7 00 00 00 00 00    	mov    DWORD PTR [eax],0x0
  4087a0:	c7 40 04 00 00 00 00 	mov    DWORD PTR [eax+0x4],0x0
  4087a7:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4087aa:	dd 41 08             	fld    QWORD PTR [ecx+0x8]
  4087ad:	dc 1d 68 05 4b 00    	fcomp  QWORD PTR ds:0x4b0568
  4087b3:	df e0                	fnstsw ax
  4087b5:	f6 c4 41             	test   ah,0x41
  4087b8:	75 07                	jne    0x4087c1
  4087ba:	b8 02 00 00 00       	mov    eax,0x2
  4087bf:	eb 02                	jmp    0x4087c3
  4087c1:	33 c0                	xor    eax,eax
  4087c3:	8b e5                	mov    esp,ebp
  4087c5:	5d                   	pop    ebp
  4087c6:	c2 08 00             	ret    0x8


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00401320 <.text+0x320>:
  401320:	55                   	push   ebp
  401321:	8b ec                	mov    ebp,esp
  401323:	51                   	push   ecx
  401324:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  401327:	8b e5                	mov    esp,ebp
  401329:	5d                   	pop    ebp
  40132a:	c2 04 00             	ret    0x4
