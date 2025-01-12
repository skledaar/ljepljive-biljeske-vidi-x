//potrebne datoteke za zaslon, grafike i touchscreen
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define TFT_CS 5    //pinovi za zaslon
#define TFT_DC 21
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define TS_CS 4     //pinovi za touchscreen
XPT2046_Touchscreen ts(TS_CS);

int x_piksel, y_piksel; //koordinate dodira
uint16_t boja;          //boja za crtanje po zaslonu
byte boja_bajt = 1;     //boja za zapis u memoriju
byte promjer = 3;       //debljina kista
byte slide = 0;         //sučelje

byte slike [2][160][240]; //pohrana nacrtanog

void setup() {
  tft.begin();        //inicijalizacija zaslona
  tft.setRotation(3); //orijentacija zaslona
  ts.begin();         //inicijalizacija touchscreena
  ts.setRotation(1);  //orijentacija touchscreena
  suceljecrno();      //početno crno sučelje
}
void loop() {
  if (ts.touched()) {     //ako je touchscreen dodirnut
    TS_Point p = ts.getPoint();             //točka dodira
    x_piksel = map(p.x, 370, 3900, 0, 319); //pretvaranje touchscreen koordinata u
    y_piksel = map(p.y, 210, 3800, 0, 239); //koordinate za crtanje po zaslonu
    
    if (x_piksel < 33) {      //ako je u lijevom djelu ekrana - izbornik
      promjer = 3;            //resetira debljinu kista
      if (y_piksel < 31) {    //odabir crvene/crne boje (ovisi o crnom/žutom sučelju)
        boja_bajt = 1;
      } else if (y_piksel < 61) { //odabir zelene/plave boje
        boja_bajt = 2;
      } else if (y_piksel < 91) { //odabir plave/crvene boje
        boja_bajt = 3;
      } else if (y_piksel < 121) {//odabir bijele/sive boje
        boja_bajt = 4;
      } else if (y_piksel < 151) {//odabir gumice
        promjer = 18;             //gumica je deblja od kista
        boja_bajt = 0;
      } else if (y_piksel > 210) {//resetiranje sučelja
        delay(1000);              //sigurnosna funkcija
        if (ts.touched()) {       //da nebi bilo slučajnih dodira
          suceljecrno();
          resetiraj(slide);
        }
        delay(250);
      } else if (y_piksel < 181) {
        if (slide == 0) {   //ako je crno sčelje
          suceljebijelo();  //mijenja sučelje u žuto
          slide = 1;
        } else {            //ako je žuto sučelje
          suceljecrno();    //mijenja sučelje u crno
          slide = 0;
        }
        redraw(slide);      //ispis vašeg crteža pohranjenog u memoriji
        boja_bajt = 1;      //resetira kist
        delay(250);         //štiti od rapidnih promjena kod dužih dodira
      } else if (y_piksel > 210) {
        delay(1000);        //sigurnosna funkcija
        if (ts.touched()) { //da nebi bilo slučajnih dodira
          if (slide == 0)
            suceljecrno();
          else
            suceljebijelo();
          resetiraj(slide);
        }
      }
    }
    
    if (x_piksel > 32) {
      tft.fillRect(x_piksel, y_piksel, promjer, promjer, vratiboju(boja_bajt,
                   slide));
      zapisi(x_piksel, y_piksel, boja_bajt, slide);
    }
  }
  delay(2);
}



//funkcije za pohranu i ponovno crtanje podataka
void zapisi(int x, int y, byte boja, byte br) {
  int kraj = 3;
  if (boja == 0) { //gumica
    boja = 0;
    kraj = 18;
  }
  
  for (int i = 0; i < kraj; i++) {
    for (int j = 0; j < kraj; j++) {
      int x_temp = (x + i) / 2;
      if ((x + i) % 2) {                        //LIJEVI pixel, npr. x=1 x=3
        //briše prva 4 b
        slike[br][x_temp][y + j] -= (slike[br][x_temp][y + j] / 16) * 16;
        slike[br][x_temp][y + j] += boja * 16; //sprema boju u "lijevi" dio bajta
      } else {                                  //DESNI pixel, npr. x=2 x=4
        //briše zadnja 4 b
        slike[br][x_temp][y + j] -= slike[br][x_temp][y + j] % 16;
        slike[br][x_temp][y + j] += boja;       //sprema boju u "desni" dio bajta
      }
    }
  }
}

void redraw(byte slide) {       //rekonstrukcija slike
  for (int i = 0; i < 320; i++) {
    for (int j = 0; j < 240; j++) {
      int b = vratibajt(slike[slide][i / 2][j], i);
      if (b != 0)
        tft.drawPixel(i, j, vratiboju(b, slide));
    }
  }
}

void resetiraj(byte slide) {    //reset gumb
  for (int i = 0; i < 160; i++)
    for (int j = 0; j < 240; j++)
      slike[slide][i][j] = 0;
}

uint16_t vratiboju(byte b, byte slide) {
  uint16_t kolor;
  if (slide == 0) {             //boje za crno sučelje
    switch (b) {
      case 1:
        return ILI9341_RED;
      case 2:
        return ILI9341_GREEN;
      case 3:
        return ILI9341_BLUE;
      case 4:
        return ILI9341_WHITE;
      case 0:
        return 0x0001;
    }
  } else {
    switch (b) {                //boje za žuto sučelje
      case 1:
        return 0x0002;
      case 2:
        return ILI9341_NAVY;
      case 3:
        return ILI9341_RED;
      case 4:
        return ILI9341_DARKGREY;
      case 0:
        return 0xFFA8;
    }
  }
}

byte vratibajt(byte b, int x) {
  if (x % 2)          //ako je piksel neparan
    return b / 16;    //vraća "lijevi" dio bajta, lijevi piksel
  return b % 16;      //vraća "desni" dio bajta, desni piksel
}

void suceljecrno() {  //crta crno sučelje
  tft.fillScreen(0x0001);
  tft.fillRect(0, 0, 32, 240, ILI9341_DARKGREY);
  tft.fillRect(0, 0, 30, 30, ILI9341_RED);
  tft.fillRect(0, 30, 30, 30, ILI9341_GREEN);
  tft.fillRect(0, 60, 30, 30, ILI9341_BLUE);
  tft.fillRect(0, 90, 30, 30, ILI9341_WHITE);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(4, 130);
  tft.print("Gumi");
  tft.setCursor(16, 160);
  tft.print(">");
  tft.setCursor(2, 220);
  tft.print("Reset");
}

void suceljebijelo() { //crta žuto sučelje
  tft.fillScreen(0xFFA8); //post-it boja
  tft.fillRect(0, 0, 32, 240, 0xFE88);
  tft.fillRect(0, 0, 30, 30, ILI9341_BLACK);
  tft.fillRect(0, 30, 30, 30, ILI9341_NAVY);
  tft.fillRect(0, 60, 30, 30, ILI9341_RED);
  tft.fillRect(0, 90, 30, 30, ILI9341_DARKGREY);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(4, 130);
  tft.print("Gumi");
  tft.setCursor(9, 160);
  tft.print("<");
  tft.setCursor(2, 220);
  tft.print("Reset");
}
