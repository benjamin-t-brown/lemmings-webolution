; Original code evidence for spawned behavior 0x2F
; Executable SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004151f0 <.text+0x141f0>:
  4151f0:	55                   	push   ebp
  4151f1:	8b ec                	mov    ebp,esp
  4151f3:	6a ff                	push   0xffffffff
  4151f5:	68 f9 9e 4a 00       	push   0x4a9ef9
  4151fa:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  415200:	50                   	push   eax
  415201:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  415208:	51                   	push   ecx
  415209:	89 4d f0             	mov    DWORD PTR [ebp-0x10],ecx
  41520c:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  41520f:	e8 7c 31 00 00       	call   0x418390
  415214:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  41521b:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  41521e:	83 c1 18             	add    ecx,0x18
  415221:	e8 aa cf 06 00       	call   0x4821d0
  415226:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  415229:	c7 00 c0 09 4b 00    	mov    DWORD PTR [eax],0x4b09c0
  41522f:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  415232:	c7 41 14 2f 00 00 00 	mov    DWORD PTR [ecx+0x14],0x2f
  415239:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  41523c:	c6 42 10 01          	mov    BYTE PTR [edx+0x10],0x1
  415240:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  415243:	c6 40 11 01          	mov    BYTE PTR [eax+0x11],0x1
  415247:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  41524e:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  415251:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  415254:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  41525b:	8b e5                	mov    esp,ebp
  41525d:	5d                   	pop    ebp
  41525e:	c3                   	ret


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0041527b <.text+0x1427b>:
  41527b:	55                   	push   ebp
  41527c:	8b ec                	mov    ebp,esp
  41527e:	6a ff                	push   0xffffffff
  415280:	68 3f 9f 4a 00       	push   0x4a9f3f
  415285:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  41528b:	50                   	push   eax
  41528c:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  415293:	81 ec b0 00 00 00    	sub    esp,0xb0
  415299:	56                   	push   esi
  41529a:	57                   	push   edi
  41529b:	89 8d 58 ff ff ff    	mov    DWORD PTR [ebp-0xa8],ecx
  4152a1:	c7 45 cc 10 6a 4c 00 	mov    DWORD PTR [ebp-0x34],0x4c6a10
  4152a8:	8d 45 d4             	lea    eax,[ebp-0x2c]
  4152ab:	50                   	push   eax
  4152ac:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  4152af:	e8 cf 9e 01 00       	call   0x42f183
  4152b4:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  4152bb:	dd 45 d4             	fld    QWORD PTR [ebp-0x2c]
  4152be:	e8 e9 72 08 00       	call   0x49c5ac
  4152c3:	89 85 54 ff ff ff    	mov    DWORD PTR [ebp-0xac],eax
  4152c9:	db 85 54 ff ff ff    	fild   DWORD PTR [ebp-0xac]
  4152cf:	83 ec 08             	sub    esp,0x8
  4152d2:	dd 1c 24             	fstp   QWORD PTR [esp]
  4152d5:	8b 4d cc             	mov    ecx,DWORD PTR [ebp-0x34]
  4152d8:	e8 e3 8b ff ff       	call   0x40dec0
  4152dd:	e8 ca 72 08 00       	call   0x49c5ac
  4152e2:	89 45 ec             	mov    DWORD PTR [ebp-0x14],eax
  4152e5:	dd 45 dc             	fld    QWORD PTR [ebp-0x24]
  4152e8:	e8 bf 72 08 00       	call   0x49c5ac
  4152ed:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  4152f0:	2b 41 44             	sub    eax,DWORD PTR [ecx+0x44]
  4152f3:	89 85 50 ff ff ff    	mov    DWORD PTR [ebp-0xb0],eax
  4152f9:	db 85 50 ff ff ff    	fild   DWORD PTR [ebp-0xb0]
  4152ff:	83 ec 08             	sub    esp,0x8
  415302:	dd 1c 24             	fstp   QWORD PTR [esp]
  415305:	8b 4d cc             	mov    ecx,DWORD PTR [ebp-0x34]
  415308:	e8 43 8c ff ff       	call   0x40df50
  41530d:	e8 9a 72 08 00       	call   0x49c5ac
  415312:	89 45 d0             	mov    DWORD PTR [ebp-0x30],eax
  415315:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  415318:	c7 02 00 00 00 00    	mov    DWORD PTR [edx],0x0
  41531e:	c7 42 04 00 00 00 00 	mov    DWORD PTR [edx+0x4],0x0
  415325:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  415328:	e8 bf a3 01 00       	call   0x42f6ec
  41532d:	25 ff 00 00 00       	and    eax,0xff
  415332:	85 c0                	test   eax,eax
  415334:	75 1e                	jne    0x415354
  415336:	c7 45 94 02 00 00 00 	mov    DWORD PTR [ebp-0x6c],0x2
  41533d:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  415344:	8d 4d d4             	lea    ecx,[ebp-0x2c]
  415347:	e8 64 eb fe ff       	call   0x403eb0
  41534c:	8b 45 94             	mov    eax,DWORD PTR [ebp-0x6c]
  41534f:	e9 f6 02 00 00       	jmp    0x41564a
  415354:	db 45 d0             	fild   DWORD PTR [ebp-0x30]
  415357:	83 ec 08             	sub    esp,0x8
  41535a:	dd 1c 24             	fstp   QWORD PTR [esp]
  41535d:	db 45 ec             	fild   DWORD PTR [ebp-0x14]
  415360:	83 ec 08             	sub    esp,0x8
  415363:	dd 1c 24             	fstp   QWORD PTR [esp]
  415366:	8d 4d bc             	lea    ecx,[ebp-0x44]
  415369:	e8 d2 f9 fe ff       	call   0x404d40
  41536e:	8b 45 d0             	mov    eax,DWORD PTR [ebp-0x30]
  415371:	50                   	push   eax
  415372:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  415375:	51                   	push   ecx
  415376:	8b 4d cc             	mov    ecx,DWORD PTR [ebp-0x34]
  415379:	e8 e2 8a ff ff       	call   0x40de60
  41537e:	8b 10                	mov    edx,DWORD PTR [eax]
  415380:	89 55 f0             	mov    DWORD PTR [ebp-0x10],edx
  415383:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  415386:	83 e0 40             	and    eax,0x40
  415389:	85 c0                	test   eax,eax
  41538b:	0f 84 be 01 00 00    	je     0x41554f
  415391:	6a 00                	push   0x0
  415393:	6a 00                	push   0x0
  415395:	db 45 d0             	fild   DWORD PTR [ebp-0x30]
  415398:	83 ec 08             	sub    esp,0x8
  41539b:	dd 1c 24             	fstp   QWORD PTR [esp]
  41539e:	db 45 ec             	fild   DWORD PTR [ebp-0x14]
  4153a1:	83 ec 08             	sub    esp,0x8
  4153a4:	dd 1c 24             	fstp   QWORD PTR [esp]
  4153a7:	8d 8d 7c ff ff ff    	lea    ecx,[ebp-0x84]
  4153ad:	e8 3e c4 fe ff       	call   0x4017f0
  4153b2:	50                   	push   eax
  4153b3:	8d 4d 9c             	lea    ecx,[ebp-0x64]
  4153b6:	e8 15 13 ff ff       	call   0x4066d0
  4153bb:	c6 45 fc 01          	mov    BYTE PTR [ebp-0x4],0x1
  4153bf:	c6 45 b4 00          	mov    BYTE PTR [ebp-0x4c],0x0
  4153c3:	c7 45 b8 00 00 00 00 	mov    DWORD PTR [ebp-0x48],0x0
  4153ca:	eb 09                	jmp    0x4153d5
  4153cc:	8b 4d b8             	mov    ecx,DWORD PTR [ebp-0x48]
  4153cf:	83 c1 01             	add    ecx,0x1
  4153d2:	89 4d b8             	mov    DWORD PTR [ebp-0x48],ecx
  4153d5:	b9 d8 89 4c 00       	mov    ecx,0x4c89d8
  4153da:	e8 11 04 00 00       	call   0x4157f0
  4153df:	39 45 b8             	cmp    DWORD PTR [ebp-0x48],eax
  4153e2:	7d 35                	jge    0x415419
  4153e4:	83 ec 18             	sub    esp,0x18
  4153e7:	b9 06 00 00 00       	mov    ecx,0x6
  4153ec:	8d 75 9c             	lea    esi,[ebp-0x64]
  4153ef:	8b fc                	mov    edi,esp
  4153f1:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4153f3:	8b 55 b8             	mov    edx,DWORD PTR [ebp-0x48]
  4153f6:	52                   	push   edx
  4153f7:	b9 d8 89 4c 00       	mov    ecx,0x4c89d8
  4153fc:	e8 cf 03 00 00       	call   0x4157d0
  415401:	8b c8                	mov    ecx,eax
  415403:	e8 4a b6 00 00       	call   0x420a52
  415408:	25 ff 00 00 00       	and    eax,0xff
  41540d:	85 c0                	test   eax,eax
  41540f:	74 06                	je     0x415417
  415411:	c6 45 b4 01          	mov    BYTE PTR [ebp-0x4c],0x1
  415415:	eb 02                	jmp    0x415419
  415417:	eb b3                	jmp    0x4153cc
  415419:	8b 45 b4             	mov    eax,DWORD PTR [ebp-0x4c]
  41541c:	25 ff 00 00 00       	and    eax,0xff
  415421:	85 c0                	test   eax,eax
  415423:	74 71                	je     0x415496
  415425:	6a 2e                	push   0x2e
  415427:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  41542a:	e8 9a 98 01 00       	call   0x42ecc9
  41542f:	25 ff 00 00 00       	and    eax,0xff
  415434:	85 c0                	test   eax,eax
  415436:	75 59                	jne    0x415491
  415438:	6a 18                	push   0x18
  41543a:	e8 69 6d 08 00       	call   0x49c1a8
  41543f:	83 c4 04             	add    esp,0x4
  415442:	89 85 74 ff ff ff    	mov    DWORD PTR [ebp-0x8c],eax
  415448:	c6 45 fc 02          	mov    BYTE PTR [ebp-0x4],0x2
  41544c:	83 bd 74 ff ff ff 00 	cmp    DWORD PTR [ebp-0x8c],0x0
  415453:	74 13                	je     0x415468
  415455:	8b 8d 74 ff ff ff    	mov    ecx,DWORD PTR [ebp-0x8c]
  41545b:	e8 c0 17 ff ff       	call   0x406c20
  415460:	89 85 4c ff ff ff    	mov    DWORD PTR [ebp-0xb4],eax
  415466:	eb 0a                	jmp    0x415472
  415468:	c7 85 4c ff ff ff 00 	mov    DWORD PTR [ebp-0xb4],0x0
  41546f:	00 00 00
  415472:	8b 8d 4c ff ff ff    	mov    ecx,DWORD PTR [ebp-0xb4]
  415478:	89 8d 78 ff ff ff    	mov    DWORD PTR [ebp-0x88],ecx
  41547e:	c6 45 fc 01          	mov    BYTE PTR [ebp-0x4],0x1
  415482:	8b 95 78 ff ff ff    	mov    edx,DWORD PTR [ebp-0x88]
  415488:	52                   	push   edx
  415489:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  41548c:	e8 05 92 01 00       	call   0x42e696
  415491:	e9 a8 00 00 00       	jmp    0x41553e
  415496:	6a 22                	push   0x22
  415498:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  41549b:	e8 29 98 01 00       	call   0x42ecc9
  4154a0:	25 ff 00 00 00       	and    eax,0xff
  4154a5:	85 c0                	test   eax,eax
  4154a7:	0f 85 91 00 00 00    	jne    0x41553e
  4154ad:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  4154b0:	e8 70 92 01 00       	call   0x42e725
  4154b5:	6a 18                	push   0x18
  4154b7:	e8 ec 6c 08 00       	call   0x49c1a8
  4154bc:	83 c4 04             	add    esp,0x4
  4154bf:	89 85 6c ff ff ff    	mov    DWORD PTR [ebp-0x94],eax
  4154c5:	c6 45 fc 03          	mov    BYTE PTR [ebp-0x4],0x3
  4154c9:	83 bd 6c ff ff ff 00 	cmp    DWORD PTR [ebp-0x94],0x0
  4154d0:	74 13                	je     0x4154e5
  4154d2:	8b 8d 6c ff ff ff    	mov    ecx,DWORD PTR [ebp-0x94]
  4154d8:	e8 23 32 ff ff       	call   0x408700
  4154dd:	89 85 48 ff ff ff    	mov    DWORD PTR [ebp-0xb8],eax
  4154e3:	eb 0a                	jmp    0x4154ef
  4154e5:	c7 85 48 ff ff ff 00 	mov    DWORD PTR [ebp-0xb8],0x0
  4154ec:	00 00 00
  4154ef:	8b 85 48 ff ff ff    	mov    eax,DWORD PTR [ebp-0xb8]
  4154f5:	89 85 70 ff ff ff    	mov    DWORD PTR [ebp-0x90],eax
  4154fb:	c6 45 fc 01          	mov    BYTE PTR [ebp-0x4],0x1
  4154ff:	8b 8d 70 ff ff ff    	mov    ecx,DWORD PTR [ebp-0x90]
  415505:	51                   	push   ecx
  415506:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  415509:	e8 88 91 01 00       	call   0x42e696
  41550e:	c7 85 68 ff ff ff 00 	mov    DWORD PTR [ebp-0x98],0x0
  415515:	00 00 00
  415518:	c6 45 fc 00          	mov    BYTE PTR [ebp-0x4],0x0
  41551c:	8d 4d 9c             	lea    ecx,[ebp-0x64]
  41551f:	e8 8c e9 fe ff       	call   0x403eb0
  415524:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  41552b:	8d 4d d4             	lea    ecx,[ebp-0x2c]
  41552e:	e8 7d e9 fe ff       	call   0x403eb0
  415533:	8b 85 68 ff ff ff    	mov    eax,DWORD PTR [ebp-0x98]
  415539:	e9 0c 01 00 00       	jmp    0x41564a
  41553e:	c6 45 fc 00          	mov    BYTE PTR [ebp-0x4],0x0
  415542:	8d 4d 9c             	lea    ecx,[ebp-0x64]
  415545:	e8 66 e9 fe ff       	call   0x403eb0
  41554a:	e9 dc 00 00 00       	jmp    0x41562b
  41554f:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  415552:	81 e2 00 01 00 00    	and    edx,0x100
  415558:	85 d2                	test   edx,edx
  41555a:	0f 84 cb 00 00 00    	je     0x41562b
  415560:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  415563:	83 c0 68             	add    eax,0x68
  415566:	50                   	push   eax
  415567:	8d 4d bc             	lea    ecx,[ebp-0x44]
  41556a:	e8 91 02 00 00       	call   0x415800
  41556f:	25 ff 00 00 00       	and    eax,0xff
  415574:	85 c0                	test   eax,eax
  415576:	0f 84 af 00 00 00    	je     0x41562b
  41557c:	6a 20                	push   0x20
  41557e:	e8 25 6c 08 00       	call   0x49c1a8
  415583:	83 c4 04             	add    esp,0x4
  415586:	89 85 60 ff ff ff    	mov    DWORD PTR [ebp-0xa0],eax
  41558c:	c6 45 fc 04          	mov    BYTE PTR [ebp-0x4],0x4
  415590:	83 bd 60 ff ff ff 00 	cmp    DWORD PTR [ebp-0xa0],0x0
  415597:	74 20                	je     0x4155b9
  415599:	8b 0d 88 bf 4c 00    	mov    ecx,DWORD PTR ds:0x4cbf88
  41559f:	81 c1 90 01 00 00    	add    ecx,0x190
  4155a5:	51                   	push   ecx
  4155a6:	8b 8d 60 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xa0]
  4155ac:	e8 5f 01 00 00       	call   0x415710
  4155b1:	89 85 44 ff ff ff    	mov    DWORD PTR [ebp-0xbc],eax
  4155b7:	eb 0a                	jmp    0x4155c3
  4155b9:	c7 85 44 ff ff ff 00 	mov    DWORD PTR [ebp-0xbc],0x0
  4155c0:	00 00 00
  4155c3:	8b 95 44 ff ff ff    	mov    edx,DWORD PTR [ebp-0xbc]
  4155c9:	89 95 64 ff ff ff    	mov    DWORD PTR [ebp-0x9c],edx
  4155cf:	c6 45 fc 00          	mov    BYTE PTR [ebp-0x4],0x0
  4155d3:	8b 85 64 ff ff ff    	mov    eax,DWORD PTR [ebp-0x9c]
  4155d9:	89 45 98             	mov    DWORD PTR [ebp-0x68],eax
  4155dc:	8b 4d 98             	mov    ecx,DWORD PTR [ebp-0x68]
  4155df:	83 c1 10             	add    ecx,0x10
  4155e2:	8b 55 bc             	mov    edx,DWORD PTR [ebp-0x44]
  4155e5:	89 11                	mov    DWORD PTR [ecx],edx
  4155e7:	8b 45 c0             	mov    eax,DWORD PTR [ebp-0x40]
  4155ea:	89 41 04             	mov    DWORD PTR [ecx+0x4],eax
  4155ed:	8b 55 c4             	mov    edx,DWORD PTR [ebp-0x3c]
  4155f0:	89 51 08             	mov    DWORD PTR [ecx+0x8],edx
  4155f3:	8b 45 c8             	mov    eax,DWORD PTR [ebp-0x38]
  4155f6:	89 41 0c             	mov    DWORD PTR [ecx+0xc],eax
  4155f9:	8b 4d 98             	mov    ecx,DWORD PTR [ebp-0x68]
  4155fc:	51                   	push   ecx
  4155fd:	b9 8c bf 4c 00       	mov    ecx,0x4cbf8c
  415602:	e8 b9 9a 02 00       	call   0x43f0c0
  415607:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  41560a:	83 c2 68             	add    edx,0x68
  41560d:	8b 45 bc             	mov    eax,DWORD PTR [ebp-0x44]
  415610:	89 02                	mov    DWORD PTR [edx],eax
  415612:	8b 4d c0             	mov    ecx,DWORD PTR [ebp-0x40]
  415615:	89 4a 04             	mov    DWORD PTR [edx+0x4],ecx
  415618:	8b 45 c4             	mov    eax,DWORD PTR [ebp-0x3c]
  41561b:	89 42 08             	mov    DWORD PTR [edx+0x8],eax
  41561e:	8b 4d c8             	mov    ecx,DWORD PTR [ebp-0x38]
  415621:	89 4a 0c             	mov    DWORD PTR [edx+0xc],ecx
  415624:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  415627:	c6 42 5f 01          	mov    BYTE PTR [edx+0x5f],0x1
  41562b:	c7 85 5c ff ff ff 00 	mov    DWORD PTR [ebp-0xa4],0x0
  415632:	00 00 00
  415635:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  41563c:	8d 4d d4             	lea    ecx,[ebp-0x2c]
  41563f:	e8 6c e8 fe ff       	call   0x403eb0
  415644:	8b 85 5c ff ff ff    	mov    eax,DWORD PTR [ebp-0xa4]
  41564a:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  41564d:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  415654:	5f                   	pop    edi
  415655:	5e                   	pop    esi
  415656:	8b e5                	mov    esp,ebp
  415658:	5d                   	pop    ebp
  415659:	c2 08 00             	ret    0x8


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00409310 <.text+0x8310>:
  409310:	55                   	push   ebp
  409311:	8b ec                	mov    ebp,esp
  409313:	51                   	push   ecx
  409314:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  409317:	8b e5                	mov    esp,ebp
  409319:	5d                   	pop    ebp
  40931a:	c2 04 00             	ret    0x4


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00401320 <.text+0x320>:
  401320:	55                   	push   ebp
  401321:	8b ec                	mov    ebp,esp
  401323:	51                   	push   ecx
  401324:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  401327:	8b e5                	mov    esp,ebp
  401329:	5d                   	pop    ebp
  40132a:	c2 04 00             	ret    0x4


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0042f183 <.text+0x2e183>:
  42f183:	55                   	push   ebp
  42f184:	8b ec                	mov    ebp,esp
  42f186:	6a ff                	push   0xffffffff
  42f188:	68 f7 af 4a 00       	push   0x4aaff7
  42f18d:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  42f193:	50                   	push   eax
  42f194:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  42f19b:	83 ec 28             	sub    esp,0x28
  42f19e:	56                   	push   esi
  42f19f:	57                   	push   edi
  42f1a0:	89 4d d4             	mov    DWORD PTR [ebp-0x2c],ecx
  42f1a3:	c7 45 d8 00 00 00 00 	mov    DWORD PTR [ebp-0x28],0x0
  42f1aa:	8d 4d dc             	lea    ecx,[ebp-0x24]
  42f1ad:	e8 ee 74 fd ff       	call   0x4066a0
  42f1b2:	8b 45 d4             	mov    eax,DWORD PTR [ebp-0x2c]
  42f1b5:	dd 40 08             	fld    QWORD PTR [eax+0x8]
  42f1b8:	e8 ef d3 06 00       	call   0x49c5ac
  42f1bd:	89 45 d0             	mov    DWORD PTR [ebp-0x30],eax
  42f1c0:	db 45 d0             	fild   DWORD PTR [ebp-0x30]
  42f1c3:	8b 4d d4             	mov    ecx,DWORD PTR [ebp-0x2c]
  42f1c6:	dc 81 60 03 00 00    	fadd   QWORD PTR [ecx+0x360]
  42f1cc:	dd 5d dc             	fstp   QWORD PTR [ebp-0x24]
  42f1cf:	8b 55 d4             	mov    edx,DWORD PTR [ebp-0x2c]
  42f1d2:	dd 42 10             	fld    QWORD PTR [edx+0x10]
  42f1d5:	e8 d2 d3 06 00       	call   0x49c5ac
  42f1da:	89 45 cc             	mov    DWORD PTR [ebp-0x34],eax
  42f1dd:	db 45 cc             	fild   DWORD PTR [ebp-0x34]
  42f1e0:	8b 45 d4             	mov    eax,DWORD PTR [ebp-0x2c]
  42f1e3:	dc 80 68 03 00 00    	fadd   QWORD PTR [eax+0x368]
  42f1e9:	dd 5d e4             	fstp   QWORD PTR [ebp-0x1c]
  42f1ec:	8b 4d d4             	mov    ecx,DWORD PTR [ebp-0x2c]
  42f1ef:	8b 51 18             	mov    edx,DWORD PTR [ecx+0x18]
  42f1f2:	89 55 ec             	mov    DWORD PTR [ebp-0x14],edx
  42f1f5:	8b 41 1c             	mov    eax,DWORD PTR [ecx+0x1c]
  42f1f8:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  42f1fb:	b9 06 00 00 00       	mov    ecx,0x6
  42f200:	8d 75 dc             	lea    esi,[ebp-0x24]
  42f203:	8b 7d 08             	mov    edi,DWORD PTR [ebp+0x8]
  42f206:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  42f208:	8b 4d d8             	mov    ecx,DWORD PTR [ebp-0x28]
  42f20b:	83 c9 01             	or     ecx,0x1
  42f20e:	89 4d d8             	mov    DWORD PTR [ebp-0x28],ecx
  42f211:	8d 4d dc             	lea    ecx,[ebp-0x24]
  42f214:	e8 97 4c fd ff       	call   0x403eb0
  42f219:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  42f21c:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  42f21f:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  42f226:	5f                   	pop    edi
  42f227:	5e                   	pop    esi
  42f228:	8b e5                	mov    esp,ebp
  42f22a:	5d                   	pop    ebp
  42f22b:	c2 04 00             	ret    0x4


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00415800 <.text+0x14800>:
  415800:	55                   	push   ebp
  415801:	8b ec                	mov    ebp,esp
  415803:	83 ec 08             	sub    esp,0x8
  415806:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  415809:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  41580c:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  41580f:	dd 00                	fld    QWORD PTR [eax]
  415811:	dc 19                	fcomp  QWORD PTR [ecx]
  415813:	df e0                	fnstsw ax
  415815:	f6 c4 40             	test   ah,0x40
  415818:	74 1c                	je     0x415836
  41581a:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  41581d:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  415820:	dd 42 08             	fld    QWORD PTR [edx+0x8]
  415823:	dc 58 08             	fcomp  QWORD PTR [eax+0x8]
  415826:	df e0                	fnstsw ax
  415828:	f6 c4 40             	test   ah,0x40
  41582b:	74 09                	je     0x415836
  41582d:	c7 45 f8 00 00 00 00 	mov    DWORD PTR [ebp-0x8],0x0
  415834:	eb 07                	jmp    0x41583d
  415836:	c7 45 f8 01 00 00 00 	mov    DWORD PTR [ebp-0x8],0x1
  41583d:	8a 45 f8             	mov    al,BYTE PTR [ebp-0x8]
  415840:	8b e5                	mov    esp,ebp
  415842:	5d                   	pop    ebp
  415843:	c2 04 00             	ret    0x4


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004821d0 <.text+0x811d0>:
  4821d0:	55                   	push   ebp
  4821d1:	8b ec                	mov    ebp,esp
  4821d3:	51                   	push   ecx
  4821d4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4821d7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4821da:	8b e5                	mov    esp,ebp
  4821dc:	5d                   	pop    ebp
  4821dd:	c3                   	ret


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0041565c <.text+0x1465c>:
  41565c:	55                   	push   ebp
  41565d:	8b ec                	mov    ebp,esp
  41565f:	83 ec 08             	sub    esp,0x8
  415662:	89 4d f8             	mov    DWORD PTR [ebp-0x8],ecx
  415665:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
  415668:	dd 40 18             	fld    QWORD PTR [eax+0x18]
  41566b:	e8 3c 6f 08 00       	call   0x49c5ac
  415670:	50                   	push   eax
  415671:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  415674:	dd 41 10             	fld    QWORD PTR [ecx+0x10]
  415677:	e8 30 6f 08 00       	call   0x49c5ac
  41567c:	50                   	push   eax
  41567d:	b9 10 6a 4c 00       	mov    ecx,0x4c6a10
  415682:	e8 d9 87 ff ff       	call   0x40de60
  415687:	89 45 fc             	mov    DWORD PTR [ebp-0x4],eax
  41568a:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  41568d:	c7 02 00 00 00 00    	mov    DWORD PTR [edx],0x0
  415693:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
  415696:	dd 40 18             	fld    QWORD PTR [eax+0x18]
  415699:	e8 0e 6f 08 00       	call   0x49c5ac
  41569e:	50                   	push   eax
  41569f:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  4156a2:	dd 41 10             	fld    QWORD PTR [ecx+0x10]
  4156a5:	e8 02 6f 08 00       	call   0x49c5ac
  4156aa:	50                   	push   eax
  4156ab:	8b 55 f8             	mov    edx,DWORD PTR [ebp-0x8]
  4156ae:	dd 42 18             	fld    QWORD PTR [edx+0x18]
  4156b1:	e8 f6 6e 08 00       	call   0x49c5ac
  4156b6:	50                   	push   eax
  4156b7:	8b 45 f8             	mov    eax,DWORD PTR [ebp-0x8]
  4156ba:	dd 40 10             	fld    QWORD PTR [eax+0x10]
  4156bd:	e8 ea 6e 08 00       	call   0x49c5ac
  4156c2:	50                   	push   eax
  4156c3:	b9 10 6a 4c 00       	mov    ecx,0x4c6a10
  4156c8:	e8 d4 59 02 00       	call   0x43b0a1
  4156cd:	33 c0                	xor    eax,eax
  4156cf:	8b e5                	mov    esp,ebp
  4156d1:	5d                   	pop    ebp
  4156d2:	c3                   	ret


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00404d40 <.text+0x3d40>:
  404d40:	55                   	push   ebp
  404d41:	8b ec                	mov    ebp,esp
  404d43:	51                   	push   ecx
  404d44:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  404d47:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  404d4a:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  404d4d:	89 08                	mov    DWORD PTR [eax],ecx
  404d4f:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  404d52:	89 50 04             	mov    DWORD PTR [eax+0x4],edx
  404d55:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  404d58:	8b 4d 10             	mov    ecx,DWORD PTR [ebp+0x10]
  404d5b:	89 48 08             	mov    DWORD PTR [eax+0x8],ecx
  404d5e:	8b 55 14             	mov    edx,DWORD PTR [ebp+0x14]
  404d61:	89 50 0c             	mov    DWORD PTR [eax+0xc],edx
  404d64:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  404d67:	8b e5                	mov    esp,ebp
  404d69:	5d                   	pop    ebp
  404d6a:	c2 10 00             	ret    0x10


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004066d0 <.text+0x56d0>:
  4066d0:	55                   	push   ebp
  4066d1:	8b ec                	mov    ebp,esp
  4066d3:	51                   	push   ecx
  4066d4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4066d7:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4066da:	e8 a1 b4 ff ff       	call   0x401b80
  4066df:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  4066e2:	50                   	push   eax
  4066e3:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4066e6:	e8 15 00 00 00       	call   0x406700
  4066eb:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4066ee:	8b e5                	mov    esp,ebp
  4066f0:	5d                   	pop    ebp
  4066f1:	c2 04 00             	ret    0x4


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00401b80 <.text+0xb80>:
  401b80:	55                   	push   ebp
  401b81:	8b ec                	mov    ebp,esp
  401b83:	51                   	push   ecx
  401b84:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  401b87:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  401b8a:	8b e5                	mov    esp,ebp
  401b8c:	5d                   	pop    ebp
  401b8d:	c3                   	ret


C:/progs/Lemmings Revolution/Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00406700 <.text+0x5700>:
  406700:	55                   	push   ebp
  406701:	8b ec                	mov    ebp,esp
  406703:	51                   	push   ecx
  406704:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  406707:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  40670a:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  40670d:	8b 11                	mov    edx,DWORD PTR [ecx]
  40670f:	89 10                	mov    DWORD PTR [eax],edx
  406711:	8b 49 04             	mov    ecx,DWORD PTR [ecx+0x4]
  406714:	89 48 04             	mov    DWORD PTR [eax+0x4],ecx
  406717:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  40671a:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  40671d:	8b 4a 08             	mov    ecx,DWORD PTR [edx+0x8]
  406720:	89 48 08             	mov    DWORD PTR [eax+0x8],ecx
  406723:	8b 52 0c             	mov    edx,DWORD PTR [edx+0xc]
  406726:	89 50 0c             	mov    DWORD PTR [eax+0xc],edx
  406729:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  40672c:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  40672f:	8b 50 10             	mov    edx,DWORD PTR [eax+0x10]
  406732:	89 51 10             	mov    DWORD PTR [ecx+0x10],edx
  406735:	8b 40 14             	mov    eax,DWORD PTR [eax+0x14]
  406738:	89 41 14             	mov    DWORD PTR [ecx+0x14],eax
  40673b:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  40673e:	8b e5                	mov    esp,ebp
  406740:	5d                   	pop    ebp
  406741:	c2 04 00             	ret    0x4
