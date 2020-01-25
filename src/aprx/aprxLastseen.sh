#!/bin/sh
tac  /var/log/aprx/aprx-rf.log | grep " R(" | head -n 1 | gawk '{match($2,/(.+)\..+/,t);match($4,/R\((.+)\,(.+)\)/,a);match($5,/([A-Z0-9\-]+)>/,c);printf("!3536.15N/13931.24E-LoRa station:%s %s RSSI=%s SNR=%s\n",t[1],c[1],a[1],a[2]);}'

