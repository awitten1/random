.text
.global SomeFunction
.type SomeFunction, @function

SomeFunction:
    movq $-1, %rax
    mov $-1, %cx
    ret

.global RegTest
.type RegTest, @function

.data
    message:  .string "Hello, World!\n"      # Null-terminated string
    number1:  .word   42                   # 16-bit word
    bigNum:   .long   1000000              # 32-bit value
    realNum:  .float  3.14                 # 32-bit floating point
    myArray:  .zero   10                   # 10 bytes of zeros
    vector:   .long   1, 2, 3, 4           # Array of 4 longs

.text
RegTest:
    pushq %rbp
    movq %rsp, %rbp

    subq $8, %rsp

    movq $-1, %rax
    mov $5, %al

    mov $17, %ax
    mov $1, %eax


    leaq message(%rip), %rdi
    xorq %rax, %rax 
    call printf

    movq %rbp, %rsp
    popq %rbp

    ret

# This fixes assembler warning.
.section .note.GNU-stack,"",@progbits