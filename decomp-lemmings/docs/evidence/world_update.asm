; EXE SHA256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Complete 4423D2 instruction bytes are in world_entity_pass.asm.
; Additional unchanged field accessors used by the full-world native oracle.

; VA 0x446790, offset 288656, length 17, SHA256 d3bc3e852e9ca92d62e0d140c341815bf8b7b6e7b043f9f8b0a5c45665694124

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00446790 <.text+0x45790>:
  446790:	55                   	push   ebp
  446791:	8b ec                	mov    ebp,esp
  446793:	51                   	push   ecx
  446794:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  446797:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  44679a:	8b 40 14             	mov    eax,DWORD PTR [eax+0x14]
  44679d:	8b e5                	mov    esp,ebp
  44679f:	5d                   	pop    ebp
  4467a0:	c3                   	ret

; VA 0x4467B0, offset 288688, length 26, SHA256 a4def840d0e8bd5fa0fb79eaa91c5f83fe3b6d6d79ebe87ce85f0ad2e86a595f

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004467b0 <.text+0x457b0>:
  4467b0:	55                   	push   ebp
  4467b1:	8b ec                	mov    ebp,esp
  4467b3:	51                   	push   ecx
  4467b4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4467b7:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  4467ba:	33 c9                	xor    ecx,ecx
  4467bc:	3b 45 fc             	cmp    eax,DWORD PTR [ebp-0x4]
  4467bf:	0f 94 c1             	sete   cl
  4467c2:	8a c1                	mov    al,cl
  4467c4:	8b e5                	mov    esp,ebp
  4467c6:	5d                   	pop    ebp
  4467c7:	c2 04 00             	ret    0x4

; VA 0x448020, offset 294944, length 27, SHA256 e6c0e76097c9132c5ab757709f42fb216379d49bc564109fb176b40fe049ffd2

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00448020 <.text+0x47020>:
  448020:	55                   	push   ebp
  448021:	8b ec                	mov    ebp,esp
  448023:	51                   	push   ecx
  448024:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  448027:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  44802a:	6b c0 28             	imul   eax,eax,0x28
  44802d:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  448030:	8b 51 0c             	mov    edx,DWORD PTR [ecx+0xc]
  448033:	03 c2                	add    eax,edx
  448035:	8b e5                	mov    esp,ebp
  448037:	5d                   	pop    ebp
  448038:	c2 04 00             	ret    0x4

; VA 0x448040, offset 294976, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00448040 <.text+0x47040>:
  448040:	55                   	push   ebp
  448041:	8b ec                	mov    ebp,esp
  448043:	51                   	push   ecx
  448044:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  448047:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  44804a:	8b 00                	mov    eax,DWORD PTR [eax]
  44804c:	8b e5                	mov    esp,ebp
  44804e:	5d                   	pop    ebp
  44804f:	c3                   	ret

; VA 0x447BC0, offset 293824, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00447bc0 <.text+0x46bc0>:
  447bc0:	55                   	push   ebp
  447bc1:	8b ec                	mov    ebp,esp
  447bc3:	51                   	push   ecx
  447bc4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  447bc7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  447bca:	8b 00                	mov    eax,DWORD PTR [eax]
  447bcc:	8b e5                	mov    esp,ebp
  447bce:	5d                   	pop    ebp
  447bcf:	c3                   	ret

; VA 0x424360, offset 148320, length 30, SHA256 88330f59fef7901916df365575b606d37bb40100278c6cea1afd33f2a6f71d33

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00424360 <.text+0x23360>:
  424360:	55                   	push   ebp
  424361:	8b ec                	mov    ebp,esp
  424363:	51                   	push   ecx
  424364:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  424367:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  42436a:	69 c0 80 01 00 00    	imul   eax,eax,0x180
  424370:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  424373:	8b 51 0c             	mov    edx,DWORD PTR [ecx+0xc]
  424376:	03 c2                	add    eax,edx
  424378:	8b e5                	mov    esp,ebp
  42437a:	5d                   	pop    ebp
  42437b:	c2 04 00             	ret    0x4

; VA 0x4157D0, offset 88016, length 30, SHA256 05d8baff4b94de09473efb8304519cef3d409b513668dc31c0d8defd1319ea64

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004157d0 <.text+0x147d0>:
  4157d0:	55                   	push   ebp
  4157d1:	8b ec                	mov    ebp,esp
  4157d3:	51                   	push   ecx
  4157d4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4157d7:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  4157da:	69 c0 78 01 00 00    	imul   eax,eax,0x178
  4157e0:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4157e3:	8b 51 0c             	mov    edx,DWORD PTR [ecx+0xc]
  4157e6:	03 c2                	add    eax,edx
  4157e8:	8b e5                	mov    esp,ebp
  4157ea:	5d                   	pop    ebp
  4157eb:	c2 04 00             	ret    0x4

; VA 0x4157F0, offset 88048, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004157f0 <.text+0x147f0>:
  4157f0:	55                   	push   ebp
  4157f1:	8b ec                	mov    ebp,esp
  4157f3:	51                   	push   ecx
  4157f4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4157f7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4157fa:	8b 00                	mov    eax,DWORD PTR [eax]
  4157fc:	8b e5                	mov    esp,ebp
  4157fe:	5d                   	pop    ebp
  4157ff:	c3                   	ret
