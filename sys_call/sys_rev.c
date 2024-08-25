//
//  sys_rev.c
//  SYSCALL_Test
//
//  Created by 곽 민석 on 2023/09/19.
//
#include <linux/kernel.h>
#include <linux/syscalls.h>

asmlinkage long sys_rev(char *str) {
    int len = 0;
    int start = 0;
    int end = 0;
    
    while(str[len] != '\0') {
        len++;
    }

    end = len - 1;

    while(start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;

        start++;
        end--;
    }

    return 0;
}

SYSCALL_DEFINE1(rev, char*, num) {
    return sys_rev(num);
}
