// 間隔測定クラス

#ifndef D_INTERVAL_CHECK_COUNTS_H
#define D_INTERVAL_CHECK_COUNTS_H

class  IntervalCheckCounts
{
private:
  volatile unsigned long m_count;
  volatile unsigned long m_limitRate;
  unsigned long m_lastTime;
  unsigned long m_intervalTime;
  bool  m_autoReset;

public:
  // コンストラクタ
  IntervalCheckCounts( unsigned long interval=1000, bool autoReset=true ){
    m_count = 0;
    m_limitRate = 10;
    m_intervalTime = interval;
    m_autoReset = autoReset;
    reset();
  }

  // 時間経過判定
  bool  check(){
    bool  result = false;

    if ((m_count-m_lastTime)>m_intervalTime) {
      if( m_autoReset == true ){
        // オートリセットが設定されいる場合、ここで自動リセット＝リピートする
       if ((m_count-m_lastTime)>(m_intervalTime*m_limitRate)) {
         reset();
       }
       else
       {
         m_lastTime += m_intervalTime;
       }
      }
      result = true;
    }    
    return result;
  }

  // 外部からのリセット  
  void  reset(){
    m_count = 0;
    m_lastTime = m_count;
  }

  void setLimitRate( unsigned long limitRate ) {
    m_limitRate = limitRate;
    if(m_limitRate == 0) {
      m_limitRate = 1;
    }
  }
  
  void updateCount() {
    m_count++;
  }
};

#endif  // D_INTERVAL_CHECK_COUNTS_H
