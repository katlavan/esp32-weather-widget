#include "ui.h"
#include "hw_display.h"
#include "config_private.h"
#include <LovyanGFX.hpp>
#include <math.h>

// ...

static lgfx::LGFX_Sprite canvas(&Display);   // <-- was lgfx::Sprite

// ---------- general utilities ----------
static inline uint16_t c(uint8_t r,uint8_t g,uint8_t b){ return lgfx::color565(r,g,b); }
static String ff(float v,int p=1){ char b[16]; dtostrf(v,0,p,b); return String(b); }

// ---------- animation state ----------
static struct {
  float sunAngle = 0.f;          // rotation of rays
  float windPulse = 0.f;         // slight "pulsation" of wind arrow
  float drip = 0.f;              // phase of drops/snow
  uint32_t last = 0;             // timepoint
} anim;

// ---------- background: smooth three-point gradient with slight vignette ----------
static void drawGradientBG(float tempC){
  uint16_t top, mid, bot;
  if (tempC <= 0)       { top=c(30,60,160);  mid=c(20,40,120);  bot=c(10,20,60);  }
  else if (tempC < 20)  { top=c(60,130,240); mid=c(30,85,180);  bot=c(16,40,90);  }
  else                  { top=c(255,165,80); mid=c(210,95,110); bot=c(110,45,95); }

  const int W = canvas.width(), H = canvas.height();
  for (int y=0; y<H; ++y) {
    float t = (float)y/(H-1);
    float t2 = t*2.f;
    uint16_t a = (t2 < 1.f) ? top : mid;
    uint16_t b = (t2 < 1.f) ? mid : bot;
    float k = (t2 < 1.f) ? t2 : (t2-1.f);
    auto ch=[&](uint16_t col,int sh,int mask,int mul){ return int(((col>>sh)&mask)*mul); };
    uint8_t r=(1-k)*ch(a,11,0x1F,8)+k*ch(b,11,0x1F,8);
    uint8_t g=(1-k)*ch(a, 5,0x3F,4)+k*ch(b, 5,0x3F,4);
    uint8_t b8=(1-k)*ch(a, 0,0x1F,8)+k*ch(b, 0,0x1F,8);
    canvas.drawFastHLine(0,y,W, lgfx::color565(r,g,b8));
  }
  // vignette
  for(int i=0;i<10;i++) {
    uint16_t col = c(0,0,0);
    canvas.drawRoundRect(1+i,1+i,W-2-2*i,H-2-2*i,14, col);
  }
}

// ---------- compact color icons with scaling + animations ----------
static void sunIcon(int cx,int cy,float s=0.72f){
  uint16_t core=c(255,210,80), glow=c(255,240,150);
  int r1=int(18*s), r2=int(24*s), rRay1=int(26*s), rRay2=int(34*s);
  canvas.fillCircle(cx,cy,r2,glow);
  canvas.fillCircle(cx,cy,r1,core);
  for(int i=0;i<12;i++){
    float a=anim.sunAngle + i*PI/6;
    canvas.drawLine(cx+cosf(a)*rRay1, cy+sinf(a)*rRay1,
                    cx+cosf(a)*rRay2, cy+sinf(a)*rRay2, glow);
  }
}
static void cloudBase(int x,int y,float s,uint16_t col){
  canvas.fillCircle(x-int(18*s),y+int(6*s), int(12*s),col);
  canvas.fillCircle(x,           y,         int(18*s),col);
  canvas.fillCircle(x+int(20*s), y+int(8*s),int(14*s),col);
  canvas.fillRoundRect(x-int(20*s), y+int(6*s), int(52*s), int(18*s), int(8*s), col);
}
static void cloudIcon(int x,int y,float s=0.72f){
  cloudBase(x,y,s,TFT_WHITE);
  canvas.drawRoundRect(x-int(22*s), y+int(4*s), int(56*s), int(22*s), int(10*s), c(210,210,210));
}
static void rainIcon(int x,int y,float s=0.72f){
  cloudIcon(x,y,s);
  for(int i=-int(10*s); i<=int(16*s); i+=int(10*s)){
    int dy=int( (sinf(anim.drip*PI*2 + i)*2) );
    canvas.drawLine(x+i, y+int(24*s)+dy, x+i-int(6*s), y+int(36*s)+dy, c(90,170,255));
  }
}
static void drizzleIcon(int x,int y,float s=0.72f){
  cloudIcon(x,y,s);
  for(int i=-int(10*s); i<=int(16*s); i+=int(10*s)){
    int dy=int( (sinf(anim.drip*PI*2 + i)*2) );
    canvas.fillCircle(x+i, y+int(30*s)+dy, 1, c(120,180,255));
  }
}
static void snowIcon(int x,int y,float s=0.72f){
  cloudIcon(x,y,s);
  for(int i=-int(10*s); i<=int(16*s); i+=int(10*s)){
    int dy=int( (sinf(anim.drip*PI*2 + i)*2) );
    canvas.drawLine(x+i, y+int(24*s)+dy, x+i, y+int(36*s)+dy, c(220,240,255));
    canvas.drawLine(x+i-int(3*s), y+int(30*s)+dy, x+i+int(3*s), y+int(30*s)+dy, c(220,240,255));
  }
}
static void thunderIcon(int x,int y,float s=0.72f){
  cloudIcon(x,y,s);
  canvas.fillTriangle(x+int(5*s),y+int(18*s), x-int(3*s),y+int(36*s), x+int(9*s),y+int(36*s), c(255,230,90));
  canvas.fillTriangle(x+int(9*s),y+int(22*s), x+int(1*s), y+int(40*s), x+int(13*s),y+int(40*s), c(255,230,90));
}
static void mistIcon(int x,int y,float s=0.72f){
  uint16_t col=c(230,230,240); for(int k=0;k<4;k++) canvas.drawFastHLine(x-int(28*s), y-int(8*s)+k*int(8*s), int(76*s), col);
}
static void iconWeather(int cx,int cy, WeatherType t, float s=0.72f){
  switch(t){ case WeatherType::Clear: sunIcon(cx,cy,s); break;
    case WeatherType::Clouds: cloudIcon(cx,cy,s); break;
    case WeatherType::Rain: rainIcon(cx,cy,s); break;
    case WeatherType::Drizzle: drizzleIcon(cx,cy,s); break;
    case WeatherType::Snow: snowIcon(cx,cy,s); break;
    case WeatherType::Thunder: thunderIcon(cx,cy,s); break;
    case WeatherType::Mist: default: mistIcon(cx,cy,s); break; }
}

// small pictograms
static void tinyIcon(int x,int y,const char* kind){
  if(!strcmp(kind,"hum")){
    canvas.fillTriangle(x,y-8, x-8,y+6, x+8,y+6, c(120,220,255));
    canvas.drawTriangle(x,y-8, x-8,y+6, x+8,y+6, c(200,255,255));
  } else if(!strcmp(kind,"press")){
    canvas.drawCircle(x,y,10,TFT_WHITE); canvas.drawLine(x,y, x+8,y-4, TFT_WHITE);
  } else if(!strcmp(kind,"wind")){
    canvas.drawLine(x-12,y, x+12,y, TFT_WHITE);
    canvas.fillTriangle(x+12,y, x+4,y-4, x+4,y+4, TFT_WHITE);
  }
}

static void degreeAt(int x,int y,int r=3,uint16_t col=TFT_WHITE){ canvas.drawCircle(x,y,r,col); }

// ---------- public ----------
void ui_begin(){
  Display.init();          
  Display.setRotation(3);  
  Display.setBrightness(150);

  canvas.setColorDepth(16);
  canvas.createSprite(Display.width(), Display.height());

  // smoke test: fill the screen with color once
  Display.fillScreen(TFT_BLACK);
}


bool ui_tick(){
  uint32_t now = millis();
  float dt = (now - anim.last) / 1000.0f;
  if (dt < 0.01f) return false;
  anim.last = now;

  anim.sunAngle += dt * 0.8f;              // ~0.8 rad/sec
  if (anim.sunAngle > PI*2) anim.sunAngle -= PI*2;

  anim.windPulse = 0.5f * sinf(now/800.0f);
  anim.drip += dt * 0.8f; if (anim.drip > 1.f) anim.drip -= 1.f;

  return true; // there is an update
}

void ui_render(const WeatherData& w){
  const int W = canvas.width();    // 320
  const int H = canvas.height();   // 170
  const int M = 10;                // general margin

  canvas.clear();
  drawGradientBG(w.temp);

  // ── Weather icon on the left (without card) ───────────────────────────
  const int iconW = 72;
  iconWeather(M + iconW/2, 48, w.type, 0.95f);

  // ── Right column ───────────────────────────────────────────────
  const int rx   = M + iconW + 12;
  const int colW = W - rx - M;

  // Temperature
  canvas.setTextColor(TFT_WHITE);
  canvas.setTextDatum(lgfx::top_left);
  canvas.setFont(&lgfx::fonts::DejaVu40);
  int tx = rx, ty = 10;
  String tStr = ff(w.temp,1);
  canvas.setCursor(tx, ty);
  canvas.print(tStr);

  // small "degree" symbol
  int degX = tx + canvas.textWidth(tStr) + 2;
  canvas.drawCircle(degX + 5, ty + 10, 3, TFT_WHITE);
  canvas.setFont(&lgfx::fonts::DejaVu24);
  canvas.setCursor(degX + 10, ty + 4);
  canvas.print( (OWM_UNITS[0]=='m') ? "C" : "F" );

  // Description
  canvas.setFont(&lgfx::fonts::DejaVu18);
  canvas.setCursor(rx, 58);
  canvas.print(w.description);

  // Feels / High–Low
  canvas.setFont(&lgfx::fonts::DejaVu12);
  canvas.setCursor(rx, 78);
  if (!isnan(w.tmin) && !isnan(w.tmax))
    canvas.printf("H:%s  L:%s", ff(w.tmax,0).c_str(), ff(w.tmin,0).c_str());
  else
    canvas.printf("Feels like %s", ff(w.feels,1).c_str());

  // Separator
  canvas.drawFastHLine(M, 100, W - 2*M, c(210,230,245));

  // ── Bottom metrics bar ─────────────────────────────────────────
  const int rowY = 122;
  const int segW = (W - 2*M) / 3;
  canvas.setFont(&lgfx::fonts::DejaVu12);   // ⬅️ font size reduced by one step

  // 1) Wind
  int x1 = M;
  tinyIcon(x1 + 6, rowY + 10, "wind");
  canvas.setCursor(x1 + 26, rowY + 2);
  canvas.printf("%s km/h", ff(w.windSpeed*3.6f,1).c_str());

  // 2) Humidity (shifted slightly to the right)
  int x2 = M + segW + 20;
  tinyIcon(x2 + 6, rowY + 10, "hum");
  canvas.setCursor(x2 + 26, rowY + 2);
  canvas.printf("%d%%", w.humidity);

  // 3) Pressure
  int x3 = M + 2*segW + 6;
  tinyIcon(x3 + 6, rowY + 10, "press");
  canvas.setCursor(x3 + 26, rowY + 2);
  canvas.printf("%d hPa", w.pressure);

  // ── Footer ────────────────────────────────────────────────────────
  canvas.setFont(&lgfx::fonts::DejaVu12);
canvas.setCursor(M, H - 14);

unsigned long minutesAgo = w.updated / 60;
if (minutesAgo < 1) {
  canvas.printf("%s  •  just now", w.location.c_str());
} else if (minutesAgo == 1) {
  canvas.printf("%s  •  1 minute ago", w.location.c_str());
} else {
  canvas.printf("%s  •  %lu minutes ago", w.location.c_str(), minutesAgo);
}
  // Output frame
  canvas.pushSprite(0,0);
}
