#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

#define UTF8_T_S(str) QString::fromUtf8(str)
#define STR_T_UTF8(str) str.toUtf8().data()
#define UNUSED(x) (void)x

#define MAX_RETRY       30   //最大重试次数
#define MAX_SEQUENCE_NUM 91
#define MAX_SEQUENCE_SPEED 20

#define s_linkTemplate "<a href='%1' style='text-decoration: none; color: #0066ec;'>%2</a>"

#endif // CONFIG_H
