
C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00467de8 <.text+0x66de8>:
  467de8:	55                   	push   ebp
  467de9:	8b ec                	mov    ebp,esp
  467deb:	53                   	push   ebx
  467dec:	51                   	push   ecx
  467ded:	52                   	push   edx
  467dee:	56                   	push   esi
  467def:	57                   	push   edi
  467df0:	8b 75 08             	mov    esi,DWORD PTR [ebp+0x8]
  467df3:	8b 7d 0c             	mov    edi,DWORD PTR [ebp+0xc]
  467df6:	8b 4d 14             	mov    ecx,DWORD PTR [ebp+0x14]
  467df9:	55                   	push   ebp
  467dfa:	e8 14 00 00 00       	call   0x467e13
  467dff:	5d                   	pop    ebp
  467e00:	b8 00 00 00 00       	mov    eax,0x0
  467e05:	72 05                	jb     0x467e0c
  467e07:	8b c7                	mov    eax,edi
  467e09:	2b 45 0c             	sub    eax,DWORD PTR [ebp+0xc]
  467e0c:	5f                   	pop    edi
  467e0d:	5e                   	pop    esi
  467e0e:	5a                   	pop    edx
  467e0f:	59                   	pop    ecx
  467e10:	5b                   	pop    ebx
  467e11:	c9                   	leave
  467e12:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00467e13 <.text+0x66e13>:
  467e13:	51                   	push   ecx
  467e14:	57                   	push   edi
  467e15:	8d 3d 90 11 4c 00    	lea    edi,ds:0x4c1190
  467e1b:	89 3d 94 0e 4c 00    	mov    DWORD PTR ds:0x4c0e94,edi
  467e21:	83 c7 11             	add    edi,0x11
  467e24:	89 3d 98 0e 4c 00    	mov    DWORD PTR ds:0x4c0e98,edi
  467e2a:	83 c7 03             	add    edi,0x3
  467e2d:	33 db                	xor    ebx,ebx
  467e2f:	e8 83 00 00 00       	call   0x467eb7
  467e34:	72 16                	jb     0x467e4c
  467e36:	bb 01 00 00 00       	mov    ebx,0x1
  467e3b:	e8 77 00 00 00       	call   0x467eb7
  467e40:	72 0a                	jb     0x467e4c
  467e42:	bb 02 00 00 00       	mov    ebx,0x2
  467e47:	e8 6b 00 00 00       	call   0x467eb7
  467e4c:	5f                   	pop    edi
  467e4d:	59                   	pop    ecx
  467e4e:	73 02                	jae    0x467e52
  467e50:	f9                   	stc
  467e51:	c3                   	ret
  467e52:	49                   	dec    ecx
  467e53:	b2 80                	mov    dl,0x80
  467e55:	fc                   	cld
  467e56:	51                   	push   ecx
  467e57:	33 db                	xor    ebx,ebx
  467e59:	e8 8f 01 00 00       	call   0x467fed
  467e5e:	85 c0                	test   eax,eax
  467e60:	74 21                	je     0x467e83
  467e62:	8b c8                	mov    ecx,eax
  467e64:	8b d8                	mov    ebx,eax
  467e66:	b0 01                	mov    al,0x1
  467e68:	8a 26                	mov    ah,BYTE PTR [esi]
  467e6a:	22 e2                	and    ah,dl
  467e6c:	d0 ca                	ror    dl,1
  467e6e:	83 d6 00             	adc    esi,0x0
  467e71:	80 c4 ff             	add    ah,0xff
  467e74:	d0 d0                	rcl    al,1
  467e76:	73 f0                	jae    0x467e68
  467e78:	aa                   	stos   BYTE PTR es:[edi],al
  467e79:	e2 eb                	loop   0x467e66
  467e7b:	8b c3                	mov    eax,ebx
  467e7d:	59                   	pop    ecx
  467e7e:	2b c8                	sub    ecx,eax
  467e80:	72 2e                	jb     0x467eb0
  467e82:	51                   	push   ecx
  467e83:	bb 01 00 00 00       	mov    ebx,0x1
  467e88:	e8 60 01 00 00       	call   0x467fed
  467e8d:	50                   	push   eax
  467e8e:	bb 02 00 00 00       	mov    ebx,0x2
  467e93:	e8 55 01 00 00       	call   0x467fed
  467e98:	8b de                	mov    ebx,esi
  467e9a:	8b f7                	mov    esi,edi
  467e9c:	f9                   	stc
  467e9d:	1b f0                	sbb    esi,eax
  467e9f:	59                   	pop    ecx
  467ea0:	83 c1 02             	add    ecx,0x2
  467ea3:	8b c1                	mov    eax,ecx
  467ea5:	f3 a4                	rep movs BYTE PTR es:[edi],BYTE PTR ds:[esi]
  467ea7:	8b f3                	mov    esi,ebx
  467ea9:	59                   	pop    ecx
  467eaa:	2b c8                	sub    ecx,eax
  467eac:	72 02                	jb     0x467eb0
  467eae:	eb a6                	jmp    0x467e56
  467eb0:	d0 c2                	rol    dl,1
  467eb2:	f5                   	cmc
  467eb3:	83 d6 00             	adc    esi,0x0
  467eb6:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00467eb7 <.text+0x66eb7>:
  467eb7:	8a 06                	mov    al,BYTE PTR [esi]
  467eb9:	46                   	inc    esi
  467eba:	a8 0f                	test   al,0xf
  467ebc:	75 0a                	jne    0x467ec8
  467ebe:	88 83 a2 0e 4c 00    	mov    BYTE PTR [ebx+0x4c0ea2],al
  467ec4:	3c 01                	cmp    al,0x1
  467ec6:	f5                   	cmc
  467ec7:	c3                   	ret
  467ec8:	53                   	push   ebx
  467ec9:	57                   	push   edi
  467eca:	8b 3d 94 0e 4c 00    	mov    edi,DWORD PTR ds:0x4c0e94
  467ed0:	8a f8                	mov    bh,al
  467ed2:	66 83 e0 0f          	and    ax,0xf
  467ed6:	04 02                	add    al,0x2
  467ed8:	8a e8                	mov    ch,al
  467eda:	b1 04                	mov    cl,0x4
  467edc:	d2 ef                	shr    bh,cl
  467ede:	38 fc                	cmp    ah,bh
  467ee0:	73 02                	jae    0x467ee4
  467ee2:	8a e7                	mov    ah,bh
  467ee4:	88 3f                	mov    BYTE PTR [edi],bh
  467ee6:	47                   	inc    edi
  467ee7:	fe cd                	dec    ch
  467ee9:	74 15                	je     0x467f00
  467eeb:	8a 1e                	mov    bl,BYTE PTR [esi]
  467eed:	46                   	inc    esi
  467eee:	8a fb                	mov    bh,bl
  467ef0:	80 e3 0f             	and    bl,0xf
  467ef3:	38 dc                	cmp    ah,bl
  467ef5:	73 02                	jae    0x467ef9
  467ef7:	8a e3                	mov    ah,bl
  467ef9:	88 1f                	mov    BYTE PTR [edi],bl
  467efb:	47                   	inc    edi
  467efc:	fe cd                	dec    ch
  467efe:	75 dc                	jne    0x467edc
  467f00:	a2 9c 0e 4c 00       	mov    ds:0x4c0e9c,al
  467f05:	88 25 9d 0e 4c 00    	mov    BYTE PTR ds:0x4c0e9d,ah
  467f0b:	5f                   	pop    edi
  467f0c:	5b                   	pop    ebx
  467f0d:	56                   	push   esi
  467f0e:	8b f3                	mov    esi,ebx
  467f10:	88 86 a2 0e 4c 00    	mov    BYTE PTR [esi+0x4c0ea2],al
  467f16:	c1 e6 02             	shl    esi,0x2
  467f19:	89 be a6 0e 4c 00    	mov    DWORD PTR [esi+0x4c0ea6],edi
  467f1f:	89 3d 9e 0e 4c 00    	mov    DWORD PTR ds:0x4c0e9e,edi
  467f25:	c6 07 00             	mov    BYTE PTR [edi],0x0
  467f28:	47                   	inc    edi
  467f29:	8b 35 98 0e 4c 00    	mov    esi,DWORD PTR ds:0x4c0e98
  467f2f:	c6 06 00             	mov    BYTE PTR [esi],0x0
  467f32:	b6 80                	mov    dh,0x80
  467f34:	b5 01                	mov    ch,0x1
  467f36:	8b 1d 94 0e 4c 00    	mov    ebx,DWORD PTR ds:0x4c0e94
  467f3c:	8a 0d 9c 0e 4c 00    	mov    cl,BYTE PTR ds:0x4c0e9c
  467f42:	38 2b                	cmp    BYTE PTR [ebx],ch
  467f44:	74 1f                	je     0x467f65
  467f46:	43                   	inc    ebx
  467f47:	fe c9                	dec    cl
  467f49:	75 f7                	jne    0x467f42
  467f4b:	88 0f                	mov    BYTE PTR [edi],cl
  467f4d:	47                   	inc    edi
  467f4e:	d0 ce                	ror    dh,1
  467f50:	83 d6 00             	adc    esi,0x0
  467f53:	8a c6                	mov    al,dh
  467f55:	f6 d0                	not    al
  467f57:	20 06                	and    BYTE PTR [esi],al
  467f59:	fe c5                	inc    ch
  467f5b:	38 2d 9d 0e 4c 00    	cmp    BYTE PTR ds:0x4c0e9d,ch
  467f61:	73 d3                	jae    0x467f36
  467f63:	eb 20                	jmp    0x467f85
  467f65:	8a 25 9c 0e 4c 00    	mov    ah,BYTE PTR ds:0x4c0e9c
  467f6b:	2a e1                	sub    ah,cl
  467f6d:	b0 00                	mov    al,0x0
  467f6f:	66 89 07             	mov    WORD PTR [edi],ax
  467f72:	47                   	inc    edi
  467f73:	47                   	inc    edi
  467f74:	8a d5                	mov    dl,ch
  467f76:	8a c5                	mov    al,ch
  467f78:	84 36                	test   BYTE PTR [esi],dh
  467f7a:	74 0c                	je     0x467f88
  467f7c:	d0 c6                	rol    dh,1
  467f7e:	83 de 00             	sbb    esi,0x0
  467f81:	fe c8                	dec    al
  467f83:	75 f3                	jne    0x467f78
  467f85:	5e                   	pop    esi
  467f86:	f8                   	clc
  467f87:	c3                   	ret
  467f88:	08 36                	or     BYTE PTR [esi],dh
  467f8a:	53                   	push   ebx
  467f8b:	b6 80                	mov    dh,0x80
  467f8d:	8b 35 98 0e 4c 00    	mov    esi,DWORD PTR ds:0x4c0e98
  467f93:	8b 1d 9e 0e 4c 00    	mov    ebx,DWORD PTR ds:0x4c0e9e
  467f99:	84 36                	test   BYTE PTR [esi],dh
  467f9b:	75 03                	jne    0x467fa0
  467f9d:	43                   	inc    ebx
  467f9e:	eb 2c                	jmp    0x467fcc
  467fa0:	8a 03                	mov    al,BYTE PTR [ebx]
  467fa2:	66 25 ff 00          	and    ax,0xff
  467fa6:	75 22                	jne    0x467fca
  467fa8:	8b c7                	mov    eax,edi
  467faa:	2b c3                	sub    eax,ebx
  467fac:	3d 00 01 00 00       	cmp    eax,0x100
  467fb1:	73 22                	jae    0x467fd5
  467fb3:	88 03                	mov    BYTE PTR [ebx],al
  467fb5:	fe ca                	dec    dl
  467fb7:	74 20                	je     0x467fd9
  467fb9:	c6 07 00             	mov    BYTE PTR [edi],0x0
  467fbc:	47                   	inc    edi
  467fbd:	d0 ce                	ror    dh,1
  467fbf:	83 d6 00             	adc    esi,0x0
  467fc2:	8a c6                	mov    al,dh
  467fc4:	f6 d0                	not    al
  467fc6:	20 06                	and    BYTE PTR [esi],al
  467fc8:	eb eb                	jmp    0x467fb5
  467fca:	03 d8                	add    ebx,eax
  467fcc:	d0 ce                	ror    dh,1
  467fce:	83 d6 00             	adc    esi,0x0
  467fd1:	fe ca                	dec    dl
  467fd3:	75 c4                	jne    0x467f99
  467fd5:	5b                   	pop    ebx
  467fd6:	5e                   	pop    esi
  467fd7:	f9                   	stc
  467fd8:	c3                   	ret
  467fd9:	5b                   	pop    ebx
  467fda:	43                   	inc    ebx
  467fdb:	fe c9                	dec    cl
  467fdd:	75 05                	jne    0x467fe4
  467fdf:	e9 67 ff ff ff       	jmp    0x467f4b
  467fe4:	38 2b                	cmp    BYTE PTR [ebx],ch
  467fe6:	75 f2                	jne    0x467fda
  467fe8:	e9 78 ff ff ff       	jmp    0x467f65


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00467fed <.text+0x66fed>:
  467fed:	8b cb                	mov    ecx,ebx
  467fef:	0f b6 83 a2 0e 4c 00 	movzx  eax,BYTE PTR [ebx+0x4c0ea2]
  467ff6:	84 c0                	test   al,al
  467ff8:	75 33                	jne    0x46802d
  467ffa:	b9 00 00 00 00       	mov    ecx,0x0
  467fff:	84 16                	test   BYTE PTR [esi],dl
  468001:	74 09                	je     0x46800c
  468003:	fe c1                	inc    cl
  468005:	d0 ca                	ror    dl,1
  468007:	83 d6 00             	adc    esi,0x0
  46800a:	eb f3                	jmp    0x467fff
  46800c:	d0 ca                	ror    dl,1
  46800e:	83 d6 00             	adc    esi,0x0
  468011:	33 c0                	xor    eax,eax
  468013:	84 c9                	test   cl,cl
  468015:	74 15                	je     0x46802c
  468017:	40                   	inc    eax
  468018:	8a 2e                	mov    ch,BYTE PTR [esi]
  46801a:	22 ea                	and    ch,dl
  46801c:	80 c5 ff             	add    ch,0xff
  46801f:	66 d1 d0             	rcl    ax,1
  468022:	d0 ca                	ror    dl,1
  468024:	83 d6 00             	adc    esi,0x0
  468027:	fe c9                	dec    cl
  468029:	75 ed                	jne    0x468018
  46802b:	48                   	dec    eax
  46802c:	c3                   	ret
  46802d:	03 d9                	add    ebx,ecx
  46802f:	d1 e3                	shl    ebx,1
  468031:	8b 9b a6 0e 4c 00    	mov    ebx,DWORD PTR [ebx+0x4c0ea6]
  468037:	84 16                	test   BYTE PTR [esi],dl
  468039:	75 03                	jne    0x46803e
  46803b:	43                   	inc    ebx
  46803c:	eb 05                	jmp    0x468043
  46803e:	0f b6 03             	movzx  eax,BYTE PTR [ebx]
  468041:	03 d8                	add    ebx,eax
  468043:	d0 ca                	ror    dl,1
  468045:	83 d6 00             	adc    esi,0x0
  468048:	80 3b 00             	cmp    BYTE PTR [ebx],0x0
  46804b:	75 ea                	jne    0x468037
  46804d:	43                   	inc    ebx
  46804e:	8a 0b                	mov    cl,BYTE PTR [ebx]
  468050:	80 f9 02             	cmp    cl,0x2
  468053:	72 1b                	jb     0x468070
  468055:	fe c9                	dec    cl
  468057:	b8 01 00 00 00       	mov    eax,0x1
  46805c:	8a 2e                	mov    ch,BYTE PTR [esi]
  46805e:	22 ea                	and    ch,dl
  468060:	80 c5 ff             	add    ch,0xff
  468063:	66 d1 d0             	rcl    ax,1
  468066:	d0 ca                	ror    dl,1
  468068:	83 d6 00             	adc    esi,0x0
  46806b:	fe c9                	dec    cl
  46806d:	75 ed                	jne    0x46805c
  46806f:	c3                   	ret
  468070:	0f b6 c1             	movzx  eax,cl
  468073:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0045015a <.text+0x4f15a>:
  45015a:	55                   	push   ebp
  45015b:	8b ec                	mov    ebp,esp
  45015d:	6a ff                	push   0xffffffff
  45015f:	68 8c cb 4a 00       	push   0x4acb8c
  450164:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  45016a:	50                   	push   eax
  45016b:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  450172:	81 ec 5c 01 00 00    	sub    esp,0x15c
  450178:	89 8d 98 fe ff ff    	mov    DWORD PTR [ebp-0x168],ecx
  45017e:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  450181:	50                   	push   eax
  450182:	8d 8d bc fe ff ff    	lea    ecx,[ebp-0x144]
  450188:	e8 e6 d9 02 00       	call   0x47db73
  45018d:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  450194:	6a 00                	push   0x0
  450196:	6a 01                	push   0x1
  450198:	8d 8d bc fe ff ff    	lea    ecx,[ebp-0x144]
  45019e:	e8 6a e2 02 00       	call   0x47e40d
  4501a3:	25 ff 00 00 00       	and    eax,0xff
  4501a8:	85 c0                	test   eax,eax
  4501aa:	75 1a                	jne    0x4501c6
  4501ac:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  4501af:	51                   	push   ecx
  4501b0:	6a 21                	push   0x21
  4501b2:	68 8c 02 4c 00       	push   0x4c028c
  4501b7:	e8 46 28 03 00       	call   0x482a02
  4501bc:	83 c4 0c             	add    esp,0xc
  4501bf:	6a 00                	push   0x0
  4501c1:	e8 bd d1 04 00       	call   0x49d383
  4501c6:	8d 8d bc fe ff ff    	lea    ecx,[ebp-0x144]
  4501cc:	e8 56 ec 02 00       	call   0x47ee27
  4501d1:	8b 95 98 fe ff ff    	mov    edx,DWORD PTR [ebp-0x168]
  4501d7:	89 02                	mov    DWORD PTR [edx],eax
  4501d9:	8b 85 98 fe ff ff    	mov    eax,DWORD PTR [ebp-0x168]
  4501df:	8b 08                	mov    ecx,DWORD PTR [eax]
  4501e1:	51                   	push   ecx
  4501e2:	e8 c1 bf 04 00       	call   0x49c1a8
  4501e7:	83 c4 04             	add    esp,0x4
  4501ea:	89 85 a4 fe ff ff    	mov    DWORD PTR [ebp-0x15c],eax
  4501f0:	8b 95 98 fe ff ff    	mov    edx,DWORD PTR [ebp-0x168]
  4501f6:	8b 85 a4 fe ff ff    	mov    eax,DWORD PTR [ebp-0x15c]
  4501fc:	89 42 04             	mov    DWORD PTR [edx+0x4],eax
  4501ff:	8b 8d 98 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x168]
  450205:	8b 11                	mov    edx,DWORD PTR [ecx]
  450207:	52                   	push   edx
  450208:	8b 85 98 fe ff ff    	mov    eax,DWORD PTR [ebp-0x168]
  45020e:	8b 48 04             	mov    ecx,DWORD PTR [eax+0x4]
  450211:	51                   	push   ecx
  450212:	8d 8d bc fe ff ff    	lea    ecx,[ebp-0x144]
  450218:	e8 93 b3 fe ff       	call   0x43b5b0
  45021d:	8d 8d bc fe ff ff    	lea    ecx,[ebp-0x144]
  450223:	e8 1c e6 02 00       	call   0x47e844
  450228:	8b 95 98 fe ff ff    	mov    edx,DWORD PTR [ebp-0x168]
  45022e:	8b 42 04             	mov    eax,DWORD PTR [edx+0x4]
  450231:	33 c9                	xor    ecx,ecx
  450233:	8a 08                	mov    cl,BYTE PTR [eax]
  450235:	83 f9 42             	cmp    ecx,0x42
  450238:	75 2a                	jne    0x450264
  45023a:	8b 95 98 fe ff ff    	mov    edx,DWORD PTR [ebp-0x168]
  450240:	8b 42 04             	mov    eax,DWORD PTR [edx+0x4]
  450243:	33 c9                	xor    ecx,ecx
  450245:	8a 48 01             	mov    cl,BYTE PTR [eax+0x1]
  450248:	83 f9 4d             	cmp    ecx,0x4d
  45024b:	75 17                	jne    0x450264
  45024d:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  450254:	8d 8d bc fe ff ff    	lea    ecx,[ebp-0x144]
  45025a:	e8 c9 d9 02 00       	call   0x47dc28
  45025f:	e9 33 02 00 00       	jmp    0x450497
  450264:	8b 95 98 fe ff ff    	mov    edx,DWORD PTR [ebp-0x168]
  45026a:	8b 42 04             	mov    eax,DWORD PTR [edx+0x4]
  45026d:	83 c0 01             	add    eax,0x1
  450270:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  450273:	8b 8d 98 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x168]
  450279:	c7 01 00 00 00 00    	mov    DWORD PTR [ecx],0x0
  45027f:	c6 85 ac fe ff ff 00 	mov    BYTE PTR [ebp-0x154],0x0
  450286:	eb 0f                	jmp    0x450297
  450288:	8a 95 ac fe ff ff    	mov    dl,BYTE PTR [ebp-0x154]
  45028e:	80 c2 01             	add    dl,0x1
  450291:	88 95 ac fe ff ff    	mov    BYTE PTR [ebp-0x154],dl
  450297:	8b 85 ac fe ff ff    	mov    eax,DWORD PTR [ebp-0x154]
  45029d:	25 ff 00 00 00       	and    eax,0xff
  4502a2:	8b 8d 98 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x168]
  4502a8:	8b 51 04             	mov    edx,DWORD PTR [ecx+0x4]
  4502ab:	33 c9                	xor    ecx,ecx
  4502ad:	8a 0a                	mov    cl,BYTE PTR [edx]
  4502af:	3b c1                	cmp    eax,ecx
  4502b1:	7d 41                	jge    0x4502f4
  4502b3:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  4502b6:	33 c0                	xor    eax,eax
  4502b8:	66 8b 02             	mov    ax,WORD PTR [edx]
  4502bb:	89 85 b0 fe ff ff    	mov    DWORD PTR [ebp-0x150],eax
  4502c1:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  4502c4:	33 d2                	xor    edx,edx
  4502c6:	66 8b 51 02          	mov    dx,WORD PTR [ecx+0x2]
  4502ca:	89 95 a8 fe ff ff    	mov    DWORD PTR [ebp-0x158],edx
  4502d0:	8b 85 98 fe ff ff    	mov    eax,DWORD PTR [ebp-0x168]
  4502d6:	8b 08                	mov    ecx,DWORD PTR [eax]
  4502d8:	03 8d a8 fe ff ff    	add    ecx,DWORD PTR [ebp-0x158]
  4502de:	8b 95 98 fe ff ff    	mov    edx,DWORD PTR [ebp-0x168]
  4502e4:	89 0a                	mov    DWORD PTR [edx],ecx
  4502e6:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  4502e9:	03 85 b0 fe ff ff    	add    eax,DWORD PTR [ebp-0x150]
  4502ef:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  4502f2:	eb 94                	jmp    0x450288
  4502f4:	8b 8d 98 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x168]
  4502fa:	8b 11                	mov    edx,DWORD PTR [ecx]
  4502fc:	52                   	push   edx
  4502fd:	e8 a6 be 04 00       	call   0x49c1a8
  450302:	83 c4 04             	add    esp,0x4
  450305:	89 85 a0 fe ff ff    	mov    DWORD PTR [ebp-0x160],eax
  45030b:	8b 85 a0 fe ff ff    	mov    eax,DWORD PTR [ebp-0x160]
  450311:	89 85 b4 fe ff ff    	mov    DWORD PTR [ebp-0x14c],eax
  450317:	8b 8d 98 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x168]
  45031d:	8b 51 04             	mov    edx,DWORD PTR [ecx+0x4]
  450320:	83 c2 01             	add    edx,0x1
  450323:	89 55 f0             	mov    DWORD PTR [ebp-0x10],edx
  450326:	8b 85 b4 fe ff ff    	mov    eax,DWORD PTR [ebp-0x14c]
  45032c:	89 85 b8 fe ff ff    	mov    DWORD PTR [ebp-0x148],eax
  450332:	c6 85 ac fe ff ff 00 	mov    BYTE PTR [ebp-0x154],0x0
  450339:	eb 0f                	jmp    0x45034a
  45033b:	8a 8d ac fe ff ff    	mov    cl,BYTE PTR [ebp-0x154]
  450341:	80 c1 01             	add    cl,0x1
  450344:	88 8d ac fe ff ff    	mov    BYTE PTR [ebp-0x154],cl
  45034a:	8b 95 ac fe ff ff    	mov    edx,DWORD PTR [ebp-0x154]
  450350:	81 e2 ff 00 00 00    	and    edx,0xff
  450356:	8b 85 98 fe ff ff    	mov    eax,DWORD PTR [ebp-0x168]
  45035c:	8b 48 04             	mov    ecx,DWORD PTR [eax+0x4]
  45035f:	33 c0                	xor    eax,eax
  450361:	8a 01                	mov    al,BYTE PTR [ecx]
  450363:	3b d0                	cmp    edx,eax
  450365:	0f 8d e3 00 00 00    	jge    0x45044e
  45036b:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  45036e:	33 d2                	xor    edx,edx
  450370:	66 8b 11             	mov    dx,WORD PTR [ecx]
  450373:	89 95 b0 fe ff ff    	mov    DWORD PTR [ebp-0x150],edx
  450379:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  45037c:	33 c9                	xor    ecx,ecx
  45037e:	66 8b 48 02          	mov    cx,WORD PTR [eax+0x2]
  450382:	89 8d a8 fe ff ff    	mov    DWORD PTR [ebp-0x158],ecx
  450388:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  45038b:	83 c2 05             	add    edx,0x5
  45038e:	89 55 f0             	mov    DWORD PTR [ebp-0x10],edx
  450391:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  450394:	33 c9                	xor    ecx,ecx
  450396:	8a 48 ff             	mov    cl,BYTE PTR [eax-0x1]
  450399:	83 f9 03             	cmp    ecx,0x3
  45039c:	75 1f                	jne    0x4503bd
  45039e:	8b 95 b0 fe ff ff    	mov    edx,DWORD PTR [ebp-0x150]
  4503a4:	83 ea 05             	sub    edx,0x5
  4503a7:	52                   	push   edx
  4503a8:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  4503ab:	50                   	push   eax
  4503ac:	8b 8d b8 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x148]
  4503b2:	51                   	push   ecx
  4503b3:	e8 58 cc 04 00       	call   0x49d010
  4503b8:	83 c4 0c             	add    esp,0xc
  4503bb:	eb 6a                	jmp    0x450427
  4503bd:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  4503c0:	33 c0                	xor    eax,eax
  4503c2:	8a 42 ff             	mov    al,BYTE PTR [edx-0x1]
  4503c5:	83 f8 04             	cmp    eax,0x4
  4503c8:	74 1a                	je     0x4503e4
  4503ca:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  4503cd:	51                   	push   ecx
  4503ce:	6a 3f                	push   0x3f
  4503d0:	68 b8 02 4c 00       	push   0x4c02b8
  4503d5:	e8 28 26 03 00       	call   0x482a02
  4503da:	83 c4 0c             	add    esp,0xc
  4503dd:	6a 00                	push   0x0
  4503df:	e8 9f cf 04 00       	call   0x49d383
  4503e4:	8b 95 a8 fe ff ff    	mov    edx,DWORD PTR [ebp-0x158]
  4503ea:	52                   	push   edx
  4503eb:	8b 85 b0 fe ff ff    	mov    eax,DWORD PTR [ebp-0x150]
  4503f1:	50                   	push   eax
  4503f2:	8b 8d b8 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x148]
  4503f8:	51                   	push   ecx
  4503f9:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  4503fc:	52                   	push   edx
  4503fd:	e8 e6 79 01 00       	call   0x467de8
  450402:	83 c4 10             	add    esp,0x10
  450405:	3b 85 a8 fe ff ff    	cmp    eax,DWORD PTR [ebp-0x158]
  45040b:	74 1a                	je     0x450427
  45040d:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  450410:	50                   	push   eax
  450411:	6a 40                	push   0x40
  450413:	68 e4 02 4c 00       	push   0x4c02e4
  450418:	e8 e5 25 03 00       	call   0x482a02
  45041d:	83 c4 0c             	add    esp,0xc
  450420:	6a 00                	push   0x0
  450422:	e8 5c cf 04 00       	call   0x49d383
  450427:	8b 8d b0 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x150]
  45042d:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  450430:	8d 44 0a fb          	lea    eax,[edx+ecx*1-0x5]
  450434:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  450437:	8b 8d b8 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x148]
  45043d:	03 8d a8 fe ff ff    	add    ecx,DWORD PTR [ebp-0x158]
  450443:	89 8d b8 fe ff ff    	mov    DWORD PTR [ebp-0x148],ecx
  450449:	e9 ed fe ff ff       	jmp    0x45033b
  45044e:	8b 95 98 fe ff ff    	mov    edx,DWORD PTR [ebp-0x168]
  450454:	8b 42 04             	mov    eax,DWORD PTR [edx+0x4]
  450457:	89 85 9c fe ff ff    	mov    DWORD PTR [ebp-0x164],eax
  45045d:	8b 8d 9c fe ff ff    	mov    ecx,DWORD PTR [ebp-0x164]
  450463:	51                   	push   ecx
  450464:	e8 d4 bc 04 00       	call   0x49c13d
  450469:	83 c4 04             	add    esp,0x4
  45046c:	8b 95 98 fe ff ff    	mov    edx,DWORD PTR [ebp-0x168]
  450472:	8b 85 b4 fe ff ff    	mov    eax,DWORD PTR [ebp-0x14c]
  450478:	89 42 04             	mov    DWORD PTR [edx+0x4],eax
  45047b:	c7 85 b4 fe ff ff 00 	mov    DWORD PTR [ebp-0x14c],0x0
  450482:	00 00 00
  450485:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  45048c:	8d 8d bc fe ff ff    	lea    ecx,[ebp-0x144]
  450492:	e8 91 d7 02 00       	call   0x47dc28
  450497:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  45049a:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  4504a1:	8b e5                	mov    esp,ebp
  4504a3:	5d                   	pop    ebp
  4504a4:	c2 04 00             	ret    0x4


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004743dd <.text+0x733dd>:
  4743dd:	55                   	push   ebp
  4743de:	8b ec                	mov    ebp,esp
  4743e0:	6a ff                	push   0xffffffff
  4743e2:	68 db df 4a 00       	push   0x4adfdb
  4743e7:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  4743ed:	50                   	push   eax
  4743ee:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  4743f5:	83 ec 18             	sub    esp,0x18
  4743f8:	89 4d e0             	mov    DWORD PTR [ebp-0x20],ecx
  4743fb:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  474402:	6a 5c                	push   0x5c
  474404:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  474407:	50                   	push   eax
  474408:	e8 b3 90 02 00       	call   0x49d4c0
  47440d:	83 c4 08             	add    esp,0x8
  474410:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  474413:	83 7d f0 00          	cmp    DWORD PTR [ebp-0x10],0x0
  474417:	75 08                	jne    0x474421
  474419:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  47441c:	89 4d f0             	mov    DWORD PTR [ebp-0x10],ecx
  47441f:	eb 09                	jmp    0x47442a
  474421:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  474424:	83 c2 01             	add    edx,0x1
  474427:	89 55 f0             	mov    DWORD PTR [ebp-0x10],edx
  47442a:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  47442d:	0f be 08             	movsx  ecx,BYTE PTR [eax]
  474430:	83 f9 5f             	cmp    ecx,0x5f
  474433:	75 09                	jne    0x47443e
  474435:	c7 45 ec 01 00 00 00 	mov    DWORD PTR [ebp-0x14],0x1
  47443c:	eb 4e                	jmp    0x47448c
  47443e:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  474441:	0f be 02             	movsx  eax,BYTE PTR [edx]
  474444:	83 f8 24             	cmp    eax,0x24
  474447:	75 09                	jne    0x474452
  474449:	c7 45 ec 02 00 00 00 	mov    DWORD PTR [ebp-0x14],0x2
  474450:	eb 3a                	jmp    0x47448c
  474452:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  474455:	0f be 11             	movsx  edx,BYTE PTR [ecx]
  474458:	83 fa 7e             	cmp    edx,0x7e
  47445b:	75 09                	jne    0x474466
  47445d:	c7 45 ec 04 00 00 00 	mov    DWORD PTR [ebp-0x14],0x4
  474464:	eb 26                	jmp    0x47448c
  474466:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  474469:	0f be 08             	movsx  ecx,BYTE PTR [eax]
  47446c:	83 f9 5e             	cmp    ecx,0x5e
  47446f:	75 09                	jne    0x47447a
  474471:	c7 45 ec 08 00 00 00 	mov    DWORD PTR [ebp-0x14],0x8
  474478:	eb 12                	jmp    0x47448c
  47447a:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  47447d:	0f be 02             	movsx  eax,BYTE PTR [edx]
  474480:	83 f8 2b             	cmp    eax,0x2b
  474483:	75 07                	jne    0x47448c
  474485:	c7 45 ec 10 00 00 00 	mov    DWORD PTR [ebp-0x14],0x10
  47448c:	6a 28                	push   0x28
  47448e:	e8 15 7d 02 00       	call   0x49c1a8
  474493:	83 c4 04             	add    esp,0x4
  474496:	89 45 e4             	mov    DWORD PTR [ebp-0x1c],eax
  474499:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  4744a0:	83 7d e4 00          	cmp    DWORD PTR [ebp-0x1c],0x0
  4744a4:	74 15                	je     0x4744bb
  4744a6:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  4744a9:	51                   	push   ecx
  4744aa:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  4744ad:	52                   	push   edx
  4744ae:	8b 4d e4             	mov    ecx,DWORD PTR [ebp-0x1c]
  4744b1:	e8 d4 dc ff ff       	call   0x47218a
  4744b6:	89 45 dc             	mov    DWORD PTR [ebp-0x24],eax
  4744b9:	eb 07                	jmp    0x4744c2
  4744bb:	c7 45 dc 00 00 00 00 	mov    DWORD PTR [ebp-0x24],0x0
  4744c2:	8b 45 dc             	mov    eax,DWORD PTR [ebp-0x24]
  4744c5:	89 45 e8             	mov    DWORD PTR [ebp-0x18],eax
  4744c8:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  4744cf:	8b 45 e8             	mov    eax,DWORD PTR [ebp-0x18]
  4744d2:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  4744d5:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  4744dc:	8b e5                	mov    esp,ebp
  4744de:	5d                   	pop    ebp
  4744df:	c2 04 00             	ret    0x4


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00474380 <.text+0x73380>:
  474380:	55                   	push   ebp
  474381:	8b ec                	mov    ebp,esp
  474383:	51                   	push   ecx
  474384:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  474387:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  47438a:	e8 01 02 00 00       	call   0x474590
  47438f:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  474392:	c7 00 e8 19 4b 00    	mov    DWORD PTR [eax],0x4b19e8
  474398:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  47439b:	8b e5                	mov    esp,ebp
  47439d:	5d                   	pop    ebp
  47439e:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0046f14d <.text+0x6e14d>:
  46f14d:	55                   	push   ebp
  46f14e:	8b ec                	mov    ebp,esp
  46f150:	6a ff                	push   0xffffffff
  46f152:	68 cc dd 4a 00       	push   0x4addcc
  46f157:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  46f15d:	50                   	push   eax
  46f15e:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  46f165:	51                   	push   ecx
  46f166:	89 4d f0             	mov    DWORD PTR [ebp-0x10],ecx
  46f169:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  46f16c:	83 c1 08             	add    ecx,0x8
  46f16f:	e8 0c 52 00 00       	call   0x474380
  46f174:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  46f17b:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  46f17e:	83 c1 48             	add    ecx,0x48
  46f181:	e8 0a 97 02 00       	call   0x498890
  46f186:	c6 45 fc 01          	mov    BYTE PTR [ebp-0x4],0x1
  46f18a:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  46f18d:	81 c1 40 02 00 00    	add    ecx,0x240
  46f193:	e8 35 f1 ff ff       	call   0x46e2cd
  46f198:	c6 45 fc 02          	mov    BYTE PTR [ebp-0x4],0x2
  46f19c:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  46f19f:	81 c1 fc 06 00 00    	add    ecx,0x6fc
  46f1a5:	e8 96 5c 01 00       	call   0x484e40
  46f1aa:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  46f1ad:	c7 80 3c 07 00 00 00 	mov    DWORD PTR [eax+0x73c],0x0
  46f1b4:	00 00 00
  46f1b7:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  46f1ba:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  46f1bd:	89 91 7c 02 00 00    	mov    DWORD PTR [ecx+0x27c],edx
  46f1c3:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  46f1ca:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  46f1cd:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  46f1d0:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  46f1d7:	8b e5                	mov    esp,ebp
  46f1d9:	5d                   	pop    ebp
  46f1da:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0047218a <.text+0x7118a>:
  47218a:	55                   	push   ebp
  47218b:	8b ec                	mov    ebp,esp
  47218d:	6a ff                	push   0xffffffff
  47218f:	68 53 df 4a 00       	push   0x4adf53
  472194:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  47219a:	50                   	push   eax
  47219b:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  4721a2:	81 ec 7c 05 00 00    	sub    esp,0x57c
  4721a8:	89 8d 78 fa ff ff    	mov    DWORD PTR [ebp-0x588],ecx
  4721ae:	8b 8d 78 fa ff ff    	mov    ecx,DWORD PTR [ebp-0x588]
  4721b4:	e8 e7 1b 00 00       	call   0x473da0
  4721b9:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  4721c0:	8b 8d 78 fa ff ff    	mov    ecx,DWORD PTR [ebp-0x588]
  4721c6:	83 c1 1c             	add    ecx,0x1c
  4721c9:	e8 e2 a3 00 00       	call   0x47c5b0
  4721ce:	c6 45 fc 01          	mov    BYTE PTR [ebp-0x4],0x1
  4721d2:	8b 85 78 fa ff ff    	mov    eax,DWORD PTR [ebp-0x588]
  4721d8:	c7 40 08 00 00 00 00 	mov    DWORD PTR [eax+0x8],0x0
  4721df:	8b 8d 78 fa ff ff    	mov    ecx,DWORD PTR [ebp-0x588]
  4721e5:	c7 01 00 00 00 00    	mov    DWORD PTR [ecx],0x0
  4721eb:	8b 95 78 fa ff ff    	mov    edx,DWORD PTR [ebp-0x588]
  4721f1:	c7 42 0c 00 00 00 00 	mov    DWORD PTR [edx+0xc],0x0
  4721f8:	8b 85 78 fa ff ff    	mov    eax,DWORD PTR [ebp-0x588]
  4721fe:	c6 40 10 00          	mov    BYTE PTR [eax+0x10],0x0
  472202:	8b 8d 78 fa ff ff    	mov    ecx,DWORD PTR [ebp-0x588]
  472208:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  47220b:	89 51 24             	mov    DWORD PTR [ecx+0x24],edx
  47220e:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  472211:	50                   	push   eax
  472212:	8d 8d b8 fc ff ff    	lea    ecx,[ebp-0x348]
  472218:	51                   	push   ecx
  472219:	e8 32 a5 02 00       	call   0x49c750
  47221e:	83 c4 08             	add    esp,0x8
  472221:	33 d2                	xor    edx,edx
  472223:	8a 15 f2 d5 5c 00    	mov    dl,BYTE PTR ds:0x5cd5f2
  472229:	85 d2                	test   edx,edx
  47222b:	74 18                	je     0x472245
  47222d:	8d 85 b8 fc ff ff    	lea    eax,[ebp-0x348]
  472233:	50                   	push   eax
  472234:	e8 12 02 00 00       	call   0x47244b
  472239:	83 c4 04             	add    esp,0x4
  47223c:	25 ff 00 00 00       	and    eax,0xff
  472241:	85 c0                	test   eax,eax
  472243:	75 22                	jne    0x472267
  472245:	33 c9                	xor    ecx,ecx
  472247:	8a 0d f2 d5 5c 00    	mov    cl,BYTE PTR ds:0x5cd5f2
  47224d:	85 c9                	test   ecx,ecx
  47224f:	75 14                	jne    0x472265
  472251:	68 e8 24 4c 00       	push   0x4c24e8
  472256:	8d 95 b8 fc ff ff    	lea    edx,[ebp-0x348]
  47225c:	52                   	push   edx
  47225d:	e8 fe a4 02 00       	call   0x49c760
  472262:	83 c4 08             	add    esp,0x8
  472265:	eb 14                	jmp    0x47227b
  472267:	68 ec 24 4c 00       	push   0x4c24ec
  47226c:	8d 85 b8 fc ff ff    	lea    eax,[ebp-0x348]
  472272:	50                   	push   eax
  472273:	e8 e8 a4 02 00       	call   0x49c760
  472278:	83 c4 08             	add    esp,0x8
  47227b:	8d 8d b8 fc ff ff    	lea    ecx,[ebp-0x348]
  472281:	51                   	push   ecx
  472282:	68 f0 24 4c 00       	push   0x4c24f0
  472287:	8d 95 bc fd ff ff    	lea    edx,[ebp-0x244]
  47228d:	52                   	push   edx
  47228e:	e8 d7 a7 02 00       	call   0x49ca6a
  472293:	83 c4 0c             	add    esp,0xc
  472296:	8d 85 bc fd ff ff    	lea    eax,[ebp-0x244]
  47229c:	50                   	push   eax
  47229d:	8d 8d c0 fe ff ff    	lea    ecx,[ebp-0x140]
  4722a3:	e8 cb b8 00 00       	call   0x47db73
  4722a8:	c6 45 fc 02          	mov    BYTE PTR [ebp-0x4],0x2
  4722ac:	6a 00                	push   0x0
  4722ae:	6a 01                	push   0x1
  4722b0:	8d 8d c0 fe ff ff    	lea    ecx,[ebp-0x140]
  4722b6:	e8 75 35 fd ff       	call   0x445830
  4722bb:	25 ff 00 00 00       	and    eax,0xff
  4722c0:	85 c0                	test   eax,eax
  4722c2:	74 0d                	je     0x4722d1
  4722c4:	8d 8d c0 fe ff ff    	lea    ecx,[ebp-0x140]
  4722ca:	e8 75 c5 00 00       	call   0x47e844
  4722cf:	eb 2e                	jmp    0x4722ff
  4722d1:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  4722d4:	51                   	push   ecx
  4722d5:	8d 95 b8 fc ff ff    	lea    edx,[ebp-0x348]
  4722db:	52                   	push   edx
  4722dc:	e8 6f a4 02 00       	call   0x49c750
  4722e1:	83 c4 08             	add    esp,0x8
  4722e4:	8d 85 b8 fc ff ff    	lea    eax,[ebp-0x348]
  4722ea:	50                   	push   eax
  4722eb:	68 00 25 4c 00       	push   0x4c2500
  4722f0:	8d 8d bc fd ff ff    	lea    ecx,[ebp-0x244]
  4722f6:	51                   	push   ecx
  4722f7:	e8 6e a7 02 00       	call   0x49ca6a
  4722fc:	83 c4 0c             	add    esp,0xc
  4722ff:	8b 95 78 fa ff ff    	mov    edx,DWORD PTR [ebp-0x588]
  472305:	83 c2 18             	add    edx,0x18
  472308:	52                   	push   edx
  472309:	8b 85 78 fa ff ff    	mov    eax,DWORD PTR [ebp-0x588]
  47230f:	83 c0 14             	add    eax,0x14
  472312:	50                   	push   eax
  472313:	8d 8d bc fd ff ff    	lea    ecx,[ebp-0x244]
  472319:	51                   	push   ecx
  47231a:	8b 8d 78 fa ff ff    	mov    ecx,DWORD PTR [ebp-0x588]
  472320:	e8 6b 01 00 00       	call   0x472490
  472325:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  472328:	83 e2 18             	and    edx,0x18
  47232b:	85 d2                	test   edx,edx
  47232d:	0f 84 da 00 00 00    	je     0x47240d
  472333:	8b 85 78 fa ff ff    	mov    eax,DWORD PTR [ebp-0x588]
  472339:	8b 8d 78 fa ff ff    	mov    ecx,DWORD PTR [ebp-0x588]
  47233f:	8b 50 14             	mov    edx,DWORD PTR [eax+0x14]
  472342:	0f af 51 18          	imul   edx,DWORD PTR [ecx+0x18]
  472346:	52                   	push   edx
  472347:	e8 5c 9e 02 00       	call   0x49c1a8
  47234c:	83 c4 04             	add    esp,0x4
  47234f:	89 85 7c fa ff ff    	mov    DWORD PTR [ebp-0x584],eax
  472355:	8b 85 78 fa ff ff    	mov    eax,DWORD PTR [ebp-0x588]
  47235b:	8b 8d 7c fa ff ff    	mov    ecx,DWORD PTR [ebp-0x584]
  472361:	89 48 0c             	mov    DWORD PTR [eax+0xc],ecx
  472364:	8d 95 b8 fc ff ff    	lea    edx,[ebp-0x348]
  47236a:	52                   	push   edx
  47236b:	68 10 25 4c 00       	push   0x4c2510
  472370:	8d 85 80 fa ff ff    	lea    eax,[ebp-0x580]
  472376:	50                   	push   eax
  472377:	e8 ee a6 02 00       	call   0x49ca6a
  47237c:	83 c4 0c             	add    esp,0xc
  47237f:	8d 8d 80 fa ff ff    	lea    ecx,[ebp-0x580]
  472385:	51                   	push   ecx
  472386:	8d 8d 84 fb ff ff    	lea    ecx,[ebp-0x47c]
  47238c:	e8 e2 b7 00 00       	call   0x47db73
  472391:	c6 45 fc 03          	mov    BYTE PTR [ebp-0x4],0x3
  472395:	6a 00                	push   0x0
  472397:	6a 01                	push   0x1
  472399:	8d 8d 84 fb ff ff    	lea    ecx,[ebp-0x47c]
  47239f:	e8 69 c0 00 00       	call   0x47e40d
  4723a4:	25 ff 00 00 00       	and    eax,0xff
  4723a9:	85 c0                	test   eax,eax
  4723ab:	75 1d                	jne    0x4723ca
  4723ad:	8d 95 80 fa ff ff    	lea    edx,[ebp-0x580]
  4723b3:	52                   	push   edx
  4723b4:	6a 5a                	push   0x5a
  4723b6:	68 20 25 4c 00       	push   0x4c2520
  4723bb:	e8 42 06 01 00       	call   0x482a02
  4723c0:	83 c4 0c             	add    esp,0xc
  4723c3:	6a 00                	push   0x0
  4723c5:	e8 b9 af 02 00       	call   0x49d383
  4723ca:	8b 85 78 fa ff ff    	mov    eax,DWORD PTR [ebp-0x588]
  4723d0:	8b 8d 78 fa ff ff    	mov    ecx,DWORD PTR [ebp-0x588]
  4723d6:	8b 50 14             	mov    edx,DWORD PTR [eax+0x14]
  4723d9:	0f af 51 18          	imul   edx,DWORD PTR [ecx+0x18]
  4723dd:	52                   	push   edx
  4723de:	8b 85 78 fa ff ff    	mov    eax,DWORD PTR [ebp-0x588]
  4723e4:	8b 48 0c             	mov    ecx,DWORD PTR [eax+0xc]
  4723e7:	51                   	push   ecx
  4723e8:	8d 8d 84 fb ff ff    	lea    ecx,[ebp-0x47c]
  4723ee:	e8 bd 91 fc ff       	call   0x43b5b0
  4723f3:	8d 8d 84 fb ff ff    	lea    ecx,[ebp-0x47c]
  4723f9:	e8 46 c4 00 00       	call   0x47e844
  4723fe:	c6 45 fc 02          	mov    BYTE PTR [ebp-0x4],0x2
  472402:	8d 8d 84 fb ff ff    	lea    ecx,[ebp-0x47c]
  472408:	e8 1b b8 00 00       	call   0x47dc28
  47240d:	8b 95 78 fa ff ff    	mov    edx,DWORD PTR [ebp-0x588]
  472413:	52                   	push   edx
  472414:	8b 8d 78 fa ff ff    	mov    ecx,DWORD PTR [ebp-0x588]
  47241a:	e8 7a 0b 00 00       	call   0x472f99
  47241f:	c6 45 fc 01          	mov    BYTE PTR [ebp-0x4],0x1
  472423:	8d 8d c0 fe ff ff    	lea    ecx,[ebp-0x140]
  472429:	e8 fa b7 00 00       	call   0x47dc28
  47242e:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  472435:	8b 85 78 fa ff ff    	mov    eax,DWORD PTR [ebp-0x588]
  47243b:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  47243e:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  472445:	8b e5                	mov    esp,ebp
  472447:	5d                   	pop    ebp
  472448:	c2 08 00             	ret    0x8


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00472490 <.text+0x71490>:
  472490:	55                   	push   ebp
  472491:	8b ec                	mov    ebp,esp
  472493:	6a ff                	push   0xffffffff
  472495:	68 66 df 4a 00       	push   0x4adf66
  47249a:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  4724a0:	50                   	push   eax
  4724a1:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  4724a8:	83 ec 24             	sub    esp,0x24
  4724ab:	89 4d d0             	mov    DWORD PTR [ebp-0x30],ecx
  4724ae:	8d 4d d8             	lea    ecx,[ebp-0x28]
  4724b1:	e8 6a dc fd ff       	call   0x450120
  4724b6:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  4724bd:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  4724c0:	50                   	push   eax
  4724c1:	8d 4d d8             	lea    ecx,[ebp-0x28]
  4724c4:	e8 91 dc fd ff       	call   0x45015a
  4724c9:	8b 4d d8             	mov    ecx,DWORD PTR [ebp-0x28]
  4724cc:	51                   	push   ecx
  4724cd:	8b 55 dc             	mov    edx,DWORD PTR [ebp-0x24]
  4724d0:	52                   	push   edx
  4724d1:	68 b8 d5 58 00       	push   0x58d5b8
  4724d6:	e8 35 ab 02 00       	call   0x49d010
  4724db:	83 c4 0c             	add    esp,0xc
  4724de:	c7 45 ec b8 d5 58 00 	mov    DWORD PTR [ebp-0x14],0x58d5b8
  4724e5:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  4724e8:	83 c0 0e             	add    eax,0xe
  4724eb:	89 45 d4             	mov    DWORD PTR [ebp-0x2c],eax
  4724ee:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  4724f1:	8b 51 0a             	mov    edx,DWORD PTR [ecx+0xa]
  4724f4:	81 c2 b8 d5 58 00    	add    edx,0x58d5b8
  4724fa:	89 55 e4             	mov    DWORD PTR [ebp-0x1c],edx
  4724fd:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  472500:	8b 4d d4             	mov    ecx,DWORD PTR [ebp-0x2c]
  472503:	8b 51 04             	mov    edx,DWORD PTR [ecx+0x4]
  472506:	89 10                	mov    DWORD PTR [eax],edx
  472508:	8b 45 10             	mov    eax,DWORD PTR [ebp+0x10]
  47250b:	8b 4d d4             	mov    ecx,DWORD PTR [ebp-0x2c]
  47250e:	8b 51 08             	mov    edx,DWORD PTR [ecx+0x8]
  472511:	89 10                	mov    DWORD PTR [eax],edx
  472513:	8b 45 d4             	mov    eax,DWORD PTR [ebp-0x2c]
  472516:	33 c9                	xor    ecx,ecx
  472518:	66 8b 48 0e          	mov    cx,WORD PTR [eax+0xe]
  47251c:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  47251f:	8b 02                	mov    eax,DWORD PTR [edx]
  472521:	0f af c1             	imul   eax,ecx
  472524:	83 c0 1f             	add    eax,0x1f
  472527:	24 e0                	and    al,0xe0
  472529:	c1 f8 03             	sar    eax,0x3
  47252c:	8b 4d 10             	mov    ecx,DWORD PTR [ebp+0x10]
  47252f:	0f af 01             	imul   eax,DWORD PTR [ecx]
  472532:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  472535:	6a 00                	push   0x0
  472537:	ff 15 4c 00 4b 00    	call   DWORD PTR ds:0x4b004c
  47253d:	89 45 e8             	mov    DWORD PTR [ebp-0x18],eax
  472540:	6a 00                	push   0x0
  472542:	6a 00                	push   0x0
  472544:	8d 55 e0             	lea    edx,[ebp-0x20]
  472547:	52                   	push   edx
  472548:	6a 00                	push   0x0
  47254a:	8b 45 d4             	mov    eax,DWORD PTR [ebp-0x2c]
  47254d:	50                   	push   eax
  47254e:	8b 4d e8             	mov    ecx,DWORD PTR [ebp-0x18]
  472551:	51                   	push   ecx
  472552:	ff 15 54 00 4b 00    	call   DWORD PTR ds:0x4b0054
  472558:	8b 55 d0             	mov    edx,DWORD PTR [ebp-0x30]
  47255b:	89 42 04             	mov    DWORD PTR [edx+0x4],eax
  47255e:	8b 45 e8             	mov    eax,DWORD PTR [ebp-0x18]
  472561:	50                   	push   eax
  472562:	ff 15 3c 00 4b 00    	call   DWORD PTR ds:0x4b003c
  472568:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  47256b:	51                   	push   ecx
  47256c:	8b 55 e4             	mov    edx,DWORD PTR [ebp-0x1c]
  47256f:	52                   	push   edx
  472570:	8b 45 e0             	mov    eax,DWORD PTR [ebp-0x20]
  472573:	50                   	push   eax
  472574:	e8 97 aa 02 00       	call   0x49d010
  472579:	83 c4 0c             	add    esp,0xc
  47257c:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  472583:	8d 4d d8             	lea    ecx,[ebp-0x28]
  472586:	e8 ad db fd ff       	call   0x450138
  47258b:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  47258e:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  472595:	8b e5                	mov    esp,ebp
  472597:	5d                   	pop    ebp
  472598:	c2 0c 00             	ret    0xc


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0047244b <.text+0x7144b>:
  47244b:	55                   	push   ebp
  47244c:	8b ec                	mov    ebp,esp
  47244e:	51                   	push   ecx
  47244f:	68 48 25 4c 00       	push   0x4c2548
  472454:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  472457:	50                   	push   eax
  472458:	e8 c3 a9 02 00       	call   0x49ce20
  47245d:	83 c4 08             	add    esp,0x8
  472460:	85 c0                	test   eax,eax
  472462:	75 1e                	jne    0x472482
  472464:	68 50 25 4c 00       	push   0x4c2550
  472469:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  47246c:	51                   	push   ecx
  47246d:	e8 ae a9 02 00       	call   0x49ce20
  472472:	83 c4 08             	add    esp,0x8
  472475:	85 c0                	test   eax,eax
  472477:	75 09                	jne    0x472482
  472479:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  472480:	eb 07                	jmp    0x472489
  472482:	c7 45 fc 01 00 00 00 	mov    DWORD PTR [ebp-0x4],0x1
  472489:	8a 45 fc             	mov    al,BYTE PTR [ebp-0x4]
  47248c:	8b e5                	mov    esp,ebp
  47248e:	5d                   	pop    ebp
  47248f:	c3                   	ret
