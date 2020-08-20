/***********************************************************************
<ユーザ設定用ヘッダファイル>

 <chead.h>

                    catsin
        MIDIシーケンサユーザ設定用ヘッダファイル
***********************************************************************/

#ifndef    DEF_MIDI_FUNC_H
#define    DEF_MIDI_FUNC_H

#include "common.h"

//    定数定義
#define    MIDI_OK        0
#define    MIDI_NG        -1
#define    MIDI_ON        1
#define    MIDI_OFF       0

#define    MIDI_STAT_METAEVENT        0xff
#define    MIDI_META_SEQNO            0x00
#define    MIDI_META_TEXT             0x01
#define    MIDI_META_COPYRIGHT        0x02
#define    MIDI_META_SEQTRKNAME       0x03
#define    MIDI_META_INSTNAME         0x04
#define    MIDI_META_LYLIC            0x05
#define    MIDI_META_CHPREFIX         0x20
#define    MIDI_META_ENDOFTRACK       0x2f
#define    MIDI_META_SETTEMPO         0x51
#define    MIDI_META_SMPTEOFFSET      0x54
#define    MIDI_META_TEMPOSIG         0x58
#define    MIDI_META_KEYSIG           0x59
#define    MIDI_META_SEQSPECIFIC      0x7f


#define    MIDI_STATBIT_MASK          0x80
#define    MIDI_STAT_MASK             0xf0
#define    MIDI_CHANNEL_MASK          0x0f

#define    MIDI_STATCH_NOTEOFF        0x80
#define    MIDI_STATCH_NOTEON         0x90
#define    MIDI_STATCH_PKEYPRES       0xa0
#define    MIDI_STATCH_CTRLCHG        0xb0
#define    MIDI_STATCH_PROGCHG        0xc0
#define    MIDI_STATCH_CNPRES         0xd0
#define    MIDI_STATCH_PBNDCHG        0xe0

#define    MIDI_STATEX_SYSEXCL        0xf0
#define    MIDI_STATEX_MIDITCQF       0xf1
#define    MIDI_STATEX_SONGPOGP       0xf2
#define    MIDI_STATEX_SONGSEL        0xf3
#define    MIDI_STATEX_CHUNEREQ       0xf6
#define    MIDI_STATEX_EOEXCL         0xf7

#define    MIDI_STATRT_TMGCLOCK       0xf8
#define    MIDI_STATRT_START          0xfa
#define    MIDI_STATRT_CONTINUE       0xfb
#define    MIDI_STATRT_STOP           0xfc
#define    MIDI_STATRT_ACTSENC        0xfe
#define    MIDI_STATRT_SYSRESET       0xff

#define    MIDI_CNMD_ALLSNDOFF        0x78
#define    MIDI_CNMD_RSTALLCTL        0x79
#define    MIDI_CNMD_LOCALCTL         0x7a
#define    MIDI_CNMD_ALLNOTOFF        0x7b
#define    MIDI_CNMD_OMNIOFF          0x7c
#define    MIDI_CNMD_OMNION           0x7d
#define    MIDI_CNMD_MONOMODON        0x7e
#define    MIDI_CNMD_POLIMODON        0x7f

#define    MIDI_CH_EVF1_VOCAL         0
#define    MIDI_CH_GM_DRUM            9

//    関数定義
int    midiOutOpen();
int    midiOutClose();
int    midiOutShortMsg( UCHAR status, UCHAR data1, UCHAR data2 );
int    midiOutLongMsg( UCHAR *Buf, ULONG Len );
int    midiOutGMReset();

#endif
