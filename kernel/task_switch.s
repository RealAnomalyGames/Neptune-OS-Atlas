.section .text

.global task_context_switch
.type task_context_switch, @function

task_context_switch:

    pusha

    /*
     * Stack after pusha:
     *
     *   0(%esp)  = EDI
     *   4(%esp)  = ESI
     *   8(%esp)  = EBP
     *  12(%esp)  = original ESP
     *  16(%esp)  = EBX
     *  20(%esp)  = EDX
     *  24(%esp)  = ECX
     *  28(%esp)  = EAX
     *  32(%esp)  = return address
     *  36(%esp)  = current task
     *  40(%esp)  = next task
     */

    movl 36(%esp), %eax

    /* Save general-purpose registers. */

    movl 28(%esp), %ecx
    movl %ecx, 0(%eax)

    movl 16(%esp), %ecx
    movl %ecx, 4(%eax)

    movl 24(%esp), %ecx
    movl %ecx, 8(%eax)

    movl 20(%esp), %ecx
    movl %ecx, 12(%eax)

    movl 4(%esp), %ecx
    movl %ecx, 16(%eax)

    movl 0(%esp), %ecx
    movl %ecx, 20(%eax)

    movl 8(%esp), %ecx
    movl %ecx, 24(%eax)

    /* Save the stack position after returning from this function. */

    movl 12(%esp), %ecx
    addl $4, %ecx
    movl %ecx, 28(%eax)

    /* Save current EIP. */

    movl 32(%esp), %ecx
    movl %ecx, 32(%eax)

    /* Save EFLAGS. */

    pushfl
    popl %ecx
    movl %ecx, 36(%eax)

    /*
     * Preserve the next-task pointer in EBP.
     * We cannot keep it in EDX because EDX itself
     * belongs to the next task.
     */

    movl 40(%esp), %ebp

    addl $32, %esp

    /* Switch to the next task's stack. */

    movl 28(%ebp), %esp

    /*
     * Put the next task's return address and flags
     * onto its stack.
     */

    pushl 32(%ebp)
    pushl 36(%ebp)

    /* Restore next task's registers. */

    movl 0(%ebp), %eax
    movl 4(%ebp), %ebx
    movl 8(%ebp), %ecx
    movl 12(%ebp), %edx
    movl 16(%ebp), %esi
    movl 20(%ebp), %edi
    movl 24(%ebp), %ebp

    popfl
    ret

.size task_context_switch, .-task_context_switch