#ifndef STDARG_H
#define STDARG_H
typedef char* va_list;
#define _VA_ALIGN(type) ((sizeof(type) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define va_start(ap, last_param) \
    (ap = (va_list)&last_param + _VA_ALIGN(last_param))
#define va_arg(ap, type) \
    (*(type*)((ap += _VA_ALIGN(type)) - _VA_ALIGN(type)))
#endif