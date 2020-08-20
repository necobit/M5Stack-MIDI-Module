/***********************************************************************
<ユーザ設定用ヘッダファイル>

 <chead.h>

                    catsin
        MIDIシーケンサユーザ設定用ヘッダファイル
***********************************************************************/

#ifndef    INC_SMF_SEQ_H
#define    INC_SMF_SEQ_H

#include "common.h"

#ifndef    NULL
#define    NULL                     0
#endif

//    定数定義
#define    SMF_OK                   0
#define    SMF_NG                   -1
#define    SMF_ON                   1
#define    SMF_OFF                  0
#define    SMF_ENDOFSMF             -1
#define    SMF_BUFFSIZEOVER         -2
#define    SMF_DELTACONTBIT         0x80
#define    SMF_DELTAMASK            0x7F
#define    SMF_DELTAMAXBYTE         4

#define    SMF_CHMAX                16
#define    SMF_NOTEOFF_VELOCITY     64

#define    SMF_WAIT_ERROR           -1
#define    SMF_WAIT_ON              1
#define    SMF_WAIT_OFF             0

#define    SMF_STAT_FILENOTREAD     0                    //SMFファイル未読み込み（演奏不能）
#define    SMF_STAT_STOP            1                    //演奏停止
#define    SMF_STAT_PLAY            2                    //演奏中
#define    SMF_STAT_PAUSE           3                    //演奏一時停止中
#define    SMF_STAT_STOPWAIT        4                    //演奏停止待ち（演奏中）

#define    SMF_TRKSTAT_ONTRACK      0
#define    SMF_TRKSTAT_TRACKEND     1

#define    SMF_NOTESTAT_OFF         -1


//#LW##define    SMF_EXBUFLNG             2048                 //汎用処理バッファサイズ
#define    SMF_EXBUFLNG             128                  //汎用処理バッファサイズ
#define    SMF_NOTESTATLNG          128                  //発音記録バッファサイズ


//#LW##define    SMF_TRACKNUM             128                  //最大トラック数
#define    SMF_TRACKNUM             32                   //最大トラック数

#define    SMF_DISPSTATUS_STRLEN    25
#define    SMF_DISPFILE_STRLEN      25
#define    SMF_DISPSEQTRK_STRLEN    35

//テーブル定義
//トラックテーブル
typedef struct {
  unsigned char       TrkNo;
  char                TrackStatus;                       //トラックシーケンス状態
  unsigned int        Ptr;                               //データポインタ
  unsigned int        Size;                              //トラックデータサイズ
  char                SeqWaitFlag;                       //デルタタイム経過待ちフラグ
  char                PreStat;

  long                TickCnt;                           //シーケンスカウンタ

//#LW#  char                SeqTrkName[SMF_DISPSEQTRK_STRLEN+1]; //トラック/シーケンス名

  unsigned long       TrkBufOffset;
}    SMF_TRACK_TABLE;

//シーケンステーブル
typedef struct {
  char                PlayStatus;                        //シーケンサ状態
  char                DispRenewFlag;                     //表示情報更新フラグ

  int                 Tick;                              //仮想割り込み間隔(ms)

//#LW#  char                Status[SMF_DISPSTATUS_STRLEN+1];   //状態文字列
//#LW#  char                FileName[LONG_DFNAME_LEN+20];      //読込みMIDIファイル名
  unsigned int        FileSize;                          //読込みMIDIファイルサイズ
  int                 chNoOffset;                        //CH番号オフセット

  int                 TrackNum;                          //格納トラック数
    
  int                 TPQN;                              //4分音符の分解能
  float               TempoVal;                          //4分音符の長さ(ms)：データ上はμs
  int                 Tempo;                             //1分に入る4分音符数:60000ms/500ms=120
  int                 SeqTickUnit;                       //1Tick毎の時間値

  SMF_TRACK_TABLE     *ptrkTbl[ SMF_TRACKNUM ];

} SMF_SEQ_TABLE;

//#SMF ファイルヘッダフォーマット
typedef struct {
  UCHAR               CyancType[4];
  UCHAR               Length[4];
  UCHAR               Format[2];
  UCHAR               Tracks[2];
  UCHAR               Division[2];
} SMF_HEADER;
//#SMF トラックヘッダフォーマット
typedef struct {
  UCHAR               CyancType[4];
  UCHAR               Length[4];
} TRACK_HEADER;

SMF_SEQ_TABLE  *SmfSeqInit( int Tick );
void    SmfSeqClrarTbl( );
void    SmfSeqInitSeqTbl( SMF_SEQ_TABLE *pseqTbl );
int     SmfSeqSetTempo( SMF_SEQ_TABLE *pseqTbl, int Tick, int TPQN, float TempoVal, int Tempo );
int     SmfSeqInitTrkTbl( SMF_SEQ_TABLE *pseqTbl );
int     SmfSeqPlayResetTrkTbl( SMF_SEQ_TABLE *pseqTbl );
void    SmfSeqNoteClear( SMF_SEQ_TABLE *pseqTbl );
int     SmfSeqAllNoteOff( SMF_SEQ_TABLE *pseqTbl );
void    SmfSeqFileLoadWithChNoOffset( SMF_SEQ_TABLE *pseqTbl, char *FileName, int chNoOffset );
void    SmfSeqFileLoad( SMF_SEQ_TABLE *pseqTbl, char *FileName );
int     SmfSeqStart( SMF_SEQ_TABLE *pseqTbl );
int     SmfSeqPauseSet( SMF_SEQ_TABLE *pseqTbl );
int     SmfSeqPauseRelease( SMF_SEQ_TABLE *pseqTbl );
int     SmfSeqStop( SMF_SEQ_TABLE *pseqTbl );
int     SmfSeqTickProc( SMF_SEQ_TABLE *pseqTbl );
int     SmfSeqEventProc( SMF_SEQ_TABLE *pseqTbl, SMF_TRACK_TABLE *ptrkTbl );
int     SmfSeqEnd( SMF_SEQ_TABLE *pseqTbl );
void    SmfSeqDispRenewReset( SMF_SEQ_TABLE *pseqTbl );
int     SmfSeqGetDispRenew( SMF_SEQ_TABLE *pseqTbl );
int     SmfSeqGetStatus( SMF_SEQ_TABLE *pseqTbl );
int     SmfSeqGMReset();
int     SmfSeqStepBack( SMF_TRACK_TABLE *ptrkTbl );
int     SmfSeqGetByteData( SMF_TRACK_TABLE *ptrkTbl );
int     SmfSeqGetData( SMF_TRACK_TABLE *ptrkTbl, UCHAR *Data, int Length );
int     SmfSeqGetExData( SMF_TRACK_TABLE *ptrkTbl, UCHAR *Data, int MaxLength );
long    SmfSeqGetNum( SMF_TRACK_TABLE *ptrkTbl );
void    SmfSeqStrcpy( char *DistStr, char *SrcStr, int MaxSize );
char    *SmfSeqGetFileName( char *FullPathName );

#endif
