# Evil Maid Detector (Plan9)

This project is one I wrote for a class. It is written for the [plan9](https://9p.io/plan9/) platform.

It is a simple project which uses the following components:
* [Adafruit Triple-Axis Accelerometer - ±2/4/8g @ 14-bit - MMA8451](https://www.adafruit.com/product/2019)
* [Adafruit FONA 808 - Mini Cellular GSM + GPS Breakout](https://www.adafruit.com/product/2542)
* [Raspberry Pi](https://www.raspberrypi.org/)
* A battery and a power booster to power the pi without the need for a connection
to the wall power outlet

It tries to detect an [evil maid attack](https://security.stackexchange.com/questions/159173/what-exactly-is-an-evil-maid-attack) and is inspired by [Haven](https://www.wired.com/story/snowden-haven-app-turns-phone-into-home-security-system/).

The idea is that in order for an attacker to physically access your laptop, they
must first disturb anything which is on top of it. The accelerometer will pick
up motion and tilts and the cellular breakout will notify the owner of an
attempt via a text message using the FONA breakout. Of course, this will have
false alerts especially if you have a cat.

Compiling and Running
---
```
$ 5c emd.c
$ 5l emd.5
$ ./5.out 1234567890
```

Replacing `1234567890` with your own target cell phone number for notification

PFAQ (Possible Freqeuntly Asked Questions)
---
* **Why does it take so long to start up?**

   It takes some time ~1 second to start up the accelerometer and it takes ~500
   samples from the accelerometer to factor in noise.

* **Why does sending a text take so long?**

   It actually doesn't.
   I didn't have time to learn how libthread works in plan9 to check for return
   codes. So I use a failsafe of 5 seconds before and after sending commands to
   flush any messages.
