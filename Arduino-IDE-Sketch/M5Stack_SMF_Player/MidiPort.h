#ifndef DEF_MIDIPORT_H
#define DEF_MIDIPORT_H

#include "common.h"

//MIDIポートアクセス関数定義
// MidiFunc内で使用するインタフェースを実装・リンクするための定義

#define D_MIDI_PORT_BPS  (31250)    //標準MIDI
//#define D_MIDI_PORT_BPS  (38400)  //PCシリアル
//#define DUMPMIDI                  //MIDI送信データキャラクタダンプ

//MIDIインタフェースオープン
int   MidiPort_open();
//MIDIインタフェースクローズ
void  MidiPort_close();
//MIDIインタフェース1バイト送信
int   MidiPort_write( UCHAR data );
//MIDIインタフェース複数バイト送信
int   MidiPort_writeBuffer( UCHAR * pData, ULONG Len );

#endif //DEF_MIDIPORT_H
