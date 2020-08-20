#ifndef DEF_SMFFILEACCESS_H
#define DEF_SMFFILEACCESS_H

#include "common.h"

//SMFファイルアクセス関数定義
// SmfSeq内で使用するインタフェースを実装・リンクするための定義

//SMFファイルオープン
bool  SmfFileAccessOpen( UCHAR * Filename );
//SMFファイルクローズ
void  SmfFileAccessClose();
//SMFファイル指定位置の1バイト取得
bool  SmfFileAccessRead( UCHAR * Buf, unsigned long Ptr );
//SMFファイル継続位置の1バイト取得
bool  SmfFileAccessReadNext( UCHAR * Buf );
//SMFファイル指定位置の複数バイト取得
int   SmfFileAccessReadBuf( UCHAR * Buf, unsigned long Ptr, int Lng );
//SMFファイルサイズ取得
unsigned int   SmfFileAccessSize();

#endif //DEF_SMFFILEACCESS_H
