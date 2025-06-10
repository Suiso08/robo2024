/*
参考
PWMの設定関連：https://lipoyang.hatenablog.com/entry/2021/12/12/201432
*/

//ライブラリ
#include <hardware/pwm.h>

//数値の定義
#define PWM_CLKDIV 122.0703125
#define PWM_WRAP 1023
#define MARGIN 182.0//足回り用
#define SPEED_MAX 800//モーター速度の最大値(1023まで設定可能)

//ピン番号の宣言
const int pwm_pin[5]={20,21,27,28,15};//足回り用:20,21,27,28、ローラー用：15
const int led = LED_BUILTIN;//マイコン内蔵LED
const int dir_pin[2]={16,17};//ロームモタドラのINA,INB
const int psb_pin = 14;//ロームモタドラのPSB

//グローバル変数の宣言
int rcv_state_counter = 0;//シリアル受信の状態を管理する

uint slice_num[5]={0};//pwm設定に使う
int val[3]={0};//受信データを格納する
int dir_L,dir_R,speed_L,speed_R;//足回り制御の変数

void setup() {
  Serial.begin(9600);
  Serial1.begin(19200);
  
  //PWMの設定
  gpio_set_function(pwm_pin[0], GPIO_FUNC_PWM);
  gpio_set_function(pwm_pin[1], GPIO_FUNC_PWM);
  gpio_set_function(pwm_pin[2], GPIO_FUNC_PWM);
  gpio_set_function(pwm_pin[3], GPIO_FUNC_PWM);
  gpio_set_function(pwm_pin[4], GPIO_FUNC_PWM);
  for(int i=1;i<5;i++){
    slice_num[i] = pwm_gpio_to_slice_num(pwm_pin[i]);//gp21(gp20),gp27,gp28,gp15のPWMスライスを取得
    // PWM周期を設定
    pwm_set_clkdiv(slice_num[i], PWM_CLKDIV);
    pwm_set_wrap(slice_num[i], PWM_WRAP);
  }
  //PWM出力の有効化
  pwm_set_enabled(slice_num[1], true);
  pwm_set_enabled(slice_num[2], true);
  pwm_set_enabled(slice_num[3], true);
  pwm_set_enabled(slice_num[4], true);

  //デジタルピンの初期化
  pinMode(led,OUTPUT);
  pinMode(psb_pin,OUTPUT);
  pinMode(dir_pin[0],OUTPUT);
  pinMode(dir_pin[1],OUTPUT);
  digitalWrite(led,LOW);
  digitalWrite(psb_pin,LOW);//PSBオフ
}

char r[64];
char n[3][4];
int i,j;

void loop() {
  //受信部
  if(Serial1.available()>0){
    String rcv=Serial1.readStringUntil('\n');
    //Serial.println(rcv);
    
    strcpy(r,rcv.c_str());
    //Im920c: "a255b128c100\n" のようなデータが来ると想定
    //xbee: "TXDA a255b128c100\n" のようなデータが来ると想定
    for(i=0;i<64;i++){
      switch(r[i]){
        case 'a':
        for(j=0;j<3;j++)
        {
          n[0][j]=r[i+1];
          i++;
        }
        val[0]=atoi(n[0]);
        val[0]-=127;
        break;
            
        case 'b':
        for(j=0;j<3;j++)
        {
          n[1][j]=r[i+1];
          i++;
        }
        val[1]=atoi(n[1]);
        val[1]-=127;
        break;
            
        case 'c':
        for(j=0;j<3;j++)
        {
          n[2][j]=r[i+1];
          i++;
        }
        val[2]=atoi(n[2]);
        i=64;
        break;
        
        default:
        break;
      }
    }

    //シリアルモニタに表示
    for(i=0;i<3;i++){
      Serial.print(val[i]);
      Serial.print(" ");
    }
    Serial.println();

    Serial.print(dir_L);
    Serial.print(" ");
    Serial.print(dir_R);
    Serial.print(" ");
    Serial.print(speed_L);
    Serial.print(" ");
    Serial.print(speed_R);
    Serial.print(" ");
    Serial.println();
        
    digitalWrite(led,LOW);
    delay(25);
    
    rcv_state_counter++;
    //受信過多の時の処理
    if(rcv_state_counter>=5){
      rcv_state_counter=0;
      digitalWrite(led,HIGH);
      Serial1.end();
      Serial.println("STOP");
      delay(3000);
      Serial1.begin(19200);
      digitalWrite(led,LOW);
    }
  }else{
    rcv_state_counter=0;
    digitalWrite(led,HIGH);
  }

  //足回り駆動部
  dir_L=(-val[1]+val[0]);//dirが正の時、モーターは正回転とする
  dir_R=(-val[1]-val[0]);
  speed_L=abs(dir_L)/MARGIN*SPEED_MAX;
  speed_R=abs(dir_R)/MARGIN*SPEED_MAX;
  
  if(speed_L>SPEED_MAX)speed_L=SPEED_MAX;
  if(speed_R>SPEED_MAX)speed_R=SPEED_MAX;
  if(dir_L>5){
    pwm_set_chan_level(slice_num[1], PWM_CHAN_A, 0);
    pwm_set_chan_level(slice_num[1], PWM_CHAN_B, speed_L);
  }else if(dir_L<-5){
    pwm_set_chan_level(slice_num[1], PWM_CHAN_A, speed_L);
    pwm_set_chan_level(slice_num[1], PWM_CHAN_B, 0);
  }else{
    pwm_set_chan_level(slice_num[1], PWM_CHAN_A, 0);
    pwm_set_chan_level(slice_num[1], PWM_CHAN_B, 0);
  }
  if(dir_R>5){
    pwm_set_chan_level(slice_num[2], PWM_CHAN_B, 0);
    pwm_set_chan_level(slice_num[3], PWM_CHAN_A, speed_R);
  }else if(dir_R<-5){                                                                                                                                                                         
    pwm_set_chan_level(slice_num[2], PWM_CHAN_B, speed_R);
    pwm_set_chan_level(slice_num[3], PWM_CHAN_A, 0);
  }else{
    pwm_set_chan_level(slice_num[2], PWM_CHAN_B, 0);
    pwm_set_chan_level(slice_num[3], PWM_CHAN_A, 0);
  }

  //ローラー駆動部
  if(val[2]>200){
    digitalWrite(psb_pin,HIGH);//psbオン
    //ローラー回転
    digitalWrite(dir_pin[0],LOW);
    digitalWrite(dir_pin[1],HIGH);
    pwm_set_chan_level(slice_num[4], PWM_CHAN_B, 1000);
  }else{
    //ローラー停止
    digitalWrite(dir_pin[0],LOW);
    digitalWrite(dir_pin[1],LOW);
    pwm_set_chan_level(slice_num[4], PWM_CHAN_B, 0);

    digitalWrite(psb_pin,LOW);//psbオフ
  }

  
}
