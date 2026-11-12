# Reference SHA-256 32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f

# ev_57f7a225ef5bddf5ce511f8e6ed87f8502bba25d5d16fe725eea75234a29b563; offset 16512; length 18
00404080 55                             push ebp
00404081 8bec                           mov ebp, esp
00404083 e818000000                     call 0x4040a0
00404088 dc25a8034b00                   fsub qword ptr [0x4b03a8]
0040408E dcc0                           fadd st(0), st(0)
00404090 5d                             pop ebp
00404091 c3                             ret

# ev_de913a4bf6e2087eb6763ad6098f170600d23c69ea9f9f4e0ebb7341c593b541; offset 16544; length 25
004040A0 55                             push ebp
004040A1 8bec                           mov ebp, esp
004040A3 51                             push ecx
004040A4 e8a1870900                     call 0x49c84a
004040A9 8945fc                         mov dword ptr [ebp - 4], eax
004040AC db45fc                         fild dword ptr [ebp - 4]
004040AF dc35b0034b00                   fdiv qword ptr [0x4b03b0]
004040B5 8be5                           mov esp, ebp
004040B7 5d                             pop ebp
004040B8 c3                             ret

# ev_6cd304055420653548c3b817ab63d30468af93a72cdaf54756739c3d7daafb63; offset 641098; length 30
0049C84A a170454c00                     mov eax, dword ptr [0x4c4570]
0049C84F 69c0fd430300                   imul eax, eax, 0x343fd
0049C855 05c39e2600                     add eax, 0x269ec3
0049C85A a370454c00                     mov dword ptr [0x4c4570], eax
0049C85F c1f810                         sar eax, 0x10
0049C862 25ff7f0000                     and eax, 0x7fff
0049C867 c3                             ret
