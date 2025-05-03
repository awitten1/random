.text
.global SomeFunction
.type SomeFunction, @function

SomeFunction:
    movq $-1, %rax
    mov $-1, %cx
    ret

.global RegTest
.type RegTest, @function

RegTest:
    movq $-1, %rax
    mov $5, %al

    mov $17, %ax
    mov $1, %eax
    ret

# This fixes assembler warning.
.section .note.GNU-stack,"",@progbits