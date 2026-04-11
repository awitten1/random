/*
 * example4.c — shellcode injection target
 *
 * Compile (disabling all modern mitigations):
 *   gcc -fno-stack-protector -z execstack -no-pie -O0 example4.c -o example4
 *
 * Disable ASLR:
 *   echo 0 | sudo tee /proc/sys/kernel/randomize_va_space
 *
 * Run exploit:
 *   python3 exploit4.py | ./example4
 *
 * Stack layout inside vulnerable() (x86-64, grows downward):
 *
 *   higher addresses
 *   ┌──────────────────┐
 *   │ return address   │  ← we overwrite this to point at buffer
 *   ├──────────────────┤
 *   │ saved rbp        │  ← 8 bytes
 *   ├──────────────────┤
 *   │ buffer[64]       │  ← shellcode lives here
 *   └──────────────────┘
 *   lower addresses
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

void vulnerable(void) {
    char buffer[64];

    /* read() so we can send raw bytes including nulls (unlike strcpy) */
    int n = read(STDIN_FILENO, buffer, 256);
    printf("Read %d bytes\n", n);

    /* After this function returns, execution jumps to whatever is in
     * the saved return address — which we've overwritten with &buffer. */
}

int main(void) {
    printf("Before call, pid=%d\n", getpid());
    vulnerable();
    printf("Returned normally (shellcode didn't execute)\n");
    return 0;
}
