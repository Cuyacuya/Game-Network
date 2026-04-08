#ifndef VAR_LCOMMON_H
#define VAR_LCOMMON_H

#include <stdint.h>

// DrawingMessage 구조체 정의 (가변 길이 텍스트 통신용)
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