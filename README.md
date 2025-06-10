# robo2024
高専ロボコン2024のロボット2-1, 2-2の制御で使用した、ArduinoIDEプロジェクトです。 マイコンはRaspberry Pi Picoです。
ラズピコ用のボードは、
https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
を「追加のボードマネージャー」に入力すると、「ボードマネージャー」に「Raspberry Pi Pico/RP2040/RP2350」が出てくるので、それをインストールして利用します。これでないと、UARTが2本使えません。
・Xbeeのボーレートは19200に設定します。
