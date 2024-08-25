#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 5000
#define LEN_RESULT 1024

// 파일을 읽어 내용을 반환하는 함수. 
void get_file_contents(char *path_file, int *arr_mem_acc) {
    FILE *file = fopen(path_file, "r");

    // 파일에서 줄 단위로 읽어와 배열에 저장
    char buffer[1024];
    
    for(int i = 0;i < MAX_INPUT;i++) {
        fgets(buffer, sizeof(buffer), file);
        arr_mem_acc[i] = atoi(buffer);
    }

    fclose(file);
}

// MAX 값에 맞는 무작위 정수형 배열로 반환하는 함수. 
void get_random_input(int arr_mem_acc[], int max) {
    FILE *file = fopen("input.in", "w");
    int tmp;
    int min = 0;
    // int min = max - (max / 16);
    printf("%d %d\n", min, max);

    for(int i = 0;i < MAX_INPUT;i++) {
        tmp = rand() % (max - min + 1) + min;
        arr_mem_acc[i] = tmp;
        fprintf(file, "%d\n", tmp);
    }

    fclose(file);
}

// 배열의 내용을 저장하는 함수. 
void save_log(char *name_file, char **contents, int op_head) {
    FILE *file = fopen(name_file, "w");

    // is_head가 1이면 배열의 첫 번째 항목을 파일에 쓰기. 
    if (op_head == 1) {
        fprintf(file, "%-10s %-10s %-10s %-10s %-10s %-10s\n", "No.", "V.A", "Page No.", "Frame No.", "P.A.", "Page Fault");
    }

    // 배열의 나머지 내용을 파일에 쓰기. 
    for (int i = 0;i < MAX_INPUT + 1; i++) {
        fprintf(file, "%s", contents[i]);
    }

    fclose(file);
}

// 프레임 내의 오프셋을 구하는 함수. 
int get_frame_offset(int size_frame) {
    int offset = 0;

    if(size_frame == 1) offset = 10;
    else if(size_frame == 2) offset = 11;
    else if(size_frame == 4) offset = 12;

    return offset;
}

// 가상 주소를 페이지 주소로 변환하는 함수. 
int get_page_number(int size_frame, int va) {
    int offset = get_frame_offset(size_frame);

    return va >> offset;
}

// 가상 주소를 실제 주소로 변환하는 함수. 
int get_physical_addr(int size_frame, int num_frame, int va) {
    int offset = get_frame_offset(size_frame);
    int num_page_base = get_page_number(size_frame, va);

    return va - (num_page_base << offset) + (num_frame * size_frame * 1024);
}

// 다음에 얼마나 뒤에 나오는지 구하는 함수. 
void find_next_page(int array[], int next_occurrences[]) {
    const int NOT_FOUND = 99999;

    // 배열을 순회하면서 각 원소가 다음으로 나오는 인덱스를 찾기
    for (int i = 0; i < MAX_INPUT; i++) {
        int found_index = NOT_FOUND;

        // 현재 원소 이후에서 해당 원소를 찾기
        for (int j = i + 1; j < MAX_INPUT; j++) {
            if (array[j] == array[i]) {
                found_index = j;
                break; // 찾으면 반복 중단
            }
        }

        next_occurrences[i] = found_index;
    }
}

// FIFO 페이지 교체 알고리즘 시뮬레이션 함수. 
char ** FIFO(int *arr_input, int len_va, int size_frame, int size_pm, int op_input) {
    // 기록 결과 배열. 
    int cnt_page_fault = 0;
    char **result = (char **)malloc(sizeof(char **) * MAX_INPUT + 1);
    for(int i = 0;i < MAX_INPUT + 1;i++) result[i] = (char *)malloc(sizeof(char *) * LEN_RESULT);

    // 현재 참조하고 있는 위치. 
    int num_frame = 0;

    // 페이지 갯수 구하기. 
    int max_pages = size_pm / size_frame;
    int *pages = (int *)malloc(sizeof(int) * max_pages);

    // page 초기화. 
    for(int i = 0;i < max_pages;i++) {
        pages[i] = -1;
    }

    for(int i = 0;i < MAX_INPUT;i++) {
        int num_phyx;
        int is_hit = -1;
        int num_page = get_page_number(size_frame, arr_input[i]);

        // HIT 확인. 
        for(int j = 0;j < max_pages;j++) {
            // printf("Page %d = %d\n", j + 1, pages[j]);
            if(pages[j] == num_page) {
                is_hit = j;
                break;
            }
        }

        if(is_hit != -1) num_phyx = get_physical_addr(size_frame, is_hit, arr_input[i]);
        else num_phyx = get_physical_addr(size_frame, num_frame % max_pages, arr_input[i]);

        // HIT가 아닌 경우 페이지 교체. 
        if(is_hit == -1) {
            is_hit = 'F';
            pages[num_frame % max_pages] = num_page;
            // printf("%-10d %-10d %-10d %-10d %-10d %-10d\n", i + 1, arr_input[i], num_page, (num_frame % max_pages), num_phyx, is_hit);
            sprintf(result[i], 
                    "%-10d %-10d %-10d %-10d %-10d %-10c\n", 
                    i + 1, arr_input[i], 
                    num_page, (num_frame % max_pages), 
                    num_phyx, (char)is_hit);
            num_frame++;
            cnt_page_fault++;
        }
        else {
            sprintf(result[i], 
                    "%-10d %-10d %-10d %-10d %-10d %-10c\n", 
                    i + 1, arr_input[i], 
                    num_page, is_hit, 
                    num_phyx, 'H');
        }
    }
    sprintf(result[MAX_INPUT], "Page Fault count: %5d\n", cnt_page_fault);

    return result;
}

// LRU 페이지 교체 알고리즘 시뮬레이션 함수. 
char ** LRU(int *arr_input, int len_va, int size_frame, int size_pm, int op_input) {
    // 기록 결과 배열. 
    int cnt_page_fault = 0;
    char **result = (char **)malloc(sizeof(char **) * MAX_INPUT + 1);
    for(int i = 0;i < MAX_INPUT + 1;i++) result[i] = (char *)malloc(sizeof(char *) * LEN_RESULT);

    // 페이지 갯수 구하기. 
    int max_pages = size_pm / size_frame;
    int *pages = (int *)malloc(sizeof(int) * max_pages);
    int *ages = (int *)malloc(sizeof(int) * max_pages);

    // page 초기화. 
    for(int i = 0;i < max_pages;i++) {
        pages[i] = -1;
        ages[i] = -9999;
    }

    for(int i = 0;i < MAX_INPUT;i++) {
        int num_page = get_page_number(size_frame, arr_input[i]);
        int pos_least_page = 0;
        int is_hit = -1;
        int num_phyx;
        char is_hit_c = 'H';

        // Hit 검사 및 가장 
        for(int j = 0;j < max_pages;j++) {
            if(pages[j] == num_page) {
                ages[j] = 0;
                is_hit = j;
                break;
            }
            else {
                if(ages[pos_least_page] > ages[j]) pos_least_page = j;
            }
        }

        // Hit 실패인 경우 페이지 교체. 
        if(is_hit == -1) {
            pages[pos_least_page] = num_page;
            ages[pos_least_page] = 0;
            is_hit = pos_least_page;
            cnt_page_fault++;
            is_hit_c = 'F';
        }

        // 물리 주소 계산. 
        num_phyx = get_physical_addr(size_frame, is_hit, arr_input[i]);

        // 에이징 계산. 
        for(int j = 0;j < max_pages;j++) ages[is_hit]--;

        // 출력. 
        sprintf(result[i], 
                "%-10d %-10d %-10d %-10d %-10d %-10c\n", 
                i + 1, arr_input[i], 
                num_page, is_hit, 
                num_phyx, is_hit_c);
    }
    sprintf(result[MAX_INPUT], "Page Fault count: %5d\n", cnt_page_fault);

    return result;
}

// Second chance 페이지 교체 알고리즘 시뮬레이션 함수. 
char ** second_chance(int *arr_input, int len_va, int size_frame, int size_pm, int op_input) {
    // 기록 결과 배열. 
    int cnt_page_fault = 0;
    char **result = (char **)malloc(sizeof(char **) * MAX_INPUT + 1);
    for(int i = 0;i < MAX_INPUT + 1;i++) result[i] = (char *)malloc(sizeof(char *) * LEN_RESULT);

    // 현재 참조하고 있는 위치. 
    int num_frame = 0;

    // 페이지 갯수 구하기. 
    int max_pages = size_pm / size_frame;
    int *pages = (int *)malloc(sizeof(int) * max_pages);
    int *hit = (int *)malloc(sizeof(int) * max_pages);

    // page, hit 여부 초기화. 
    for(int i = 0;i < max_pages;i++) {
        pages[i] = -1;
        hit[i] = 0;
    }

    for(int i = 0;i < MAX_INPUT;i++) {
        int num_phyx;
        int is_hit = -1;
        int num_page = get_page_number(size_frame, arr_input[i]);

        // HIT 확인. 
        for(int j = 0;j < max_pages;j++) {
            // printf("Page %d = %d\n", j + 1, pages[j]);
            if(pages[j] == num_page) {
                is_hit = j;
                hit[j] = 1;
                break;
            }
        }

        // HIT이 아닌 경우 페이지 교체. 
        if(is_hit == -1) {
            int is_replaced = 0;

            // 현재 순번의 HIT 확인. 
            for(int j = num_frame % max_pages;j < max_pages;j++) {
                if(hit[j] == 0) {
                    pages[j] = num_page;
                    hit[j] = 0;
                    is_replaced = 1;
                    break;
                }
                else hit[j] = 0;

                num_frame++;
            }

            // 페이지 교체 실패시 앞부터 다시 순환. 
            if(!is_replaced) {
                for(int j = 0;j < (num_frame % max_pages) + 1;j++) {
                    if(hit[j] == 0) {
                        pages[j] = num_page;
                        hit[j] = 0;
                        break;
                    }
                    else hit[j] = 0;

                    num_frame++;
                }
            }
        }

        if(is_hit == -1) {
            num_phyx = get_physical_addr(size_frame, (num_frame % max_pages), arr_input[i]);
            // printf("%-10d %-10d %-10d %-10d %-10d %-10d\n", i + 1, arr_input[i], num_page, (num_frame % max_pages), num_phyx, is_hit);
            sprintf(result[i], 
                    "%-10d %-10d %-10d %-10d %-10d %-10c\n", 
                    i + 1, arr_input[i], 
                    num_page, (num_frame % max_pages), 
                    num_phyx, 'F');
            num_frame++;
            cnt_page_fault++;
        }
        else {
            num_phyx = get_physical_addr(size_frame, is_hit, arr_input[i]);
            // printf("%-10d %-10d %-10d %-10d %-10d %-10d\n", i + 1, arr_input[i], num_page, is_hit, num_phyx, is_hit);
            sprintf(result[i], 
                    "%-10d %-10d %-10d %-10d %-10d %-10c\n", 
                    i + 1, arr_input[i], 
                    num_page, is_hit, 
                    num_phyx, 'H');
        }
    }
    sprintf(result[MAX_INPUT], "Page Fault count: %5d\n", cnt_page_fault);

    return result;
}

// Optimal 페이지 교체 알고리즘 시뮬레이션 함수. 
char ** OPT(int *arr_input, int len_va, int size_frame, int size_pm, int op_input) {
    // 기록 결과 배열. 
    char **result = (char **)malloc(sizeof(char **) * MAX_INPUT + 1);
    for(int i = 0;i < MAX_INPUT + 1;i++) result[i] = (char *)malloc(sizeof(char *) * LEN_RESULT);

    // 페이지 갯수 구하기. 
    int max_pages = size_pm / size_frame;
    int *pages = (int *)malloc(sizeof(int) * max_pages);
    int *next_hit = (int *)malloc(sizeof(int) * max_pages);
    int page_num[MAX_INPUT];
    int cnt_page_fault = 0;
    int num_frame = 0;

    // page, hit 여부 초기화. 
    for(int i = 0;i < max_pages;i++) {
        pages[i] = -1;
        next_hit[i] = 99999;
    }

    // 페이지 넘버 선 계산. 
    for(int i = 0;i < MAX_INPUT;i++) {
        page_num[i] = get_page_number(size_frame, arr_input[i]);
    }
    find_next_page(page_num, page_num);

    for(int i = 0;i < MAX_INPUT;i++) {
        int num_phyx;
        int is_hit = -1;
        int num_page = get_page_number(size_frame, arr_input[i]);

        // HIT 확인. 
        for(int j = 0;j < max_pages;j++) {
            // printf("Page %d = %d\n", j + 1, pages[j]);
            if(pages[j] == num_page) {
                is_hit = j;
                next_hit[j] = page_num[i];
                break;
            }
        }

        // HIT이 아닌 경우 페이지 교체 후 출력. 
        if(is_hit == -1) {
            int idx_victim = 0;

            // 가장 오래 쓰이지 않을 페이지 찾기. 
            for(int j = 1;j < max_pages;j++) {
                if(next_hit[idx_victim] < next_hit[j]) idx_victim = j;
            }

            // 찾은 페이지 교체. 
            pages[idx_victim] = num_page;
            next_hit[idx_victim] = page_num[i];
            
            num_phyx = get_physical_addr(size_frame, (num_frame % max_pages), arr_input[i]);
            // printf("%-10d %-10d %-10d %-10d %-10d %-10d\n", i + 1, arr_input[i], num_page, (num_frame % max_pages), num_phyx, is_hit);
            sprintf(result[i], 
                    "%-10d %-10d %-10d %-10d %-10d %-10c\n", 
                    i + 1, arr_input[i], 
                    num_page, (num_frame % max_pages), 
                    num_phyx, 'F');
            num_frame++;
            cnt_page_fault++;
        }
        else {
            num_phyx = get_physical_addr(size_frame, is_hit, arr_input[i]);
            // printf("%-10d %-10d %-10d %-10d %-10d %-10d\n", i + 1, arr_input[i], num_page, is_hit, num_phyx, is_hit);
            sprintf(result[i], 
                    "%-10d %-10d %-10d %-10d %-10d %-10c\n", 
                    i + 1, arr_input[i], 
                    num_page, is_hit, 
                    num_phyx, 'H');
        }
    }
    sprintf(result[MAX_INPUT], "Page Fault count: %5d\n", cnt_page_fault);

    return result;
}

int main() {
    int arr_input[MAX_INPUT];
    int len_va, size_frame, size_pm, op_algo, op_input;
    char path_input_file[1024];
    char **result;

    printf("A. Simulation에 사용할 가상주소 길이를 선택하시오 (1. 18bits 2. 19bits 3. 20bits): ");
    scanf("%d", &len_va);
    printf("\n");
    
    printf("B. Simulation에 사용할 페이지(프레임)의 크기를 선택하기오 (1. 1KB 2. 2KB 3. 4KB): ");
    scanf("%d", &size_frame);
    printf("\n");
    
    printf("C. Simulation에 사용할 물리메모리의 크기를 선택하시오 (1. 32KB 2. 64KB): ");
    scanf("%d", &size_pm);
    printf("\n");
    
    printf("D. Simulation에 적용할 Page Replacement 알고리즘을 선택하시오\n");
    printf("(1. Optimal 2. FIFO 3. LRU 4. Second-Chance): ");
    scanf("%d", &op_algo);
    printf("\n");

    printf("E. 가상주소 스트링 입력방식을 선택하시오\n");
    printf("(1. input.in 자동 생성 2. 기존 파일 사용): ");
    scanf("%d", &op_input);
    printf("\n");

    if(op_input == 2) {
        printf("F. 입력 파일 이름을 입력하시오: ");
        scanf("%s", path_input_file);
        printf("%s\n", path_input_file);
        // fgets(path_input_file, sizeof(path_input_file), stdin);
    }

    // 랜덤 함수 초기화. 
    srand(time(NULL));

    // 옵션에 따른 파일 입력. 
    if(op_input == 1) {
        if(size_frame == 1)
            get_random_input(arr_input, 262144);
        else if(size_frame == 2)
            get_random_input(arr_input, 524288);
        else if(size_frame == 3) 
            get_random_input(arr_input, 1048576);
    }
    else if(op_input == 2) {
        get_file_contents(path_input_file, arr_input);
    }

    // 실제 크기로 변환. 
    len_va += 17;
    if(size_frame == 3) size_frame = 4;
    size_pm *= 32;

    switch(op_algo) {
        case 1:
            result = OPT(arr_input, len_va, size_frame, size_pm, op_input);
            break;
        case 2:
            result = FIFO(arr_input, len_va, size_frame, size_pm, op_input);
            break;
        case 3:
            result = LRU(arr_input, len_va, size_frame, size_pm, op_input);
            break;
        case 4:
            result = second_chance(arr_input, len_va, size_frame, size_pm, op_input);
            break;
        default:
            printf("올바른 옵션을 입력해주세요. \n");
            exit(0);
    }

    printf("%-10s %-10s %-10s %-10s %-10s %-10s\n", "No.", "V.A", "Page No.", "Frame No.", "P.A.", "Page Fault");
    for(int i = 0;i < MAX_INPUT + 1;i++) {
        printf("%s", result[i]);
    }

    switch(op_algo) {
        case 1:
            save_log("output.opt", result, 1);
            break;
        case 2:
            save_log("output.fifo", result, 1);
            break;
        case 3:
            save_log("output.lru", result, 1);
            break;
        case 4:
            save_log("output.sc", result, 1);
            break;
    }

    return 0;
}