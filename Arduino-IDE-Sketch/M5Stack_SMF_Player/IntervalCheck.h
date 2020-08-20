// 間隔測定クラス

#ifndef D_INTERVAL_CHECK_H
#define D_INTERVAL_CHECK_H

class  IntervalCheck
{
private:
  unsigned long m_lastTime;
  unsigned long m_intervalTime;
  bool  m_autoReset;

public:
  // コンストラクタ
  IntervalCheck( unsigned long interval=1000, bool autoReset=true ){
    m_intervalTime = interval;
    m_autoReset = autoReset;
    reset();
  }

  // 時間経過判定
  bool  check(){
    bool  result = false;
    
    if ((millis()-m_lastTime)>m_intervalTime) {
      if( m_autoReset == true ){
        // オートリセットが設定されいる場合、ここで自動リセット＝リピートする
        reset();
      }
      result = true;
    }    
    return result;
  }

  // 外部からのリセット  
  void  reset(){
    m_lastTime = millis();
  }
};

#endif  // D_INTERVAL_CHECK_H
