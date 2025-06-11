        #
        # interrupts and exceptions while in supervisor
        # mode come here.
        #
        # the current stack is a kernel stack.
        # push registers, call kerneltrap().
        # when kerneltrap() returns, restore registers, return.
        #
.globl kerneltrap
.globl kernelvec
.align 4
/*
 * kernelvec:
 *   这是 xv6 中 RISC-V 的内核陷入向量入口点。
 *   - 分配栈空间以保存所有调用者保存的寄存器。
 *   - 将通用寄存器的当前值保存到栈上。
 *   - 调用 C 语言的陷入处理函数（kerneltrap）来处理异常/中断。
 *   - 处理函数返回后，从栈中恢复保存的寄存器。
 *   - 恢复栈指针，并使用 sret 指令从陷入返回。
 *   - 注意：tp 寄存器（线程指针，通常为 hartid）不会被恢复，以避免陷入处理函数切换 CPU 时出现问题。
 */
kernelvec:
        # 为保存寄存器分配空间
        addi sp, sp, -256

        # 保存调用者保存的寄存器
        sd ra, 0(sp)
        sd sp, 8(sp)
        sd gp, 16(sp)
        sd tp, 24(sp)
        sd t0, 32(sp)
        sd t1, 40(sp)
        sd t2, 48(sp)qi
