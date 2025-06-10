#include <stdio.h>

//ピン番号の設定
const int power = LED_BUILTIN;//マイコン内蔵LED,picoは25番ピンらしい
const int led = 3;
const int adc_pin[2] = {26,27};//ジョイスティックの操作に使用,x:26,y:27
const int switch_pin = 7;//ローラー回転ボタン:7(自動psbオン操作)

//グローバル変数の宣言
bool led_state = false;//内蔵LEDの状態
char valNum[] = {'a','b','c'};//識別用データ
int val[3]={0};//スティック・スイッチのデータ
char snd[3][4];

void setup() {
  Serial.begin(9600);
  Serial1.begin(19200);
  Serial2.begin(19200);

  //ピンの初期化
  pinMode(power,OUTPUT);
  digitalWrite(power,HIGH);
  
  analogReadResolution(12);//ADCの分解能を12bitに設定
  pinMode(switch_pin,INPUT);
  pinMode(led,OUTPUT);
  digitalWrite(led,LOW);
}

void loop() {
  //LEDで操作の状況を表示
  digitalWrite(led, led_state);

  //足回り操作
  val[0] = analogRead(adc_pin[0])/16;//0~4095を0~255に圧縮
  val[1] = analogRead(adc_pin[1])/16;

  //ローラー操作
  if(digitalRead(switch_pin)==HIGH){
    led_state = !led_state;//ローラー回転の操作していることを示す
    val[2] = 250;//ローラー回転ボタンが押されたらval[2]を250にする(250は適当な数値)
  }else{
    led_state = false;
    val[2] = 100;
  }

  //送信用のデータ整理
  for(int i=0;i<3;i++)
  {
    snprintf(snd[i],4,"%03d",val[i]);
  }

  //送信部
  Serial1.print("TXDA "); 
  Serial2.print("TXDA "); 
  
  for(int i=0;i<3;i++){
    Serial.print(valNum[i]);
    Serial.print(snd[i]);
    
    Serial1.print(valNum[i]);//文字列として送信
    Serial1.print(snd[i]);
    
    Serial2.print(valNum[i]);//文字列として送信
    Serial2.print(snd[i]); 
  }
  Serial.println("\r");
  Serial1.println("\r");
  Serial2.println("\r");
  delay(100);//送信データが混み合わないようにしている(要調整)
}
