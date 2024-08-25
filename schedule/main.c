// 주의: 프로그램 실행 시 root 권한을 이용하여 실행시켜야 합니다. 
//      또한, RR으로 실행하기 위해 아래의 명령어를 이용하여 타임슬라이스를 지정하여야 합니다. 
//      sudo sysctl kernel.sched_rr_timeslice_ms=N
//      * N: 단위가 ms인 정수. 

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sched.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <time.h>
#include <string.h>

// 실행 옵션을 출력하기 위한 함수. 
void printMenu() {
    printf("Input the Scheduling Polity to apply:\n");
    printf("1. CFS_DEFAULT\n");
    printf("2. CFS_NICE\n");
    printf("3. RT_FIFO\n");
    printf("4. RT_RR\n");
    printf("0. Exit\n");
}

// 시간을 "HH:MM:SS.XXXXXX" 포맷으로 출력하는 함수. 
void printTime(struct timeval tv) {
    struct tm tmStruct;
    time_t unixTime = tv.tv_sec;
    gmtime_r(&unixTime, &tmStruct);

    // "HH:MM:SS.XXXXXX" 포맷으로 변경. 
    char timeString[20];
    strftime(timeString, sizeof(timeString), "%H:%M:%S", &tmStruct);

    // ms 단위로 변환, 
    int milliseconds = tv.tv_usec;
    
    snprintf(timeString + 8, 8, ".%06d", milliseconds);

    printf("%s", timeString);
}

// 실행 정보 출력을 위한 함수. 
void printInfo(int option, int pid, int nice, struct timeval time_start, struct timeval time_end) {
    double execution_time = (time_end.tv_sec - time_start.tv_sec) + (time_end.tv_usec - time_start.tv_usec) / 1000000.0;

    if(option <= 2)
        printf("PID: %d | NICE: %d | Start time: ", pid, nice);
    else 
        printf("PID: %d | Start time: ", pid);

    printTime(time_start);
    printf(" | End time: ");
    printTime(time_end);
    printf(" | Elapsed time: %.6lf\n", execution_time);
}

// 실행시 걸린 시간 반환 함수. 
void benchFunction() {
    int which = PRIO_PROCESS;
    int result[100][100];
    int A[100][100];
    int B[100][100];
    int count = 0;
    int i, j, k;
    
    time_t time_start, time_end;
    cpu_set_t set;

    // CPU 코어 설정. 
    CPU_ZERO(&set);
    CPU_SET(1, &set);
    sched_setaffinity(getpid(), sizeof(cpu_set_t), &set);

    while(count < 100){
        for(k = 0; k < 100; k++){
            for(i = 0; i < 100; i++) { 
                for(j = 0; j < 100; j++) {
                    result[k][j] += A[k][i] * B[i][j]; 
                }
            }
        } 
        count++;
    }
}

// CFS Nice 구동을 위한 함수. 
void runCFSNice(int i) {
    int which = PRIO_PROCESS;
    if(i / 7 == 2) setpriority(which, 0, -20);
    else if(i / 7 == 1) setpriority(which, 0, 0);
    else if(i / 7 == 0) setpriority(which, 0, 19);
    else printf("%d\n", i / 7);
}

// FIFO 구동을 위한 함수. 
void runFIFO() {
    int policy = SCHED_FIFO;
    struct sched_param param;

    param.sched_priority = 99;

    sched_setscheduler(0, policy, &param);
}

// RR 구동을 위한 함수. 
void runRR() {
    int policy = SCHED_RR;
    struct sched_param param;

    param.sched_priority = 99;

    sched_setscheduler(0, policy, &param);
}

// 현재 타임 슬라이스를 반환하는 함수. 
int getTimeSlice() {
    FILE* fp = fopen("/proc/sys/kernel/sched_rr_timeslice_ms", "r");
    int timeslice;

    if(fscanf(fp, "%d", &timeslice) != 1) {
        perror("fscanf");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    return timeslice;
}

// Main 함수. 
int main() {
    int     which;
    int     menu;
    int     shmid;
    pid_t   child_pids;
    key_t   key;

    menu            = 1;
    which           = PRIO_PROCESS;
    key             = ftok("result_space", 65);
    shmid           = shmget(key, 21 * sizeof(int), IPC_CREAT | 0666);

    // Shared Memory 생성. 
    int     *shared_memory = shmat(shmid, NULL, 0);

    while(menu) {
        // 메뉴 출력 및 선택 받기. 
        printMenu();
        scanf("%d", &menu);

        // 옵션에 따른 기본 설정. 
        switch(menu) {
            case 0:
                printf("EXIT\n");
                exit(0);
                break;
            case 1:
            case 2:
            case 3:
            case 4:
                break;
            default:
                printf("Wrong option. \n");
        }

        if(menu >= 1 && menu <= 4) {
            // 자식 프로세스 생성 후 실행. 
            for(int i = 0;i < 21;i++) {
                child_pids = fork();
                
                if(child_pids == 0) {
                    struct timeval time_start, time_end;
                    // 시작시간 기록. 
                    gettimeofday(&time_start, NULL);

                    // 옵션에 맞는 자식 프로세스 설정. 
                    switch(menu) {
                        case 1:
                            // 기본 CFS 실행을 위해 속성을 설정하지 않음. 
                            break;
                        case 2:
                            runCFSNice(i);
                            break;
                        case 3:
                            runFIFO();
                            break;
                        case 4:
                            runRR();
                            break;
                    }

                    // 계산 함수 시작. 
                    benchFunction();

                    // 종료시간 기록. 
                    gettimeofday(&time_end, NULL);
                    printInfo(menu, getpid(), getpriority(which, 0), time_start, time_end);
                    double execution_time = (time_end.tv_sec - time_start.tv_sec) + (time_end.tv_usec - time_start.tv_usec) / 1000000.0;

                    // 공유 메모리에 결과 저장. 
                    shared_memory[i] = (int)(execution_time * 1000000);
                    shmdt(shared_memory);
                    exit(0);
                }   
            }

            // 자식 프로세스 종료 대기. 
            for(int i = 0; i < 21; i++) {
                wait(NULL);
            }

            // 자식 프로세스의 결과값 합산. 
            double sum = 0;
            for(int i = 0;i < 21;i++) {
                sum += shared_memory[i] / 1000000.0;
            }

            // 결과 출력. 
            printf("Scheduling Policy: ");
            switch(menu) {
                case 1:
                    printf("CFS_DEFAULT | ");
                    break;
                case 2:
                    printf("CFS_NICE | ");
                    break;
                case 3:
                    printf("RT_FIFO | ");
                    break;
                case 4:
                    printf("RT_RR | Time Quantum: %d ms | ", getTimeSlice());
                    break;
            }
            
            printf("Average elapsed time: %.6lf\n", (sum / 21.0));
        }

        // 스케줄러 복구. 
        int policy = SCHED_OTHER;
        struct sched_param param;
        param.sched_priority = 0;
        sched_setscheduler(0, policy, &param);
    }

    return 0;
}