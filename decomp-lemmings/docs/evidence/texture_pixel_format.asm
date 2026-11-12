
C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00473b1e <.text+0x72b1e>:
  473b1e:	55                   	push   ebp
  473b1f:	8b ec                	mov    ebp,esp
  473b21:	83 ec 2c             	sub    esp,0x2c
  473b24:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  473b27:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
  473b2a:	83 7d fc 00          	cmp    DWORD PTR [ebp-0x4],0x0
  473b2e:	74 16                	je     0x473b46
  473b30:	83 7d 08 00          	cmp    DWORD PTR [ebp+0x8],0x0
  473b34:	74 10                	je     0x473b46
  473b36:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  473b39:	8b 51 04             	mov    edx,DWORD PTR [ecx+0x4]
  473b3c:	81 e2 00 00 0e 00    	and    edx,0xe0000
  473b42:	85 d2                	test   edx,edx
  473b44:	74 0a                	je     0x473b50
  473b46:	b8 01 00 00 00       	mov    eax,0x1
  473b4b:	e9 44 02 00 00       	jmp    0x473d94
  473b50:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  473b53:	33 c9                	xor    ecx,ecx
  473b55:	8a 48 06             	mov    cl,BYTE PTR [eax+0x6]
  473b58:	85 c9                	test   ecx,ecx
  473b5a:	74 35                	je     0x473b91
  473b5c:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  473b5f:	8b 42 04             	mov    eax,DWORD PTR [edx+0x4]
  473b62:	83 e0 20             	and    eax,0x20
  473b65:	85 c0                	test   eax,eax
  473b67:	74 23                	je     0x473b8c
  473b69:	6a 20                	push   0x20
  473b6b:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  473b6e:	51                   	push   ecx
  473b6f:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  473b72:	8b 42 10             	mov    eax,DWORD PTR [edx+0x10]
  473b75:	50                   	push   eax
  473b76:	e8 95 94 02 00       	call   0x49d010
  473b7b:	83 c4 0c             	add    esp,0xc
  473b7e:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  473b81:	c6 41 08 01          	mov    BYTE PTR [ecx+0x8],0x1
  473b85:	33 c0                	xor    eax,eax
  473b87:	e9 08 02 00 00       	jmp    0x473d94
  473b8c:	e9 fe 01 00 00       	jmp    0x473d8f
  473b91:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  473b94:	33 c0                	xor    eax,eax
  473b96:	8a 42 04             	mov    al,BYTE PTR [edx+0x4]
  473b99:	85 c0                	test   eax,eax
  473b9b:	0f 84 1b 01 00 00    	je     0x473cbc
  473ba1:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  473ba4:	33 d2                	xor    edx,edx
  473ba6:	83 79 08 00          	cmp    DWORD PTR [ecx+0x8],0x0
  473baa:	0f 95 c2             	setne  dl
  473bad:	88 55 f4             	mov    BYTE PTR [ebp-0xc],dl
  473bb0:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  473bb3:	8b 48 04             	mov    ecx,DWORD PTR [eax+0x4]
  473bb6:	83 e1 01             	and    ecx,0x1
  473bb9:	83 e9 01             	sub    ecx,0x1
  473bbc:	f7 d9                	neg    ecx
  473bbe:	1b c9                	sbb    ecx,ecx
  473bc0:	41                   	inc    ecx
  473bc1:	88 4d ec             	mov    BYTE PTR [ebp-0x14],cl
  473bc4:	c7 45 f0 00 00 00 00 	mov    DWORD PTR [ebp-0x10],0x0
  473bcb:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  473bce:	8b 42 1c             	mov    eax,DWORD PTR [edx+0x1c]
  473bd1:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
  473bd4:	83 7d f8 00          	cmp    DWORD PTR [ebp-0x8],0x0
  473bd8:	74 19                	je     0x473bf3
  473bda:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  473bdd:	83 e9 01             	sub    ecx,0x1
  473be0:	8b 55 f8             	mov    edx,DWORD PTR [ebp-0x8]
  473be3:	23 d1                	and    edx,ecx
  473be5:	89 55 f8             	mov    DWORD PTR [ebp-0x8],edx
  473be8:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  473beb:	83 c0 01             	add    eax,0x1
  473bee:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  473bf1:	eb e1                	jmp    0x473bd4
  473bf3:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  473bf6:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  473bf9:	8b 41 0c             	mov    eax,DWORD PTR [ecx+0xc]
  473bfc:	3b 02                	cmp    eax,DWORD PTR [edx]
  473bfe:	0f 85 b3 00 00 00    	jne    0x473cb7
  473c04:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  473c07:	33 d2                	xor    edx,edx
  473c09:	8a 51 07             	mov    dl,BYTE PTR [ecx+0x7]
  473c0c:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
  473c0f:	25 ff 00 00 00       	and    eax,0xff
  473c14:	3b d0                	cmp    edx,eax
  473c16:	0f 85 9b 00 00 00    	jne    0x473cb7
  473c1c:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  473c1f:	81 e1 ff 00 00 00    	and    ecx,0xff
  473c25:	85 c9                	test   ecx,ecx
  473c27:	0f 84 8a 00 00 00    	je     0x473cb7
  473c2d:	83 7d f0 01          	cmp    DWORD PTR [ebp-0x10],0x1
  473c31:	0f 8e 80 00 00 00    	jle    0x473cb7
  473c37:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  473c3a:	33 c0                	xor    eax,eax
  473c3c:	8a 42 08             	mov    al,BYTE PTR [edx+0x8]
  473c3f:	85 c0                	test   eax,eax
  473c41:	74 45                	je     0x473c88
  473c43:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  473c46:	8b 51 10             	mov    edx,DWORD PTR [ecx+0x10]
  473c49:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  473c4c:	8b 48 10             	mov    ecx,DWORD PTR [eax+0x10]
  473c4f:	8b 52 10             	mov    edx,DWORD PTR [edx+0x10]
  473c52:	0b 51 14             	or     edx,DWORD PTR [ecx+0x14]
  473c55:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  473c58:	8b 48 10             	mov    ecx,DWORD PTR [eax+0x10]
  473c5b:	0b 51 18             	or     edx,DWORD PTR [ecx+0x18]
  473c5e:	89 55 e8             	mov    DWORD PTR [ebp-0x18],edx
  473c61:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  473c64:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  473c67:	8b 4a 10             	mov    ecx,DWORD PTR [edx+0x10]
  473c6a:	0b 48 14             	or     ecx,DWORD PTR [eax+0x14]
  473c6d:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  473c70:	0b 4a 18             	or     ecx,DWORD PTR [edx+0x18]
  473c73:	89 4d e4             	mov    DWORD PTR [ebp-0x1c],ecx
  473c76:	8b 45 e4             	mov    eax,DWORD PTR [ebp-0x1c]
  473c79:	3b 45 e8             	cmp    eax,DWORD PTR [ebp-0x18]
  473c7c:	7d 0a                	jge    0x473c88
  473c7e:	b8 01 00 00 00       	mov    eax,0x1
  473c83:	e9 0c 01 00 00       	jmp    0x473d94
  473c88:	6a 20                	push   0x20
  473c8a:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  473c8d:	51                   	push   ecx
  473c8e:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  473c91:	8b 42 10             	mov    eax,DWORD PTR [edx+0x10]
  473c94:	50                   	push   eax
  473c95:	e8 76 93 02 00       	call   0x49d010
  473c9a:	83 c4 0c             	add    esp,0xc
  473c9d:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  473ca0:	c6 41 08 01          	mov    BYTE PTR [ecx+0x8],0x1
  473ca4:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  473ca7:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  473caa:	89 42 0c             	mov    DWORD PTR [edx+0xc],eax
  473cad:	b8 01 00 00 00       	mov    eax,0x1
  473cb2:	e9 dd 00 00 00       	jmp    0x473d94
  473cb7:	e9 d3 00 00 00       	jmp    0x473d8f
  473cbc:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  473cbf:	33 d2                	xor    edx,edx
  473cc1:	83 79 08 00          	cmp    DWORD PTR [ecx+0x8],0x0
  473cc5:	0f 95 c2             	setne  dl
  473cc8:	88 55 e0             	mov    BYTE PTR [ebp-0x20],dl
  473ccb:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  473cce:	8b 48 04             	mov    ecx,DWORD PTR [eax+0x4]
  473cd1:	83 e1 01             	and    ecx,0x1
  473cd4:	83 e9 01             	sub    ecx,0x1
  473cd7:	f7 d9                	neg    ecx
  473cd9:	1b c9                	sbb    ecx,ecx
  473cdb:	41                   	inc    ecx
  473cdc:	88 4d dc             	mov    BYTE PTR [ebp-0x24],cl
  473cdf:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  473ce2:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  473ce5:	8b 4a 0c             	mov    ecx,DWORD PTR [edx+0xc]
  473ce8:	3b 08                	cmp    ecx,DWORD PTR [eax]
  473cea:	0f 85 9f 00 00 00    	jne    0x473d8f
  473cf0:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  473cf3:	33 c0                	xor    eax,eax
  473cf5:	8a 42 07             	mov    al,BYTE PTR [edx+0x7]
  473cf8:	8b 4d e0             	mov    ecx,DWORD PTR [ebp-0x20]
  473cfb:	81 e1 ff 00 00 00    	and    ecx,0xff
  473d01:	3b c1                	cmp    eax,ecx
  473d03:	0f 85 86 00 00 00    	jne    0x473d8f
  473d09:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  473d0c:	33 c0                	xor    eax,eax
  473d0e:	8a 42 05             	mov    al,BYTE PTR [edx+0x5]
  473d11:	8b 4d dc             	mov    ecx,DWORD PTR [ebp-0x24]
  473d14:	81 e1 ff 00 00 00    	and    ecx,0xff
  473d1a:	3b c1                	cmp    eax,ecx
  473d1c:	75 71                	jne    0x473d8f
  473d1e:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  473d21:	33 c0                	xor    eax,eax
  473d23:	8a 42 08             	mov    al,BYTE PTR [edx+0x8]
  473d26:	85 c0                	test   eax,eax
  473d28:	74 42                	je     0x473d6c
  473d2a:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  473d2d:	8b 51 10             	mov    edx,DWORD PTR [ecx+0x10]
  473d30:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  473d33:	8b 48 10             	mov    ecx,DWORD PTR [eax+0x10]
  473d36:	8b 52 10             	mov    edx,DWORD PTR [edx+0x10]
  473d39:	0b 51 14             	or     edx,DWORD PTR [ecx+0x14]
  473d3c:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  473d3f:	8b 48 10             	mov    ecx,DWORD PTR [eax+0x10]
  473d42:	0b 51 18             	or     edx,DWORD PTR [ecx+0x18]
  473d45:	89 55 d8             	mov    DWORD PTR [ebp-0x28],edx
  473d48:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  473d4b:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  473d4e:	8b 4a 10             	mov    ecx,DWORD PTR [edx+0x10]
  473d51:	0b 48 14             	or     ecx,DWORD PTR [eax+0x14]
  473d54:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  473d57:	0b 4a 18             	or     ecx,DWORD PTR [edx+0x18]
  473d5a:	89 4d d4             	mov    DWORD PTR [ebp-0x2c],ecx
  473d5d:	8b 45 d4             	mov    eax,DWORD PTR [ebp-0x2c]
  473d60:	3b 45 d8             	cmp    eax,DWORD PTR [ebp-0x28]
  473d63:	7d 07                	jge    0x473d6c
  473d65:	b8 01 00 00 00       	mov    eax,0x1
  473d6a:	eb 28                	jmp    0x473d94
  473d6c:	6a 20                	push   0x20
  473d6e:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  473d71:	51                   	push   ecx
  473d72:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  473d75:	8b 42 10             	mov    eax,DWORD PTR [edx+0x10]
  473d78:	50                   	push   eax
  473d79:	e8 92 92 02 00       	call   0x49d010
  473d7e:	83 c4 0c             	add    esp,0xc
  473d81:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  473d84:	c6 41 08 01          	mov    BYTE PTR [ecx+0x8],0x1
  473d88:	b8 01 00 00 00       	mov    eax,0x1
  473d8d:	eb 05                	jmp    0x473d94
  473d8f:	b8 01 00 00 00       	mov    eax,0x1
  473d94:	8b e5                	mov    esp,ebp
  473d96:	5d                   	pop    ebp
  473d97:	c2 08 00             	ret    0x8
