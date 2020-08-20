/***********************************************************************
<ユーザ設定関数>

 <csrc.c>

                    catsin
        MIDIシーケンサユーザ設定関数
***********************************************************************/
#include "MidiFunc.h"
#include "MidiPort.h" //MIDIポートアクセス関数定義

int midiOutOpen(){
  int            Status;

  /* オプションポートをオープン */
  /* オプションの値（下記コメントのかっこ内はその設定値）は適宜変更してください */
  /* /B 転送速度       (31250bps) */
  /* /P パリティ       (なし)     */
  /* /L データ長       (8ビット)  */
  /* /S ストップビット (1ビット)  */
  /* /C ハードフロー   (行わない) */
  /* /X XON/XOFFフロー (行わない) */
  /* 他のオプションはデフォルト値を使用 */
  MidiPort_open();
  
  /* エラー処理（アラートボックスの表示）*/
  Status = MIDI_OK;

  return( Status );
}

int midiOutClose(){
  int            Status;

  Status = MIDI_OK;

  /* オプションポートのクローズ */
  MidiPort_close();
    
  return( Status );
}

int midiOutShortMsg( UCHAR status, UCHAR data1, UCHAR data2 ){
  int          Status;
  UCHAR        Buf[3];
  ULONG        Len;

  Status = MIDI_OK;

  Buf[0] = status;
  Buf[1] = data1;
  Buf[2] = data2;

  Len = 3;
  MidiPort_writeBuffer( Buf, Len );

  return( Status );
}

int midiOutLongMsg( UCHAR *Buf, ULONG Len ){
  int            Status;

  Status = MIDI_OK;

  MidiPort_writeBuffer( Buf, Len );

  return( Status );
}

int midiOutGMReset(){
  int          Status;
  UCHAR        GMResetData[] = {0xf0,0x7e,0x7f,0x09,0x01,0xf7};

  //GMSystemON送信
  Status = midiOutLongMsg( (UCHAR *)GMResetData, (ULONG)sizeof(GMResetData) );

  return( Status );
}
