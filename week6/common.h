#ifndef common_H
#define common_H

#include <stdint.h>

#define PORT 5000

// DrawingMessage 구조체 정의 (리눅스 공통)
struct DrawingMessage {
int32_t x1;
int32_t y1;
int32_t x2;
int32_t y2;
int32_t width;
int32_t color;
};

// 에러 처리 함수 선언
void error_handling(char *message);

#endif