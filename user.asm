[bits 32]
dd END
SECTION code vstart=0
    mov ecx,100;循环三次
RE:
    mov edi,0x01
    mov ebx,ecx
    int 0x66;以16进制打印ECX寄存器的值
    xor edi,edi 
    mov ebx,STRING
    int 0x66;打印字符串
    mov ebx,2000
    mov edi,3
    int 0x66
    loop RE
    ;终止程序
    mov edi,2
    int 0x66;程序终止调用


STRING db "Hello I'm a user program run in ring 3.I was printed my ecx register.",0x0a,0x0d,0

SECTION ref 
END: