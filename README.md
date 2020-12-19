# LoRaKissTNC

430MHzのLoRaモジュールで以下の機能を実現するスケッチです。※1

1. KISSモードTNC
1. テキストチャット

「KISSモードTNC」モードでは、LoRaモジュールをKISSモードのターミナルノードコントローラとして動作させます。シリアル回線からAX25形式で送られたパケットを指定された周波数でLoRa変調により送信します。※２

「テキストチャット」モードでは、LoRaモジュールにシリアル接続された端末から送られたテキストメッセージの平文を指定された周波数でLoRa変調により送信します。

※1[Arduino LoRa](https://github.com/sandeepmistry/arduino-LoRa/blob/master/README.md)をベースにLoRaのChannel Activity Detection機能を追加したものを用いています。

※2[APRS on LoRa](https://github.com/josefmtd/lora-aprs)のKISSモードTNCを流用しています。

## 通信方式
詳細な通信方式は[プロトコル](https://github.com/w-ockham/LoRaKissTNC/blob/master/Protocol.md)を参照して下さい。

## ハードウェア

 * [Semtech SX1276/77/78/79](https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1276)を使ったボードで動作します。:
   * [BSFrance LoRa32u4](https://bsfrance.fr/lora-long-range/1311-BSFrance-LoRa32u4-1KM-Long-Range-Board-Based-Atmega32u4-433MHz-LoRA-RA02-Module.html)

### Semtech SX1276/77/78/79

| Semtech SX1276/77/78/79 | Arduino |
| :---------------------: | :------:|
| VCC | 3.3V |
| GND | GND |
| SCK | SCK |
| MISO | MISO |
| MOSI | MOSI |
| NSS | 8 |
| NRESET | 4 |
| DIO0 | 7 |


`NSS`,`NRESET`,`DIO0`は`LoRa.setPins(csPin, resetPin, irqPin)`. で接続することが出来ます。ボードによってピン接続を変更して下さい。

## インストール方法
### Arduino IDEにLoRa32u4ボードを追加
1. ファイル→環境設定で、追加のボードマネージャのURLに以下を追加  
   `https://adafruit.github.io/arduino-board-index/package_adafruit_index.json`
2. ツール→ボードで`AdaFruit Feather32u4`を選択
3. ツール→書込装置で`ArduinoISP`を選択

### コンパイルと書き込み
1. LoRaKissTNC.inoを開く  
2. スケッチ→検証・コンパイルでコンパイル  
3. スケッチ→マイコンボードに書き込むでArduinoへ書き込み  

### スマートフォンとの接続
1. スマホに[USBシリアル端末](https://play.google.com/store/apps/details?id=jp.sugnakys.usbserialconsole&hl=ja)をインストール
2. [OTGケーブル](https://www.amazon.co.jp/dp/B012V56C8K)とmicroUSBケーブルを使ってLoRa32u4をスマホに接続
3. シリアルポートの設定で  
  ボーレート9600 データビット8 パリティnone ストップビット1 フロー制御off  
  を選択
4. 接続の設定で送信フォームの表示、改行コードCR+LFを選択

## KISSモードTNCモード
### APRSクライアントの設定
APRSクライアントとしてAndroid端末上で[APRSDroid](https://aprsdroid.org/)を用います。まずAPRSDroidの設定画面でコールサインを設定して下さい。次にLoRaトランシーバをOTGケーブルで接続し、設定画面で以下を設定してください。

1. 接続方式 TNC(KISS)を選択します
2. TNC初期化設定 初期化設定文字列にTNCの初期化文字列をURLエンコードした文字を設定します。
```
%0DKISS%2043851%2C3%2C8%2C8%2C10000%0D
```
これはTNCの以下の初期化文字列をURLエンコード(`%0D=改行 %20=スペース %2C=,`)したものです。
```
 （改行)KISS 43851,3,8,8,10000(改行)
```
初期化文字列の詳細については後述します。

3. 接続タイプ USBシリアルを設定します。
4. 機器通信速度 9600bpsを設定します。

以上で完了です。

ARPSDroidを起動するとTNCへAX.25形式のパケットが送られます。LoRa対応のi-gate局からデジピートされます。

### TNC初期化文字列について
KISSコマンドにより、LoRaモジュールをKISS TNCモードにします。
初期化文字列を用いてモジュールのパラメータを以下の通り指定できます。
```
  KISS <キャリア周波数(10kHz単位)> , <帯域> , <拡散率> , <コーディングレート>, <最大バックオフ時間(ms)>
  ```
各パラメータの詳細については「テキストチャット」モードのコマンドを参照して下さい。
#### キャリア周波数
 438.0MHz～439MHzまで10kHz単位で指定します。範囲外の周波数が指定された場合は送信出来ません。
#### 帯域
以下のBW値を指定します。デフォルト値は15.6kHzです。

LoRaの電波型式はF1Dとなります。430MHz帯では総務省告示第百二十五号`無線設備規則別表第二号第54の規定に基づくアマチュア局の無線設備の占有周波数帯幅の許容値`に基づき占有帯域幅30kHz以下となりますのでご注意下さい。

| BW値 | 帯域幅 |
|:-----|:-------|
|0 | 7.8kHz |
|1 | 10.4kHz |
|2 | 15.6kHz（デフォルト値) |
|3 | 20.8kHz |

#### 拡散率
拡散率を`6(2^6=64) ～ 12(2^12=4096)`の範囲で指定できます。
#### コーディングレート
コーディングレートを `5(4/5) ～ 8(4/8)`の範囲で指定できます。
#### 最大バックオフ時間
本プログラムでは送信前に所定時間(3sec)チャンネルのアクティビティを監視し、他局が送信をしていない場合に自局からの送信を行います。  
衝突が起きた場合には3秒からここで指定された時間の範囲でランダムに待ち時間を入れます。拡散率が高い場合は衝突が起きる可能性が高いので、フレーム送出時間と同程度の長めのバックオフ時間(BW=62.5,SF=11,CR=8で10000ms程度)を設定してください。

### APRSゲートウェイへのパッチ
LoRaKissTNCはKISSプロトコルで受信パケットのRSSI/SNRを返します。APRSゲートウェイ側でも表示できるように[APRX](https://github.com/PhirePhly/aprx/blob/master/README)のパッチを用意しました。
gitからクローン後(パッチはv2.9.0をベースにしています)、patchコマンドでパッチをあててください。

```
2020-01-14 16:41:53.564 JL1NIE-6  R(-55,11.50) JL1NIE-5>APDR15,WIDE1-1::JL1NIE-10:dx{23
```

aprf-rf.logの受信パケットを示すRフラグにRSSI/SNRが`R(RSSI,SNR)`の形式で入ります。
またログからビーコン用のメッセージパケットを作るスクリプトも用意しました。
自局位置についてはスクリプト埋め込みですので適宜修正してください。

```
#!/bin/sh
tac  /var/log/aprx/aprx-rf.log | grep " R(" | head -n 1 | gawk '{match($2,/(.+)\..+/,t);match($4,/R\((.+)\,(.+)\)/,a);match($5,/([A-Z0-9\-]+)>/,c);printf("!3536.15N/13931.24E-LoRa station:%s %s RSSI=%s SNR=%s\n",t[1],c[1],a[1],a[2]);}'
```

### APRSゲートウェイの設定
参考までにaprx.confの例を掲載します。
運用に際してはAPRS網への負荷等、他局への影響を十分考慮の上設定するようお願いします。
```
mycall  JS1YFC-6　#自局コールサイン
myloc lat 3536.05N lon 13931.22E　#自局位置

<aprsis>
        passcode 12345 #APRS-ISのパスワード
        server    rotate.aprs2.net
</aprsis>

<logging>
        pidfile /var/run/aprx.pid
        rflog /var/log/aprx/aprx-rf.log
        aprxlog /var/log/aprx/aprx.log
</logging>

<interface>
        serial-device /dev/ttyACM0  9600 8n1    KISS
        # TNCの初期文字列
        # Freq=438.51MHz BW=15.6kHz SF=8 CR=8 Backofftime=10000ms
        # <FEND><RET><FEND><CR>SET KISS 43851,3,8,8,10000<CR>
        initstring "\xc0\xff\xc0\x0dSET KISS 43851,3,8,8,10000\x0d"
        callsign     $mycall
        tx-ok        true
</interface>

<beacon>
        beaconmode aprsis
        cycle-size  5m
        #最後の受信局をビーコン表示
        beacon exec /usr/local/bin/aprxLastseen.sh
</beacon>

<digipeater>
        transmitter     $mycall
        <source>
                source        APRSIS
                relay-type    third-party
                ratelimit     240 480
                via-path      WIDE1-1
                msg-path      WIDE1-1
                filter -t/st
    </source>
</digipeater>
```
## テキストチャットモード
テキストチャットモードでは端末から入力された文字を行単位で平文で送信します。
行頭が`set`で始まる行はコマンド列として解釈されます。

まず交信の前にコールサインを設定して下さい。（コールサインを設定しないと送信出来ません）
```sh
 set call <あなたのコールサイン>
```

次にコマンドを使って運用周波数やLoRa変調のパラメータを設定して下さい。デフォルトでは周波数438.51MHz、SF=8、BW=15.6kHz 出力20dBm(100mW)の設定になっています。
```sh
set
Freq=43851
SF=8
BW=2　(15.6kHz)
TXpower=20
```
ターミナルからメッセージを入れ、最後に改行(CR/LF)を入力して下さい。
メッセージの送信が完了すると以下のように表示されます。
```sh
 <コールサイン> >: <送信したメッセージ>
```
相手局からメッセージを受信すると以下のように表示されます。
```sh
 <相手局コールサイン> (<RSSI値>,<SNR値>,<周波数エラー値>)<: <受信したメッセージ>
 ```

## コマンド

### 自局コールサインの指定
自局のコールサインを指定します。パケット先頭には必ず自局コールサインが入ります。
またコールサインが指定されていない場合送信できません。
```sh
set call コールサイン
```

### 周波数の設定
運用周波数を10kHz単位で指定します。
指定できる範囲は438MHz-439MHz(全電波形式の範囲)です。
デフォルト値は438.51MHzです。
```sh
 set freq 43851
```
### 送信出力の設定
送信出力をdBmで指定します。2dBm - 20dBmの範囲です。
デフォルト値は20dBmです。
```sh
 set pwr 20
```
### 拡散率(SF)の指定
拡散率(Spreading Factor)を指定します。6 - 12の範囲です。
  小さい値ほど高速に送信できますがSNRでは不利になります。
  デフォルト値は9です。
```sh
 set sf 9
```
### 帯域幅(BW)の指定
チャープスペクトラムの帯域幅を指定します。0-8の範囲です。帯域幅が広いほど高速に送信できますがSNRでは不利になります。

```sh
 set bw 2
 ```
| BW値 | 帯域幅 |
|:-----|:-------|
|0 | 7.8kHz |
|1 | 10.4kHz |
|2 | 15.6kHz (デフォルト値) |
|3 | 20.8kHz |

### 設定の確認
`set`コマンドでパラメータを指定しないと現在の設定値が表示されます。
```sh
Freq=43851
SF=9
BW=2　(15.6kHz)
TXpower=20
```

### 設定の初期化
設定をデフォルト値に戻します。
```sh
set init
```
# 変更申請について
本スケッチを用いたLoRaトランシーバの申請方法はこちらです。

[LoRaトランシーバの変更申請の届出方法](doc/申請方法.md)
