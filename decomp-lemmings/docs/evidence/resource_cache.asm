; EXE SHA256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; VA 0042FF90, fileoffset 196496, length 178, SHA256 bb1d636337916f8eb82b34865e31ffd9b3b511a623178daaf0028d0eb8fcdeaa

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0042ff90 <.text+0x2ef90>:
  42ff90:	55                   	push   ebp
  42ff91:	8b ec                	mov    ebp,esp
  42ff93:	83 ec 0c             	sub    esp,0xc
  42ff96:	89 4d f4             	mov    DWORD PTR [ebp-0xc],ecx
  42ff99:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  42ff9c:	50                   	push   eax
  42ff9d:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  42ffa0:	83 c1 30             	add    ecx,0x30
  42ffa3:	e8 f8 07 00 00       	call   0x4307a0
  42ffa8:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
  42ffab:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  42ffae:	83 b9 80 00 00 00 00 	cmp    DWORD PTR [ecx+0x80],0x0
  42ffb5:	75 5f                	jne    0x430016
  42ffb7:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  42ffba:	52                   	push   edx
  42ffbb:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  42ffbe:	e8 2d 07 00 00       	call   0x4306f0
  42ffc3:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  42ffc6:	89 81 80 00 00 00    	mov    DWORD PTR [ecx+0x80],eax
  42ffcc:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  42ffcf:	83 ba 80 00 00 00 00 	cmp    DWORD PTR [edx+0x80],0x0
  42ffd6:	75 3e                	jne    0x430016
  42ffd8:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  42ffdb:	50                   	push   eax
  42ffdc:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  42ffdf:	e8 7c 06 00 00       	call   0x430660
  42ffe4:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
  42ffe7:	83 7d 0c 00          	cmp    DWORD PTR [ebp+0xc],0x0
  42ffeb:	74 0f                	je     0x42fffc
  42ffed:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  42fff0:	e8 0b 65 fd ff       	call   0x406500
  42fff5:	50                   	push   eax
  42fff6:	ff 55 0c             	call   DWORD PTR [ebp+0xc]
  42fff9:	83 c4 04             	add    esp,0x4
  42fffc:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  42ffff:	51                   	push   ecx
  430000:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  430003:	e8 e8 06 00 00       	call   0x4306f0
  430008:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  43000b:	89 82 80 00 00 00    	mov    DWORD PTR [edx+0x80],eax
  430011:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
  430014:	eb 26                	jmp    0x43003c
  430016:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  430019:	8b 88 80 00 00 00    	mov    ecx,DWORD PTR [eax+0x80]
  43001f:	8b 51 08             	mov    edx,DWORD PTR [ecx+0x8]
  430022:	83 c2 01             	add    edx,0x1
  430025:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  430028:	8b 88 80 00 00 00    	mov    ecx,DWORD PTR [eax+0x80]
  43002e:	89 51 08             	mov    DWORD PTR [ecx+0x8],edx
  430031:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  430034:	8b 82 80 00 00 00    	mov    eax,DWORD PTR [edx+0x80]
  43003a:	8b 00                	mov    eax,DWORD PTR [eax]
  43003c:	8b e5                	mov    esp,ebp
  43003e:	5d                   	pop    ebp
  43003f:	c2 08 00             	ret    0x8
; VA 00430050, fileoffset 196688, length 55, SHA256 930ae18523a7fe6bde9b56b59842e51d84eb14a72f7229844573fa34821222f1

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00430050 <.text+0x2f050>:
  430050:	55                   	push   ebp
  430051:	8b ec                	mov    ebp,esp
  430053:	83 ec 08             	sub    esp,0x8
  430056:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  430059:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  43005c:	50                   	push   eax
  43005d:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  430060:	83 c1 30             	add    ecx,0x30
  430063:	e8 38 07 00 00       	call   0x4307a0
  430068:	8b 88 80 00 00 00    	mov    ecx,DWORD PTR [eax+0x80]
  43006e:	83 c1 08             	add    ecx,0x8
  430071:	89 4d f8             	mov    DWORD PTR [ebp-0x8],ecx
  430074:	8b 55 f8             	mov    edx,DWORD PTR [ebp-0x8]
  430077:	8b 02                	mov    eax,DWORD PTR [edx]
  430079:	83 e8 01             	sub    eax,0x1
  43007c:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  43007f:	89 01                	mov    DWORD PTR [ecx],eax
  430081:	8b e5                	mov    esp,ebp
  430083:	5d                   	pop    ebp
  430084:	c2 04 00             	ret    0x4
; VA 00446010, fileoffset 286736, length 47, SHA256 4d72c8cd7a7947d6b7170dcc631bb5cf29aba80f71e3d6563cb5a42d14d9e204

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00446010 <.text+0x45010>:
  446010:	55                   	push   ebp
  446011:	8b ec                	mov    ebp,esp
  446013:	83 ec 08             	sub    esp,0x8
  446016:	89 4d f8             	mov    DWORD PTR [ebp-0x8],ecx
  446019:	6a 00                	push   0x0
  44601b:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  44601e:	50                   	push   eax
  44601f:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  446022:	e8 69 9f fe ff       	call   0x42ff90
  446027:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
  44602a:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  44602d:	51                   	push   ecx
  44602e:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  446031:	e8 1a a0 fe ff       	call   0x430050
  446036:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  446039:	8b e5                	mov    esp,ebp
  44603b:	5d                   	pop    ebp
  44603c:	c2 04 00             	ret    0x4
; VA 004307A0, fileoffset 198560, length 30, SHA256 3f6d2d5c7711f92761fa78394636a1ea22836c94e313ce6fc2378ef7e44d1c34

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004307a0 <.text+0x2f7a0>:
  4307a0:	55                   	push   ebp
  4307a1:	8b ec                	mov    ebp,esp
  4307a3:	51                   	push   ecx
  4307a4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4307a7:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  4307aa:	69 c0 88 00 00 00    	imul   eax,eax,0x88
  4307b0:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4307b3:	8b 51 0c             	mov    edx,DWORD PTR [ecx+0xc]
  4307b6:	03 c2                	add    eax,edx
  4307b8:	8b e5                	mov    esp,ebp
  4307ba:	5d                   	pop    ebp
  4307bb:	c2 04 00             	ret    0x4
; VA 00406500, fileoffset 25856, length 14, SHA256 befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c

C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00406500 <.text+0x5500>:
  406500:	55                   	push   ebp
  406501:	8b ec                	mov    ebp,esp
  406503:	51                   	push   ecx
  406504:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  406507:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  40650a:	8b e5                	mov    esp,ebp
  40650c:	5d                   	pop    ebp
  40650d:	c3                   	ret
