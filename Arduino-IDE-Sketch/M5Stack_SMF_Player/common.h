#ifndef DEF_COMMON_H
#define DEF_COMMON_H

//システム依存する定義
#include <M5Stack.h>
#include <Arduino.h>
#define LGFX_M5STACK
#include <LovyanGFX.hpp>
static LGFX lcd;


typedef unsigned char UCHAR;  //1バイト符号なしデータ（SMFファイルアクセスに使用）
typedef unsigned long ULONG;  //4バイト符号なしデータ（SMFファイルアクセスに使用）

typedef int ERRORCODE;

#define SMF_FILENAME    ("playdata.mid")  //SMFファイル名
#define ZTICK           (30)               //[ms]定期更新処理起動間隔

#define LONG_DFNAME_LEN (128)
#define FC_NO_DATA_ERROR (0)

//#define DEBUG
#ifdef  DEBUG
#define DPRINT  Serial.print
#define DPRINTLN  Serial.println
#else //DEBUG
#define DPRINT
#define DPRINTLN
#endif  //DEBUG

#endif  //DEF_COMMON_H
