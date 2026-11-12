; Region-selected terminal behavior original code evidence
; Executable SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00406c20 <.text+0x5c20>:
  406c20:	55                   	push   ebp
  406c21:	8b ec                	mov    ebp,esp
  406c23:	51                   	push   ecx
  406c24:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  406c27:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  406c2a:	e8 a1 a4 ff ff       	call   0x4010d0
  406c2f:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  406c32:	c7 00 d0 04 4b 00    	mov    DWORD PTR [eax],0x4b04d0
  406c38:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  406c3b:	c7 41 14 2e 00 00 00 	mov    DWORD PTR [ecx+0x14],0x2e
  406c42:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  406c45:	c6 42 11 00          	mov    BYTE PTR [edx+0x11],0x0
  406c49:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  406c4c:	8b e5                	mov    esp,ebp
  406c4e:	5d                   	pop    ebp
  406c4f:	c3                   	ret


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00406c6c <.text+0x5c6c>:
  406c6c:	55                   	push   ebp
  406c6d:	8b ec                	mov    ebp,esp
  406c6f:	6a ff                	push   0xffffffff
  406c71:	68 5a 95 4a 00       	push   0x4a955a
  406c76:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  406c7c:	50                   	push   eax
  406c7d:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  406c84:	81 ec a4 01 00 00    	sub    esp,0x1a4
  406c8a:	56                   	push   esi
  406c8b:	57                   	push   edi
  406c8c:	89 8d 68 fe ff ff    	mov    DWORD PTR [ebp-0x198],ecx
  406c92:	8b 75 08             	mov    esi,DWORD PTR [ebp+0x8]
  406c95:	83 c6 08             	add    esi,0x8
  406c98:	b9 06 00 00 00       	mov    ecx,0x6
  406c9d:	8d 7d d8             	lea    edi,[ebp-0x28]
  406ca0:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  406ca2:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  406ca9:	dd 45 e8             	fld    QWORD PTR [ebp-0x18]
  406cac:	dc 05 d0 02 4b 00    	fadd   QWORD PTR ds:0x4b02d0
  406cb2:	dd 5d e8             	fstp   QWORD PTR [ebp-0x18]
  406cb5:	dd 45 e0             	fld    QWORD PTR [ebp-0x20]
  406cb8:	dc 05 c8 02 4b 00    	fadd   QWORD PTR ds:0x4b02c8
  406cbe:	dd 5d e0             	fstp   QWORD PTR [ebp-0x20]
  406cc1:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  406cc4:	83 78 4c 03          	cmp    DWORD PTR [eax+0x4c],0x3
  406cc8:	0f 84 c0 01 00 00    	je     0x406e8e
  406cce:	68 28 01 00 00       	push   0x128
  406cd3:	e8 d0 54 09 00       	call   0x49c1a8
  406cd8:	83 c4 04             	add    esp,0x4
  406cdb:	89 45 90             	mov    DWORD PTR [ebp-0x70],eax
  406cde:	c6 45 fc 01          	mov    BYTE PTR [ebp-0x4],0x1
  406ce2:	83 7d 90 00          	cmp    DWORD PTR [ebp-0x70],0x0
  406ce6:	74 10                	je     0x406cf8
  406ce8:	8b 4d 90             	mov    ecx,DWORD PTR [ebp-0x70]
  406ceb:	e8 f0 ab ff ff       	call   0x4018e0
  406cf0:	89 85 64 fe ff ff    	mov    DWORD PTR [ebp-0x19c],eax
  406cf6:	eb 0a                	jmp    0x406d02
  406cf8:	c7 85 64 fe ff ff 00 	mov    DWORD PTR [ebp-0x19c],0x0
  406cff:	00 00 00
  406d02:	8b 8d 64 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x19c]
  406d08:	89 4d 94             	mov    DWORD PTR [ebp-0x6c],ecx
  406d0b:	c6 45 fc 00          	mov    BYTE PTR [ebp-0x4],0x0
  406d0f:	8b 55 94             	mov    edx,DWORD PTR [ebp-0x6c]
  406d12:	89 55 d0             	mov    DWORD PTR [ebp-0x30],edx
  406d15:	6a 06                	push   0x6
  406d17:	8b 4d d0             	mov    ecx,DWORD PTR [ebp-0x30]
  406d1a:	e8 91 ae ff ff       	call   0x401bb0
  406d1f:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  406d22:	83 c0 08             	add    eax,0x8
  406d25:	50                   	push   eax
  406d26:	8b 4d d0             	mov    ecx,DWORD PTR [ebp-0x30]
  406d29:	8b 11                	mov    edx,DWORD PTR [ecx]
  406d2b:	8b 4d d0             	mov    ecx,DWORD PTR [ebp-0x30]
  406d2e:	ff 52 08             	call   DWORD PTR [edx+0x8]
  406d31:	68 00 00 e0 3f       	push   0x3fe00000
  406d36:	6a 00                	push   0x0
  406d38:	68 00 00 10 40       	push   0x40100000
  406d3d:	6a 00                	push   0x0
  406d3f:	8b 4d d0             	mov    ecx,DWORD PTR [ebp-0x30]
  406d42:	e8 a9 b1 ff ff       	call   0x401ef0
  406d47:	6a 01                	push   0x1
  406d49:	8b 4d d0             	mov    ecx,DWORD PTR [ebp-0x30]
  406d4c:	e8 df b1 ff ff       	call   0x401f30
  406d51:	6a 0c                	push   0xc
  406d53:	8b 4d d0             	mov    ecx,DWORD PTR [ebp-0x30]
  406d56:	e8 f5 b1 ff ff       	call   0x401f50
  406d5b:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  406d5e:	83 c0 08             	add    eax,0x8
  406d61:	50                   	push   eax
  406d62:	8d 8d 60 ff ff ff    	lea    ecx,[ebp-0xa0]
  406d68:	51                   	push   ecx
  406d69:	b9 10 6a 4c 00       	mov    ecx,0x4c6a10
  406d6e:	e8 f9 67 03 00       	call   0x43d56c
  406d73:	50                   	push   eax
  406d74:	8d 95 48 ff ff ff    	lea    edx,[ebp-0xb8]
  406d7a:	52                   	push   edx
  406d7b:	8d 45 d8             	lea    eax,[ebp-0x28]
  406d7e:	50                   	push   eax
  406d7f:	8d 8d 78 ff ff ff    	lea    ecx,[ebp-0x88]
  406d85:	51                   	push   ecx
  406d86:	b9 10 6a 4c 00       	mov    ecx,0x4c6a10
  406d8b:	e8 dc 67 03 00       	call   0x43d56c
  406d90:	8b c8                	mov    ecx,eax
  406d92:	e8 99 aa ff ff       	call   0x401830
  406d97:	50                   	push   eax
  406d98:	8b 55 d0             	mov    edx,DWORD PTR [ebp-0x30]
  406d9b:	8b 02                	mov    eax,DWORD PTR [edx]
  406d9d:	8b 4d d0             	mov    ecx,DWORD PTR [ebp-0x30]
  406da0:	ff 50 54             	call   DWORD PTR [eax+0x54]
  406da3:	dd 45 e0             	fld    QWORD PTR [ebp-0x20]
  406da6:	e8 01 58 09 00       	call   0x49c5ac
  406dab:	50                   	push   eax
  406dac:	dd 45 d8             	fld    QWORD PTR [ebp-0x28]
  406daf:	e8 f8 57 09 00       	call   0x49c5ac
  406db4:	50                   	push   eax
  406db5:	b9 10 6a 4c 00       	mov    ecx,0x4c6a10
  406dba:	e8 81 d1 ff ff       	call   0x403f40
  406dbf:	8b f0                	mov    esi,eax
  406dc1:	b9 06 00 00 00       	mov    ecx,0x6
  406dc6:	8d 7d b8             	lea    edi,[ebp-0x48]
  406dc9:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  406dcb:	c7 45 d4 00 00 00 00 	mov    DWORD PTR [ebp-0x2c],0x0
  406dd2:	eb 09                	jmp    0x406ddd
  406dd4:	8b 4d d4             	mov    ecx,DWORD PTR [ebp-0x2c]
  406dd7:	83 c1 01             	add    ecx,0x1
  406dda:	89 4d d4             	mov    DWORD PTR [ebp-0x2c],ecx
  406ddd:	8b 4d d0             	mov    ecx,DWORD PTR [ebp-0x30]
  406de0:	81 c1 b0 00 00 00    	add    ecx,0xb0
  406de6:	e8 e5 aa ff ff       	call   0x4018d0
  406deb:	39 45 d4             	cmp    DWORD PTR [ebp-0x2c],eax
  406dee:	7d 25                	jge    0x406e15
  406df0:	8d 55 b8             	lea    edx,[ebp-0x48]
  406df3:	52                   	push   edx
  406df4:	8b 45 d4             	mov    eax,DWORD PTR [ebp-0x2c]
  406df7:	50                   	push   eax
  406df8:	8b 4d d0             	mov    ecx,DWORD PTR [ebp-0x30]
  406dfb:	81 c1 b0 00 00 00    	add    ecx,0xb0
  406e01:	e8 aa aa ff ff       	call   0x4018b0
  406e06:	8b c8                	mov    ecx,eax
  406e08:	81 c1 b0 00 00 00    	add    ecx,0xb0
  406e0e:	e8 0d ca ff ff       	call   0x403820
  406e13:	eb bf                	jmp    0x406dd4
  406e15:	6a 48                	push   0x48
  406e17:	e8 8c 53 09 00       	call   0x49c1a8
  406e1c:	83 c4 04             	add    esp,0x4
  406e1f:	89 85 3c ff ff ff    	mov    DWORD PTR [ebp-0xc4],eax
  406e25:	c6 45 fc 02          	mov    BYTE PTR [ebp-0x4],0x2
  406e29:	83 bd 3c ff ff ff 00 	cmp    DWORD PTR [ebp-0xc4],0x0
  406e30:	74 20                	je     0x406e52
  406e32:	6a 01                	push   0x1
  406e34:	8b 4d d0             	mov    ecx,DWORD PTR [ebp-0x30]
  406e37:	51                   	push   ecx
  406e38:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  406e3b:	83 c2 08             	add    edx,0x8
  406e3e:	52                   	push   edx
  406e3f:	8b 8d 3c ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc4]
  406e45:	e8 36 b2 02 00       	call   0x432080
  406e4a:	89 85 60 fe ff ff    	mov    DWORD PTR [ebp-0x1a0],eax
  406e50:	eb 0a                	jmp    0x406e5c
  406e52:	c7 85 60 fe ff ff 00 	mov    DWORD PTR [ebp-0x1a0],0x0
  406e59:	00 00 00
  406e5c:	8b 85 60 fe ff ff    	mov    eax,DWORD PTR [ebp-0x1a0]
  406e62:	89 85 40 ff ff ff    	mov    DWORD PTR [ebp-0xc0],eax
  406e68:	c6 45 fc 00          	mov    BYTE PTR [ebp-0x4],0x0
  406e6c:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  406e72:	89 8d 44 ff ff ff    	mov    DWORD PTR [ebp-0xbc],ecx
  406e78:	8d 95 44 ff ff ff    	lea    edx,[ebp-0xbc]
  406e7e:	52                   	push   edx
  406e7f:	b9 a8 88 4c 00       	mov    ecx,0x4c88a8
  406e84:	e8 07 aa ff ff       	call   0x401890
  406e89:	e9 ca 01 00 00       	jmp    0x407058
  406e8e:	68 28 01 00 00       	push   0x128
  406e93:	e8 10 53 09 00       	call   0x49c1a8
  406e98:	83 c4 04             	add    esp,0x4
  406e9b:	89 85 34 ff ff ff    	mov    DWORD PTR [ebp-0xcc],eax
  406ea1:	c6 45 fc 03          	mov    BYTE PTR [ebp-0x4],0x3
  406ea5:	83 bd 34 ff ff ff 00 	cmp    DWORD PTR [ebp-0xcc],0x0
  406eac:	74 13                	je     0x406ec1
  406eae:	8b 8d 34 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xcc]
  406eb4:	e8 e7 03 00 00       	call   0x4072a0
  406eb9:	89 85 5c fe ff ff    	mov    DWORD PTR [ebp-0x1a4],eax
  406ebf:	eb 0a                	jmp    0x406ecb
  406ec1:	c7 85 5c fe ff ff 00 	mov    DWORD PTR [ebp-0x1a4],0x0
  406ec8:	00 00 00
  406ecb:	8b 85 5c fe ff ff    	mov    eax,DWORD PTR [ebp-0x1a4]
  406ed1:	89 85 38 ff ff ff    	mov    DWORD PTR [ebp-0xc8],eax
  406ed7:	c6 45 fc 00          	mov    BYTE PTR [ebp-0x4],0x0
  406edb:	8b 8d 38 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc8]
  406ee1:	89 4d b0             	mov    DWORD PTR [ebp-0x50],ecx
  406ee4:	6a 06                	push   0x6
  406ee6:	8b 4d b0             	mov    ecx,DWORD PTR [ebp-0x50]
  406ee9:	e8 72 06 00 00       	call   0x407560
  406eee:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  406ef1:	83 c2 08             	add    edx,0x8
  406ef4:	52                   	push   edx
  406ef5:	8b 45 b0             	mov    eax,DWORD PTR [ebp-0x50]
  406ef8:	8b 10                	mov    edx,DWORD PTR [eax]
  406efa:	8b 4d b0             	mov    ecx,DWORD PTR [ebp-0x50]
  406efd:	ff 52 08             	call   DWORD PTR [edx+0x8]
  406f00:	68 00 00 e0 3f       	push   0x3fe00000
  406f05:	6a 00                	push   0x0
  406f07:	68 00 00 10 40       	push   0x40100000
  406f0c:	6a 00                	push   0x0
  406f0e:	8b 4d b0             	mov    ecx,DWORD PTR [ebp-0x50]
  406f11:	e8 ba 07 00 00       	call   0x4076d0
  406f16:	6a 01                	push   0x1
  406f18:	8b 4d b0             	mov    ecx,DWORD PTR [ebp-0x50]
  406f1b:	e8 f0 07 00 00       	call   0x407710
  406f20:	6a 0c                	push   0xc
  406f22:	8b 4d b0             	mov    ecx,DWORD PTR [ebp-0x50]
  406f25:	e8 06 08 00 00       	call   0x407730
  406f2a:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  406f2d:	83 c0 08             	add    eax,0x8
  406f30:	50                   	push   eax
  406f31:	8d 8d 04 ff ff ff    	lea    ecx,[ebp-0xfc]
  406f37:	51                   	push   ecx
  406f38:	b9 10 6a 4c 00       	mov    ecx,0x4c6a10
  406f3d:	e8 2a 66 03 00       	call   0x43d56c
  406f42:	50                   	push   eax
  406f43:	8d 95 ec fe ff ff    	lea    edx,[ebp-0x114]
  406f49:	52                   	push   edx
  406f4a:	8d 45 d8             	lea    eax,[ebp-0x28]
  406f4d:	50                   	push   eax
  406f4e:	8d 8d 1c ff ff ff    	lea    ecx,[ebp-0xe4]
  406f54:	51                   	push   ecx
  406f55:	b9 10 6a 4c 00       	mov    ecx,0x4c6a10
  406f5a:	e8 0d 66 03 00       	call   0x43d56c
  406f5f:	8b c8                	mov    ecx,eax
  406f61:	e8 ca a8 ff ff       	call   0x401830
  406f66:	50                   	push   eax
  406f67:	8b 55 b0             	mov    edx,DWORD PTR [ebp-0x50]
  406f6a:	8b 02                	mov    eax,DWORD PTR [edx]
  406f6c:	8b 4d b0             	mov    ecx,DWORD PTR [ebp-0x50]
  406f6f:	ff 50 54             	call   DWORD PTR [eax+0x54]
  406f72:	dd 45 e0             	fld    QWORD PTR [ebp-0x20]
  406f75:	e8 32 56 09 00       	call   0x49c5ac
  406f7a:	50                   	push   eax
  406f7b:	dd 45 d8             	fld    QWORD PTR [ebp-0x28]
  406f7e:	e8 29 56 09 00       	call   0x49c5ac
  406f83:	50                   	push   eax
  406f84:	b9 10 6a 4c 00       	mov    ecx,0x4c6a10
  406f89:	e8 b2 cf ff ff       	call   0x403f40
  406f8e:	8b f0                	mov    esi,eax
  406f90:	b9 06 00 00 00       	mov    ecx,0x6
  406f95:	8d 7d 98             	lea    edi,[ebp-0x68]
  406f98:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  406f9a:	c7 45 b4 00 00 00 00 	mov    DWORD PTR [ebp-0x4c],0x0
  406fa1:	eb 09                	jmp    0x406fac
  406fa3:	8b 4d b4             	mov    ecx,DWORD PTR [ebp-0x4c]
  406fa6:	83 c1 01             	add    ecx,0x1
  406fa9:	89 4d b4             	mov    DWORD PTR [ebp-0x4c],ecx
  406fac:	8b 4d b0             	mov    ecx,DWORD PTR [ebp-0x50]
  406faf:	81 c1 b0 00 00 00    	add    ecx,0xb0
  406fb5:	e8 16 a9 ff ff       	call   0x4018d0
  406fba:	39 45 b4             	cmp    DWORD PTR [ebp-0x4c],eax
  406fbd:	7d 25                	jge    0x406fe4
  406fbf:	8d 55 98             	lea    edx,[ebp-0x68]
  406fc2:	52                   	push   edx
  406fc3:	8b 45 b4             	mov    eax,DWORD PTR [ebp-0x4c]
  406fc6:	50                   	push   eax
  406fc7:	8b 4d b0             	mov    ecx,DWORD PTR [ebp-0x50]
  406fca:	81 c1 b0 00 00 00    	add    ecx,0xb0
  406fd0:	e8 db a8 ff ff       	call   0x4018b0
  406fd5:	8b c8                	mov    ecx,eax
  406fd7:	81 c1 b0 00 00 00    	add    ecx,0xb0
  406fdd:	e8 3e c8 ff ff       	call   0x403820
  406fe2:	eb bf                	jmp    0x406fa3
  406fe4:	6a 48                	push   0x48
  406fe6:	e8 bd 51 09 00       	call   0x49c1a8
  406feb:	83 c4 04             	add    esp,0x4
  406fee:	89 85 e0 fe ff ff    	mov    DWORD PTR [ebp-0x120],eax
  406ff4:	c6 45 fc 04          	mov    BYTE PTR [ebp-0x4],0x4
  406ff8:	83 bd e0 fe ff ff 00 	cmp    DWORD PTR [ebp-0x120],0x0
  406fff:	74 20                	je     0x407021
  407001:	6a 01                	push   0x1
  407003:	8b 4d b0             	mov    ecx,DWORD PTR [ebp-0x50]
  407006:	51                   	push   ecx
  407007:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  40700a:	83 c2 08             	add    edx,0x8
  40700d:	52                   	push   edx
  40700e:	8b 8d e0 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x120]
  407014:	e8 67 b0 02 00       	call   0x432080
  407019:	89 85 58 fe ff ff    	mov    DWORD PTR [ebp-0x1a8],eax
  40701f:	eb 0a                	jmp    0x40702b
  407021:	c7 85 58 fe ff ff 00 	mov    DWORD PTR [ebp-0x1a8],0x0
  407028:	00 00 00
  40702b:	8b 85 58 fe ff ff    	mov    eax,DWORD PTR [ebp-0x1a8]
  407031:	89 85 e4 fe ff ff    	mov    DWORD PTR [ebp-0x11c],eax
  407037:	c6 45 fc 00          	mov    BYTE PTR [ebp-0x4],0x0
  40703b:	8b 8d e4 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x11c]
  407041:	89 8d e8 fe ff ff    	mov    DWORD PTR [ebp-0x118],ecx
  407047:	8d 95 e8 fe ff ff    	lea    edx,[ebp-0x118]
  40704d:	52                   	push   edx
  40704e:	b9 a8 88 4c 00       	mov    ecx,0x4c88a8
  407053:	e8 38 a8 ff ff       	call   0x401890
  407058:	68 58 01 00 00       	push   0x158
  40705d:	e8 46 51 09 00       	call   0x49c1a8
  407062:	83 c4 04             	add    esp,0x4
  407065:	89 85 d8 fe ff ff    	mov    DWORD PTR [ebp-0x128],eax
  40706b:	c6 45 fc 05          	mov    BYTE PTR [ebp-0x4],0x5
  40706f:	83 bd d8 fe ff ff 00 	cmp    DWORD PTR [ebp-0x128],0x0
  407076:	74 13                	je     0x40708b
  407078:	8b 8d d8 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x128]
  40707e:	e8 ed b7 ff ff       	call   0x402870
  407083:	89 85 54 fe ff ff    	mov    DWORD PTR [ebp-0x1ac],eax
  407089:	eb 0a                	jmp    0x407095
  40708b:	c7 85 54 fe ff ff 00 	mov    DWORD PTR [ebp-0x1ac],0x0
  407092:	00 00 00
  407095:	8b 85 54 fe ff ff    	mov    eax,DWORD PTR [ebp-0x1ac]
  40709b:	89 85 dc fe ff ff    	mov    DWORD PTR [ebp-0x124],eax
  4070a1:	c6 45 fc 00          	mov    BYTE PTR [ebp-0x4],0x0
  4070a5:	8b 8d dc fe ff ff    	mov    ecx,DWORD PTR [ebp-0x124]
  4070ab:	89 4d f0             	mov    DWORD PTR [ebp-0x10],ecx
  4070ae:	6a 0a                	push   0xa
  4070b0:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  4070b3:	e8 28 bc ff ff       	call   0x402ce0
  4070b8:	68 00 c0 5f 40       	push   0x405fc000
  4070bd:	6a 00                	push   0x0
  4070bf:	68 00 c0 5f 40       	push   0x405fc000
  4070c4:	6a 00                	push   0x0
  4070c6:	68 00 e0 6f 40       	push   0x406fe000
  4070cb:	6a 00                	push   0x0
  4070cd:	8d 8d c0 fe ff ff    	lea    ecx,[ebp-0x140]
  4070d3:	e8 18 a7 ff ff       	call   0x4017f0
  4070d8:	50                   	push   eax
  4070d9:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  4070dc:	81 c1 b0 00 00 00    	add    ecx,0xb0
  4070e2:	e8 e9 a6 ff ff       	call   0x4017d0
  4070e7:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  4070ea:	83 c2 08             	add    edx,0x8
  4070ed:	52                   	push   edx
  4070ee:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  4070f1:	8b 10                	mov    edx,DWORD PTR [eax]
  4070f3:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  4070f6:	ff 52 08             	call   DWORD PTR [edx+0x8]
  4070f9:	68 00 00 e0 3f       	push   0x3fe00000
  4070fe:	6a 00                	push   0x0
  407100:	68 00 00 10 40       	push   0x40100000
  407105:	6a 00                	push   0x0
  407107:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  40710a:	e8 71 bc ff ff       	call   0x402d80
  40710f:	6a 04                	push   0x4
  407111:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  407114:	e8 a7 bc ff ff       	call   0x402dc0
  407119:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  40711c:	83 c0 08             	add    eax,0x8
  40711f:	50                   	push   eax
  407120:	8d 8d 90 fe ff ff    	lea    ecx,[ebp-0x170]
  407126:	51                   	push   ecx
  407127:	b9 10 6a 4c 00       	mov    ecx,0x4c6a10
  40712c:	e8 3b 64 03 00       	call   0x43d56c
  407131:	50                   	push   eax
  407132:	8d 95 78 fe ff ff    	lea    edx,[ebp-0x188]
  407138:	52                   	push   edx
  407139:	8d 45 d8             	lea    eax,[ebp-0x28]
  40713c:	50                   	push   eax
  40713d:	8d 8d a8 fe ff ff    	lea    ecx,[ebp-0x158]
  407143:	51                   	push   ecx
  407144:	b9 10 6a 4c 00       	mov    ecx,0x4c6a10
  407149:	e8 1e 64 03 00       	call   0x43d56c
  40714e:	8b c8                	mov    ecx,eax
  407150:	e8 db a6 ff ff       	call   0x401830
  407155:	50                   	push   eax
  407156:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  407159:	8b 02                	mov    eax,DWORD PTR [edx]
  40715b:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  40715e:	ff 50 54             	call   DWORD PTR [eax+0x54]
  407161:	6a 48                	push   0x48
  407163:	e8 40 50 09 00       	call   0x49c1a8
  407168:	83 c4 04             	add    esp,0x4
  40716b:	89 85 6c fe ff ff    	mov    DWORD PTR [ebp-0x194],eax
  407171:	c6 45 fc 06          	mov    BYTE PTR [ebp-0x4],0x6
  407175:	83 bd 6c fe ff ff 00 	cmp    DWORD PTR [ebp-0x194],0x0
  40717c:	74 20                	je     0x40719e
  40717e:	6a 01                	push   0x1
  407180:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  407183:	51                   	push   ecx
  407184:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  407187:	83 c2 08             	add    edx,0x8
  40718a:	52                   	push   edx
  40718b:	8b 8d 6c fe ff ff    	mov    ecx,DWORD PTR [ebp-0x194]
  407191:	e8 ea ae 02 00       	call   0x432080
  407196:	89 85 50 fe ff ff    	mov    DWORD PTR [ebp-0x1b0],eax
  40719c:	eb 0a                	jmp    0x4071a8
  40719e:	c7 85 50 fe ff ff 00 	mov    DWORD PTR [ebp-0x1b0],0x0
  4071a5:	00 00 00
  4071a8:	8b 85 50 fe ff ff    	mov    eax,DWORD PTR [ebp-0x1b0]
  4071ae:	89 85 70 fe ff ff    	mov    DWORD PTR [ebp-0x190],eax
  4071b4:	c6 45 fc 00          	mov    BYTE PTR [ebp-0x4],0x0
  4071b8:	8b 8d 70 fe ff ff    	mov    ecx,DWORD PTR [ebp-0x190]
  4071be:	89 8d 74 fe ff ff    	mov    DWORD PTR [ebp-0x18c],ecx
  4071c4:	8d 95 74 fe ff ff    	lea    edx,[ebp-0x18c]
  4071ca:	52                   	push   edx
  4071cb:	b9 a8 88 4c 00       	mov    ecx,0x4c88a8
  4071d0:	e8 bb a6 ff ff       	call   0x401890
  4071d5:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  4071dc:	8d 4d d8             	lea    ecx,[ebp-0x28]
  4071df:	e8 cc cc ff ff       	call   0x403eb0
  4071e4:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  4071e7:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  4071ee:	5f                   	pop    edi
  4071ef:	5e                   	pop    esi
  4071f0:	8b e5                	mov    esp,ebp
  4071f2:	5d                   	pop    ebp
  4071f3:	c2 04 00             	ret    0x4


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004071f6 <.text+0x61f6>:
  4071f6:	55                   	push   ebp
  4071f7:	8b ec                	mov    ebp,esp
  4071f9:	51                   	push   ecx
  4071fa:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4071fd:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  407200:	8b 48 04             	mov    ecx,DWORD PTR [eax+0x4]
  407203:	51                   	push   ecx
  407204:	8b 10                	mov    edx,DWORD PTR [eax]
  407206:	52                   	push   edx
  407207:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  40720a:	83 c1 78             	add    ecx,0x78
  40720d:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  407210:	8b 50 78             	mov    edx,DWORD PTR [eax+0x78]
  407213:	ff 52 2c             	call   DWORD PTR [edx+0x2c]
  407216:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  407219:	8b 4d 0c             	mov    ecx,DWORD PTR [ebp+0xc]
  40721c:	dd 40 08             	fld    QWORD PTR [eax+0x8]
  40721f:	dc 01                	fadd   QWORD PTR [ecx]
  407221:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  407224:	dd 5a 08             	fstp   QWORD PTR [edx+0x8]
  407227:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  40722a:	c7 00 00 00 00 00    	mov    DWORD PTR [eax],0x0
  407230:	c7 40 04 00 00 00 00 	mov    DWORD PTR [eax+0x4],0x0
  407237:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  40723a:	83 c1 78             	add    ecx,0x78
  40723d:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  407240:	8b 42 78             	mov    eax,DWORD PTR [edx+0x78]
  407243:	ff 50 44             	call   DWORD PTR [eax+0x44]
  407246:	25 ff 00 00 00       	and    eax,0xff
  40724b:	85 c0                	test   eax,eax
  40724d:	74 07                	je     0x407256
  40724f:	b8 02 00 00 00       	mov    eax,0x2
  407254:	eb 02                	jmp    0x407258
  407256:	33 c0                	xor    eax,eax
  407258:	8b e5                	mov    esp,ebp
  40725a:	5d                   	pop    ebp
  40725b:	c2 08 00             	ret    0x8


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00407260 <.text+0x6260>:
  407260:	55                   	push   ebp
  407261:	8b ec                	mov    ebp,esp
  407263:	51                   	push   ecx
  407264:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  407267:	8b e5                	mov    esp,ebp
  407269:	5d                   	pop    ebp
  40726a:	c2 04 00             	ret    0x4


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004010d0 <.text+0xd0>:
  4010d0:	55                   	push   ebp
  4010d1:	8b ec                	mov    ebp,esp
  4010d3:	51                   	push   ecx
  4010d4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4010d7:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4010da:	e8 b1 72 01 00       	call   0x418390
  4010df:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4010e2:	c7 00 74 02 4b 00    	mov    DWORD PTR [eax],0x4b0274
  4010e8:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4010eb:	8b e5                	mov    esp,ebp
  4010ed:	5d                   	pop    ebp
  4010ee:	c3                   	ret


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00418390 <.text+0x17390>:
  418390:	55                   	push   ebp
  418391:	8b ec                	mov    ebp,esp
  418393:	51                   	push   ecx
  418394:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  418397:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  41839a:	c7 00 40 0b 4b 00    	mov    DWORD PTR [eax],0x4b0b40
  4183a0:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4183a3:	c7 41 08 00 00 00 00 	mov    DWORD PTR [ecx+0x8],0x0
  4183aa:	c7 41 0c 00 00 00 00 	mov    DWORD PTR [ecx+0xc],0x0
  4183b1:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  4183b4:	c6 42 10 00          	mov    BYTE PTR [edx+0x10],0x0
  4183b8:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4183bb:	8b e5                	mov    esp,ebp
  4183bd:	5d                   	pop    ebp
  4183be:	c3                   	ret
