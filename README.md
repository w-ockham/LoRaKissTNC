# LoRaKissTNC

[Arduino LoRa](https://github.com/sandeepmistry/arduino-LoRa/blob/master/README.md)を用いたKISSモードTNCです。
オリジナルは[APRS on LoRa](https://github.com/josefmtd/lora-aprs)のKISSモードTNCです。  

## 通信方式
[プロトコル](https://github.com/w-ockham/LoRaBot/blob/master/Protocol.md)を参照して下さい。

## ハードウェア
[LoRaBot](https://github.com/w-ockham/LoRaBot/blob/master/README.md)と同じ[BSFrance LoRa32u4](https://bsfrance.fr/lora-long-range/1311-BSFrance-LoRa32u4-1KM-Long-Range-Board-Based-Atmega32u4-433MHz-LoRA-RA02-Module.html)
を用います。

## インストール方法
### Arduino IDEでのコンパイル
Arduino LoRaをライブラリに追加したArduino IDEでLoRaKissTNCをコンパイルし、ボードにインストールしてください。

### APRSクライアントの設定
APRSクライアントとしてAndroid端末上で[APRSDroid](https://aprsdroid.org/)を用います。
LoRaトランシーバをOTGケーブルで接続後、設定画面で以下を設定してください。
1. 接続方式 TNC(KISS)を選択します
2. TNC初期化設定 初期化設定文字列にTNCの初期化文字列をURLエンコードした文字を設定します。
```
%0DSET%2043851%2C6%2C11%2C8%2C10000%0D
```
これはTNCの以下の初期化文字列をURLエンコード(`%0D=改行 %20=スペース %2C=,`)したものです。
```
 （改行)SET 4351,6,11,8,10000(改行)
```
初期化文字列の詳細については後述します。

3. 接続タイプ USBシリアルを設定します。
4. 機器通信速度 9600bpsを設定します。

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
        # Freq=438.51MHz BW=62.5kHz SF=11 CR=8 Backofftime=10000ms
        # <FEND><RET><FEND><CR>SET 43851,6,11,8,10000<CR>
        initstring "\xc0\xff\xc0\x0dSET 43851,7,11,8,10000\x0d"
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
## TNC初期化文字列について
初期化文字列は以下の通り指定できます。
```
  SET <キャリア周波数(10kHz単位)> , <帯域> , <拡散率> , <コーディングレート>, <最大バックオフ時間(ms)>
  ```
### キャリア周波数
 431.0MHz～439MHzまで10kHz単位で指定します。全電波形式の438MHz-439MHzを指定するようにしましょう。
### 帯域
以下のBW値を指定します。

| BW値 | 帯域幅 |
|:-----|:-------|
|0 | 7.8kHz |
|1 | 10.4kHz |
|2 | 15.6kHz |
|3 | 20.8kHz |
|4 | 31.25kHz |
|5 | 41.7kHz |
|6 | 62.5kHz |
|7 | 125kHz |
|8 | 250kHz |

### 拡散率
拡散率を`7(2^7=128) ～ 12(2^12=4096)`の範囲で指定できます。
### コーディングレート
コーディングレートを `5(4/5) ～ 8(4/8)`の範囲で指定できます。
### 最大バックオフ時間
本プログラムでは送信前に所定時間(3sec)チャンネルのアクティビティを監視し、他局が送信をしていない場合に自局からの送信を行います。  
衝突が起きた場合には3秒からここで指定された時間の範囲でランダムに待ち時間を入れます。拡散率が高い場合は衝突が起きる可能性が高いので、フレーム送出時間と同程度の長めのバックオフ時間(BW=62.5,SF=11,CR=8で10000ms程度)を設定してください。
