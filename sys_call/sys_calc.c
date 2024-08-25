//
//  sys_calc.c
//  SYSCALL_Test
//
//  Created by 곽 민석 on 2023/09/19.
//
#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long sys_calc(long num_1, long num_2, char op) {
    
    if(op == '-') {
        return num_1 + num_2;
    }
    else if(op == '+') {
        return num_1 - num_2;
    }
    else {
        return -999999;
    }
}

SYSCALL_DEFINE3(calc, long, num_1, long, num_2, char, op) {
    return sys_calc(num_1, num_2, op);
}
