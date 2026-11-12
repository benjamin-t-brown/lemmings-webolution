; Reference SHA-256: 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f
; Only 0x442B00..0x44315F is claimed by update_world_entity_collections.

; VA 0x004423D2, file offset 271314, length 3569, SHA256 e1b02be8cfe9fb35345494b637c8a07db697ff288f9e52914372293637628fbf

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004423d2 <.text+0x413d2>:
  4423d2:	55                   	push   ebp
  4423d3:	8b ec                	mov    ebp,esp
  4423d5:	6a ff                	push   0xffffffff
  4423d7:	68 fd c2 4a 00       	push   0x4ac2fd
  4423dc:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  4423e2:	50                   	push   eax
  4423e3:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  4423ea:	81 ec e8 00 00 00    	sub    esp,0xe8
  4423f0:	89 8d 40 ff ff ff    	mov    DWORD PTR [ebp-0xc0],ecx
  4423f6:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  4423fc:	33 c9                	xor    ecx,ecx
  4423fe:	8a 88 39 51 00 00    	mov    cl,BYTE PTR [eax+0x5139]
  442404:	85 c9                	test   ecx,ecx
  442406:	75 11                	jne    0x442419
  442408:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  44240e:	c7 02 01 00 00 00    	mov    DWORD PTR [edx],0x1
  442414:	e9 9a 0d 00 00       	jmp    0x4431b3
  442419:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  44241f:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  442422:	89 88 20 51 00 00    	mov    DWORD PTR [eax+0x5120],ecx
  442428:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  44242b:	89 90 24 51 00 00    	mov    DWORD PTR [eax+0x5124],edx
  442431:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  442434:	50                   	push   eax
  442435:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  442438:	51                   	push   ecx
  442439:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  44243f:	81 c1 40 51 00 00    	add    ecx,0x5140
  442445:	e8 94 57 ff ff       	call   0x437bde
  44244a:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  442450:	83 ba b8 50 00 00 00 	cmp    DWORD PTR [edx+0x50b8],0x0
  442457:	74 19                	je     0x442472
  442459:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  44245c:	50                   	push   eax
  44245d:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  442460:	51                   	push   ecx
  442461:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  442467:	8b 8a b8 50 00 00    	mov    ecx,DWORD PTR [edx+0x50b8]
  44246d:	e8 77 ed 04 00       	call   0x4911e9
  442472:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  442478:	83 b8 c0 50 00 00 00 	cmp    DWORD PTR [eax+0x50c0],0x0
  44247f:	74 19                	je     0x44249a
  442481:	8b 4d 0c             	mov    ecx,DWORD PTR [ebp+0xc]
  442484:	51                   	push   ecx
  442485:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  442488:	52                   	push   edx
  442489:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  44248f:	8b 88 c0 50 00 00    	mov    ecx,DWORD PTR [eax+0x50c0]
  442495:	e8 46 33 00 00       	call   0x4457e0
  44249a:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4424a0:	83 b9 b4 50 00 00 00 	cmp    DWORD PTR [ecx+0x50b4],0x0
  4424a7:	74 19                	je     0x4424c2
  4424a9:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  4424ac:	52                   	push   edx
  4424ad:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  4424b0:	50                   	push   eax
  4424b1:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4424b7:	8b 89 b4 50 00 00    	mov    ecx,DWORD PTR [ecx+0x50b4]
  4424bd:	e8 9a f3 04 00       	call   0x49185c
  4424c2:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  4424c8:	83 ba bc 50 00 00 00 	cmp    DWORD PTR [edx+0x50bc],0x0
  4424cf:	74 19                	je     0x4424ea
  4424d1:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  4424d4:	50                   	push   eax
  4424d5:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  4424d8:	51                   	push   ecx
  4424d9:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  4424df:	8b 8a bc 50 00 00    	mov    ecx,DWORD PTR [edx+0x50bc]
  4424e5:	e8 b2 00 05 00       	call   0x49259c
  4424ea:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  4424f0:	83 b8 c8 50 00 00 00 	cmp    DWORD PTR [eax+0x50c8],0x0
  4424f7:	74 19                	je     0x442512
  4424f9:	8b 4d 0c             	mov    ecx,DWORD PTR [ebp+0xc]
  4424fc:	51                   	push   ecx
  4424fd:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  442500:	52                   	push   edx
  442501:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  442507:	8b 88 c8 50 00 00    	mov    ecx,DWORD PTR [eax+0x50c8]
  44250d:	e8 de 32 00 00       	call   0x4457f0
  442512:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442518:	81 c1 8c 53 00 00    	add    ecx,0x538c
  44251e:	e8 1d 5b 00 00       	call   0x448040
  442523:	85 c0                	test   eax,eax
  442525:	0f 85 cf 00 00 00    	jne    0x4425fa
  44252b:	6a 00                	push   0x0
  44252d:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442533:	81 c1 70 01 00 00    	add    ecx,0x170
  442539:	e8 52 09 fd ff       	call   0x412e90
  44253e:	8b 08                	mov    ecx,DWORD PTR [eax]
  442540:	89 4d e0             	mov    DWORD PTR [ebp-0x20],ecx
  442543:	8b 55 e0             	mov    edx,DWORD PTR [ebp-0x20]
  442546:	52                   	push   edx
  442547:	b9 48 cb 58 00       	mov    ecx,0x58cb48
  44254c:	e8 bf 24 fd ff       	call   0x414a10
  442551:	89 45 e4             	mov    DWORD PTR [ebp-0x1c],eax
  442554:	8d 4d b8             	lea    ecx,[ebp-0x48]
  442557:	e8 34 4e 00 00       	call   0x447390
  44255c:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  442563:	8d 4d 90             	lea    ecx,[ebp-0x70]
  442566:	e8 25 4e 00 00       	call   0x447390
  44256b:	c6 45 fc 01          	mov    BYTE PTR [ebp-0x4],0x1
  44256f:	68 51 b8 ae 3f       	push   0x3faeb851
  442574:	68 b8 1e 85 eb       	push   0xeb851eb8
  442579:	68 f7 00 00 00       	push   0xf7
  44257e:	68 f0 00 00 00       	push   0xf0
  442583:	8b 45 e4             	mov    eax,DWORD PTR [ebp-0x1c]
  442586:	83 c0 1c             	add    eax,0x1c
  442589:	50                   	push   eax
  44258a:	8d 4d b8             	lea    ecx,[ebp-0x48]
  44258d:	e8 4e 86 03 00       	call   0x47abe0
  442592:	68 51 b8 ae 3f       	push   0x3faeb851
  442597:	68 b8 1e 85 eb       	push   0xeb851eb8
  44259c:	68 ff 00 00 00       	push   0xff
  4425a1:	68 f8 00 00 00       	push   0xf8
  4425a6:	8b 4d e4             	mov    ecx,DWORD PTR [ebp-0x1c]
  4425a9:	83 c1 1c             	add    ecx,0x1c
  4425ac:	51                   	push   ecx
  4425ad:	8d 4d 90             	lea    ecx,[ebp-0x70]
  4425b0:	e8 2b 86 03 00       	call   0x47abe0
  4425b5:	8d 55 b8             	lea    edx,[ebp-0x48]
  4425b8:	52                   	push   edx
  4425b9:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4425bf:	81 c1 8c 53 00 00    	add    ecx,0x538c
  4425c5:	e8 86 5a 00 00       	call   0x448050
  4425ca:	8d 45 90             	lea    eax,[ebp-0x70]
  4425cd:	50                   	push   eax
  4425ce:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4425d4:	81 c1 8c 53 00 00    	add    ecx,0x538c
  4425da:	e8 71 5a 00 00       	call   0x448050
  4425df:	c6 45 fc 00          	mov    BYTE PTR [ebp-0x4],0x0
  4425e3:	8d 4d 90             	lea    ecx,[ebp-0x70]
  4425e6:	e8 1a 86 03 00       	call   0x47ac05
  4425eb:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  4425f2:	8d 4d b8             	lea    ecx,[ebp-0x48]
  4425f5:	e8 0b 86 03 00       	call   0x47ac05
  4425fa:	8b 4d 0c             	mov    ecx,DWORD PTR [ebp+0xc]
  4425fd:	51                   	push   ecx
  4425fe:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  442601:	52                   	push   edx
  442602:	6a 00                	push   0x0
  442604:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  44260a:	81 c1 8c 53 00 00    	add    ecx,0x538c
  442610:	e8 0b 5a 00 00       	call   0x448020
  442615:	8b c8                	mov    ecx,eax
  442617:	e8 ff 85 03 00       	call   0x47ac1b
  44261c:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  44261f:	50                   	push   eax
  442620:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  442623:	51                   	push   ecx
  442624:	6a 01                	push   0x1
  442626:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  44262c:	81 c1 8c 53 00 00    	add    ecx,0x538c
  442632:	e8 e9 59 00 00       	call   0x448020
  442637:	8b c8                	mov    ecx,eax
  442639:	e8 dd 85 03 00       	call   0x47ac1b
  44263e:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  442644:	33 c0                	xor    eax,eax
  442646:	8a 82 38 51 00 00    	mov    al,BYTE PTR [edx+0x5138]
  44264c:	85 c0                	test   eax,eax
  44264e:	0f 84 de 03 00 00    	je     0x442a32
  442654:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  44265a:	0f bf 91 08 53 00 00 	movsx  edx,WORD PTR [ecx+0x5308]
  442661:	85 d2                	test   edx,edx
  442663:	0f 85 ea 00 00 00    	jne    0x442753
  442669:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  44266f:	83 b8 9c 53 00 00 00 	cmp    DWORD PTR [eax+0x539c],0x0
  442676:	0f 85 ba 00 00 00    	jne    0x442736
  44267c:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442682:	33 d2                	xor    edx,edx
  442684:	8a 91 d3 50 00 00    	mov    dl,BYTE PTR [ecx+0x50d3]
  44268a:	85 d2                	test   edx,edx
  44268c:	0f 85 a4 00 00 00    	jne    0x442736
  442692:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  442698:	c6 80 a0 53 00 00 00 	mov    BYTE PTR [eax+0x53a0],0x0
  44269f:	6a 50                	push   0x50
  4426a1:	e8 02 9b 05 00       	call   0x49c1a8
  4426a6:	83 c4 04             	add    esp,0x4
  4426a9:	89 85 5c ff ff ff    	mov    DWORD PTR [ebp-0xa4],eax
  4426af:	c7 45 fc 02 00 00 00 	mov    DWORD PTR [ebp-0x4],0x2
  4426b6:	83 bd 5c ff ff ff 00 	cmp    DWORD PTR [ebp-0xa4],0x0
  4426bd:	74 17                	je     0x4426d6
  4426bf:	6a 05                	push   0x5
  4426c1:	6a 05                	push   0x5
  4426c3:	8b 8d 5c ff ff ff    	mov    ecx,DWORD PTR [ebp-0xa4]
  4426c9:	e8 72 16 fd ff       	call   0x413d40
  4426ce:	89 85 3c ff ff ff    	mov    DWORD PTR [ebp-0xc4],eax
  4426d4:	eb 0a                	jmp    0x4426e0
  4426d6:	c7 85 3c ff ff ff 00 	mov    DWORD PTR [ebp-0xc4],0x0
  4426dd:	00 00 00
  4426e0:	8b 8d 3c ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc4]
  4426e6:	89 8d 60 ff ff ff    	mov    DWORD PTR [ebp-0xa0],ecx
  4426ec:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  4426f3:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  4426f9:	8b 85 60 ff ff ff    	mov    eax,DWORD PTR [ebp-0xa0]
  4426ff:	89 82 9c 53 00 00    	mov    DWORD PTR [edx+0x539c],eax
  442705:	c7 45 80 00 00 00 00 	mov    DWORD PTR [ebp-0x80],0x0
  44270c:	c7 45 84 00 00 00 00 	mov    DWORD PTR [ebp-0x7c],0x0
  442713:	8d 4d 80             	lea    ecx,[ebp-0x80]
  442716:	51                   	push   ecx
  442717:	6a 00                	push   0x0
  442719:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  44271f:	8b 8a 9c 53 00 00    	mov    ecx,DWORD PTR [edx+0x539c]
  442725:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  44272b:	8b 90 9c 53 00 00    	mov    edx,DWORD PTR [eax+0x539c]
  442731:	8b 02                	mov    eax,DWORD PTR [edx]
  442733:	ff 50 04             	call   DWORD PTR [eax+0x4]
  442736:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  44273c:	81 c1 c8 1e 00 00    	add    ecx,0x1ec8
  442742:	e8 49 40 00 00       	call   0x446790
  442747:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  44274d:	89 81 04 53 00 00    	mov    DWORD PTR [ecx+0x5304],eax
  442753:	b9 08 6a 4c 00       	mov    ecx,0x4c6a08
  442758:	e8 d9 2c 00 00       	call   0x445436
  44275d:	85 c0                	test   eax,eax
  44275f:	74 1a                	je     0x44277b
  442761:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442767:	e8 c8 22 00 00       	call   0x444a34
  44276c:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  442772:	66 c7 82 98 50 00 00 	mov    WORD PTR [edx+0x5098],0x0
  442779:	00 00
  44277b:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  44277e:	89 45 88             	mov    DWORD PTR [ebp-0x78],eax
  442781:	8b 4d 0c             	mov    ecx,DWORD PTR [ebp+0xc]
  442784:	89 4d 8c             	mov    DWORD PTR [ebp-0x74],ecx
  442787:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  44278d:	83 ba 9c 53 00 00 00 	cmp    DWORD PTR [edx+0x539c],0x0
  442794:	0f 84 e7 00 00 00    	je     0x442881
  44279a:	6a 01                	push   0x1
  44279c:	68 f0 00 00 00       	push   0xf0
  4427a1:	68 40 01 00 00       	push   0x140
  4427a6:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  4427ac:	8b 88 9c 53 00 00    	mov    ecx,DWORD PTR [eax+0x539c]
  4427b2:	e8 7b 19 fd ff       	call   0x414132
  4427b7:	8d 4d 88             	lea    ecx,[ebp-0x78]
  4427ba:	51                   	push   ecx
  4427bb:	6a 00                	push   0x0
  4427bd:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  4427c3:	8b 8a 9c 53 00 00    	mov    ecx,DWORD PTR [edx+0x539c]
  4427c9:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  4427cf:	8b 90 9c 53 00 00    	mov    edx,DWORD PTR [eax+0x539c]
  4427d5:	8b 02                	mov    eax,DWORD PTR [edx]
  4427d7:	ff 50 04             	call   DWORD PTR [eax+0x4]
  4427da:	89 85 7c ff ff ff    	mov    DWORD PTR [ebp-0x84],eax
  4427e0:	83 bd 7c ff ff ff 01 	cmp    DWORD PTR [ebp-0x84],0x1
  4427e7:	0f 85 94 00 00 00    	jne    0x442881
  4427ed:	6a 00                	push   0x0
  4427ef:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4427f5:	8b 89 9c 53 00 00    	mov    ecx,DWORD PTR [ecx+0x539c]
  4427fb:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  442801:	8b 82 9c 53 00 00    	mov    eax,DWORD PTR [edx+0x539c]
  442807:	8b 10                	mov    edx,DWORD PTR [eax]
  442809:	ff 52 0c             	call   DWORD PTR [edx+0xc]
  44280c:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  442812:	8b 88 9c 53 00 00    	mov    ecx,DWORD PTR [eax+0x539c]
  442818:	89 8d 54 ff ff ff    	mov    DWORD PTR [ebp-0xac],ecx
  44281e:	8b 95 54 ff ff ff    	mov    edx,DWORD PTR [ebp-0xac]
  442824:	89 95 58 ff ff ff    	mov    DWORD PTR [ebp-0xa8],edx
  44282a:	83 bd 58 ff ff ff 00 	cmp    DWORD PTR [ebp-0xa8],0x0
  442831:	74 1a                	je     0x44284d
  442833:	6a 01                	push   0x1
  442835:	8b 85 58 ff ff ff    	mov    eax,DWORD PTR [ebp-0xa8]
  44283b:	8b 10                	mov    edx,DWORD PTR [eax]
  44283d:	8b 8d 58 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xa8]
  442843:	ff 12                	call   DWORD PTR [edx]
  442845:	89 85 38 ff ff ff    	mov    DWORD PTR [ebp-0xc8],eax
  44284b:	eb 0a                	jmp    0x442857
  44284d:	c7 85 38 ff ff ff 00 	mov    DWORD PTR [ebp-0xc8],0x0
  442854:	00 00 00
  442857:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  44285d:	c7 80 9c 53 00 00 00 	mov    DWORD PTR [eax+0x539c],0x0
  442864:	00 00 00
  442867:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  44286d:	c6 81 a0 53 00 00 01 	mov    BYTE PTR [ecx+0x53a0],0x1
  442874:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  44287a:	c6 82 d3 50 00 00 01 	mov    BYTE PTR [edx+0x50d3],0x1
  442881:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  442887:	33 c9                	xor    ecx,ecx
  442889:	8a 88 a0 53 00 00    	mov    cl,BYTE PTR [eax+0x53a0]
  44288f:	85 c9                	test   ecx,ecx
  442891:	0f 84 80 01 00 00    	je     0x442a17
  442897:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  44289d:	8b 82 04 53 00 00    	mov    eax,DWORD PTR [edx+0x5304]
  4428a3:	50                   	push   eax
  4428a4:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4428aa:	81 c1 c8 1e 00 00    	add    ecx,0x1ec8
  4428b0:	e8 fb 3e 00 00       	call   0x4467b0
  4428b5:	25 ff 00 00 00       	and    eax,0xff
  4428ba:	85 c0                	test   eax,eax
  4428bc:	0f 85 55 01 00 00    	jne    0x442a17
  4428c2:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4428c8:	dd 81 28 51 00 00    	fld    QWORD PTR [ecx+0x5128]
  4428ce:	dc 1d 38 03 4b 00    	fcomp  QWORD PTR ds:0x4b0338
  4428d4:	df e0                	fnstsw ax
  4428d6:	f6 c4 01             	test   ah,0x1
  4428d9:	0f 84 38 01 00 00    	je     0x442a17
  4428df:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  4428e5:	8b 82 04 53 00 00    	mov    eax,DWORD PTR [edx+0x5304]
  4428eb:	8b 08                	mov    ecx,DWORD PTR [eax]
  4428ed:	89 8d 74 ff ff ff    	mov    DWORD PTR [ebp-0x8c],ecx
  4428f3:	8b 95 74 ff ff ff    	mov    edx,DWORD PTR [ebp-0x8c]
  4428f9:	8b 82 40 03 00 00    	mov    eax,DWORD PTR [edx+0x340]
  4428ff:	89 85 78 ff ff ff    	mov    DWORD PTR [ebp-0x88],eax
  442905:	6a 18                	push   0x18
  442907:	e8 9c 98 05 00       	call   0x49c1a8
  44290c:	83 c4 04             	add    esp,0x4
  44290f:	89 85 4c ff ff ff    	mov    DWORD PTR [ebp-0xb4],eax
  442915:	c7 45 fc 03 00 00 00 	mov    DWORD PTR [ebp-0x4],0x3
  44291c:	83 bd 4c ff ff ff 00 	cmp    DWORD PTR [ebp-0xb4],0x0
  442923:	74 13                	je     0x442938
  442925:	8b 8d 4c ff ff ff    	mov    ecx,DWORD PTR [ebp-0xb4]
  44292b:	e8 e0 3f fd ff       	call   0x416910
  442930:	89 85 34 ff ff ff    	mov    DWORD PTR [ebp-0xcc],eax
  442936:	eb 0a                	jmp    0x442942
  442938:	c7 85 34 ff ff ff 00 	mov    DWORD PTR [ebp-0xcc],0x0
  44293f:	00 00 00
  442942:	8b 8d 34 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xcc]
  442948:	89 8d 50 ff ff ff    	mov    DWORD PTR [ebp-0xb0],ecx
  44294e:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  442955:	8b 95 50 ff ff ff    	mov    edx,DWORD PTR [ebp-0xb0]
  44295b:	52                   	push   edx
  44295c:	8b 8d 74 ff ff ff    	mov    ecx,DWORD PTR [ebp-0x8c]
  442962:	e8 2f bd fe ff       	call   0x42e696
  442967:	8b 85 74 ff ff ff    	mov    eax,DWORD PTR [ebp-0x8c]
  44296d:	8b 8d 78 ff ff ff    	mov    ecx,DWORD PTR [ebp-0x88]
  442973:	3b 88 40 03 00 00    	cmp    ecx,DWORD PTR [eax+0x340]
  442979:	74 45                	je     0x4429c0
  44297b:	8b 95 78 ff ff ff    	mov    edx,DWORD PTR [ebp-0x88]
  442981:	89 95 44 ff ff ff    	mov    DWORD PTR [ebp-0xbc],edx
  442987:	8b 85 44 ff ff ff    	mov    eax,DWORD PTR [ebp-0xbc]
  44298d:	89 85 48 ff ff ff    	mov    DWORD PTR [ebp-0xb8],eax
  442993:	83 bd 48 ff ff ff 00 	cmp    DWORD PTR [ebp-0xb8],0x0
  44299a:	74 1a                	je     0x4429b6
  44299c:	6a 01                	push   0x1
  44299e:	8b 8d 48 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xb8]
  4429a4:	8b 11                	mov    edx,DWORD PTR [ecx]
  4429a6:	8b 8d 48 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xb8]
  4429ac:	ff 12                	call   DWORD PTR [edx]
  4429ae:	89 85 30 ff ff ff    	mov    DWORD PTR [ebp-0xd0],eax
  4429b4:	eb 0a                	jmp    0x4429c0
  4429b6:	c7 85 30 ff ff ff 00 	mov    DWORD PTR [ebp-0xd0],0x0
  4429bd:	00 00 00
  4429c0:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  4429c6:	8b 88 04 53 00 00    	mov    ecx,DWORD PTR [eax+0x5304]
  4429cc:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  4429d2:	8b 41 08             	mov    eax,DWORD PTR [ecx+0x8]
  4429d5:	89 82 04 53 00 00    	mov    DWORD PTR [edx+0x5304],eax
  4429db:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4429e1:	66 8b 91 08 53 00 00 	mov    dx,WORD PTR [ecx+0x5308]
  4429e8:	66 83 c2 01          	add    dx,0x1
  4429ec:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  4429f2:	66 89 90 08 53 00 00 	mov    WORD PTR [eax+0x5308],dx
  4429f9:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4429ff:	dd 05 e8 15 4b 00    	fld    QWORD PTR ds:0x4b15e8
  442a05:	dc 81 28 51 00 00    	fadd   QWORD PTR [ecx+0x5128]
  442a0b:	8b 95 40 ff ff ff    	mov    edx,DWORD PTR [ebp-0xc0]
  442a11:	dd 9a 28 51 00 00    	fstp   QWORD PTR [edx+0x5128]
  442a17:	8b 85 40 ff ff ff    	mov    eax,DWORD PTR [ebp-0xc0]
  442a1d:	dd 80 28 51 00 00    	fld    QWORD PTR [eax+0x5128]
  442a23:	dc 65 08             	fsub   QWORD PTR [ebp+0x8]
  442a26:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442a2c:	dd 99 28 51 00 00    	fstp   QWORD PTR [ecx+0x5128]
  442a32:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  442a39:	eb 09                	jmp    0x442a44
  442a3b:	8b 55 ec             	mov    edx,DWORD PTR [ebp-0x14]
  442a3e:	83 c2 01             	add    edx,0x1
  442a41:	89 55 ec             	mov    DWORD PTR [ebp-0x14],edx
  442a44:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442a4a:	81 c1 00 51 00 00    	add    ecx,0x5100
  442a50:	e8 6b 51 00 00       	call   0x447bc0
  442a55:	39 45 ec             	cmp    DWORD PTR [ebp-0x14],eax
  442a58:	7d 4a                	jge    0x442aa4
  442a5a:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  442a5d:	50                   	push   eax
  442a5e:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442a64:	81 c1 00 51 00 00    	add    ecx,0x5100
  442a6a:	e8 f1 18 fe ff       	call   0x424360
  442a6f:	89 85 70 ff ff ff    	mov    DWORD PTR [ebp-0x90],eax
  442a75:	8b 8d 70 ff ff ff    	mov    ecx,DWORD PTR [ebp-0x90]
  442a7b:	33 d2                	xor    edx,edx
  442a7d:	8a 91 68 01 00 00    	mov    dl,BYTE PTR [ecx+0x168]
  442a83:	85 d2                	test   edx,edx
  442a85:	75 1b                	jne    0x442aa2
  442a87:	8b 85 70 ff ff ff    	mov    eax,DWORD PTR [ebp-0x90]
  442a8d:	dd 80 70 01 00 00    	fld    QWORD PTR [eax+0x170]
  442a93:	dc 65 08             	fsub   QWORD PTR [ebp+0x8]
  442a96:	8b 8d 70 ff ff ff    	mov    ecx,DWORD PTR [ebp-0x90]
  442a9c:	dd 99 70 01 00 00    	fstp   QWORD PTR [ecx+0x170]
  442aa2:	eb 97                	jmp    0x442a3b
  442aa4:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442aaa:	83 c1 08             	add    ecx,0x8
  442aad:	e8 67 89 ff ff       	call   0x43b419
  442ab2:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  442ab9:	eb 09                	jmp    0x442ac4
  442abb:	8b 55 ec             	mov    edx,DWORD PTR [ebp-0x14]
  442abe:	83 c2 01             	add    edx,0x1
  442ac1:	89 55 ec             	mov    DWORD PTR [ebp-0x14],edx
  442ac4:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442aca:	81 c1 d0 1f 00 00    	add    ecx,0x1fd0
  442ad0:	e8 1b 2d fd ff       	call   0x4157f0
  442ad5:	39 45 ec             	cmp    DWORD PTR [ebp-0x14],eax
  442ad8:	7d 26                	jge    0x442b00
  442ada:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  442add:	50                   	push   eax
  442ade:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  442ae1:	51                   	push   ecx
  442ae2:	8b 55 ec             	mov    edx,DWORD PTR [ebp-0x14]
  442ae5:	52                   	push   edx
  442ae6:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442aec:	81 c1 d0 1f 00 00    	add    ecx,0x1fd0
  442af2:	e8 d9 2c fd ff       	call   0x4157d0
  442af7:	8b c8                	mov    ecx,eax
  442af9:	e8 30 ce fd ff       	call   0x41f92e
  442afe:	eb bb                	jmp    0x442abb
  442b00:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442b06:	81 c1 a0 1e 00 00    	add    ecx,0x1ea0
  442b0c:	e8 ef 38 00 00       	call   0x446400
  442b11:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  442b14:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  442b17:	50                   	push   eax
  442b18:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442b1e:	81 c1 a0 1e 00 00    	add    ecx,0x1ea0
  442b24:	e8 f7 38 00 00       	call   0x446420
  442b29:	25 ff 00 00 00       	and    eax,0xff
  442b2e:	85 c0                	test   eax,eax
  442b30:	75 4f                	jne    0x442b81
  442b32:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  442b35:	8b 51 04             	mov    edx,DWORD PTR [ecx+0x4]
  442b38:	89 95 6c ff ff ff    	mov    DWORD PTR [ebp-0x94],edx
  442b3e:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  442b41:	50                   	push   eax
  442b42:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  442b45:	51                   	push   ecx
  442b46:	8b 55 f0             	mov    edx,DWORD PTR [ebp-0x10]
  442b49:	8b 0a                	mov    ecx,DWORD PTR [edx]
  442b4b:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  442b4e:	8b 10                	mov    edx,DWORD PTR [eax]
  442b50:	8b 02                	mov    eax,DWORD PTR [edx]
  442b52:	ff 50 14             	call   DWORD PTR [eax+0x14]
  442b55:	25 ff 00 00 00       	and    eax,0xff
  442b5a:	85 c0                	test   eax,eax
  442b5c:	75 18                	jne    0x442b76
  442b5e:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  442b61:	8b 11                	mov    edx,DWORD PTR [ecx]
  442b63:	52                   	push   edx
  442b64:	b9 78 33 4e 00       	mov    ecx,0x4e3378
  442b69:	e8 42 65 fc ff       	call   0x4090b0
  442b6e:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  442b71:	e8 da 2d 00 00       	call   0x445950
  442b76:	8b 85 6c ff ff ff    	mov    eax,DWORD PTR [ebp-0x94]
  442b7c:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  442b7f:	eb 93                	jmp    0x442b14
  442b81:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  442b88:	eb 09                	jmp    0x442b93
  442b8a:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  442b8d:	83 c1 01             	add    ecx,0x1
  442b90:	89 4d ec             	mov    DWORD PTR [ebp-0x14],ecx
  442b93:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442b99:	81 c1 b8 1e 00 00    	add    ecx,0x1eb8
  442b9f:	e8 1c 3a 00 00       	call   0x4465c0
  442ba4:	39 45 ec             	cmp    DWORD PTR [ebp-0x14],eax
  442ba7:	7d 73                	jge    0x442c1c
  442ba9:	8b 55 ec             	mov    edx,DWORD PTR [ebp-0x14]
  442bac:	52                   	push   edx
  442bad:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442bb3:	81 c1 b8 1e 00 00    	add    ecx,0x1eb8
  442bb9:	e8 e2 39 00 00       	call   0x4465a0
  442bbe:	8b 00                	mov    eax,DWORD PTR [eax]
  442bc0:	89 85 68 ff ff ff    	mov    DWORD PTR [ebp-0x98],eax
  442bc6:	8b 4d 0c             	mov    ecx,DWORD PTR [ebp+0xc]
  442bc9:	51                   	push   ecx
  442bca:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  442bcd:	52                   	push   edx
  442bce:	8b 85 68 ff ff ff    	mov    eax,DWORD PTR [ebp-0x98]
  442bd4:	8b 10                	mov    edx,DWORD PTR [eax]
  442bd6:	8b 8d 68 ff ff ff    	mov    ecx,DWORD PTR [ebp-0x98]
  442bdc:	ff 52 14             	call   DWORD PTR [edx+0x14]
  442bdf:	25 ff 00 00 00       	and    eax,0xff
  442be4:	85 c0                	test   eax,eax
  442be6:	75 2f                	jne    0x442c17
  442be8:	8b 85 68 ff ff ff    	mov    eax,DWORD PTR [ebp-0x98]
  442bee:	50                   	push   eax
  442bef:	b9 78 33 4e 00       	mov    ecx,0x4e3378
  442bf4:	e8 b7 64 fc ff       	call   0x4090b0
  442bf9:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  442bfc:	51                   	push   ecx
  442bfd:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442c03:	81 c1 b8 1e 00 00    	add    ecx,0x1eb8
  442c09:	e8 c2 39 00 00       	call   0x4465d0
  442c0e:	8b 55 ec             	mov    edx,DWORD PTR [ebp-0x14]
  442c11:	83 ea 01             	sub    edx,0x1
  442c14:	89 55 ec             	mov    DWORD PTR [ebp-0x14],edx
  442c17:	e9 6e ff ff ff       	jmp    0x442b8a
  442c1c:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442c22:	81 c1 c8 1e 00 00    	add    ecx,0x1ec8
  442c28:	e8 c3 64 fc ff       	call   0x4090f0
  442c2d:	89 45 e8             	mov    DWORD PTR [ebp-0x18],eax
  442c30:	8b 45 e8             	mov    eax,DWORD PTR [ebp-0x18]
  442c33:	50                   	push   eax
  442c34:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442c3a:	81 c1 c8 1e 00 00    	add    ecx,0x1ec8
  442c40:	e8 cb 64 fc ff       	call   0x409110
  442c45:	25 ff 00 00 00       	and    eax,0xff
  442c4a:	85 c0                	test   eax,eax
  442c4c:	75 73                	jne    0x442cc1
  442c4e:	8b 4d e8             	mov    ecx,DWORD PTR [ebp-0x18]
  442c51:	8b 51 04             	mov    edx,DWORD PTR [ecx+0x4]
  442c54:	89 95 64 ff ff ff    	mov    DWORD PTR [ebp-0x9c],edx
  442c5a:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  442c5d:	50                   	push   eax
  442c5e:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  442c61:	51                   	push   ecx
  442c62:	8b 55 e8             	mov    edx,DWORD PTR [ebp-0x18]
  442c65:	8b 0a                	mov    ecx,DWORD PTR [edx]
  442c67:	8b 45 e8             	mov    eax,DWORD PTR [ebp-0x18]
  442c6a:	8b 10                	mov    edx,DWORD PTR [eax]
  442c6c:	8b 02                	mov    eax,DWORD PTR [edx]
  442c6e:	ff 50 14             	call   DWORD PTR [eax+0x14]
  442c71:	25 ff 00 00 00       	and    eax,0xff
  442c76:	85 c0                	test   eax,eax
  442c78:	75 39                	jne    0x442cb3
  442c7a:	8b 4d e8             	mov    ecx,DWORD PTR [ebp-0x18]
  442c7d:	8b 11                	mov    edx,DWORD PTR [ecx]
  442c7f:	52                   	push   edx
  442c80:	b9 78 33 4e 00       	mov    ecx,0x4e3378
  442c85:	e8 26 64 fc ff       	call   0x4090b0
  442c8a:	a1 88 bf 4c 00       	mov    eax,ds:0x4cbf88
  442c8f:	50                   	push   eax
  442c90:	8b 4d e8             	mov    ecx,DWORD PTR [ebp-0x18]
  442c93:	8b 09                	mov    ecx,DWORD PTR [ecx]
  442c95:	e8 66 3d fc ff       	call   0x406a00
  442c9a:	0f bf d0             	movsx  edx,ax
  442c9d:	52                   	push   edx
  442c9e:	68 50 00 4c 00       	push   0x4c0050
  442ca3:	e8 b8 cb fc ff       	call   0x40f860
  442ca8:	83 c4 0c             	add    esp,0xc
  442cab:	8b 4d e8             	mov    ecx,DWORD PTR [ebp-0x18]
  442cae:	e8 0d 63 fc ff       	call   0x408fc0
  442cb3:	8b 85 64 ff ff ff    	mov    eax,DWORD PTR [ebp-0x9c]
  442cb9:	89 45 e8             	mov    DWORD PTR [ebp-0x18],eax
  442cbc:	e9 6f ff ff ff       	jmp    0x442c30
  442cc1:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  442cc8:	eb 09                	jmp    0x442cd3
  442cca:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  442ccd:	83 c1 01             	add    ecx,0x1
  442cd0:	89 4d ec             	mov    DWORD PTR [ebp-0x14],ecx
  442cd3:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442cd9:	81 c1 f0 1e 00 00    	add    ecx,0x1ef0
  442cdf:	e8 5c 3e 00 00       	call   0x446b40
  442ce4:	39 45 ec             	cmp    DWORD PTR [ebp-0x14],eax
  442ce7:	7d 5d                	jge    0x442d46
  442ce9:	8b 55 ec             	mov    edx,DWORD PTR [ebp-0x14]
  442cec:	52                   	push   edx
  442ced:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442cf3:	81 c1 f0 1e 00 00    	add    ecx,0x1ef0
  442cf9:	e8 22 3e 00 00       	call   0x446b20
  442cfe:	8b 08                	mov    ecx,DWORD PTR [eax]
  442d00:	e8 9b 9e fd ff       	call   0x41cba0
  442d05:	25 ff 00 00 00       	and    eax,0xff
  442d0a:	85 c0                	test   eax,eax
  442d0c:	74 36                	je     0x442d44
  442d0e:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  442d11:	50                   	push   eax
  442d12:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  442d15:	51                   	push   ecx
  442d16:	8b 55 ec             	mov    edx,DWORD PTR [ebp-0x14]
  442d19:	52                   	push   edx
  442d1a:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442d20:	81 c1 f0 1e 00 00    	add    ecx,0x1ef0
  442d26:	e8 f5 3d 00 00       	call   0x446b20
  442d2b:	8b 00                	mov    eax,DWORD PTR [eax]
  442d2d:	89 85 2c ff ff ff    	mov    DWORD PTR [ebp-0xd4],eax
  442d33:	8b 8d 2c ff ff ff    	mov    ecx,DWORD PTR [ebp-0xd4]
  442d39:	8b 11                	mov    edx,DWORD PTR [ecx]
  442d3b:	8b 8d 2c ff ff ff    	mov    ecx,DWORD PTR [ebp-0xd4]
  442d41:	ff 52 14             	call   DWORD PTR [edx+0x14]
  442d44:	eb 84                	jmp    0x442cca
  442d46:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  442d4d:	eb 09                	jmp    0x442d58
  442d4f:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  442d52:	83 c0 01             	add    eax,0x1
  442d55:	89 45 ec             	mov    DWORD PTR [ebp-0x14],eax
  442d58:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442d5e:	81 c1 00 1f 00 00    	add    ecx,0x1f00
  442d64:	e8 f7 3e 00 00       	call   0x446c60
  442d69:	39 45 ec             	cmp    DWORD PTR [ebp-0x14],eax
  442d6c:	0f 8d b6 00 00 00    	jge    0x442e28
  442d72:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  442d75:	51                   	push   ecx
  442d76:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442d7c:	81 c1 00 1f 00 00    	add    ecx,0x1f00
  442d82:	e8 b9 3e 00 00       	call   0x446c40
  442d87:	8b 08                	mov    ecx,DWORD PTR [eax]
  442d89:	e8 12 9e fd ff       	call   0x41cba0
  442d8e:	25 ff 00 00 00       	and    eax,0xff
  442d93:	85 c0                	test   eax,eax
  442d95:	0f 84 88 00 00 00    	je     0x442e23
  442d9b:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  442d9e:	52                   	push   edx
  442d9f:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  442da2:	50                   	push   eax
  442da3:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  442da6:	51                   	push   ecx
  442da7:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442dad:	81 c1 00 1f 00 00    	add    ecx,0x1f00
  442db3:	e8 88 3e 00 00       	call   0x446c40
  442db8:	8b 10                	mov    edx,DWORD PTR [eax]
  442dba:	89 95 28 ff ff ff    	mov    DWORD PTR [ebp-0xd8],edx
  442dc0:	8b 85 28 ff ff ff    	mov    eax,DWORD PTR [ebp-0xd8]
  442dc6:	8b 10                	mov    edx,DWORD PTR [eax]
  442dc8:	8b 8d 28 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xd8]
  442dce:	ff 52 14             	call   DWORD PTR [edx+0x14]
  442dd1:	25 ff 00 00 00       	and    eax,0xff
  442dd6:	85 c0                	test   eax,eax
  442dd8:	75 49                	jne    0x442e23
  442dda:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  442ddd:	50                   	push   eax
  442dde:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442de4:	81 c1 00 1f 00 00    	add    ecx,0x1f00
  442dea:	e8 51 3e 00 00       	call   0x446c40
  442def:	8b 08                	mov    ecx,DWORD PTR [eax]
  442df1:	51                   	push   ecx
  442df2:	b9 78 33 4e 00       	mov    ecx,0x4e3378
  442df7:	e8 b4 62 fc ff       	call   0x4090b0
  442dfc:	8b 55 ec             	mov    edx,DWORD PTR [ebp-0x14]
  442dff:	52                   	push   edx
  442e00:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442e06:	81 c1 00 1f 00 00    	add    ecx,0x1f00
  442e0c:	e8 2f 3e 00 00       	call   0x446c40
  442e11:	50                   	push   eax
  442e12:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442e18:	81 c1 00 1f 00 00    	add    ecx,0x1f00
  442e1e:	e8 4d 3e 00 00       	call   0x446c70
  442e23:	e9 27 ff ff ff       	jmp    0x442d4f
  442e28:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  442e2f:	eb 09                	jmp    0x442e3a
  442e31:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  442e34:	83 c0 01             	add    eax,0x1
  442e37:	89 45 ec             	mov    DWORD PTR [ebp-0x14],eax
  442e3a:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442e40:	81 c1 50 1f 00 00    	add    ecx,0x1f50
  442e46:	e8 15 9d fd ff       	call   0x41cb60
  442e4b:	39 45 ec             	cmp    DWORD PTR [ebp-0x14],eax
  442e4e:	7d 5d                	jge    0x442ead
  442e50:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  442e53:	51                   	push   ecx
  442e54:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442e5a:	81 c1 50 1f 00 00    	add    ecx,0x1f50
  442e60:	e8 db 9c fd ff       	call   0x41cb40
  442e65:	8b 08                	mov    ecx,DWORD PTR [eax]
  442e67:	e8 34 9d fd ff       	call   0x41cba0
  442e6c:	25 ff 00 00 00       	and    eax,0xff
  442e71:	85 c0                	test   eax,eax
  442e73:	74 36                	je     0x442eab
  442e75:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  442e78:	52                   	push   edx
  442e79:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  442e7c:	50                   	push   eax
  442e7d:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  442e80:	51                   	push   ecx
  442e81:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442e87:	81 c1 50 1f 00 00    	add    ecx,0x1f50
  442e8d:	e8 ae 9c fd ff       	call   0x41cb40
  442e92:	8b 10                	mov    edx,DWORD PTR [eax]
  442e94:	89 95 24 ff ff ff    	mov    DWORD PTR [ebp-0xdc],edx
  442e9a:	8b 85 24 ff ff ff    	mov    eax,DWORD PTR [ebp-0xdc]
  442ea0:	8b 10                	mov    edx,DWORD PTR [eax]
  442ea2:	8b 8d 24 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xdc]
  442ea8:	ff 52 14             	call   DWORD PTR [edx+0x14]
  442eab:	eb 84                	jmp    0x442e31
  442ead:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  442eb4:	eb 09                	jmp    0x442ebf
  442eb6:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  442eb9:	83 c0 01             	add    eax,0x1
  442ebc:	89 45 ec             	mov    DWORD PTR [ebp-0x14],eax
  442ebf:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442ec5:	81 c1 60 1f 00 00    	add    ecx,0x1f60
  442ecb:	e8 10 03 fd ff       	call   0x4131e0
  442ed0:	39 45 ec             	cmp    DWORD PTR [ebp-0x14],eax
  442ed3:	7d 5d                	jge    0x442f32
  442ed5:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  442ed8:	51                   	push   ecx
  442ed9:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442edf:	81 c1 60 1f 00 00    	add    ecx,0x1f60
  442ee5:	e8 d6 02 fd ff       	call   0x4131c0
  442eea:	8b 08                	mov    ecx,DWORD PTR [eax]
  442eec:	e8 af 9c fd ff       	call   0x41cba0
  442ef1:	25 ff 00 00 00       	and    eax,0xff
  442ef6:	85 c0                	test   eax,eax
  442ef8:	74 36                	je     0x442f30
  442efa:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  442efd:	52                   	push   edx
  442efe:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  442f01:	50                   	push   eax
  442f02:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  442f05:	51                   	push   ecx
  442f06:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442f0c:	81 c1 60 1f 00 00    	add    ecx,0x1f60
  442f12:	e8 a9 02 fd ff       	call   0x4131c0
  442f17:	8b 10                	mov    edx,DWORD PTR [eax]
  442f19:	89 95 20 ff ff ff    	mov    DWORD PTR [ebp-0xe0],edx
  442f1f:	8b 85 20 ff ff ff    	mov    eax,DWORD PTR [ebp-0xe0]
  442f25:	8b 10                	mov    edx,DWORD PTR [eax]
  442f27:	8b 8d 20 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xe0]
  442f2d:	ff 52 14             	call   DWORD PTR [edx+0x14]
  442f30:	eb 84                	jmp    0x442eb6
  442f32:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  442f39:	eb 09                	jmp    0x442f44
  442f3b:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  442f3e:	83 c0 01             	add    eax,0x1
  442f41:	89 45 ec             	mov    DWORD PTR [ebp-0x14],eax
  442f44:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442f4a:	81 c1 70 1f 00 00    	add    ecx,0x1f70
  442f50:	e8 3b 9c fd ff       	call   0x41cb90
  442f55:	39 45 ec             	cmp    DWORD PTR [ebp-0x14],eax
  442f58:	7d 5d                	jge    0x442fb7
  442f5a:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  442f5d:	51                   	push   ecx
  442f5e:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442f64:	81 c1 70 1f 00 00    	add    ecx,0x1f70
  442f6a:	e8 01 9c fd ff       	call   0x41cb70
  442f6f:	8b 08                	mov    ecx,DWORD PTR [eax]
  442f71:	e8 2a 9c fd ff       	call   0x41cba0
  442f76:	25 ff 00 00 00       	and    eax,0xff
  442f7b:	85 c0                	test   eax,eax
  442f7d:	74 36                	je     0x442fb5
  442f7f:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  442f82:	52                   	push   edx
  442f83:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  442f86:	50                   	push   eax
  442f87:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  442f8a:	51                   	push   ecx
  442f8b:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442f91:	81 c1 70 1f 00 00    	add    ecx,0x1f70
  442f97:	e8 d4 9b fd ff       	call   0x41cb70
  442f9c:	8b 10                	mov    edx,DWORD PTR [eax]
  442f9e:	89 95 1c ff ff ff    	mov    DWORD PTR [ebp-0xe4],edx
  442fa4:	8b 85 1c ff ff ff    	mov    eax,DWORD PTR [ebp-0xe4]
  442faa:	8b 10                	mov    edx,DWORD PTR [eax]
  442fac:	8b 8d 1c ff ff ff    	mov    ecx,DWORD PTR [ebp-0xe4]
  442fb2:	ff 52 14             	call   DWORD PTR [edx+0x14]
  442fb5:	eb 84                	jmp    0x442f3b
  442fb7:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  442fbe:	eb 09                	jmp    0x442fc9
  442fc0:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  442fc3:	83 c0 01             	add    eax,0x1
  442fc6:	89 45 ec             	mov    DWORD PTR [ebp-0x14],eax
  442fc9:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442fcf:	81 c1 90 1f 00 00    	add    ecx,0x1f90
  442fd5:	e8 66 46 00 00       	call   0x447640
  442fda:	39 45 ec             	cmp    DWORD PTR [ebp-0x14],eax
  442fdd:	7d 38                	jge    0x443017
  442fdf:	8b 4d 0c             	mov    ecx,DWORD PTR [ebp+0xc]
  442fe2:	51                   	push   ecx
  442fe3:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  442fe6:	52                   	push   edx
  442fe7:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  442fea:	50                   	push   eax
  442feb:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  442ff1:	81 c1 90 1f 00 00    	add    ecx,0x1f90
  442ff7:	e8 24 46 00 00       	call   0x447620
  442ffc:	8b 08                	mov    ecx,DWORD PTR [eax]
  442ffe:	89 8d 18 ff ff ff    	mov    DWORD PTR [ebp-0xe8],ecx
  443004:	8b 95 18 ff ff ff    	mov    edx,DWORD PTR [ebp-0xe8]
  44300a:	8b 02                	mov    eax,DWORD PTR [edx]
  44300c:	8b 8d 18 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xe8]
  443012:	ff 50 14             	call   DWORD PTR [eax+0x14]
  443015:	eb a9                	jmp    0x442fc0
  443017:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  44301e:	eb 09                	jmp    0x443029
  443020:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  443023:	83 c1 01             	add    ecx,0x1
  443026:	89 4d ec             	mov    DWORD PTR [ebp-0x14],ecx
  443029:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  44302f:	81 c1 20 1f 00 00    	add    ecx,0x1f20
  443035:	e8 46 3f 00 00       	call   0x446f80
  44303a:	39 45 ec             	cmp    DWORD PTR [ebp-0x14],eax
  44303d:	7d 38                	jge    0x443077
  44303f:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  443042:	52                   	push   edx
  443043:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  443046:	50                   	push   eax
  443047:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  44304a:	51                   	push   ecx
  44304b:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  443051:	81 c1 20 1f 00 00    	add    ecx,0x1f20
  443057:	e8 04 3f 00 00       	call   0x446f60
  44305c:	8b 10                	mov    edx,DWORD PTR [eax]
  44305e:	89 95 14 ff ff ff    	mov    DWORD PTR [ebp-0xec],edx
  443064:	8b 85 14 ff ff ff    	mov    eax,DWORD PTR [ebp-0xec]
  44306a:	8b 10                	mov    edx,DWORD PTR [eax]
  44306c:	8b 8d 14 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xec]
  443072:	ff 52 14             	call   DWORD PTR [edx+0x14]
  443075:	eb a9                	jmp    0x443020
  443077:	c7 45 ec 00 00 00 00 	mov    DWORD PTR [ebp-0x14],0x0
  44307e:	eb 09                	jmp    0x443089
  443080:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  443083:	83 c0 01             	add    eax,0x1
  443086:	89 45 ec             	mov    DWORD PTR [ebp-0x14],eax
  443089:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  44308f:	81 c1 a0 1f 00 00    	add    ecx,0x1fa0
  443095:	e8 36 b0 fd ff       	call   0x41e0d0
  44309a:	39 45 ec             	cmp    DWORD PTR [ebp-0x14],eax
  44309d:	0f 8d bd 00 00 00    	jge    0x443160
  4430a3:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  4430a6:	51                   	push   ecx
  4430a7:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4430ad:	81 c1 a0 1f 00 00    	add    ecx,0x1fa0
  4430b3:	e8 f8 af fd ff       	call   0x41e0b0
  4430b8:	8b 08                	mov    ecx,DWORD PTR [eax]
  4430ba:	e8 e1 9a fd ff       	call   0x41cba0
  4430bf:	25 ff 00 00 00       	and    eax,0xff
  4430c4:	85 c0                	test   eax,eax
  4430c6:	0f 84 8f 00 00 00    	je     0x44315b
  4430cc:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  4430cf:	52                   	push   edx
  4430d0:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  4430d3:	50                   	push   eax
  4430d4:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  4430d7:	51                   	push   ecx
  4430d8:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4430de:	81 c1 a0 1f 00 00    	add    ecx,0x1fa0
  4430e4:	e8 c7 af fd ff       	call   0x41e0b0
  4430e9:	8b 10                	mov    edx,DWORD PTR [eax]
  4430eb:	89 95 10 ff ff ff    	mov    DWORD PTR [ebp-0xf0],edx
  4430f1:	8b 85 10 ff ff ff    	mov    eax,DWORD PTR [ebp-0xf0]
  4430f7:	8b 10                	mov    edx,DWORD PTR [eax]
  4430f9:	8b 8d 10 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xf0]
  4430ff:	ff 52 14             	call   DWORD PTR [edx+0x14]
  443102:	25 ff 00 00 00       	and    eax,0xff
  443107:	85 c0                	test   eax,eax
  443109:	75 50                	jne    0x44315b
  44310b:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  44310e:	50                   	push   eax
  44310f:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  443115:	81 c1 a0 1f 00 00    	add    ecx,0x1fa0
  44311b:	e8 90 af fd ff       	call   0x41e0b0
  443120:	8b 08                	mov    ecx,DWORD PTR [eax]
  443122:	89 8d 0c ff ff ff    	mov    DWORD PTR [ebp-0xf4],ecx
  443128:	8b 95 0c ff ff ff    	mov    edx,DWORD PTR [ebp-0xf4]
  44312e:	8b 02                	mov    eax,DWORD PTR [edx]
  443130:	8b 8d 0c ff ff ff    	mov    ecx,DWORD PTR [ebp-0xf4]
  443136:	ff 50 08             	call   DWORD PTR [eax+0x8]
  443139:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  44313c:	51                   	push   ecx
  44313d:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  443143:	81 c1 a0 1f 00 00    	add    ecx,0x1fa0
  443149:	e8 62 af fd ff       	call   0x41e0b0
  44314e:	8b 10                	mov    edx,DWORD PTR [eax]
  443150:	52                   	push   edx
  443151:	b9 10 6a 4c 00       	mov    ecx,0x4c6a10
  443156:	e8 db 83 ff ff       	call   0x43b536
  44315b:	e9 20 ff ff ff       	jmp    0x443080
  443160:	8b 45 0c             	mov    eax,DWORD PTR [ebp+0xc]
  443163:	50                   	push   eax
  443164:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  443167:	51                   	push   ecx
  443168:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  44316e:	81 c1 d0 27 00 00    	add    ecx,0x27d0
  443174:	e8 95 7d 01 00       	call   0x45af0e
  443179:	8b 55 0c             	mov    edx,DWORD PTR [ebp+0xc]
  44317c:	52                   	push   edx
  44317d:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  443180:	50                   	push   eax
  443181:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  443187:	81 c1 18 20 00 00    	add    ecx,0x2018
  44318d:	e8 9a a0 00 00       	call   0x44d22c
  443192:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  443198:	83 c1 08             	add    ecx,0x8
  44319b:	e8 b0 7f ff ff       	call   0x43b150
  4431a0:	8b 4d 0c             	mov    ecx,DWORD PTR [ebp+0xc]
  4431a3:	51                   	push   ecx
  4431a4:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  4431a7:	52                   	push   edx
  4431a8:	8b 8d 40 ff ff ff    	mov    ecx,DWORD PTR [ebp-0xc0]
  4431ae:	e8 e5 22 00 00       	call   0x445498
  4431b3:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  4431b6:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  4431bd:	8b e5                	mov    esp,ebp
  4431bf:	5d                   	pop    ebp
  4431c0:	c2 08 00             	ret    0x8

; VA 0x00446400, file offset 287744, length 17, SHA256 31f46434012fd7ba8bbf8b14308e9a805714de8400146ddb80ab47e40c27d964

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00446400 <.text+0x45400>:
  446400:	55                   	push   ebp
  446401:	8b ec                	mov    ebp,esp
  446403:	51                   	push   ecx
  446404:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  446407:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  44640a:	8b 40 04             	mov    eax,DWORD PTR [eax+0x4]
  44640d:	8b e5                	mov    esp,ebp
  44640f:	5d                   	pop    ebp
  446410:	c3                   	ret

; VA 0x00446420, file offset 287776, length 29, SHA256 8e254846ffcac263c367901dff6450f94b61c566b4e29c0a90f464ecdeea2ffc

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00446420 <.text+0x45420>:
  446420:	55                   	push   ebp
  446421:	8b ec                	mov    ebp,esp
  446423:	51                   	push   ecx
  446424:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  446427:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  44642a:	83 c0 0c             	add    eax,0xc
  44642d:	33 c9                	xor    ecx,ecx
  44642f:	39 45 08             	cmp    DWORD PTR [ebp+0x8],eax
  446432:	0f 94 c1             	sete   cl
  446435:	8a c1                	mov    al,cl
  446437:	8b e5                	mov    esp,ebp
  446439:	5d                   	pop    ebp
  44643a:	c2 04 00             	ret    0x4

; VA 0x004465A0, file offset 288160, length 25, SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004465a0 <.text+0x455a0>:
  4465a0:	55                   	push   ebp
  4465a1:	8b ec                	mov    ebp,esp
  4465a3:	51                   	push   ecx
  4465a4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4465a7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4465aa:	8b 48 0c             	mov    ecx,DWORD PTR [eax+0xc]
  4465ad:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  4465b0:	8d 04 91             	lea    eax,[ecx+edx*4]
  4465b3:	8b e5                	mov    esp,ebp
  4465b5:	5d                   	pop    ebp
  4465b6:	c2 04 00             	ret    0x4

; VA 0x004465C0, file offset 288192, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004465c0 <.text+0x455c0>:
  4465c0:	55                   	push   ebp
  4465c1:	8b ec                	mov    ebp,esp
  4465c3:	51                   	push   ecx
  4465c4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4465c7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4465ca:	8b 00                	mov    eax,DWORD PTR [eax]
  4465cc:	8b e5                	mov    esp,ebp
  4465ce:	5d                   	pop    ebp
  4465cf:	c3                   	ret

; VA 0x00446B20, file offset 289568, length 25, SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00446b20 <.text+0x45b20>:
  446b20:	55                   	push   ebp
  446b21:	8b ec                	mov    ebp,esp
  446b23:	51                   	push   ecx
  446b24:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  446b27:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  446b2a:	8b 48 0c             	mov    ecx,DWORD PTR [eax+0xc]
  446b2d:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  446b30:	8d 04 91             	lea    eax,[ecx+edx*4]
  446b33:	8b e5                	mov    esp,ebp
  446b35:	5d                   	pop    ebp
  446b36:	c2 04 00             	ret    0x4

; VA 0x00446B40, file offset 289600, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00446b40 <.text+0x45b40>:
  446b40:	55                   	push   ebp
  446b41:	8b ec                	mov    ebp,esp
  446b43:	51                   	push   ecx
  446b44:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  446b47:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  446b4a:	8b 00                	mov    eax,DWORD PTR [eax]
  446b4c:	8b e5                	mov    esp,ebp
  446b4e:	5d                   	pop    ebp
  446b4f:	c3                   	ret

; VA 0x00446C40, file offset 289856, length 25, SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00446c40 <.text+0x45c40>:
  446c40:	55                   	push   ebp
  446c41:	8b ec                	mov    ebp,esp
  446c43:	51                   	push   ecx
  446c44:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  446c47:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  446c4a:	8b 48 0c             	mov    ecx,DWORD PTR [eax+0xc]
  446c4d:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  446c50:	8d 04 91             	lea    eax,[ecx+edx*4]
  446c53:	8b e5                	mov    esp,ebp
  446c55:	5d                   	pop    ebp
  446c56:	c2 04 00             	ret    0x4

; VA 0x00446C60, file offset 289888, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00446c60 <.text+0x45c60>:
  446c60:	55                   	push   ebp
  446c61:	8b ec                	mov    ebp,esp
  446c63:	51                   	push   ecx
  446c64:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  446c67:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  446c6a:	8b 00                	mov    eax,DWORD PTR [eax]
  446c6c:	8b e5                	mov    esp,ebp
  446c6e:	5d                   	pop    ebp
  446c6f:	c3                   	ret

; VA 0x0041CB40, file offset 117568, length 25, SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0041cb40 <.text+0x1bb40>:
  41cb40:	55                   	push   ebp
  41cb41:	8b ec                	mov    ebp,esp
  41cb43:	51                   	push   ecx
  41cb44:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  41cb47:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  41cb4a:	8b 48 0c             	mov    ecx,DWORD PTR [eax+0xc]
  41cb4d:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  41cb50:	8d 04 91             	lea    eax,[ecx+edx*4]
  41cb53:	8b e5                	mov    esp,ebp
  41cb55:	5d                   	pop    ebp
  41cb56:	c2 04 00             	ret    0x4

; VA 0x0041CB60, file offset 117600, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0041cb60 <.text+0x1bb60>:
  41cb60:	55                   	push   ebp
  41cb61:	8b ec                	mov    ebp,esp
  41cb63:	51                   	push   ecx
  41cb64:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  41cb67:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  41cb6a:	8b 00                	mov    eax,DWORD PTR [eax]
  41cb6c:	8b e5                	mov    esp,ebp
  41cb6e:	5d                   	pop    ebp
  41cb6f:	c3                   	ret

; VA 0x004131C0, file offset 78272, length 25, SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004131c0 <.text+0x121c0>:
  4131c0:	55                   	push   ebp
  4131c1:	8b ec                	mov    ebp,esp
  4131c3:	51                   	push   ecx
  4131c4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4131c7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4131ca:	8b 48 0c             	mov    ecx,DWORD PTR [eax+0xc]
  4131cd:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  4131d0:	8d 04 91             	lea    eax,[ecx+edx*4]
  4131d3:	8b e5                	mov    esp,ebp
  4131d5:	5d                   	pop    ebp
  4131d6:	c2 04 00             	ret    0x4

; VA 0x004131E0, file offset 78304, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004131e0 <.text+0x121e0>:
  4131e0:	55                   	push   ebp
  4131e1:	8b ec                	mov    ebp,esp
  4131e3:	51                   	push   ecx
  4131e4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4131e7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4131ea:	8b 00                	mov    eax,DWORD PTR [eax]
  4131ec:	8b e5                	mov    esp,ebp
  4131ee:	5d                   	pop    ebp
  4131ef:	c3                   	ret

; VA 0x0041CB70, file offset 117616, length 25, SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0041cb70 <.text+0x1bb70>:
  41cb70:	55                   	push   ebp
  41cb71:	8b ec                	mov    ebp,esp
  41cb73:	51                   	push   ecx
  41cb74:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  41cb77:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  41cb7a:	8b 48 0c             	mov    ecx,DWORD PTR [eax+0xc]
  41cb7d:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  41cb80:	8d 04 91             	lea    eax,[ecx+edx*4]
  41cb83:	8b e5                	mov    esp,ebp
  41cb85:	5d                   	pop    ebp
  41cb86:	c2 04 00             	ret    0x4

; VA 0x0041CB90, file offset 117648, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0041cb90 <.text+0x1bb90>:
  41cb90:	55                   	push   ebp
  41cb91:	8b ec                	mov    ebp,esp
  41cb93:	51                   	push   ecx
  41cb94:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  41cb97:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  41cb9a:	8b 00                	mov    eax,DWORD PTR [eax]
  41cb9c:	8b e5                	mov    esp,ebp
  41cb9e:	5d                   	pop    ebp
  41cb9f:	c3                   	ret

; VA 0x00447620, file offset 292384, length 25, SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00447620 <.text+0x46620>:
  447620:	55                   	push   ebp
  447621:	8b ec                	mov    ebp,esp
  447623:	51                   	push   ecx
  447624:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  447627:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  44762a:	8b 48 0c             	mov    ecx,DWORD PTR [eax+0xc]
  44762d:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  447630:	8d 04 91             	lea    eax,[ecx+edx*4]
  447633:	8b e5                	mov    esp,ebp
  447635:	5d                   	pop    ebp
  447636:	c2 04 00             	ret    0x4

; VA 0x00447640, file offset 292416, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00447640 <.text+0x46640>:
  447640:	55                   	push   ebp
  447641:	8b ec                	mov    ebp,esp
  447643:	51                   	push   ecx
  447644:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  447647:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  44764a:	8b 00                	mov    eax,DWORD PTR [eax]
  44764c:	8b e5                	mov    esp,ebp
  44764e:	5d                   	pop    ebp
  44764f:	c3                   	ret

; VA 0x00446F60, file offset 290656, length 25, SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00446f60 <.text+0x45f60>:
  446f60:	55                   	push   ebp
  446f61:	8b ec                	mov    ebp,esp
  446f63:	51                   	push   ecx
  446f64:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  446f67:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  446f6a:	8b 48 0c             	mov    ecx,DWORD PTR [eax+0xc]
  446f6d:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  446f70:	8d 04 91             	lea    eax,[ecx+edx*4]
  446f73:	8b e5                	mov    esp,ebp
  446f75:	5d                   	pop    ebp
  446f76:	c2 04 00             	ret    0x4

; VA 0x00446F80, file offset 290688, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00446f80 <.text+0x45f80>:
  446f80:	55                   	push   ebp
  446f81:	8b ec                	mov    ebp,esp
  446f83:	51                   	push   ecx
  446f84:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  446f87:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  446f8a:	8b 00                	mov    eax,DWORD PTR [eax]
  446f8c:	8b e5                	mov    esp,ebp
  446f8e:	5d                   	pop    ebp
  446f8f:	c3                   	ret

; VA 0x0041E0B0, file offset 123056, length 25, SHA256 66f2dfff238aafc078498d1b1698406ac5cff32fe751255495e8670ec90db1be

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0041e0b0 <.text+0x1d0b0>:
  41e0b0:	55                   	push   ebp
  41e0b1:	8b ec                	mov    ebp,esp
  41e0b3:	51                   	push   ecx
  41e0b4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  41e0b7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  41e0ba:	8b 48 0c             	mov    ecx,DWORD PTR [eax+0xc]
  41e0bd:	8b 55 08             	mov    edx,DWORD PTR [ebp+0x8]
  41e0c0:	8d 04 91             	lea    eax,[ecx+edx*4]
  41e0c3:	8b e5                	mov    esp,ebp
  41e0c5:	5d                   	pop    ebp
  41e0c6:	c2 04 00             	ret    0x4

; VA 0x0041E0D0, file offset 123088, length 16, SHA256 8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0041e0d0 <.text+0x1d0d0>:
  41e0d0:	55                   	push   ebp
  41e0d1:	8b ec                	mov    ebp,esp
  41e0d3:	51                   	push   ecx
  41e0d4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  41e0d7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  41e0da:	8b 00                	mov    eax,DWORD PTR [eax]
  41e0dc:	8b e5                	mov    esp,ebp
  41e0de:	5d                   	pop    ebp
  41e0df:	c3                   	ret

; VA 0x004090F0, file offset 37104, length 17, SHA256 31f46434012fd7ba8bbf8b14308e9a805714de8400146ddb80ab47e40c27d964

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004090f0 <.text+0x80f0>:
  4090f0:	55                   	push   ebp
  4090f1:	8b ec                	mov    ebp,esp
  4090f3:	51                   	push   ecx
  4090f4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4090f7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4090fa:	8b 40 04             	mov    eax,DWORD PTR [eax+0x4]
  4090fd:	8b e5                	mov    esp,ebp
  4090ff:	5d                   	pop    ebp
  409100:	c3                   	ret

; VA 0x00409110, file offset 37136, length 29, SHA256 8e254846ffcac263c367901dff6450f94b61c566b4e29c0a90f464ecdeea2ffc

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00409110 <.text+0x8110>:
  409110:	55                   	push   ebp
  409111:	8b ec                	mov    ebp,esp
  409113:	51                   	push   ecx
  409114:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  409117:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  40911a:	83 c0 0c             	add    eax,0xc
  40911d:	33 c9                	xor    ecx,ecx
  40911f:	39 45 08             	cmp    DWORD PTR [ebp+0x8],eax
  409122:	0f 94 c1             	sete   cl
  409125:	8a c1                	mov    al,cl
  409127:	8b e5                	mov    esp,ebp
  409129:	5d                   	pop    ebp
  40912a:	c2 04 00             	ret    0x4

; VA 0x0041CBA0, file offset 117664, length 17, SHA256 70169696f07dda40d76cec558fc5a4b7f4bcaa4410401b5889072b73e4930537

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0041cba0 <.text+0x1bba0>:
  41cba0:	55                   	push   ebp
  41cba1:	8b ec                	mov    ebp,esp
  41cba3:	51                   	push   ecx
  41cba4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  41cba7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  41cbaa:	8a 40 38             	mov    al,BYTE PTR [eax+0x38]
  41cbad:	8b e5                	mov    esp,ebp
  41cbaf:	5d                   	pop    ebp
  41cbb0:	c3                   	ret

; VA 0x00406A00, file offset 27136, length 18, SHA256 45713e9071f67175de33bff8071f8358847bcf139043b6cfbb69f2f7744aaa47

C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00406a00 <.text+0x5a00>:
  406a00:	55                   	push   ebp
  406a01:	8b ec                	mov    ebp,esp
  406a03:	51                   	push   ecx
  406a04:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  406a07:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  406a0a:	66 8b 40 3a          	mov    ax,WORD PTR [eax+0x3a]
  406a0e:	8b e5                	mov    esp,ebp
  406a10:	5d                   	pop    ebp
  406a11:	c3                   	ret
