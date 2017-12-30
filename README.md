# 概要
LEDの点滅速度を1～9段階で変えることができます。

# 実演
https://www.youtube.com/watch?v=G6paYJmgrhE

# 必要なもの
* Raspberry Pi 3 
* LED
* 抵抗　100Ω


# 使用方法
LEDをGPIOの25番ピンに接続しておきます。

以下のようにコマンドを入力すると段階が指定されて点滅速度が変わります。 xの部分を指定したい段階に置き換えて入力してください。  
`echo x > /dev/myled0`

2段階目を指定する場合次のようになります。  
`echo 2 > /dev/myled0`
