#!/bin/bash

# install.sh 실행 시, 파일 install.sh의 접근 권한을 맞춰주신 후 root 권한으로 실행시켜 주시기 바랍니다. 

# 기존 커널 경로는 다음과 같습니다. 
# /usr/src/linux/linux-5.15.120/
# 
# 변경을 원하시는 경우, 아래의 변수 KERNEL_PATH를 수정해주시고, 
# 마지막에 "/"로 끝내주시기 바랍니다. 
KERNEL_PATH="/usr/src/linux/linux-5.15.120/"

# 사용자 정의 함수 파일 복사. 
cp ./sys_calc.c ${KERNEL_PATH}kernel/
cp ./sys_rev.c ${KERNEL_PATH}kernel/

# Makefile 백업 후 변경. 
mv ${KERNEL_PATH}kernel/Makefile ${KERNEL_PATH}kernel/Makefile.back
cp ./Makefile.def ${KERNEL_PATH}kernel/Makefile

# syscalls.h 백업 후 변경. 
mv ${KERNEL_PATH}include/linux/syscalls.h ${KERNEL_PATH}include/linux/syscalls.h.back
cp ./syscalls.h ${KERNEL_PATH}include/linux/

# syscall_64.tbl 백업 후 변경. 
mv ${KERNEL_PATH}arch/x86/entry/syscalls/syscall_64.tbl ${KERNEL_PATH}arch/x86/entry/syscalls/syscall_64.tbl.back
cp ./syscall_64.tbl ${KERNEL_PATH}arch/x86/entry/syscalls/