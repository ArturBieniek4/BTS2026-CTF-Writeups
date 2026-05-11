// gcc a.c -m32 -Wl,-rpath,.,-dynamic-linker,./ld-linux.so.2 -nostdlib -fno-stack-protector -fno-pie -no-pie -L. -lponi

_Noreturn void exit(int ret);
int read(int fd, char *buf, int count);
int write(int fd, char *buf, int count);

int main(void)
{
    char da_shellcode[8];
    write(1, da_shellcode, 64);
    read(0, da_shellcode, 28);

    return 0;
}

void _start(void)
{
    main();
    exit(0);
}