//
//  main.c
//  SYSCALL_Test
//
//  Created by 곽 민석 on 2023/09/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/kernel.h>
#include <sys/syscall.h>

// System call table
#define SYSCALL_CALC 449
#define SYSCALL_REV  450

#define OP_ERR 0
#define OP_PRINT 1

// SYS CALL 시작. 
// SYS CALL 끝. 

// 문자열 내의 숫자 여부를 판별하는 함수. 
int isDigit(char *num) {
    int len = strlen(num);

    for(int i = 0;i < len;i++) {
        if(num[i] >= '0' && num[i] <= '9' || num[i] == '\n') continue;
        else return 0;
    }

    return 1;
}

// + 혹은 -가 있는 index를 반환하는 함수.
int whereOp(char *input) {
    for(int i = 0;i < strlen(input);i++) {
        if(input[i] == '+' || input[i] == '-') return i;
    }

    return -1;
}

// 시작 인덱스부터 말단 인덱스까지의 글자를 반환하는 함수.
char* getNumberString(char *input, int start_idx, int end_idx) {
    char *number = (char *)malloc(sizeof(char) * 1024);

    for(int i = start_idx;i < end_idx;i++) {
        number[i - start_idx] = input[i];
    }
    // 끝 자리에 NULL 삽입.
    number[strlen(input) - start_idx] = '\0';

    return number;
}

// 입력된 문자열에 대해 공백 제거 후 반환하는 함수. 
char* removeSpace(char *input) {
    char *input_removed = (char*)malloc(sizeof(char) * 1024);
    int len = strlen(input);
    int pos_removed = 0;

    for(int i = 0;i < len - 1;i++) {
        if(input[i] == ' ') continue;

        input_removed[pos_removed++] = input[i];
    }
    input_removed[pos_removed] = '\0';

    return input_removed;
}

long sys_rev(char *str) {
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

int main(int argc, char const *argv[]) {

    while(1) {
        int idx = -1;
        int op_state = OP_PRINT;
        char input[2048];

        printf("Input: ");
        fgets(input, sizeof(input), stdin);
        strcpy(input, removeSpace(input));
        
        idx = whereOp(input);

        // 아무것도 입력되지 않을 시 종료. 
        if(strlen(input) == 0) return 0;
        
        // 부호의 입력 여부에 따른 동작. 
        if(idx == -1) {
            if(!isDigit(input)) 
                printf("Wrong Input!\n");
            else {
                // 에러가 발생하지 않는경우 SYS Call
                // syscall(SYSCALL_REV, input);
                sys_rev(input);
                printf("Output: %s\n", input);
            }
        }
        else {
            char *num_1 = getNumberString(input, 0, idx);
            char *num_2 = getNumberString(input, idx + 1, (int)strlen(input));
            char op     = input[idx];

            // 예외 발생시 오류코드 저장. 
            if(!(isDigit(num_1) && isDigit(num_2))) op_state = OP_ERR;
            if(!(op == '-' || op == '+'))           op_state = OP_ERR;

            // 에러가 발생하지 않는경우 SYS Call
            if(op_state) {
                long int result = syscall(SYSCALL_CALC, atol(num_1), atol(num_2), op);
                // long int result = sys_calc(atol(num_1), atol(num_2), op);
                printf("Output: %ld\n", result);
            }
            else printf("Wrong Input!\n");
        }

    }
    
    return 0;
}

