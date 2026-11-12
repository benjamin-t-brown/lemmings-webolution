
C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484580 <.text+0x83580>:
  484580:	55                   	push   ebp
  484581:	8b ec                	mov    ebp,esp
  484583:	83 ec 10             	sub    esp,0x10
  484586:	89 4d f4             	mov    DWORD PTR [ebp-0xc],ecx
  484589:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
  48458c:	83 78 0c 00          	cmp    DWORD PTR [eax+0xc],0x0
  484590:	74 3f                	je     0x4845d1
  484592:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  484595:	c7 41 04 00 00 00 00 	mov    DWORD PTR [ecx+0x4],0x0
  48459c:	8b 55 f4             	mov    edx,DWORD PTR [ebp-0xc]
  48459f:	8b 42 0c             	mov    eax,DWORD PTR [edx+0xc]
  4845a2:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
  4845a5:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  4845a8:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4845ab:	83 7d fc 00          	cmp    DWORD PTR [ebp-0x4],0x0
  4845af:	74 0f                	je     0x4845c0
  4845b1:	6a 03                	push   0x3
  4845b3:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4845b6:	e8 d5 03 00 00       	call   0x484990
  4845bb:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  4845be:	eb 07                	jmp    0x4845c7
  4845c0:	c7 45 f0 00 00 00 00 	mov    DWORD PTR [ebp-0x10],0x0
  4845c7:	8b 55 f4             	mov    edx,DWORD PTR [ebp-0xc]
  4845ca:	c7 42 0c 00 00 00 00 	mov    DWORD PTR [edx+0xc],0x0
  4845d1:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  4845d4:	e8 b7 06 00 00       	call   0x484c90
  4845d9:	8b e5                	mov    esp,ebp
  4845db:	5d                   	pop    ebp
  4845dc:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004847a0 <.text+0x837a0>:
  4847a0:	55                   	push   ebp
  4847a1:	8b ec                	mov    ebp,esp
  4847a3:	83 ec 10             	sub    esp,0x10
  4847a6:	89 4d f4             	mov    DWORD PTR [ebp-0xc],ecx
  4847a9:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
  4847ac:	83 78 0c 00          	cmp    DWORD PTR [eax+0xc],0x0
  4847b0:	74 3f                	je     0x4847f1
  4847b2:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  4847b5:	c7 41 04 00 00 00 00 	mov    DWORD PTR [ecx+0x4],0x0
  4847bc:	8b 55 f4             	mov    edx,DWORD PTR [ebp-0xc]
  4847bf:	8b 42 0c             	mov    eax,DWORD PTR [edx+0xc]
  4847c2:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
  4847c5:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  4847c8:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4847cb:	83 7d fc 00          	cmp    DWORD PTR [ebp-0x4],0x0
  4847cf:	74 0f                	je     0x4847e0
  4847d1:	6a 03                	push   0x3
  4847d3:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4847d6:	e8 75 02 00 00       	call   0x484a50
  4847db:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  4847de:	eb 07                	jmp    0x4847e7
  4847e0:	c7 45 f0 00 00 00 00 	mov    DWORD PTR [ebp-0x10],0x0
  4847e7:	8b 55 f4             	mov    edx,DWORD PTR [ebp-0xc]
  4847ea:	c7 42 0c 00 00 00 00 	mov    DWORD PTR [edx+0xc],0x0
  4847f1:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  4847f4:	e8 b7 04 00 00       	call   0x484cb0
  4847f9:	8b e5                	mov    esp,ebp
  4847fb:	5d                   	pop    ebp
  4847fc:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484990 <.text+0x83990>:
  484990:	55                   	push   ebp
  484991:	8b ec                	mov    ebp,esp
  484993:	51                   	push   ecx
  484994:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484997:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  48499a:	83 e0 02             	and    eax,0x2
  48499d:	85 c0                	test   eax,eax
  48499f:	74 3b                	je     0x4849dc
  4849a1:	68 70 44 48 00       	push   0x484470
  4849a6:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4849a9:	8b 51 fc             	mov    edx,DWORD PTR [ecx-0x4]
  4849ac:	52                   	push   edx
  4849ad:	68 98 00 00 00       	push   0x98
  4849b2:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4849b5:	50                   	push   eax
  4849b6:	e8 9a 7c 01 00       	call   0x49c655
  4849bb:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  4849be:	83 e1 01             	and    ecx,0x1
  4849c1:	85 c9                	test   ecx,ecx
  4849c3:	74 0f                	je     0x4849d4
  4849c5:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  4849c8:	83 ea 04             	sub    edx,0x4
  4849cb:	52                   	push   edx
  4849cc:	e8 6c 77 01 00       	call   0x49c13d
  4849d1:	83 c4 04             	add    esp,0x4
  4849d4:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4849d7:	83 e8 04             	sub    eax,0x4
  4849da:	eb 21                	jmp    0x4849fd
  4849dc:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4849df:	e8 8c fa ff ff       	call   0x484470
  4849e4:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  4849e7:	83 e0 01             	and    eax,0x1
  4849ea:	85 c0                	test   eax,eax
  4849ec:	74 0c                	je     0x4849fa
  4849ee:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4849f1:	51                   	push   ecx
  4849f2:	e8 46 77 01 00       	call   0x49c13d
  4849f7:	83 c4 04             	add    esp,0x4
  4849fa:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4849fd:	8b e5                	mov    esp,ebp
  4849ff:	5d                   	pop    ebp
  484a00:	c2 04 00             	ret    0x4


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484470 <.text+0x83470>:
  484470:	55                   	push   ebp
  484471:	8b ec                	mov    ebp,esp
  484473:	6a ff                	push   0xffffffff
  484475:	68 99 e8 4a 00       	push   0x4ae899
  48447a:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  484480:	50                   	push   eax
  484481:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  484488:	51                   	push   ecx
  484489:	89 4d f0             	mov    DWORD PTR [ebp-0x10],ecx
  48448c:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  484493:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  484496:	83 c1 18             	add    ecx,0x18
  484499:	e8 12 60 f9 ff       	call   0x41a4b0
  48449e:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  4844a5:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  4844a8:	e8 53 00 00 00       	call   0x484500
  4844ad:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  4844b0:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  4844b7:	8b e5                	mov    esp,ebp
  4844b9:	5d                   	pop    ebp
  4844ba:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0041a4b0 <.text+0x194b0>:
  41a4b0:	55                   	push   ebp
  41a4b1:	8b ec                	mov    ebp,esp
  41a4b3:	51                   	push   ecx
  41a4b4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  41a4b7:	8b e5                	mov    esp,ebp
  41a4b9:	5d                   	pop    ebp
  41a4ba:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484500 <.text+0x83500>:
  484500:	55                   	push   ebp
  484501:	8b ec                	mov    ebp,esp
  484503:	51                   	push   ecx
  484504:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484507:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  48450a:	e8 11 00 00 00       	call   0x484520
  48450f:	8b e5                	mov    esp,ebp
  484511:	5d                   	pop    ebp
  484512:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484520 <.text+0x83520>:
  484520:	55                   	push   ebp
  484521:	8b ec                	mov    ebp,esp
  484523:	83 ec 10             	sub    esp,0x10
  484526:	89 4d f4             	mov    DWORD PTR [ebp-0xc],ecx
  484529:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
  48452c:	83 78 0c 00          	cmp    DWORD PTR [eax+0xc],0x0
  484530:	74 3f                	je     0x484571
  484532:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  484535:	c7 41 04 00 00 00 00 	mov    DWORD PTR [ecx+0x4],0x0
  48453c:	8b 55 f4             	mov    edx,DWORD PTR [ebp-0xc]
  48453f:	8b 42 0c             	mov    eax,DWORD PTR [edx+0xc]
  484542:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
  484545:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  484548:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  48454b:	83 7d fc 00          	cmp    DWORD PTR [ebp-0x4],0x0
  48454f:	74 0f                	je     0x484560
  484551:	6a 03                	push   0x3
  484553:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  484556:	e8 25 03 00 00       	call   0x484880
  48455b:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  48455e:	eb 07                	jmp    0x484567
  484560:	c7 45 f0 00 00 00 00 	mov    DWORD PTR [ebp-0x10],0x0
  484567:	8b 55 f4             	mov    edx,DWORD PTR [ebp-0xc]
  48456a:	c7 42 0c 00 00 00 00 	mov    DWORD PTR [edx+0xc],0x0
  484571:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  484574:	e8 f7 06 00 00       	call   0x484c70
  484579:	8b e5                	mov    esp,ebp
  48457b:	5d                   	pop    ebp
  48457c:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484880 <.text+0x83880>:
  484880:	55                   	push   ebp
  484881:	8b ec                	mov    ebp,esp
  484883:	51                   	push   ecx
  484884:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484887:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  48488a:	83 e0 02             	and    eax,0x2
  48488d:	85 c0                	test   eax,eax
  48488f:	74 38                	je     0x4848c9
  484891:	68 f0 48 48 00       	push   0x4848f0
  484896:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  484899:	8b 51 fc             	mov    edx,DWORD PTR [ecx-0x4]
  48489c:	52                   	push   edx
  48489d:	6a 10                	push   0x10
  48489f:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4848a2:	50                   	push   eax
  4848a3:	e8 ad 7d 01 00       	call   0x49c655
  4848a8:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  4848ab:	83 e1 01             	and    ecx,0x1
  4848ae:	85 c9                	test   ecx,ecx
  4848b0:	74 0f                	je     0x4848c1
  4848b2:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  4848b5:	83 ea 04             	sub    edx,0x4
  4848b8:	52                   	push   edx
  4848b9:	e8 7f 78 01 00       	call   0x49c13d
  4848be:	83 c4 04             	add    esp,0x4
  4848c1:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4848c4:	83 e8 04             	sub    eax,0x4
  4848c7:	eb 21                	jmp    0x4848ea
  4848c9:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4848cc:	e8 1f 00 00 00       	call   0x4848f0
  4848d1:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  4848d4:	83 e0 01             	and    eax,0x1
  4848d7:	85 c0                	test   eax,eax
  4848d9:	74 0c                	je     0x4848e7
  4848db:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4848de:	51                   	push   ecx
  4848df:	e8 59 78 01 00       	call   0x49c13d
  4848e4:	83 c4 04             	add    esp,0x4
  4848e7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  4848ea:	8b e5                	mov    esp,ebp
  4848ec:	5d                   	pop    ebp
  4848ed:	c2 04 00             	ret    0x4


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

004848f0 <.text+0x838f0>:
  4848f0:	55                   	push   ebp
  4848f1:	8b ec                	mov    ebp,esp
  4848f3:	51                   	push   ecx
  4848f4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  4848f7:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  4848fa:	e8 11 00 00 00       	call   0x484910
  4848ff:	8b e5                	mov    esp,ebp
  484901:	5d                   	pop    ebp
  484902:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484910 <.text+0x83910>:
  484910:	55                   	push   ebp
  484911:	8b ec                	mov    ebp,esp
  484913:	51                   	push   ecx
  484914:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484917:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  48491a:	e8 11 00 00 00       	call   0x484930
  48491f:	8b e5                	mov    esp,ebp
  484921:	5d                   	pop    ebp
  484922:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484930 <.text+0x83930>:
  484930:	55                   	push   ebp
  484931:	8b ec                	mov    ebp,esp
  484933:	83 ec 10             	sub    esp,0x10
  484936:	89 4d f4             	mov    DWORD PTR [ebp-0xc],ecx
  484939:	8b 45 f4             	mov    eax,DWORD PTR [ebp-0xc]
  48493c:	83 78 0c 00          	cmp    DWORD PTR [eax+0xc],0x0
  484940:	74 3f                	je     0x484981
  484942:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  484945:	c7 41 04 00 00 00 00 	mov    DWORD PTR [ecx+0x4],0x0
  48494c:	8b 55 f4             	mov    edx,DWORD PTR [ebp-0xc]
  48494f:	8b 42 0c             	mov    eax,DWORD PTR [edx+0xc]
  484952:	89 45 f8             	mov    DWORD PTR [ebp-0x8],eax
  484955:	8b 4d f8             	mov    ecx,DWORD PTR [ebp-0x8]
  484958:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  48495b:	83 7d fc 00          	cmp    DWORD PTR [ebp-0x4],0x0
  48495f:	74 0f                	je     0x484970
  484961:	6a 03                	push   0x3
  484963:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  484966:	e8 95 fe ff ff       	call   0x484800
  48496b:	89 45 f0             	mov    DWORD PTR [ebp-0x10],eax
  48496e:	eb 07                	jmp    0x484977
  484970:	c7 45 f0 00 00 00 00 	mov    DWORD PTR [ebp-0x10],0x0
  484977:	8b 55 f4             	mov    edx,DWORD PTR [ebp-0xc]
  48497a:	c7 42 0c 00 00 00 00 	mov    DWORD PTR [edx+0xc],0x0
  484981:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  484984:	e8 37 01 00 00       	call   0x484ac0
  484989:	8b e5                	mov    esp,ebp
  48498b:	5d                   	pop    ebp
  48498c:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484800 <.text+0x83800>:
  484800:	55                   	push   ebp
  484801:	8b ec                	mov    ebp,esp
  484803:	51                   	push   ecx
  484804:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484807:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  48480a:	83 e0 02             	and    eax,0x2
  48480d:	85 c0                	test   eax,eax
  48480f:	74 38                	je     0x484849
  484811:	68 70 48 48 00       	push   0x484870
  484816:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  484819:	8b 51 fc             	mov    edx,DWORD PTR [ecx-0x4]
  48481c:	52                   	push   edx
  48481d:	6a 48                	push   0x48
  48481f:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  484822:	50                   	push   eax
  484823:	e8 2d 7e 01 00       	call   0x49c655
  484828:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  48482b:	83 e1 01             	and    ecx,0x1
  48482e:	85 c9                	test   ecx,ecx
  484830:	74 0f                	je     0x484841
  484832:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  484835:	83 ea 04             	sub    edx,0x4
  484838:	52                   	push   edx
  484839:	e8 ff 78 01 00       	call   0x49c13d
  48483e:	83 c4 04             	add    esp,0x4
  484841:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  484844:	83 e8 04             	sub    eax,0x4
  484847:	eb 21                	jmp    0x48486a
  484849:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  48484c:	e8 1f 00 00 00       	call   0x484870
  484851:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  484854:	83 e0 01             	and    eax,0x1
  484857:	85 c0                	test   eax,eax
  484859:	74 0c                	je     0x484867
  48485b:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  48485e:	51                   	push   ecx
  48485f:	e8 d9 78 01 00       	call   0x49c13d
  484864:	83 c4 04             	add    esp,0x4
  484867:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  48486a:	8b e5                	mov    esp,ebp
  48486c:	5d                   	pop    ebp
  48486d:	c2 04 00             	ret    0x4


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484870 <.text+0x83870>:
  484870:	55                   	push   ebp
  484871:	8b ec                	mov    ebp,esp
  484873:	51                   	push   ecx
  484874:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484877:	8b e5                	mov    esp,ebp
  484879:	5d                   	pop    ebp
  48487a:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484a50 <.text+0x83a50>:
  484a50:	55                   	push   ebp
  484a51:	8b ec                	mov    ebp,esp
  484a53:	51                   	push   ecx
  484a54:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484a57:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  484a5a:	83 e0 02             	and    eax,0x2
  484a5d:	85 c0                	test   eax,eax
  484a5f:	74 38                	je     0x484a99
  484a61:	68 d3 c5 47 00       	push   0x47c5d3
  484a66:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  484a69:	8b 51 fc             	mov    edx,DWORD PTR [ecx-0x4]
  484a6c:	52                   	push   edx
  484a6d:	6a 08                	push   0x8
  484a6f:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  484a72:	50                   	push   eax
  484a73:	e8 dd 7b 01 00       	call   0x49c655
  484a78:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  484a7b:	83 e1 01             	and    ecx,0x1
  484a7e:	85 c9                	test   ecx,ecx
  484a80:	74 0f                	je     0x484a91
  484a82:	8b 55 fc             	mov    edx,DWORD PTR [ebp-0x4]
  484a85:	83 ea 04             	sub    edx,0x4
  484a88:	52                   	push   edx
  484a89:	e8 af 76 01 00       	call   0x49c13d
  484a8e:	83 c4 04             	add    esp,0x4
  484a91:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  484a94:	83 e8 04             	sub    eax,0x4
  484a97:	eb 21                	jmp    0x484aba
  484a99:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  484a9c:	e8 32 7b ff ff       	call   0x47c5d3
  484aa1:	8b 45 08             	mov    eax,DWORD PTR [ebp+0x8]
  484aa4:	83 e0 01             	and    eax,0x1
  484aa7:	85 c0                	test   eax,eax
  484aa9:	74 0c                	je     0x484ab7
  484aab:	8b 4d fc             	mov    ecx,DWORD PTR [ebp-0x4]
  484aae:	51                   	push   ecx
  484aaf:	e8 89 76 01 00       	call   0x49c13d
  484ab4:	83 c4 04             	add    esp,0x4
  484ab7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  484aba:	8b e5                	mov    esp,ebp
  484abc:	5d                   	pop    ebp
  484abd:	c2 04 00             	ret    0x4


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0047c5d3 <.text+0x7b5d3>:
  47c5d3:	55                   	push   ebp
  47c5d4:	8b ec                	mov    ebp,esp
  47c5d6:	6a ff                	push   0xffffffff
  47c5d8:	68 ac e4 4a 00       	push   0x4ae4ac
  47c5dd:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  47c5e3:	50                   	push   eax
  47c5e4:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  47c5eb:	83 ec 08             	sub    esp,0x8
  47c5ee:	89 4d ec             	mov    DWORD PTR [ebp-0x14],ecx
  47c5f1:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [ebp-0x4],0x0
  47c5f8:	8b 45 ec             	mov    eax,DWORD PTR [ebp-0x14]
  47c5fb:	83 78 04 00          	cmp    DWORD PTR [eax+0x4],0x0
  47c5ff:	74 15                	je     0x47c616
  47c601:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  47c604:	8b 51 04             	mov    edx,DWORD PTR [ecx+0x4]
  47c607:	89 55 f0             	mov    DWORD PTR [ebp-0x10],edx
  47c60a:	8b 45 f0             	mov    eax,DWORD PTR [ebp-0x10]
  47c60d:	50                   	push   eax
  47c60e:	e8 2a fb 01 00       	call   0x49c13d
  47c613:	83 c4 04             	add    esp,0x4
  47c616:	c7 45 fc ff ff ff ff 	mov    DWORD PTR [ebp-0x4],0xffffffff
  47c61d:	8b 4d ec             	mov    ecx,DWORD PTR [ebp-0x14]
  47c620:	83 c1 04             	add    ecx,0x4
  47c623:	e8 d8 01 00 00       	call   0x47c800
  47c628:	8b 4d f4             	mov    ecx,DWORD PTR [ebp-0xc]
  47c62b:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  47c632:	8b e5                	mov    esp,ebp
  47c634:	5d                   	pop    ebp
  47c635:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0047c800 <.text+0x7b800>:
  47c800:	55                   	push   ebp
  47c801:	8b ec                	mov    ebp,esp
  47c803:	51                   	push   ecx
  47c804:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  47c807:	8b e5                	mov    esp,ebp
  47c809:	5d                   	pop    ebp
  47c80a:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484ac0 <.text+0x83ac0>:
  484ac0:	55                   	push   ebp
  484ac1:	8b ec                	mov    ebp,esp
  484ac3:	51                   	push   ecx
  484ac4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484ac7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  484aca:	c7 00 00 00 00 00    	mov    DWORD PTR [eax],0x0
  484ad0:	8b e5                	mov    esp,ebp
  484ad2:	5d                   	pop    ebp
  484ad3:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484c70 <.text+0x83c70>:
  484c70:	55                   	push   ebp
  484c71:	8b ec                	mov    ebp,esp
  484c73:	51                   	push   ecx
  484c74:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484c77:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  484c7a:	c7 00 00 00 00 00    	mov    DWORD PTR [eax],0x0
  484c80:	8b e5                	mov    esp,ebp
  484c82:	5d                   	pop    ebp
  484c83:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484c90 <.text+0x83c90>:
  484c90:	55                   	push   ebp
  484c91:	8b ec                	mov    ebp,esp
  484c93:	51                   	push   ecx
  484c94:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484c97:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  484c9a:	c7 00 00 00 00 00    	mov    DWORD PTR [eax],0x0
  484ca0:	8b e5                	mov    esp,ebp
  484ca2:	5d                   	pop    ebp
  484ca3:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

00484cb0 <.text+0x83cb0>:
  484cb0:	55                   	push   ebp
  484cb1:	8b ec                	mov    ebp,esp
  484cb3:	51                   	push   ecx
  484cb4:	89 4d fc             	mov    DWORD PTR [ebp-0x4],ecx
  484cb7:	8b 45 fc             	mov    eax,DWORD PTR [ebp-0x4]
  484cba:	c7 00 00 00 00 00    	mov    DWORD PTR [eax],0x0
  484cc0:	8b e5                	mov    esp,ebp
  484cc2:	5d                   	pop    ebp
  484cc3:	c3                   	ret


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0049c655 <.text+0x9b655>:
  49c655:	55                   	push   ebp
  49c656:	8b ec                	mov    ebp,esp
  49c658:	6a ff                	push   0xffffffff
  49c65a:	68 c8 3f 4b 00       	push   0x4b3fc8
  49c65f:	68 00 f2 49 00       	push   0x49f200
  49c664:	64 a1 00 00 00 00    	mov    eax,fs:0x0
  49c66a:	50                   	push   eax
  49c66b:	64 89 25 00 00 00 00 	mov    DWORD PTR fs:0x0,esp
  49c672:	83 ec 0c             	sub    esp,0xc
  49c675:	53                   	push   ebx
  49c676:	56                   	push   esi
  49c677:	57                   	push   edi
  49c678:	83 65 e4 00          	and    DWORD PTR [ebp-0x1c],0x0
  49c67c:	8b 75 0c             	mov    esi,DWORD PTR [ebp+0xc]
  49c67f:	8b c6                	mov    eax,esi
  49c681:	0f af 45 10          	imul   eax,DWORD PTR [ebp+0x10]
  49c685:	01 45 08             	add    DWORD PTR [ebp+0x8],eax
  49c688:	83 65 fc 00          	and    DWORD PTR [ebp-0x4],0x0
  49c68c:	ff 4d 10             	dec    DWORD PTR [ebp+0x10]
  49c68f:	78 0b                	js     0x49c69c
  49c691:	29 75 08             	sub    DWORD PTR [ebp+0x8],esi
  49c694:	8b 4d 08             	mov    ecx,DWORD PTR [ebp+0x8]
  49c697:	ff 55 14             	call   DWORD PTR [ebp+0x14]
  49c69a:	eb f0                	jmp    0x49c68c
  49c69c:	c7 45 e4 01 00 00 00 	mov    DWORD PTR [ebp-0x1c],0x1
  49c6a3:	83 4d fc ff          	or     DWORD PTR [ebp-0x4],0xffffffff
  49c6a7:	e8 11 00 00 00       	call   0x49c6bd
  49c6ac:	8b 4d f0             	mov    ecx,DWORD PTR [ebp-0x10]
  49c6af:	64 89 0d 00 00 00 00 	mov    DWORD PTR fs:0x0,ecx
  49c6b6:	5f                   	pop    edi
  49c6b7:	5e                   	pop    esi
  49c6b8:	5b                   	pop    ebx
  49c6b9:	c9                   	leave
  49c6ba:	c2 10 00             	ret    0x10


C:\progs\Lemmings Revolution\Lemmings Revolution.exe:     file format pei-i386


Disassembly of section .text:

0049c6bd <.text+0x9b6bd>:
  49c6bd:	83 7d e4 00          	cmp    DWORD PTR [ebp-0x1c],0x0
  49c6c1:	75 11                	jne    0x49c6d4
  49c6c3:	ff 75 14             	push   DWORD PTR [ebp+0x14]
  49c6c6:	ff 75 10             	push   DWORD PTR [ebp+0x10]
  49c6c9:	ff 75 0c             	push   DWORD PTR [ebp+0xc]
  49c6cc:	ff 75 08             	push   DWORD PTR [ebp+0x8]
  49c6cf:	e8 01 00 00 00       	call   0x49c6d5
  49c6d4:	c3                   	ret


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
