/*
  Name:		Arduino_DS1302_DS1B820.ino
  Created:	18.04.2018 00:31:53
  Author:	strz3
*/

#include <TimerOne.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <virtuabotixRTC.h>

#define BUTON_YUKARI	    1
#define BUTON_SET		      2
#define BUTON_ASAGI		    3
#define BUTON_YOK		      0

#define ZAMAN_MOD_DAKIKA	0
#define ZAMAN_MOD_SAAT		1
#define ZAMAN_MOD_GUNN		2
#define ZAMAN_MOD_GUN		  3
#define ZAMAN_MOD_AY		  4
#define ZAMAN_MOD_YIL		  5

#define SEG_G			        10
#define SEG_F			        9
#define SEG_E			        11
#define SEG_D			        12
#define SEG_C			        6
#define SEG_B			        7
#define SEG_A			        8
#define SEG_DP			      18
#define SEG_CN			      13

#define ANOT1			        16
#define ANOT2			        15
#define ANOT3			        14
#define ANOT4			        17

#define DISPLAY_SAYISI	  4

#define DS18B20_DATA	    2
#define DS1302_CLK		    3
#define DS1302_IO		      4
#define DS1302_CE		      5
#define TIMER_1MSN		    1000

#define DERECE_ISARETI	  10
#define EKSI_ISARETI	    11
#define DISPLAY_KAPAT	    12
#define DISPLAY_D		      13
#define DISPLAY_S		      14
#define DISPLAY_G		      15
#define DISPLAY_A		      16
#define DISPLAY_Y		      17
#define DISPLAY_H		      18
#define NORMAL_MOD		    0
#define AYAR_MODU		      1

const uint8_t RAKAMLAR[] = {
  0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x1F, 0x70, 0x7F, 0x73, 0x63, 0x01, 0x00, 0x3D, 0x5A, 0x5E, 0x77, 0x3B, 0x37
};


const uint8_t segment[] = { SEG_G, SEG_F, SEG_E, SEG_D, SEG_C, SEG_B, SEG_A };
const uint8_t anot[] = { ANOT1, ANOT2, ANOT3, ANOT4 };
boolean donusumTablosuKullan = true;
int tekNokta = -1;
int durumCN = -1;
uint8_t displayData[DISPLAY_SAYISI];
//DS18B20
OneWire onewire(DS18B20_DATA);
DallasTemperature sensor(&onewire);
//DS1302
virtuabotixRTC myRTC(DS1302_CLK, DS1302_IO, DS1302_CE);

uint8_t saniye;
uint8_t calismaModu = NORMAL_MOD;

void setup() {
  Serial.begin(9600);
  pinMode(SEG_CN, OUTPUT);
  pinMode(SEG_DP, OUTPUT);
  for (uint8_t i = 0; i < DISPLAY_SAYISI; i++)
    pinMode(anot[i], OUTPUT);
  for (uint8_t i = 0; i < 7; i++)
    pinMode(segment[i], OUTPUT);
  Timer1.initialize(TIMER_1MSN);
  Timer1.attachInterrupt(timer1Isr, TIMER_1MSN);			//1msn aral�klarla timer1ISR �al���r
  sensor.begin();
}

void loop() {
  boolean ayarModu = false;
  switch (calismaModu)
  {
    case NORMAL_MOD:
      switch (saniye)
      {
        case 0:
          animasyonMod2();
          saatGoster();
          break;
        case 40:
          durumCN = 0;
          animasyonMod1();
          dereceOlc();
          break;
      }
      break;
    case AYAR_MODU:
      myRTC.updateTime();
      uint8_t saat, dakika, saniye, gun, ay, haftaninGunu, _yil;
      int yil;
      saniye = myRTC.seconds;
      dakika = myRTC.minutes;
      saat = myRTC.hours;
      haftaninGunu = myRTC.dayofweek;
      gun = myRTC.dayofmonth;
      ay = myRTC.month;
      yil = myRTC.year;
      dakika = zamanAyarla(ZAMAN_MOD_DAKIKA, dakika, 0, 59);
      saat = zamanAyarla(ZAMAN_MOD_SAAT, saat, 0, 23);
      haftaninGunu = zamanAyarla(ZAMAN_MOD_GUNN, haftaninGunu, 1, 7);
      gun = zamanAyarla(ZAMAN_MOD_GUN, gun, 0, 31);
      ay = zamanAyarla(ZAMAN_MOD_AY, ay, 1, 12);
      _yil = zamanAyarla(ZAMAN_MOD_YIL, yil % 100, 0, 99);
      int temp = yil % 100;
      yil -= temp;
      yil += _yil;
      myRTC.setDS1302Time(saniye, dakika, saat, haftaninGunu, gun, ay, yil);
      calismaModu = NORMAL_MOD;
      saatGoster();
      saniye = 0;
      break;
  }
}

uint8_t zamanAyarla(uint8_t zamanMod, uint8_t zaman, uint8_t minDeger, uint8_t maxDeger) {
  boolean ayarlaniyor = true;
  uint8_t _zaman = 0;
  durumCN = 1;
  delay(250);
  while (ayarlaniyor)
  {
    uint8_t buton = butonTara();
    switch (buton)
    {
      case BUTON_YOK:
        break;
      case BUTON_YUKARI:
        delay(250);
        if (zaman == maxDeger)
          zaman = minDeger;
        else
          zaman++;
        break;
      case BUTON_SET:
        delay(250);
        durumCN = 0;
        return zaman;
      case BUTON_ASAGI:
        delay(250);
        if (zaman == minDeger)
          zaman = maxDeger;
        else
          zaman--;
        break;
      default:
        break;
    }

    if (_zaman != zaman) {
      zamanGuncelle(zaman, zamanMod);
      _zaman = zaman;
    }
  }
}

void zamanGuncelle(uint8_t zaman, uint8_t zamanMod) {
  uint8_t zamanDizi[DISPLAY_SAYISI];
  zamanDizi[1] = DISPLAY_KAPAT;
  switch (zamanMod) {
    case ZAMAN_MOD_DAKIKA:
      zamanDizi[0] = DISPLAY_D;
      break;
    case ZAMAN_MOD_SAAT:
      zamanDizi[0] = DISPLAY_S;
      break;
    case ZAMAN_MOD_GUNN:
      zamanDizi[0] = DISPLAY_H;
      zamanDizi[1] = DISPLAY_G;
      break;
    case ZAMAN_MOD_GUN:
      zamanDizi[0] = DISPLAY_G;
      break;
    case ZAMAN_MOD_AY:
      zamanDizi[0] = DISPLAY_A;
      break;
    case ZAMAN_MOD_YIL:
      zamanDizi[0] = DISPLAY_Y;
      break;
  }
  zamanDizi[3] = zaman % 10;
  zamanDizi[2] = zaman / 10;
  rakamDonusumunuYap(zamanDizi);
}

uint8_t butonTara() {
  int adc = analogRead(5);
  if (adc > 800)
    return BUTON_YOK;
  else if (adc > 600)
    return BUTON_ASAGI;
  else if (adc > 450)
    return BUTON_YUKARI;
  else if (adc > 250)
    return BUTON_SET;
  else
    return BUTON_YOK;
}

void rakamlarinaAyir(int sayi, uint8_t* rakamlar) {
  int sira = DISPLAY_SAYISI - 1;
  for (int i = 0; i < DISPLAY_SAYISI; i++) {
    rakamlar[sira - i] = sayi % 10;
    sayi -= rakamlar[sira - i];
    sayi /= 10;
  }
}

void rakamDonusumunuYap(uint8_t* rakamlar) {
  for (int i = 0; i < DISPLAY_SAYISI; i++)
    displayData[i] = RAKAMLAR[rakamlar[i]];
}

void dereceOlc() {
  static uint8_t dereceDizi[DISPLAY_SAYISI] = { 2, 5, 0, DERECE_ISARETI };
  tekNokta = 1;
  static int _dereceInt = 0;
  rakamDonusumunuYap(dereceDizi);
  sensor.requestTemperatures();
  float derece = sensor.getTempCByIndex(0);
  int dereceInt = derece * 100;
  if (abs(dereceInt - _dereceInt) > 20) {
    rakamlarinaAyir(dereceInt, dereceDizi);
    dereceDizi[DISPLAY_SAYISI - 1] = DERECE_ISARETI;
    if (dereceInt < 1000)
      dereceDizi[0] = DISPLAY_KAPAT;
    if (dereceInt < 0) {
      if (dereceInt > -1000) {
        dereceDizi[0] = EKSI_ISARETI;
      }
      else
      {
        dereceDizi[2] = dereceDizi[1];
        dereceDizi[1] = dereceDizi[0];
        dereceDizi[0] = EKSI_ISARETI;
        tekNokta = -1;
      }
    }
    rakamDonusumunuYap(dereceDizi);
    _dereceInt = dereceInt;
  }
}

void saatGoster() {
  myRTC.updateTime();
  uint8_t zaman[DISPLAY_SAYISI];
  uint8_t saat, dakika;
  saat = myRTC.hours;
  dakika = myRTC.minutes;
  zaman[1] = saat % 10;
  zaman[3] = dakika % 10;
  zaman[0] = saat / 10;
  zaman[2] = dakika / 10;
  if (zaman[0] == 0)
    zaman[0] = DISPLAY_KAPAT;
  rakamDonusumunuYap(zaman);
  durumCN = 2;
}


void timer1Isr() {
  static uint8_t displaySirasi = 0;
  static unsigned int timerKesmeSay = 0;
  uint8_t data;
  for (uint8_t i = 0; i < DISPLAY_SAYISI; i++)
    digitalWrite(anot[i], LOW);
  data = displayData[displaySirasi];
  for (uint8_t i = 0; i < 7; i++) {
    digitalWrite(segment[i], bitRead(data, i));
  }
  if (tekNokta == displaySirasi)
    digitalWrite(SEG_DP, HIGH);
  else
    digitalWrite(SEG_DP, LOW);
  digitalWrite(anot[displaySirasi], HIGH);
  displaySirasi++;
  if (displaySirasi >= DISPLAY_SAYISI)
    displaySirasi = 0;

  if (timerKesmeSay == 1000) {
    timerKesmeSay = 0;
    saniye++;
    if (saniye > 59)
      saniye = 0;
  }

  if (durumCN == 0) {
    digitalWrite(SEG_CN, LOW);

  }
  else if (durumCN == 1) {
    digitalWrite(SEG_CN, HIGH);
  }
  else if (durumCN == 2) {
    if (timerKesmeSay == 0)
      digitalWrite(SEG_CN, digitalRead(SEG_CN) ^ 1);
  }
  timerKesmeSay++;
  if (calismaModu == NORMAL_MOD) {
    int adc = analogRead(5);
    if (adc < 400)
      calismaModu = AYAR_MODU;
  }
}

void animasyonMod1() {
  tekNokta = -1;
  uint8_t anim[] = { 0x40, 0x41, 0x49, 0x09, 0x08, 0x00 };
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < DISPLAY_SAYISI; j++) {
      displayData[j] = anim[i];
    }
    delay(250);
  }
}

void animasyonMod2() {
  uint8_t anim[][4] {
    { 0x00, 0x10, 0x04, 0x00 }, { 0x00, 0x18, 0x0C, 0x00 }, { 0x00, 0x1C, 0x1C, 0x00 }, { 0x00, 0x1E, 0x3C, 0x00 }, { 0x00, 0x5E, 0x7C, 0x00 }, { 0x00, 0x7E, 0x7E, 0x00 }, { 0x00, 0x7F, 0x7F, 0x00 },
    { 0x01, 0x7F, 0x7F, 0x01 }, { 0x03, 0x7F, 0x7F, 0x21 }, { 0x43, 0x7F, 0x7F, 0x61 }, { 0x63, 0x7F, 0x7F, 0x63 }, { 0x73, 0x7F, 0x7F, 0x67 }, { 0x7B, 0x7F, 0x7F, 0x6F }, { 0x7F, 0x7F, 0x7F, 0x7F },
    { 0x7F, 0x6F, 0x7B, 0x7F }, { 0x7F, 0x67, 0x73, 0x7F }, { 0x7F, 0x63, 0x63, 0x7F }, { 0x7F, 0x61, 0x43, 0x7F }, { 0x7F, 0x21, 0x03, 0x7F }, { 0x7F, 0x01, 0x01, 0x7F }, { 0x7F, 0x00, 0x00, 0x7F },
    { 0x7E, 0x00, 0x00, 0x7E }, { 0x7C, 0x00, 0x00, 0x5E }, { 0x3C, 0x00, 0x00, 0x1E }, { 0x1C, 0x00, 0x00, 0x1C }, { 0x0C, 0x00, 0x00, 0x18 }, { 0x04, 0x00, 0x00, 0x10 }, { 0x00, 0x00, 0x00, 0x00 }
  };
  tekNokta = -1;
  for (int i = 0; i < 28; i++) {
    for (int j = 0; j < DISPLAY_SAYISI; j++) {
      displayData[j] = anim[i][j];
    }
    delay(100);
  }

}
