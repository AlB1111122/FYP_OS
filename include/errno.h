#pragma once
// allows the use of libm sqrt
int32_t __errno;

extern "C" int32_t *__errno(void);