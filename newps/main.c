#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <unistd.h>
#include <string.h>

#define PID_STAT_INFO_STR_LEN   52
#define PROC_PATH               "/proc"
#define PROC_STAT_PATH          "/proc/%s/stat"

// 옵션 명령어 정의.
#define MENU_UID                "uid"
#define MENU_TTY                "tty"
#define MENU_ALL                "all"

// /proc/PID/stat 정보를 담기위한 구조체.
typedef struct pid_info {
    char info[52][128];
} PID_STAT_INFO;

// 입력 옵션을 저장하기 위한 구조체.
typedef struct options {
    int is_checked;
    int is_uid;
    int is_tty;
    int is_all;
} ARG_OPTIONS;

// 문자열에 대해 숫자로만 이루어져 있는지 판단하는 함수.
int isDigit(char *name) {
    for(int i = 0;name[i] != '\0';i++) {
        if(!isdigit(name[i])) return 0;
    }

    return 1;
}

// 문자열에서 PID에 관한 정보 구조체를 가져오는 함수.
PID_STAT_INFO getPIDInfo(char *str_info) {
    // PID의 정보를 담기 위한 구조체 선언.
    PID_STAT_INFO info = {0};

    // 현재 저장하고있는 PID 정보 순번.
    int pos_pid_info = 0;
    // 현재 저장하고있는 PID 정보의 문자열 길이.
    int pos_pid_info_str = 0;

    for(int i = 0; ;i++) {
        // 공백이 경우 다음 PID 정보로 이동.
        if(str_info[i] == ' ') {
            pos_pid_info++;
            pos_pid_info_str = 0;
            continue;
        }
        // 문자열이 끝난 경우 정지.
        if(str_info[i] == '\0') break;

        // 구조체에 문자 저장.
        info.info[pos_pid_info][pos_pid_info_str] = str_info[i];
        pos_pid_info_str++;
    }

    return info;
}

// 클럭 틱을 초로 변환하는 함수.
double tick2sec(long tick, char *time) {
    return (double)atoi(time) / tick;
}

// PID의 TTY를 가져오는 함수.
char* getTTY(char *pid) {
    char tty_path[128];
    snprintf(tty_path, sizeof(tty_path), "/proc/%s/fd/0", pid);

    static char tty_name[128];
    ssize_t tty_name_length = readlink(tty_path, tty_name, sizeof(tty_name) - 1);

    // 가져온 TTY의 정보가 없는 경우.
    if (tty_name_length == -1) {
        static char tty_err[] = "NONE";
        return tty_err;
    }
    tty_name[tty_name_length] = '\0';

    return tty_name;
}

// PID의 UID를 가져오는 함수.
char* getPIDUID(char *pid) {
    static char uid[8] = "NONE";
    char status_path[64];
    snprintf(status_path, sizeof(status_path), "/proc/%s/status", pid);

    // /proc/PID/status 파일 열기.
    FILE *status_file = fopen(status_path, "r");
    if (status_file == NULL) {
        return uid;
    }

    char line[64];

    // 파일에서 UID 정보 찾기.
    while (fgets(line, sizeof(line), status_file) != NULL) {
        if (strncmp(line, "Uid:", 4) == 0) {
            sscanf(line, "Uid:\t%s", uid);
            break;
        }
    }

    fclose(status_file);

    return uid;
}

// TIME 출력
void printCPUTime(double time) {
    printf("%02d:", (int)(time / (60 * 60)));
    time = time - ((int)(time / (60 * 60)) * (60 * 60));
    printf("%02d:", (int)(time / 60));
    time = time - ((int)(time / 60) * 60);
    printf("%02d ", (int)(time));
}

// 옵션 값을 가져와 구조체로 변환하는 함수.
ARG_OPTIONS menuSelector(int argc, const char **select) {
    ARG_OPTIONS options = {0};

    for(int i = 1;i < argc;i++) {
        if(!strncmp(select[i], MENU_UID, 3))
            {options.is_checked = 1; options.is_uid = 1; continue;}
        if(!strncmp(select[i], MENU_TTY, 3))
            {options.is_checked = 1; options.is_tty = 1; continue;}
        if(!strncmp(select[i], MENU_ALL, 3))
            {options.is_checked = 1; options.is_all = 1; continue;}
    }

    return options;
}

int main(int argc, char const *argv[]) {
    // 현재 실행시킨 UID 가져오기.
    const uid_t CURRENT_UID     = getuid();
    // 현재 시스템의 초당 클럭 틱 가져오기.
    const long  CLK_TCK         = sysconf(_SC_CLK_TCK);
    // 현재 TTY 가져오기.
    char        *CURRENT_TTY    = ttyname(STDIN_FILENO);

    // /proc 폴더용 DIR 구조체.
    DIR *dir_proc;
    // /proc/PID/stat를 읽기 위한 FILE 구조체.
    FILE *file_stat;
    // /proc/PID/stat의 정보를 담기위한 PID_STAT_INFO 구조체.
    PID_STAT_INFO pid_stat_info;
    // 입력된 옵션을 가져오기 및 저장후 구조체 선언.
    ARG_OPTIONS options = menuSelector(argc, argv);

    // /proc 내의 파일 정보를 가져오기 위한 dirent 구조체.
    struct dirent *dir;

    // /proc/PID/stat의 경로를 가져오기 위한 문자열.
    char path_stat[1024] = "";
    // /proc/PID/stat 내용을 저장하기 위한 문자열.
    char stat_info[1024] = "";
    // TTY가 동일한 부모의 PID를 저장하기 위한 문자열 배열.
    char list_tty_parent_pid[512][8];
    // 부모 TTY 경로를 저장하기 위한 문자열 배열.
    char tty_parent_name[512]       = "";
    int cnt_tty_parent_pid          = 0;

    // proc 경로 설정.
    dir_proc = opendir(PROC_PATH);

    // HEAD 출력.
    printf("    PID        TTY     TIME  CMD\n");

    // /proc 폴더가 열린 경우 작업 진행.
    if(dir_proc) {
        while ((dir = readdir(dir_proc)) != NULL) {

            if(isDigit(dir->d_name)) {
                // /proc/PID/stat 경로에서 stat 파일 읽어오기.
                sprintf(path_stat, PROC_STAT_PATH, dir->d_name);
                file_stat = fopen(path_stat, "r");

                // 파일이 읽히지 않은 경우 넘어가기.
                if (!file_stat) {
                    fclose(file_stat);
                    continue;
                }

                // 파일 내용 불러오기.
                fgets(stat_info, 1024, file_stat);
                fclose(file_stat);

                // 파일 내용을 구조체로 변환.
                pid_stat_info = getPIDInfo(stat_info);

                // 옵션이 all이 아닌 경우 다른 요소 고려.
                if(!options.is_all) {
                    // UID가 다른경우 넘어가기.
                    if((atoi(getPIDUID(pid_stat_info.info[0])) != CURRENT_UID)
                        && options.is_uid) continue;

                    // TTY가 다른경우 넘어가기.
                    if(strncmp(getTTY(pid_stat_info.info[0]), CURRENT_TTY, 128)
                        && options.is_tty) continue;

                    // 현재 TTY와 같은지 판별.
                    if(!strncmp(getTTY(pid_stat_info.info[0]), CURRENT_TTY, 128) &&
                    !options.is_checked) {
                        // 같으면 부모로 추가.
                        strcpy(list_tty_parent_pid[cnt_tty_parent_pid++], pid_stat_info.info[0]);

                        // 가장 부모 TTY 저장.
                        if(cnt_tty_parent_pid == 1)
                            strcpy(tty_parent_name, getTTY(pid_stat_info.info[0]));
                    }
                    else if(!options.is_checked) {
                        int is_child = 0;

                        for(int i = 0;i < cnt_tty_parent_pid;i++) {
                            // 부모가 있는경우 자식을 부모 리스트에 추가.
                            if(!strncmp(list_tty_parent_pid[i], pid_stat_info.info[5], 128)) {
                                is_child = 1;
                                strcpy(list_tty_parent_pid[++cnt_tty_parent_pid], pid_stat_info.info[0]);
                                break;
                            }
                        }

                        // 자식이 아닌경우 넘어가기.
                        if(!is_child) continue;
                    }
                }

                // PID 출력.
                printf("%7s ", pid_stat_info.info[0]);

                // TTY 출력.
                if(!options.is_checked)
                    printf("%10s ", tty_parent_name);
                else
                    printf("%10s ", getTTY(pid_stat_info.info[0]));

                // utime과 stime을 합산한 TIME 결과 출력.
                double utime = tick2sec(CLK_TCK, pid_stat_info.info[13]);
                double stime = tick2sec(CLK_TCK, pid_stat_info.info[14]);
                printCPUTime(utime + stime);

                // 실행 명령어 출력.
                pid_stat_info.info[1][0] = ' ';
                pid_stat_info.info[1][strlen(pid_stat_info.info[1]) - 1] = '\0';
                printf("%s\n", pid_stat_info.info[1]);
            }
        }
        closedir(dir_proc);
    }

    return 0;
}
