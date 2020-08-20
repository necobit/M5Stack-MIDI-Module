/***********************************************************************
<ユーザ設定関数>

 <csrc.c>

          catsin
    MIDIシーケンサユーザ設定関数
***********************************************************************/
#include "SmfFileAccess.h" //SMFファイルアクセス関数定義
#include "MidiFunc.h"      //MIDIインタフェース
#include "SmfSeq.h"

//-----------------------------------------------------------------------------
//SMFシーケンス用テーブル
//-----------------------------------------------------------------------------
SMF_SEQ_TABLE seqTbl;                 //SMFシーケンステーブル
SMF_TRACK_TABLE trkTbl[SMF_TRACKNUM]; //SMFトラックテーブル

//-----------------------------------------------------------------------------
//  初期化        全テーブル初期化、状態をファイル未読み込みに移行
//    割り込み情報
//-----------------------------------------------------------------------------
SMF_SEQ_TABLE *SmfSeqInit(int Tick)
{
  SMF_SEQ_TABLE *pseqTbl;
  int Ret;

  //ファイルに関係なくMIDIオープン
  Ret = midiOutOpen();

  if (Ret != MIDI_NG)
  {
    pseqTbl = &seqTbl;
    //リセット
    //とりあえずバイパスしておく
    // Ret = SmfSeqGMReset();
    if (Ret == SMF_OK)
    {
      SmfSeqClrarTbl();
      SmfSeqInitSeqTbl(pseqTbl);
      //テンポ情報強制初期化
      Ret = SmfSeqSetTempo(pseqTbl, Tick, 480, 500000, 120);

      SmfSeqNoteClear(pseqTbl);
    }
    else
    {
      pseqTbl = (SMF_SEQ_TABLE *)NULL;
    }
  }
  else
  {
    pseqTbl = (SMF_SEQ_TABLE *)NULL;
  }

  return (pseqTbl);
}

void SmfSeqClrarTbl()
{
  memset(&seqTbl, 0, sizeof(SMF_SEQ_TABLE));
  memset(&trkTbl, 0, sizeof(SMF_TRACK_TABLE) * SMF_TRACKNUM);
}

void SmfSeqInitSeqTbl(SMF_SEQ_TABLE *pseqTbl)
{
  int Cnt;

  if (pseqTbl == NULL)
  {
    return;
  }

  pseqTbl->PlayStatus = SMF_STAT_FILENOTREAD; //シーケンサ状態
  pseqTbl->DispRenewFlag = SMF_OFF;           //表示情報更新フラグ

  //読込みMIDIファイル名
  //#LW#  memset( pseqTbl->FileName, 0, sizeof( pseqTbl->FileName ) );
  pseqTbl->FileSize = 0; //読込みMIDIファイルサイズ
  pseqTbl->chNoOffset = 0;
  pseqTbl->TrackNum = 0; //格納トラック数

  for (Cnt = 0; Cnt < SMF_TRACKNUM; Cnt++)
  {
    pseqTbl->ptrkTbl[Cnt] = &trkTbl[Cnt];
  }
}

//  テンポ設定
int SmfSeqSetTempo(SMF_SEQ_TABLE *pseqTbl, int Tick, int TPQN, float TempoVal, int Tempo)
{
  int Ret;

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  pseqTbl->Tick = Tick;         //仮想割り込み間隔(10ms):10ms
  pseqTbl->TPQN = TPQN;         //4分音符の分解能
  pseqTbl->TempoVal = TempoVal; //4分音符の長さ(μs)
  pseqTbl->Tempo = Tempo;       //1分に入る4分音符数:60000ms/500ms=120
  pseqTbl->SeqTickUnit = (int)((float)(((float)Tick * 1000) / ((float)TempoVal / (float)TPQN)) * 1000);

  if (pseqTbl->SeqTickUnit == 0)
  { //SeqTickUnitが0なのは致命的にエラーです
    Ret = SMF_NG;
  }
  else
  {
    Ret = SMF_OK;
  }
  return (Ret);
}

//-----------------------------------------------------------------------------
//  一部初期化      トラックテーブル初期化、状態を停止に移行（現状態維持）
//-----------------------------------------------------------------------------
int SmfSeqInitTrkTbl(SMF_SEQ_TABLE *pseqTbl)
{
  int Ret;
  int Cnt;
  UCHAR *pSmfData;
  SMF_HEADER *pSmfHeader;
  TRACK_HEADER *pTrkHeader;
  SMF_TRACK_TABLE *ptrkTbl;
  int TPQN;

  unsigned long fileAccessPtr = 0;
  int fileAccessReadSize = 0;

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  Ret = SMF_OK;

  for (Cnt = 0; Cnt < SMF_TRACKNUM; Cnt++)
  {
    ptrkTbl = pseqTbl->ptrkTbl[Cnt];

    //#LW#    SmfSeqStrcpy( ptrkTbl->SeqTrkName, "", SMF_DISPSEQTRK_STRLEN );
    pseqTbl->DispRenewFlag = SMF_ON; //表示情報更新あり

    ptrkTbl->TrkNo = Cnt;
    ptrkTbl->TrkBufOffset = 0;
    ptrkTbl->Size = 0; //トラックデータサイズ
  }

  if (pseqTbl->FileSize < sizeof(SMF_HEADER))
  {
    Ret = SMF_NG;
  }
  else
  {
    SMF_HEADER smfHeader;
    fileAccessReadSize = SmfFileAccessReadBuf((UCHAR *)&smfHeader, fileAccessPtr, sizeof(SMF_HEADER));
    pSmfHeader = &smfHeader;
    fileAccessPtr = fileAccessPtr + fileAccessReadSize;
    DPRINT(F("fileAccessPtr:"));
    DPRINTLN(fileAccessPtr);

    //収容トラック数取得
    pseqTbl->TrackNum = pSmfHeader->Tracks[0];
    pseqTbl->TrackNum = (pseqTbl->TrackNum << 8) | pSmfHeader->Tracks[1];

    //TPQN取得
    TPQN = pSmfHeader->Division[0];
    TPQN = (TPQN << 8) | pSmfHeader->Division[1];

    Ret = SmfSeqSetTempo(pseqTbl, pseqTbl->Tick, TPQN, pseqTbl->TempoVal, pseqTbl->Tempo);

    if (Ret == SMF_OK)
    {
      TRACK_HEADER trkHeader;
      for (Cnt = 0; Cnt < pseqTbl->TrackNum; Cnt++)
      {
        fileAccessReadSize = SmfFileAccessReadBuf((UCHAR *)&trkHeader, fileAccessPtr, sizeof(TRACK_HEADER));
        if (fileAccessReadSize < sizeof(TRACK_HEADER))
        {
          Ret = SMF_NG;
          break;
        }
        pTrkHeader = &trkHeader;
        fileAccessPtr = fileAccessPtr + fileAccessReadSize;
        ptrkTbl = pseqTbl->ptrkTbl[Cnt];

        ptrkTbl->Size = pTrkHeader->Length[0];
        ptrkTbl->Size = (ptrkTbl->Size << 8) | pTrkHeader->Length[1];
        ptrkTbl->Size = (ptrkTbl->Size << 8) | pTrkHeader->Length[2];
        ptrkTbl->Size = (ptrkTbl->Size << 8) | pTrkHeader->Length[3];
        DPRINT(F("ptrkTbl->Size:"));
        DPRINTLN(ptrkTbl->Size);

        ptrkTbl->TrkBufOffset = fileAccessPtr;
        DPRINT(F("ptrkTbl->TrkBufOffset:"));
        DPRINTLN(ptrkTbl->TrkBufOffset);

        fileAccessPtr = fileAccessPtr + ptrkTbl->Size;
      }
    }
  }
  if (Ret == SMF_OK)
  {
    Ret = SmfSeqPlayResetTrkTbl(pseqTbl);
  }
  return (Ret);
}

int SmfSeqPlayResetTrkTbl(SMF_SEQ_TABLE *pseqTbl)
{
  int Ret;
  int Cnt;
  SMF_TRACK_TABLE *ptrkTbl;

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  DPRINTLN(F("SmfSeqPlayResetTrkTbl"));
  for (Cnt = 0; Cnt < pseqTbl->TrackNum; Cnt++)
  {
    ptrkTbl = pseqTbl->ptrkTbl[Cnt];
    ptrkTbl->TrackStatus = SMF_TRKSTAT_ONTRACK;
    ptrkTbl->Ptr = 0;                    //データポインタ
    ptrkTbl->SeqWaitFlag = SMF_WAIT_OFF; //デルタタイム経過待ちフラグ
    ptrkTbl->PreStat = 0;
    ptrkTbl->TickCnt = 0; //シーケンスカウンタ
  }
  //テンポ情報強制初期化
  Ret = SmfSeqSetTempo(pseqTbl, pseqTbl->Tick, pseqTbl->TPQN, 500000, 120);

  return (Ret);
}

void SmfSeqNoteClear(SMF_SEQ_TABLE *pseqTbl)
{
  int Cnt;

  if (pseqTbl == NULL)
  {
    return;
  }
}

//  ノートオンをすべてノートオフ
int SmfSeqAllNoteOff(SMF_SEQ_TABLE *pseqTbl)
{
  int cnt;
  int ch;
  int Ret;

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  Ret = SMF_OK;
  for (cnt = 0; cnt < SMF_NOTESTATLNG; cnt++)
  {
    //0縲鰀16bitの状態を順にチェック
    for (ch = 0; ch < SMF_CHMAX; ch++)
    {
      Ret = midiOutShortMsg(
          (UCHAR)(MIDI_STATCH_NOTEOFF | ch),
          (UCHAR)cnt,
          (UCHAR)SMF_NOTEOFF_VELOCITY);
      if (Ret == MIDI_NG)
      {
        Ret = SMF_NG;
        break;
      }
    }
  }
  return (Ret);
}

//-----------------------------------------------------------------------------
//  ファイル読み込み  ファイル読み込み、全テーブル設定、トラックテーブル初期化、状態を停止状態に移行
//    ファイル名
//-----------------------------------------------------------------------------
void SmfSeqFileLoadWithChNoOffset(SMF_SEQ_TABLE *pseqTbl, char *FileName, int chNoOffset)
{
  if (pseqTbl == NULL)
  {
    return;
  }

  SmfSeqFileLoad(pseqTbl, FileName);
  pseqTbl->chNoOffset = chNoOffset;
}
void SmfSeqFileLoad(SMF_SEQ_TABLE *pseqTbl, char *FileName)
{
  //
  int Ret;
  int cnt;
  ULONG DataSize;

  if (pseqTbl == NULL)
  {
    return;
  }

  //ファイル情報クリア
  //#LW#  SmfSeqStrcpy( pseqTbl->FileName, "", SMF_DISPFILE_STRLEN );
  pseqTbl->DispRenewFlag = SMF_ON; //表示情報更新あり
  pseqTbl->FileSize = 0;

  SmfSeqInitSeqTbl(pseqTbl);       //共通テーブル初期化
                                   //#LW#  SmfSeqStrcpy( pseqTbl->Status, "共通テーブル初期化", SMF_DISPSTATUS_STRLEN );
  pseqTbl->DispRenewFlag = SMF_ON; //表示情報更新あり

  if (strlen((char *)FileName) > 0)
  {
    //#LW#    SmfSeqStrcpy( pseqTbl->FileName, SmfSeqGetFileName( FileName ), SMF_DISPFILE_STRLEN );
    pseqTbl->DispRenewFlag = SMF_ON; //表示情報更新あり

    /* コピー元ファイルをオープン */
    //ファイルオープンは必ずバイナリだとさ
    bool openResult = SmfFileAccessOpen((UCHAR *)FileName);

    /* エラー処理（アラートボックスの表示）*/
    if (openResult != true)
    {
      DPRINTLN(F("openResult != true"));
      //#LW#      SmfSeqStrcpy( pseqTbl->Status, "ファイルオープンエラー", SMF_DISPSTATUS_STRLEN );
      pseqTbl->DispRenewFlag = SMF_ON; //表示情報更新あり
    }
    else
    {
      //いけてるね
      pseqTbl->FileSize = SmfFileAccessSize(); //SMFファイルデータ長設定
      DPRINT(F("pseqTbl->FileSize:"));
      DPRINTLN(pseqTbl->FileSize);
      //トラック毎テーブル初期化
      Ret = SmfSeqInitTrkTbl(pseqTbl);
      if (Ret == SMF_NG)
      {
        DPRINTLN(F("Ret == SMF_NG"));
        //#LW#            SmfSeqStrcpy( pseqTbl->Status, "ファイル内容エラー", SMF_DISPSTATUS_STRLEN );
        //表示情報更新あり
        pseqTbl->DispRenewFlag = SMF_ON;
      }
      else
      {
        //#LW#            SmfSeqStrcpy( pseqTbl->Status, "初期化完了", SMF_DISPSTATUS_STRLEN );
        //表示情報更新あり
        pseqTbl->DispRenewFlag = SMF_ON;
        //停止状態に初期化
        pseqTbl->PlayStatus = SMF_STAT_STOP;
      }
    }
  }
}

//-----------------------------------------------------------------------------
//  再生開始      状態を再生中に移行
//-----------------------------------------------------------------------------
int SmfSeqStart(SMF_SEQ_TABLE *pseqTbl)
{
  int Ret;

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  switch (pseqTbl->PlayStatus)
  {
  case SMF_STAT_STOP:
    Ret = SMF_OK;

    //#LW#    SmfSeqStrcpy( pseqTbl->Status, "演奏中", SMF_DISPSTATUS_STRLEN );
    pseqTbl->DispRenewFlag = SMF_ON;

    pseqTbl->PlayStatus = SMF_STAT_PLAY;
    break;
  default:
    Ret = SMF_NG;
    break;
  }

  return (Ret);
}
//-----------------------------------------------------------------------------
//  一時停止    状態を一時停止中
//-----------------------------------------------------------------------------
int SmfSeqPauseSet(SMF_SEQ_TABLE *pseqTbl)
{
  int Ret;

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  switch (pseqTbl->PlayStatus)
  {
  case SMF_STAT_PLAY:
    Ret = SMF_OK;

    //#LW#    SmfSeqStrcpy( pseqTbl->Status, "一時停止中", SMF_DISPSTATUS_STRLEN );
    pseqTbl->DispRenewFlag = SMF_ON;

    pseqTbl->PlayStatus = SMF_STAT_PAUSE;
    break;
  default:
    Ret = SMF_NG;
    break;
  }

  return (Ret);
}
//-----------------------------------------------------------------------------
//  一時停止解除    状態を再生中に移行
//-----------------------------------------------------------------------------
int SmfSeqPauseRelease(SMF_SEQ_TABLE *pseqTbl)
{
  int Ret;

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  switch (pseqTbl->PlayStatus)
  {
  case SMF_STAT_PAUSE:
    Ret = SMF_OK;

    //#LW#    SmfSeqStrcpy( pseqTbl->Status, "演奏中", SMF_DISPSTATUS_STRLEN );
    pseqTbl->DispRenewFlag = SMF_ON;

    pseqTbl->PlayStatus = SMF_STAT_PLAY;
    break;
  default:
    Ret = SMF_NG;
    break;
  }

  return (Ret);
}
//-----------------------------------------------------------------------------
//  再生停止      状態を停止待ちに移行
//-----------------------------------------------------------------------------
int SmfSeqStop(SMF_SEQ_TABLE *pseqTbl)
{
  int Ret;

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  DPRINTLN(F("SmfSeqStop"));
  switch (pseqTbl->PlayStatus)
  {
  case SMF_STAT_PLAY:
  case SMF_STAT_PAUSE:
    Ret = SMF_OK;

    //#LW#    SmfSeqStrcpy( pseqTbl->Status, "演奏停止", SMF_DISPSTATUS_STRLEN );
    pseqTbl->DispRenewFlag = SMF_ON;

    SmfSeqAllNoteOff(pseqTbl); //ノートオフ漏れ停止

    pseqTbl->PlayStatus = SMF_STAT_STOP;
    break;
  default:
    Ret = SMF_NG;
    break;
  }

  return (Ret);
}
//-----------------------------------------------------------------------------
//  割り込み毎起動    割り込み毎処理
//-----------------------------------------------------------------------------
int SmfSeqTickProc(SMF_SEQ_TABLE *pseqTbl)
{
  int PlayStat;
  int Cnt;
  int Ret;

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  switch (pseqTbl->PlayStatus)
  {
  case SMF_STAT_PLAY:
  case SMF_STAT_STOPWAIT:
    Ret = SMF_OK;

    PlayStat = SMF_STAT_STOPWAIT;
    for (Cnt = 0; Cnt < pseqTbl->TrackNum; Cnt++)
    {
      Ret = SmfSeqEventProc(pseqTbl, pseqTbl->ptrkTbl[Cnt]);
      //異常発生しているか？
      if ((Ret == SMF_ENDOFSMF) || (Ret == SMF_NG))
      {
        PlayStat = SMF_STAT_STOPWAIT;
        break;
      }
      //1トラックでも演奏中なら状態継続
      if (pseqTbl->ptrkTbl[Cnt]->TrackStatus == SMF_TRKSTAT_ONTRACK)
      {
        PlayStat = SMF_STAT_PLAY;
      }
    }
    //全トラックシーケンス終了か？
    if ((pseqTbl->PlayStatus == SMF_STAT_STOPWAIT) ||
        (PlayStat == SMF_STAT_STOPWAIT))
    {
      //#LW#      SmfSeqStrcpy( pseqTbl->Status, "演奏終了", SMF_DISPSTATUS_STRLEN );
      pseqTbl->DispRenewFlag = SMF_ON;

      SmfSeqAllNoteOff(pseqTbl); //ノートオフ漏れ停止

      pseqTbl->PlayStatus = SMF_STAT_STOP;
    }
    break;
  default:
    Ret = SMF_NG;
    break;
  }

  return (Ret);
}

int SmfSeqEventProc(SMF_SEQ_TABLE *pseqTbl, SMF_TRACK_TABLE *ptrkTbl)
{
  UCHAR ExBuff[SMF_EXBUFLNG];
  UCHAR MidiData[2];
  bool result;
  long MidiDeltaTime;
  int MidiStatus;
  int MidiData1;
  int MidiData2;
  int Ret;
  int Cnt;
  long TempTempo;
  int dx;
  int dy;

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  Ret = SMF_OK;
  //トラックデータをシーケンス
  while (ptrkTbl->TrackStatus == SMF_TRKSTAT_ONTRACK)
  {
    //デルタ時間経過待ち？
    if (ptrkTbl->SeqWaitFlag == SMF_WAIT_OFF)
    {
      //待ってない
      MidiDeltaTime = SmfSeqGetNum(ptrkTbl);
      if (MidiDeltaTime == SMF_ENDOFSMF)
      {
        ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
        Ret = MidiDeltaTime;
        break;
      }
      ptrkTbl->TickCnt = ptrkTbl->TickCnt + MidiDeltaTime * 1000;
    }
    else
    {
      //待ってる
      MidiDeltaTime = 0;
      ptrkTbl->TickCnt = ptrkTbl->TickCnt - pseqTbl->SeqTickUnit;
    }
    if (ptrkTbl->TickCnt <= 0)
    {
      MidiStatus = SmfSeqGetByteData(ptrkTbl);
      if (MidiStatus == SMF_ENDOFSMF)
      {
        ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
        Ret = MidiStatus;
        break;
      }

      if (MidiStatus == MIDI_STAT_METAEVENT)
      {
        MidiData1 = SmfSeqGetByteData(ptrkTbl);
        if (MidiData1 == SMF_ENDOFSMF)
        {
          ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
          Ret = MidiData1;
          break;
        }
        MidiData2 = SmfSeqGetNum(ptrkTbl);
        if (MidiData2 == SMF_ENDOFSMF)
        {
          ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
          Ret = MidiData2;
          break;
        }
        Ret = SmfSeqGetData(ptrkTbl, ExBuff, MidiData2);
        if (Ret == SMF_ENDOFSMF)
        {
          ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
          Ret = MidiData2;
          break;
        }
        switch (MidiData1)
        {
        case MIDI_META_ENDOFTRACK:
          //トラックシーケンス状態をトラックエンドに
          ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
          break;
        case MIDI_META_SETTEMPO:
          TempTempo = 0;
          if (0 < MidiData2)
          {
            TempTempo = ExBuff[0];
            for (Cnt = 1; Cnt < MidiData2; Cnt++)
            {
              TempTempo = TempTempo << 8 | ExBuff[Cnt];
            }
            Ret = SmfSeqSetTempo(pseqTbl, pseqTbl->Tick, pseqTbl->TPQN, TempTempo, pseqTbl->Tempo);
            if (Ret == SMF_NG)
            {
              ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
              Ret = SMF_NG;
              break;
            }
          }
          break;
        case MIDI_META_SEQTRKNAME:
          ExBuff[MidiData2] = 0x00;        //念の為デリミタ
                                           //#LW#          SmfSeqStrcpy( ptrkTbl->SeqTrkName, (char *)ExBuff, SMF_DISPSEQTRK_STRLEN );
          pseqTbl->DispRenewFlag = SMF_ON; //表示情報更新あり
          break;
        case MIDI_META_SEQNO:
        case MIDI_META_TEXT:
        case MIDI_META_COPYRIGHT:
        case MIDI_META_INSTNAME:
        case MIDI_META_LYLIC:
        case MIDI_META_CHPREFIX:
        case MIDI_META_SMPTEOFFSET:
        case MIDI_META_TEMPOSIG:
        case MIDI_META_KEYSIG:
        case MIDI_META_SEQSPECIFIC:
        default:
          break;
        }
      }
      else
      {
        //ステータスビットが立ってない場合は一つ前のイベントと同じ
        if ((MidiStatus & MIDI_STATBIT_MASK) != MIDI_STATBIT_MASK)
        {
          MidiStatus = ptrkTbl->PreStat; //無理矢理前と同じステータスにする
          Ret = SmfSeqStepBack(ptrkTbl);
        }
        else
        {
          ptrkTbl->PreStat = MidiStatus;
        }
        //eVF1ではch1がボーカル固定のためGM音源として使用する場合chをずらす
        if (pseqTbl->chNoOffset > 0)
        {
          int ch = MidiStatus & MIDI_CHANNEL_MASK;
          //ただしGM音源のCH10はドラム固定のためそのまま使用する
          if (ch != MIDI_CH_GM_DRUM)
          {
            ch = ch + pseqTbl->chNoOffset;
            if (ch == MIDI_CH_GM_DRUM)
            {
              ch++;
            }
            MidiStatus = (MidiStatus & MIDI_STAT_MASK) | ch;
          }
        }
        switch (MidiStatus & MIDI_STAT_MASK)
        {
        case MIDI_STATCH_NOTEOFF:
          result = SmfSeqGetData(ptrkTbl, MidiData, 2);
          if (result == false)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = SMF_ENDOFSMF;
            break;
          }
          MidiData1 = (int)MidiData[0];
          MidiData2 = (int)MidiData[1];

          Ret = midiOutShortMsg((UCHAR)MidiStatus,
                                (UCHAR)MidiData1,
                                (UCHAR)MidiData2);
          if (Ret == MIDI_NG)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = SMF_NG;
            break;
          }
          dx = 30 + MidiData1 * 2;
          dy = 60 + ((MidiStatus - 128) * 10);
          lcd.drawFastVLine(dx, dy, 5, TFT_BLACK);
          Ret = SMF_OK;
          break;
        case MIDI_STATCH_NOTEON:
          result = SmfSeqGetData(ptrkTbl, MidiData, 2);
          if (result == false)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = SMF_ENDOFSMF;
            break;
          }
          MidiData1 = (int)MidiData[0];
          MidiData2 = (int)MidiData[1];
          //ノートオンによるノートオフ対策
          if (MidiData2 == 0)
          {
            //ベロシティ0のノートオンはノートオフに書換える
            MidiStatus = MIDI_STATCH_NOTEOFF | (MidiStatus & MIDI_CHANNEL_MASK);
          }
          Ret = midiOutShortMsg((UCHAR)MidiStatus,
                                (UCHAR)MidiData1,
                                (UCHAR)MidiData2);
          if (Ret == MIDI_NG)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = SMF_NG;
            break;
          }
          dx = 30 + MidiData1 * 2;
          dy = 60 + ((MidiStatus - 144) * 10);
          lcd.drawFastVLine(dx, dy, 5, TFT_WHITE);

          Ret = SMF_OK;
          break;
        case MIDI_STATCH_PKEYPRES:
        case MIDI_STATCH_CTRLCHG:
        case MIDI_STATCH_PBNDCHG:
          result = SmfSeqGetData(ptrkTbl, MidiData, 2);
          if (result == false)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = SMF_ENDOFSMF;
            break;
          }
          MidiData1 = (int)MidiData[0];
          MidiData2 = (int)MidiData[1];
          Ret = midiOutShortMsg((UCHAR)MidiStatus,
                                (UCHAR)MidiData1,
                                (UCHAR)MidiData2);
          if (Ret == MIDI_NG)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = SMF_NG;
            break;
          }
          Ret = SMF_OK;
          break;
        case MIDI_STATCH_PROGCHG:
        case MIDI_STATCH_CNPRES:
          MidiData1 = SmfSeqGetNum(ptrkTbl);
          if (MidiData1 == SMF_ENDOFSMF)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = MidiData1;
            break;
          }
          ExBuff[0] = MidiStatus;
          ExBuff[1] = MidiData1;

          Ret = midiOutLongMsg(ExBuff, 2);
          if (Ret == MIDI_NG)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = SMF_NG;
            break;
          }
          Ret = SMF_OK;
          break;
        case MIDI_STATEX_SYSEXCL:
          MidiData1 = SmfSeqGetNum(ptrkTbl);
          if (MidiData1 == SMF_ENDOFSMF)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = MidiData1;
            break;
          }
          ExBuff[0] = MidiStatus;
          Ret = SmfSeqGetData(ptrkTbl, &ExBuff[1], MidiData1);
          if (Ret == SMF_ENDOFSMF)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            break;
          }
          Ret = midiOutLongMsg(ExBuff, MidiData1 + 1);
          if (Ret == MIDI_NG)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = SMF_NG;
            break;
          }
          Ret = SMF_OK;
          break;
        case MIDI_STATEX_EOEXCL:
          MidiData1 = SmfSeqGetNum(ptrkTbl);
          if (MidiData1 == SMF_ENDOFSMF)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = MidiData1;
            break;
          }
          Ret = SmfSeqGetData(ptrkTbl, ExBuff, MidiData1);
          if (Ret == SMF_ENDOFSMF)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            break;
          }
          Ret = midiOutLongMsg(ExBuff, MidiData1);
          if (Ret == MIDI_NG)
          {
            ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
            Ret = SMF_NG;
            break;
          }
          Ret = SMF_OK;
          break;
        case MIDI_STATEX_MIDITCQF:
        case MIDI_STATEX_SONGPOGP:
        case MIDI_STATEX_SONGSEL:
        case MIDI_STATEX_CHUNEREQ:
        case MIDI_STATRT_TMGCLOCK:
        case MIDI_STATRT_START:
        case MIDI_STATRT_CONTINUE:
        case MIDI_STATRT_STOP:
        case MIDI_STATRT_ACTSENC:
        case MIDI_STATRT_SYSRESET:
        default:
          ptrkTbl->TrackStatus = SMF_TRKSTAT_TRACKEND;
          Ret = SMF_NG;
          break;
        }
        if ((Ret == SMF_NG) || (Ret == SMF_ENDOFSMF))
        {
          break;
        }
      }
      ptrkTbl->SeqWaitFlag = SMF_WAIT_OFF;
    }
    else
    {
      ptrkTbl->SeqWaitFlag = SMF_WAIT_ON;
      break;
    }
  }
  return (Ret);
}

//-----------------------------------------------------------------------------
//  終了        全テーブル初期化
//-----------------------------------------------------------------------------
int SmfSeqEnd(SMF_SEQ_TABLE *pseqTbl)
{
  int Ret;

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  //状態をファイル未読み込みに遷移
  pseqTbl->PlayStatus = SMF_STAT_FILENOTREAD;

  SmfSeqAllNoteOff(pseqTbl); //ノートオフ漏れ停止

  Ret = SmfSeqGMReset(); //音源リセット

  Ret = midiOutClose(); //MIDIクローズ

  SmfFileAccessClose(); //ファイルクローズ

  if (Ret == MIDI_NG)
  {
    Ret = SMF_NG;
  }
  else
  {
    Ret = SMF_OK;
  }
  return (Ret);
}
//------------------
//-----------------------------------------------------------------------------
//  表示情報更新有無リセット
//-----------------------------------------------------------------------------
void SmfSeqDispRenewReset(SMF_SEQ_TABLE *pseqTbl)
{

  if (pseqTbl == NULL)
  {
    return;
  }

  pseqTbl->DispRenewFlag = SMF_OFF;
}
//-----------------------------------------------------------------------------
//  表示情報更新有無取得
//-----------------------------------------------------------------------------
int SmfSeqGetDispRenew(SMF_SEQ_TABLE *pseqTbl)
{

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  return (pseqTbl->DispRenewFlag);
}
//-----------------------------------------------------------------------------
//  演奏状態取得
//-----------------------------------------------------------------------------
int SmfSeqGetStatus(SMF_SEQ_TABLE *pseqTbl)
{

  if (pseqTbl == NULL)
  {
    return (SMF_NG);
  }

  return (pseqTbl->PlayStatus);
}

//-----------------------------------------------------------------------------
//  GMリセット
//-----------------------------------------------------------------------------
int SmfSeqGMReset()
{
  int Ret;

  Ret = midiOutGMReset();
  if (Ret == MIDI_NG)
  {
    Ret = SMF_NG;
  }
  else
  {
    Ret = SMF_OK;
  }
  return (Ret);
}

//-----------------------------------------------------------------------------
int SmfSeqStepBack(SMF_TRACK_TABLE *ptrkTbl)
{
  int Ret;

  if (ptrkTbl == NULL)
  {
    return (SMF_NG);
  }

  Ret = SMF_OK;

  if (ptrkTbl->Ptr != 0)
  {
    ptrkTbl->Ptr--;
  }

  return (Ret);
}

int SmfSeqGetByteData(SMF_TRACK_TABLE *ptrkTbl)
{
  int Ret;

  if (ptrkTbl == NULL)
  {
    return (SMF_NG);
  }

  Ret = SMF_OK;

  //トラックサイズを越えてアクセスしようとしている
  if (ptrkTbl->Ptr >= ptrkTbl->Size)
  {
    Ret = SMF_ENDOFSMF; //エラーを返す
  }
  else
  {
    UCHAR data;
    bool result = SmfFileAccessRead(&data, (ptrkTbl->TrkBufOffset + ptrkTbl->Ptr));
    ptrkTbl->Ptr++;
    if (result == true)
    {
      Ret = (int)data;
    }
    else
    {
      Ret = SMF_ENDOFSMF; //エラーを返す
    }
  }

  return (Ret);
}

int SmfSeqGetData(SMF_TRACK_TABLE *ptrkTbl, UCHAR *Data, int Length)
{
  int Cnt = 0;
  int Ret;

  if (ptrkTbl == NULL)
  {
    return (SMF_NG);
  }

  Ret = SmfFileAccessReadBuf(Data, (ptrkTbl->TrkBufOffset + ptrkTbl->Ptr), Length);
  ptrkTbl->Ptr = ptrkTbl->Ptr + Ret;

  if (Ret != Length)
  {
    Ret = SMF_ENDOFSMF; //エラーを返す
  }

  return (Ret);
}

int SmfSeqGetExData(SMF_TRACK_TABLE *ptrkTbl, UCHAR *Data, int MaxLength)
{
  int Cnt;
  int Ret;
  int MidiData;

  if (ptrkTbl == NULL)
  {
    return (SMF_NG);
  }

  Ret = SMF_OK;

  UCHAR data;
  bool result;
  for (Cnt = 0; Cnt < MaxLength; Cnt++)
  {
    if (ptrkTbl->Ptr >= ptrkTbl->Size)
    {
      MidiData = SMF_ENDOFSMF; //エラーを返す
    }
    else
    {
      if (Cnt == 0)
      {
        result = SmfFileAccessRead(&data, (ptrkTbl->TrkBufOffset + ptrkTbl->Ptr));
        ptrkTbl->Ptr++;
      }
      else
      {
        result = SmfFileAccessReadNext(&data);
        ptrkTbl->Ptr++;
      }
      if (result == true)
      {
        MidiData = (int)data;
      }
      else
      {
        MidiData = SMF_ENDOFSMF; //エラーを返す
      }
    }
    if (MidiData == SMF_ENDOFSMF)
    {
      Ret = MidiData;
      break;
    }
    if (MidiData == MIDI_STATEX_EOEXCL)
    {
      Ret = Cnt;
      break;
    }
    Data[Cnt] = MidiData;
  }

  return (Ret);
}

//数値取得、主にデルタタイム
long SmfSeqGetNum(SMF_TRACK_TABLE *ptrkTbl)
{
  long Ret;
  long DeltaTime;
  long Cnt;
  long MidiData;

  if (ptrkTbl == NULL)
  {
    return (SMF_NG);
  }

  Ret = SMF_OK;
  DeltaTime = 0;

  DPRINT(F("getNum"));

  UCHAR data;
  bool result;
  for (Cnt = 0; Cnt < SMF_DELTAMAXBYTE; Cnt++)
  {
    if (ptrkTbl->Ptr >= ptrkTbl->Size)
    {
      MidiData = SMF_ENDOFSMF; //エラーを返す
    }
    else
    {
      if (Cnt == 0)
      {
        result = SmfFileAccessRead(&data, (ptrkTbl->TrkBufOffset + ptrkTbl->Ptr));
        ptrkTbl->Ptr++;
      }
      else
      {
        result = SmfFileAccessReadNext(&data);
        ptrkTbl->Ptr++;
      }
      if (result == true)
      {
        MidiData = (int)data;
      }
      else
      {
        MidiData = SMF_ENDOFSMF; //エラーを返す
      }
    }
    DPRINT(F(":"));
    DPRINT(MidiData, HEX);
    if (MidiData == SMF_ENDOFSMF)
    {
      Ret = MidiData;
      break;
    }
    DeltaTime = DeltaTime | (MidiData & SMF_DELTAMASK);
    if ((MidiData & SMF_DELTACONTBIT) != SMF_DELTACONTBIT)
    {
      Ret = DeltaTime;
      DPRINT(F("=>"));
      DPRINT(Ret);
      break;
    }
    DeltaTime = (DeltaTime << 7);
  }
  DPRINTLN(F(""));

  return (Ret);
}

//  サイズ制限付き文字列コピー
void SmfSeqStrcpy(char *DistStr, char *SrcStr, int MaxSize)
{
  int Cnt;

  for (Cnt = 0; Cnt < MaxSize; Cnt++)
  {
    DistStr[Cnt] = SrcStr[Cnt];
    if (SrcStr[Cnt] == 0x00)
    {
      break;
    }
  }
}

//  フルパスファイル名からファイル名を取得
char *SmfSeqGetFileName(char *FullPathName)
{
  int Cnt;
  int Lng;

  Lng = strlen(FullPathName);
  for (Cnt = 0; Cnt < Lng; Cnt++)
  {
    if (FullPathName[Lng - Cnt - 1] == '\\')
    {
      break;
    }
  }

  return (&FullPathName[Lng - Cnt]);
}
