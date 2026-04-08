#include <stdio.h>
#include <stdlib.h>
#include "common.h"

// 에러 메시지 출력 후 프로그램 종료 (공통 함수)
void error_handling(char *message) {
perror(message);
exit(1);
}