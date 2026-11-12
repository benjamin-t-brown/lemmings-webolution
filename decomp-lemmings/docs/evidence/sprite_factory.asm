
C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484eb0 <.text+0x83eb0>:
  484eb0:	55                   	push   ebp
  484eb1:	8b ec                	mov    ebp,esp
  484eb3:	6a ff                	push   0xffffffff
  484eb5:	68 3d e9 4a 00       	push   0x4ae93d
  484eba:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  484ec0:	50                   	push   eax
  484ec1:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  484ec8:	81 ec 50 02 00 00    	sub    esp,0x250
  484ece:	89 8d a8 fd ff ff    	mov    DWORD PTR [ebp-0x258],ecx
  484ed4:	6a 20                	push   0x20
  484ed6:	e8 cd 72 01 00       	call   0x49c1a8
  484edb:	83 c4 04             	add    esp,0x4
  484ede:	89 85 b0 fd ff ff    	mov    DWORD PTR [ebp-0x250],eax
  484ee4:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  484eeb:	83 bd b0 fd ff ff 00 	cmp    DWORD PTR [ebp-0x250],0x0
  484ef2:	74 13                	je     0x484f07
  484ef4:	8b 8d b0 fd ff ff    	mov    ecx,DWORD PTR [ebp-0x250]
  484efa:	e8 51 01 00 00       	call   0x485050
  484eff:	89 85 a4 fd ff ff    	mov    DWORD PTR [ebp-0x25c],eax
  484f05:	eb 0a                	jmp    0x484f11
  484f07:	c7 85 a4 fd ff ff 00 	mov    DWORD PTR [ebp-0x25c],0x0
  484f0e:	00 00 00
  484f11:	8b 85 a4 fd ff ff    	mov    eax,DWORD PTR [ebp-0x25c]
  484f17:	89 85 b4 fd ff ff    	mov    DWORD PTR [ebp-0x24c],eax
  484f1d:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  484f24:	8b 8d b4 fd ff ff    	mov    ecx,DWORD PTR [ebp-0x24c]
  484f2a:	89 8d b8 fd ff ff    	mov    DWORD PTR [ebp-0x248],ecx
  484f30:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  484f33:	52                   	push   edx
  484f34:	68 c4 33 4c 00       	push   0x4c33c4
  484f39:	8d 85 bc fd ff ff    	lea    eax,[ebp-0x244]
  484f3f:	50                   	push   eax
  484f40:	e8 25 7b 01 00       	call   0x49ca6a
  484f45:	83 c4 0c             	add    esp,0xc
  484f48:	8d 8d bc fd ff ff    	lea    ecx,[ebp-0x244]
  484f4e:	51                   	push   ecx
  484f4f:	8d 8d c0 fe ff ff    	lea    ecx,[ebp-0x140]
  484f55:	e8 19 8c ff ff       	call   0x47db73
  484f5a:	c7 45 fc 01 00 00 00 	mov    DWORD PTR [ebp-0x4],0x1
  484f61:	6a 00                	push   0x0
  484f63:	6a 01                	push   0x1
  484f65:	8d 8d c0 fe ff ff    	lea    ecx,[ebp-0x140]
  484f6b:	e8 9d 94 ff ff       	call   0x47e40d
  484f70:	25 ff 00 00 00       	and    eax,0xff
  484f75:	85 c0                	test   eax,eax
  484f77:	75 1d                	jne    0x484f96
  484f79:	8d 95 bc fd ff ff    	lea    edx,[ebp-0x244]
  484f7f:	52                   	push   edx
  484f80:	6a 1f                	push   0x1f
  484f82:	68 d4 33 4c 00       	push   0x4c33d4
  484f87:	e8 76 da ff ff       	call   0x482a02
  484f8c:	83 c4 0c             	add    esp,0xc
  484f8f:	6a 00                	push   0x0
  484f91:	e8 ed 83 01 00       	call   0x49d383
  484f96:	8d 85 c0 fe ff ff    	lea    eax,[ebp-0x140]
  484f9c:	50                   	push   eax
  484f9d:	8b 8d b8 fd ff ff    	mov    ecx,DWORD PTR [ebp-0x248]
  484fa3:	e8 48 ef ff ff       	call   0x483ef0
  484fa8:	8d 8d c0 fe ff ff    	lea    ecx,[ebp-0x140]
  484fae:	e8 91 98 ff ff       	call   0x47e844
  484fb3:	8b 8d b8 fd ff ff    	mov    ecx,DWORD PTR [ebp-0x248]
  484fb9:	89 8d ac fd ff ff    	mov    DWORD PTR [ebp-0x254],ecx
  484fbf:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  484fc6:	8d 8d c0 fe ff ff    	lea    ecx,[ebp-0x140]
  484fcc:	e8 57 8c ff ff       	call   0x47dc28
  484fd1:	8b 85 ac fd ff ff    	mov    eax,DWORD PTR [ebp-0x254]
  484fd7:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  484fda:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  484fe1:	8b e5                	mov    esp,ebp
  484fe3:	5d                   	pop    ebp
  484fe4:	c2 04 00             	ret    0x4


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484fe7 <.text+0x83fe7>:
  484fe7:	55                   	push   ebp
  484fe8:	8b ec                	mov    ebp,esp
  484fea:	83 ec 10             	sub    esp,0x10
  484fed:	89 4d f4             	mov    DWORD PTR [ebp-0xc],ecx
  484ff0:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  484ff3:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
  484ff6:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  484ff9:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484ffc:	83 7d fc 00          	cmp    DWORD PTR [ebp-0x4],0x0
  485000:	74 0f                	je     0x485011
  485002:	6a 01                	push   0x1
  485004:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  485007:	e8 94 00 00 00       	call   0x4850a0
  48500c:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  48500f:	eb 07                	jmp    0x485018
  485011:	c7 45 f0 00 00 00 00 	mov    DWORD PTR [ebp-0x10],0x0
  485018:	8b e5                	mov    esp,ebp
  48501a:	5d                   	pop    ebp
  48501b:	c2 04 00             	ret    0x4


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00485050 <.text+0x84050>:
  485050:	55                   	push   ebp
  485051:	8b ec                	mov    ebp,esp
  485053:	6a ff                	push   0xffffffff
  485055:	68 59 e9 4a 00       	push   0x4ae959
  48505a:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  485060:	50                   	push   eax
  485061:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  485068:	51                   	push   ecx
  485069:	89 4d f0             	mov    DWORD PTR [ebp-0x10],ecx
  48506c:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  48506f:	e8 9c 01 00 00       	call   0x485210
  485074:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  48507b:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  48507e:	83 c1 10             	add    ecx,0x10
  485081:	e8 ea 01 00 00       	call   0x485270
  485086:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  48508d:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  485090:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  485093:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  48509a:	8b e5                	mov    esp,ebp
  48509c:	5d                   	pop    ebp
  48509d:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00485210 <.text+0x84210>:
  485210:	55                   	push   ebp
  485211:	8b ec                	mov    ebp,esp
  485213:	51                   	push   ecx
  485214:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  485217:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  48521a:	c7 00 00 00 00 00    	mov    DWORD PTR [eax],0x0
  485220:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  485223:	c7 41 04 00 00 00 00 	mov    DWORD PTR [ecx+0x4],0x0
  48522a:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  48522d:	c7 42 08 01 00 00 00 	mov    DWORD PTR [edx+0x8],0x1
  485234:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  485237:	c7 40 0c 00 00 00 00 	mov    DWORD PTR [eax+0xc],0x0
  48523e:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  485241:	8b e5                	mov    esp,ebp
  485243:	5d                   	pop    ebp
  485244:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00485270 <.text+0x84270>:
  485270:	55                   	push   ebp
  485271:	8b ec                	mov    ebp,esp
  485273:	51                   	push   ecx
  485274:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  485277:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  48527a:	c7 00 00 00 00 00    	mov    DWORD PTR [eax],0x0
  485280:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  485283:	c7 41 04 00 00 00 00 	mov    DWORD PTR [ecx+0x4],0x0
  48528a:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  48528d:	c7 42 08 01 00 00 00 	mov    DWORD PTR [edx+0x8],0x1
  485294:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  485297:	c7 40 0c 00 00 00 00 	mov    DWORD PTR [eax+0xc],0x0
  48529e:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4852a1:	8b e5                	mov    esp,ebp
  4852a3:	5d                   	pop    ebp
  4852a4:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004850a0 <.text+0x840a0>:
  4850a0:	55                   	push   ebp
  4850a1:	8b ec                	mov    ebp,esp
  4850a3:	51                   	push   ecx
  4850a4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4850a7:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4850aa:	e8 21 00 00 00       	call   0x4850d0
  4850af:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  4850b2:	83 e0 01             	and    eax,0x1
  4850b5:	85 c0                	test   eax,eax
  4850b7:	74 0c                	je     0x4850c5
  4850b9:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4850bc:	51                   	push   ecx
  4850bd:	e8 7b 70 01 00       	call   0x49c13d
  4850c2:	83 c4 04             	add    esp,0x4
  4850c5:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4850c8:	8b e5                	mov    esp,ebp
  4850ca:	5d                   	pop    ebp
  4850cb:	c2 04 00             	ret    0x4


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004850d0 <.text+0x840d0>:
  4850d0:	55                   	push   ebp
  4850d1:	8b ec                	mov    ebp,esp
  4850d3:	6a ff                	push   0xffffffff
  4850d5:	68 79 e9 4a 00       	push   0x4ae979
  4850da:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  4850e0:	50                   	push   eax
  4850e1:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  4850e8:	51                   	push   ecx
  4850e9:	89 4d f0             	mov    DWORD PTR [ebp-0x10],ecx
  4850ec:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  4850f3:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  4850f6:	83 c1 10             	add    ecx,0x10
  4850f9:	e8 b2 01 00 00       	call   0x4852b0
  4850fe:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  485105:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  485108:	e8 43 01 00 00       	call   0x485250
  48510d:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  485110:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  485117:	8b e5                	mov    esp,ebp
  485119:	5d                   	pop    ebp
  48511a:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00485250 <.text+0x84250>:
  485250:	55                   	push   ebp
  485251:	8b ec                	mov    ebp,esp
  485253:	51                   	push   ecx
  485254:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  485257:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  48525a:	e8 21 f3 ff ff       	call   0x484580
  48525f:	8b e5                	mov    esp,ebp
  485261:	5d                   	pop    ebp
  485262:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004852b0 <.text+0x842b0>:
  4852b0:	55                   	push   ebp
  4852b1:	8b ec                	mov    ebp,esp
  4852b3:	51                   	push   ecx
  4852b4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4852b7:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4852ba:	e8 e1 f4 ff ff       	call   0x4847a0
  4852bf:	8b e5                	mov    esp,ebp
  4852c1:	5d                   	pop    ebp
  4852c2:	c3                   	ret
