/**
 * 协程环切换
 *                  by colin
 */
.globl coctx_swap
#if !defined( __APPLE__ ) && !defined( __FreeBSD__ )
.type  coctx_swap, @function
#endif

coctx_swap:

#if defined(__x86_64__)
    /* 保存当前环境 */
    movq        %rbx, 0(%rdi)
    movq        %rbp, 8(%rdi)
    movq        %r12, 16(%rdi)
    movq        %r13, 24(%rdi)
    movq        %r14, 32(%rdi)
    movq        %r15, 40(%rdi)
    movq        %rdi, 48(%rdi)
    movq        %rsi, 56(%rdi)
    movq        %rdx, 64(%rdi)
    movq        %rcx, 72(%rdi)
    movq        %r8, 80(%rdi)
    movq        %r9, 88(%rdi)

    movq        (%rsp), %rcx    /* 返回地址 */
    movq        %rcx, 96(%rdi)
    leaq        8(%rsp), %rcx   /* 栈顶 */
    movq        %rcx, 104(%rdi)
    
    fnstcw      112(%rdi)       /* 浮点数环境 */
    stmxcsr     116(%rdi)

    /* 恢复将启动的协程环境 */
    movq        %rsi, %rax
    movq        0(%rax), %rbx
    movq        8(%rax), %rbp
    movq        16(%rax), %r12
    movq        24(%rax), %r13
    movq        32(%rax), %r14
    movq        40(%rax), %r15
    movq        48(%rax), %rdi
    movq        56(%rax), %rsi
    movq        64(%rax), %rdx
    movq        80(%rax), %r8
    movq        88(%rax), %r9
    fldcw       112(%rax)       /* 浮点数环境 */
    ldmxcsr     116(%rax)

    movq        104(%rax), %rsp  /* 栈顶 */
    movq        96(%rax), %rcx   /* 弹出返回地址到栈顶 */
    pushq        %rcx

    movq        72(%rax), %rcx
    
    /* Clear rax to indicate success.  */
    xorl        %eax, %eax
    ret                         /* ret时就弹出栈顶的地址并跳过去 */
#else
    #error "platform not support"
#endif