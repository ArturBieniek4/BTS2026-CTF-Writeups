// gcc -m32 -shared -fPIC -nostdlib -o libponi.so libponi.c -fno-stack-protector

_Noreturn void exit(ret)
    int ret;
{
    __asm__ __volatile__(
        "movl $1, %%eax\n\t"
        "movl %0, %%ebx\n\t"
        "mov $panic, %%esi\n\t"
        "cmp $1, %%eax\n\t"
        "je OK_EXIT\n\t"
        "call *%%esi\n\t"
        "OK_EXIT: int $0x80\n\t"
        : : "m" (ret) : "%ebx");

    __builtin_unreachable();
}

/* ( ˶°ㅁ°) !! */
void panic(void)
{
    exit(-1);
}

int read(fd, buf, count)
    int fd;
    char *buf;
    int count;
{
    int result;
    __asm__ __volatile__(
        "movl $3, %%eax\n\t"
        "movl %1, %%ebx\n\t"
        "movl %2, %%ecx\n\t"
        "movl %3, %%edx\n\t"
        "mov $panic, %%esi\n\t"
        "cmp $3, %%eax\n\t"
        "je OK_READ\n\t"
        "call *%%esi\n\t"
        "OK_READ:\n\t"
        "pushl %%eax\n\t"
        "int $0x80\n\t"
        "popl %%edi\n\t"
        "cmpl $3, %%edi\n\t"
        "je OK_READ_POST\n\t"
        "call *%%esi\n\t"
        "OK_READ_POST:\n\t"
        "movl %%eax, %0"
        : "=m" (result)
        : "m" (fd), "m" (buf), "m" (count)
        : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");

    return result;
}

int write(fd, buf, count)
    int fd;
    char *buf;
    int count;
{
    int result;
    __asm__ __volatile__(
        "movl $4, %%eax\n\t"
        "movl %1, %%ebx\n\t"
        "movl %2, %%ecx\n\t"
        "movl %3, %%edx\n\t"
        "mov $panic, %%esi\n\t"
        "cmp $4, %%eax\n\t"
        "je OK_WRITE\n\t"
        "call *%%esi\n\t"
        "OK_WRITE:\n\t"
        "pushl %%eax\n\t"
        "int $0x80\n\t"
        "popl %%edi\n\t"
        "cmpl $4, %%edi\n\t"
        "je OK_WRITE_POST\n\t"
        "call *%%esi\n\t"
        "OK_WRITE_POST:\n\t"
        "movl %%eax, %0"
        : "=m" (result)
        : "m" (fd), "m" (buf), "m" (count)
        : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");

    return result;
}