// Mini SMF(Standard Midi File) Sequencer Sample Program
//
// SDカード内に格納したSMFファイル"playdat0.mid"～"playdat9.mid"を自動で演奏します。
//
// 以下のI/F/ライブラリを使用します
//  M5Stack用MIDIモジュール2 https://necobit.com/denshi/m5-midi-module2/
//  MSTimer2
//  LovyanGFX
//
//  オリジナルは @catsin さんの https://bitbucket.org/kyoto-densouan/smfseq/src/m5stack/
//  necobitでは画面描画部分と、起動時自動スタートの処理への変更をしています。
//
//　コメントやコメントアウトなど、取っ散らかっているところがありますがご了承ください。

#include "common.h"
#include "SmfSeq.h"
#include "IntervalCheck.h"
#include "IntervalCheckMicros.h"

SMF_SEQ_TABLE *pseqTbl; //SMFシーケンサハンドル

LGFX lcd;

//----------------------------------------------------------------------
//MIDIポートアクセス関数定義
// MidiFunc.c/hから呼び出されるMIDI I/Fアクセス関数の実体を記述する。
// 以下ではハードウェアシリアルを使用した場合の例を記述。
#include "MidiPort.h"
int MidiPort_open()
{
  Serial2.begin(D_MIDI_PORT_BPS);
  return (0);
}
void MidiPort_close()
{
  Serial2.end();
}
int MidiPort_write(UCHAR data)
{
#ifdef DUMPMIDI
  DPRINT("1:");
  int n = (int)data;
  DPRINTLN(n, HEX);
#else
  Serial2.write(data);
#endif
  //  Serial.flush();
  return (1);
}
int MidiPort_writeBuffer(UCHAR *pData, ULONG Len)
{
#ifdef DUMPMIDI
  int n;
  int i;
  DPRINT.print(Len);
  DPRINT.print(":");
  for (i = 0; i < Len; i++)
  {
    n = (int)pData[i];
    DPRINT.print(n, HEX);
  }
  DPRINTLN.println("");
#else
  Serial2.write(pData, Len);
#endif
  //  Serial.flush();
  return (Len);
}
//----------------------------------------------------------------------
//SMFファイルアクセス関数定義
// SmfSeq.c/hから呼び出されるSMFファイルへのアクセス関数の実体を記述する。
// 以下ではSDカードシールドライブラリに対し、ファイルポインタで直接読み出し位置を指定する方法での例を記述。
// ライブラリ自体の初期化はsetup関数に記述している。
//#define D_SD_CHIP_SELECT_PIN   4
//#include <SPI.h>
//#include <SD.h>
#include "SmfFileAccess.h"

File s_FileHd;
bool SmfFileAccessOpen(UCHAR *Filename)
{
  bool result = false;

  if (Filename != NULL)
  {
    // lcd.print(F("filename:"));
    lcd.setFont(&fonts::Font4);
    lcd.setTextSize(1);
    lcd.setCursor(5, 0);
    lcd.println((const char *)Filename);
    s_FileHd = SD.open((const char *)Filename);

    result = s_FileHd.available();
  }
  return (result);
}
void SmfFileAccessClose()
{
  s_FileHd.close();
}
bool SmfFileAccessRead(UCHAR *Buf, unsigned long Ptr)
{
  bool result = true;
  if (Buf != NULL)
  {
    if (s_FileHd.position() != Ptr)
    {
      s_FileHd.seek(Ptr);
    }
    int data = s_FileHd.read();
    if (data >= 0)
    {
      *Buf = (UCHAR)data;
    }
    else
    {
      result = false;
    }
  }
  return (result);
}
bool SmfFileAccessReadNext(UCHAR *Buf)
{
  bool result = true;
  if (Buf != NULL)
  {
    int data = s_FileHd.read();
    if (data >= 0)
    {
      *Buf = (UCHAR)data;
    }
    else
    {
      result = false;
    }
  }
  return (result);
}
int SmfFileAccessReadBuf(UCHAR *Buf, unsigned long Ptr, int Lng)
{
  int result = 0;
  if (Buf != NULL)
  {
    if (s_FileHd.position() != Ptr)
    {
      s_FileHd.seek(Ptr);
    }

    int i;
    int data;
    for (i = 0; i < Lng; i++)
    {
      data = s_FileHd.read();
      if (data >= 0)
      {
        Buf[i] = (UCHAR)data;
        result++;
      }
      else
      {
        break;
      }
    }
  }
  return (result);
}
unsigned int SmfFileAccessSize()
{
  unsigned int result = 0;
  result = s_FileHd.size();

  return (result);
}

//----------------------------------------------------------------------
//#define D_PLAY_BUTTON_PIN   2  //開始/停止ボタン
#define D_CH_OFFSET_PIN 3 //チャンネル番号オフセット（eVY1のGM音源としての演奏）
//#define D_FF_BUTTON_PIN     5  //送りボタン
#define D_STATUS_LED 10 //状態表示LED

int playdataCnt = 0;       //選曲番号
#define D_PLAY_DATA_NUM 10 //SDカード格納ＳＭＦファイル数

IntervalCheck sButtonCheckInterval(100, true);
//IntervalCheck sTickProcInterval( ZTICK, true );
IntervalCheckMicros sTickProcInterval(ZTICK * 1000, true);
IntervalCheck sStatusLedCheckInterval(100, true);
unsigned int sLedPattern = 0x0f0f;
IntervalCheck sUpdateScreenInterval(500, true);

char filename[14] = {'/', 'p', 'l', 'a', 'y', 'd', 'a', 't', '0', '.', 'm', 'i', 'd', 0x00};

//SMFファイル名生成
// playdat0.mid～playdat9.midの文字列を順次返す。
char *makeFilename()
{
  int cnt = 0;
  //  char * filename = (char*)"playdat0.mid";
  for (cnt = 0; cnt < D_PLAY_DATA_NUM; cnt++)
  {
    filename[8] = 0x30 + playdataCnt;
    playdataCnt++;
    if (playdataCnt >= D_PLAY_DATA_NUM)
    {
      playdataCnt = 0;
    }
    if (SD.exists(filename) == true)
    {
      break;
    }
  }
  return (filename);
}

void updateScreen()
{
  static String last_filename = "";
  static int last_status = -1;
  static int last_chNoOffset = -1;

  int status = SmfSeqGetStatus(pseqTbl);

  int chNoOffset = 0;
  //とりあえずバイパスしておく
  //if( digitalRead( D_CH_OFFSET_PIN )==LOW ){
  //CH番号を1ずらす（eVY1のCH0が音声合成専用のため、GM音源として使用する場合CH0を使用しない）
  //  chNoOffset = 1;
  //}

  if ((last_filename != filename) || (last_status != status) || (last_chNoOffset != chNoOffset))
  {
    lcd.fillScreen(TFT_BLACK);
    backscreen();
    lcd.setFont(&fonts::Font4);
    lcd.setTextSize(1);

    lcd.setCursor(5, 0);
    // lcd.print(F("Filename: "));
    lcd.println(filename);
    lcd.setCursor(5, 27);
    lcd.print(F("Status:"));
    switch (status)
    {
    case SMF_STAT_FILENOTREAD: //SMFファイル未読み込み（演奏不能）
      lcd.println(F("File load failed."));
      break;
    case SMF_STAT_STOP: //演奏停止
      lcd.println(F("stop."));
      lcd.fillRect(260, 5, 40, 40, TFT_WHITE);
      break;
    case SMF_STAT_PLAY: //演奏中
      lcd.println(F("playing."));
      lcd.fillRect(280, 5, 310, 45, TFT_BLACK);
      lcd.fillTriangle(280, 5, 280, 45, 310, 25, TFT_YELLOW);
      backscreen();
      break;
    case SMF_STAT_PAUSE: //演奏一時停止中
      lcd.println(F("pause."));
      break;
    case SMF_STAT_STOPWAIT: //演奏停止待ち（演奏中）
      lcd.println(F("wait."));
      break;
    default:
      break;
    }

    // lcd.print(F("ch shift: "));
    // lcd.println(chNoOffset);

    last_filename = filename;
    last_status = status;
    last_chNoOffset = chNoOffset;
  }
}

void backscreen()
{
  lcd.drawRect(10, 58, 300, 161, 0xF660);
  lcd.setFont(&fonts::Font0);
  lcd.setTextSize(1);
  for (int chd = 1; chd <= 16; chd++)
  {

    lcd.drawNumber(chd, 13, 50 + chd * 10);
    lcd.drawLine(11, 58 + chd * 10, 308, 58 + chd * 10, 0xF660);
  }
  lcd.drawLine(26, 58, 26, 217, 0xF660);
}

void setup()
{
  //  delay(2000);

  M5.begin();
  M5.Power.begin();

  // 最初に初期化関数を呼び出します。
  lcd.init();

  // 回転方向を 0～3 の4方向から設定します。(4～7を使用すると上下反転になります。)
  lcd.setRotation(1);

  // バックライトの輝度を 0～255 の範囲で設定します。
  lcd.setBrightness(255); // の範囲で設定
                          // M5Stick-Cのバックライト調整は現在非対応です。
                          // AXP192ライブラリを別途includeして設定してください。


  // clearまたはfillScreenで画面全体を塗り潰します。
  // どちらも同じ動作をしますが、clearは引数を省略でき、その場合は黒で塗り潰します。
  lcd.clear(0x0000); // 黒で塗り潰し

  Serial.begin(115200);

  //  pinMode( D_PLAY_BUTTON_PIN, INPUT_PULLUP );
  //  pinMode( D_FF_BUTTON_PIN, INPUT_PULLUP );
  pinMode(D_CH_OFFSET_PIN, INPUT_PULLUP);
  pinMode(D_STATUS_LED, OUTPUT);

  lcd.println(F("Initializing SD card..."));

  //  if (!SD.begin(D_SD_CHIP_SELECT_PIN)) {
  //    DPRINTLN(F("Card failed, or not present"));
  //    // don't do anything more:
  //    delay(2000);
  //    return;
  //  }
  //  DPRINTLN(F("card initialized."));
  // すぐにファイルアクセスするとフォーマット破壊することがあったため待ち
  delay(2000);

  // digitalWrite( D_STATUS_LED, HIGH );
  int Ret;
  pseqTbl = SmfSeqInit(ZTICK);
  if (pseqTbl == NULL)
  {
    lcd.println(F("SmfSeqInit failed."));
    delay(2000);
    return;
  }

  int chNoOffset = 0;
  //  シフトは不要なのでとりあえずバイパスしておく
  //  if( digitalRead( D_CH_OFFSET_PIN )==LOW ){
  //CH番号を1ずらす（eVY1のCH0が音声合成専用のため、GM音源として使用する場合CH0を使用しない）
  //    chNoOffset = 1;
  //  }

  //SMFファイル読込
  SmfSeqFileLoadWithChNoOffset(pseqTbl, (char *)makeFilename(), chNoOffset);
  //GMリセット送信
  //Ret = SmfSeqGMReset();
  //発音中全キーノートオフ
  Ret = SmfSeqAllNoteOff(pseqTbl);
  //トラックテーブルリセット
  SmfSeqPlayResetTrkTbl(pseqTbl);
  //演奏開始
  //SmfSeqStart(pseqTbl);

  // digitalWrite( D_STATUS_LED, LOW );

  //  sButtonCheckInterval.reset();
  //  sTickProcInterval.reset();
}

int prePlayButtonStatus = HIGH;
int preFfButtonStatus = HIGH;
void loop()
{
  int Ret;

  // 定期起動処理
  if (sTickProcInterval.check() == true)
  {
    if (SmfSeqGetStatus(pseqTbl) != SMF_STAT_STOP)
    {
      //状態が演奏停止中以外の場合
      //定期処理を実行
      Ret = SmfSeqTickProc(pseqTbl);
      //処理が間に合わない場合のリカバリ
      while (sTickProcInterval.check() == true)
      {
        //定期処理を実行
        Ret = SmfSeqTickProc(pseqTbl);
      }
      if (SmfSeqGetStatus(pseqTbl) == SMF_STAT_STOP)
      {
        //状態が演奏停止中になった場合
        //発音中全キーノートオフ
        Ret = SmfSeqAllNoteOff(pseqTbl);
        //トラックテーブルリセット
        SmfSeqPlayResetTrkTbl(pseqTbl);
        // ファイルクローズ
        SmfSeqEnd(pseqTbl);
        lcd.fillRect(280, 5, 310, 45, BLACK);
        lcd.setFont(&fonts::Font4);
        lcd.setCursor(5, 27);
        lcd.setTextSize(1);
        lcd.print(F("Status:"));
        lcd.println(F("SEQ end.  "));

        // digitalWrite( D_STATUS_LED, HIGH );
        int chNoOffset = 0;
        //とりあえずバイパスしておく
        //if( digitalRead( D_CH_OFFSET_PIN )==LOW ){
        //CH番号を1ずらす（eVY1のCH0が音声合成専用のため、GM音源として使用する場合CH0を使用しない）
        //  chNoOffset = 1;
        //}

        pseqTbl = SmfSeqInit(ZTICK);
        // //SMFファイル読込n
        SmfSeqFileLoadWithChNoOffset(pseqTbl, (char *)makeFilename(), chNoOffset);
        // //GMリセット送信
        // //Ret = SmfSeqGMReset();
        // //発音中全キーノートオフ
        //Ret = SmfSeqAllNoteOff(pseqTbl);
        // //トラックテーブルリセット
        SmfSeqPlayResetTrkTbl(pseqTbl);
        // //演奏開始
        SmfSeqStart(pseqTbl);
        // digitalWrite( D_STATUS_LED, LOW );
      }
    }
  }

  // ボタン操作処理
  if (sButtonCheckInterval.check() == true)
  {
    M5.update();
    //スイッチ状態取得
    //    int buttonPlayStatus = digitalRead( D_PLAY_BUTTON_PIN );
    int buttonPlayStatus = M5.BtnB.wasPressed();
    if (prePlayButtonStatus != buttonPlayStatus)
    {
      //スイッチ状態が変化していた場合
      if (buttonPlayStatus == LOW)
      {
        //スイッチ状態がONの場合
        if (SmfSeqGetStatus(pseqTbl) == SMF_STAT_STOP)
        {
          //演奏開始
          SmfSeqStart(pseqTbl);
        }
        else
        {
          //演奏中なら演奏停止
          SmfSeqStop(pseqTbl);
          //発音中全キーノートオフ
          Ret = SmfSeqAllNoteOff(pseqTbl);
        }
      }
    }
    //スイッチ状態保持
    prePlayButtonStatus = buttonPlayStatus;

    //    int buttonFfStatus = digitalRead( D_FF_BUTTON_PIN );
    int buttonFfStatus = M5.BtnC.wasPressed();
    if (preFfButtonStatus != buttonFfStatus)
    {
      //スイッチ状態が変化していた場合
      if (preFfButtonStatus == LOW)
      {
        //スイッチ状態がONの場合
        bool playing = false;
        if (SmfSeqGetStatus(pseqTbl) != SMF_STAT_STOP)
        {
          //演奏中なら演奏停止
          SmfSeqStop(pseqTbl);
          //発音中全キーノートオフ
          Ret = SmfSeqAllNoteOff(pseqTbl);
          //トラックテーブルリセット
          SmfSeqPlayResetTrkTbl(pseqTbl);
          // ファイルクローズ
          SmfSeqEnd(pseqTbl);
          playing = true;
        }
        else
        {
          playing = false;
        }

        // digitalWrite( D_STATUS_LED, HIGH );
        int chNoOffset = 0;
        //とりあえずバイパスしておく
        //if( digitalRead( D_CH_OFFSET_PIN )==LOW ){
        //CH番号を1ずらす（eVY1のCH0が音声合成専用のため、GM音源として使用する場合CH0を使用しない）
        //  chNoOffset = 1;
        //}

        pseqTbl = SmfSeqInit(ZTICK);
        //SMFファイル読込
        SmfSeqFileLoadWithChNoOffset(pseqTbl, (char *)makeFilename(), chNoOffset);
        //GMリセット送信
        //Ret = SmfSeqGMReset();
        //発音中全キーノートオフ
        Ret = SmfSeqAllNoteOff(pseqTbl);
        //トラックテーブルリセット
        SmfSeqPlayResetTrkTbl(pseqTbl);
        if (playing == true)
        {
          //演奏開始
          SmfSeqStart(pseqTbl);
        }
        // digitalWrite( D_STATUS_LED, LOW );
      }
    }
    //スイッチ状態保持
    preFfButtonStatus = buttonFfStatus;
  }

  // 状態表示更新
  if (SmfSeqGetStatus(pseqTbl) != SMF_STAT_STOP)
  {
    if (sStatusLedCheckInterval.check() == true)
    {
      unsigned int led = sLedPattern & 0x0001;
      if (led > 0)
      {
        // digitalWrite( D_STATUS_LED, HIGH );
      }
      else
      {
        // digitalWrite( D_STATUS_LED, LOW );
      }
      sLedPattern = (sLedPattern >> 1) | (led << 15);
    }
  }
  else
  {
    // digitalWrite( D_STATUS_LED, LOW );
  }

  if (sUpdateScreenInterval.check() == true)
  {
    updateScreen();
  }
}
