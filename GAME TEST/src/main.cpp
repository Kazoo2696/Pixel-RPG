#define UNICODE
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include <gdiplus.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#pragma comment(lib, "gdiplus.lib")

namespace {
constexpr int SW = 320, SH = 180;
constexpr float PI = 3.14159265f;
struct V { float x{}, y{}; };
V operator+(V a,V b){return {a.x+b.x,a.y+b.y};} V operator-(V a,V b){return {a.x-b.x,a.y-b.y};}
V operator*(V a,float s){return {a.x*s,a.y*s};} float len(V a){return std::sqrt(a.x*a.x+a.y*a.y);}
V norm(V a){float l=len(a);return l>.01f?a*(1.f/l):V{};} float dist(V a,V b){return len(a-b);}
uint32_t hash2d(int x,int y){uint32_t h=(uint32_t)(x*374761393u+y*668265263u);h=(h^(h>>13))*1274126177u;return h^(h>>16);}

struct Canvas {
 std::array<uint32_t,SW*SH> px{};
 void clear(uint32_t c){px.fill(c);}
 void p(int x,int y,uint32_t c){if((unsigned)x<SW&&(unsigned)y<SH)px[y*SW+x]=c;}
 void rect(int x,int y,int w,int h,uint32_t c){for(int yy=std::max(0,y);yy<std::min(SH,y+h);++yy)for(int xx=std::max(0,x);xx<std::min(SW,x+w);++xx)p(xx,yy,c);}
 void line(int x0,int y0,int x1,int y1,uint32_t c){int dx=abs(x1-x0),sx=x0<x1?1:-1,dy=-abs(y1-y0),sy=y0<y1?1:-1,e=dx+dy;for(;;){p(x0,y0,c);if(x0==x1&&y0==y1)break;int e2=2*e;if(e2>=dy){e+=dy;x0+=sx;}if(e2<=dx){e+=dx;y0+=sy;}}}
 void box(int x,int y,int w,int h,uint32_t fill,uint32_t edge){rect(x,y,w,h,edge);rect(x+1,y+1,w-2,h-2,fill);}
};
// A 32-bit BI_RGB DIB stores pixels as 0x00RRGGBB (B, G, R in memory).
constexpr uint32_t C(int r,int g,int b){return (uint32_t)r<<16|(uint32_t)g<<8|(uint32_t)b;}
const uint32_t GRASS=C(48,122,67), GRASS2=C(59,143,75), DARK=C(19,33,39), WHITE=C(241,233,196);

struct Texture {
 int w=0,h=0; std::vector<uint32_t> pixels;
 bool load(const std::wstring& path){
  Gdiplus::Bitmap image(path.c_str()); if(image.GetLastStatus()!=Gdiplus::Ok)return false;
  w=(int)image.GetWidth();h=(int)image.GetHeight();pixels.resize((size_t)w*h);
  Gdiplus::Rect area(0,0,w,h);Gdiplus::BitmapData data{};
  if(image.LockBits(&area,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&data)!=Gdiplus::Ok)return false;
  for(int y=0;y<h;++y){auto* row=(uint32_t*)((uint8_t*)data.Scan0+y*data.Stride);for(int x=0;x<w;++x)pixels[(size_t)y*w+x]=row[x];}
  image.UnlockBits(&data);return true;
 }
 void draw(Canvas& c,int dx,int dy,int sx,int sy,int sw,int sh,bool flip=false)const{
  if(pixels.empty())return;for(int y=0;y<sh;++y)for(int x=0;x<sw;++x){int tx=flip?sx+sw-1-x:sx+x;if(tx<0||tx>=w||sy+y<0||sy+y>=h)continue;uint32_t s=pixels[(size_t)(sy+y)*w+tx];int a=(int)(s>>24);if(!a)continue;int px=dx+x,py=dy+y;if((unsigned)px>=SW||(unsigned)py>=SH)continue;uint32_t d=c.px[py*SW+px];int sr=(s>>16)&255,sg=(s>>8)&255,sb=s&255;int dr=(d>>16)&255,dg=(d>>8)&255,db=d&255;c.px[py*SW+px]=C((sr*a+dr*(255-a))/255,(sg*a+dg*(255-a))/255,(sb*a+db*(255-a))/255);}
 }
 void drawScaledChroma(Canvas& c,int dx,int dy,int dw,int dh,int sx,int sy,int sw,int sh,bool flip=false)const{
  if(pixels.empty()||dw<=0||dh<=0)return;for(int y=0;y<dh;++y)for(int x=0;x<dw;++x){int ux=x*sw/dw,uy=y*sh/dh;int tx=flip?sx+sw-1-ux:sx+ux,ty=sy+uy;if(tx<0||tx>=w||ty<0||ty>=h)continue;uint32_t s=pixels[(size_t)ty*w+tx];int sr=(s>>16)&255,sg=(s>>8)&255,sb=s&255;if(sr<=6&&sg<=6&&sb<=6)continue;int px=dx+x,py=dy+y;if((unsigned)px<SW&&(unsigned)py<SH)c.px[py*SW+px]=C(sr,sg,sb);}
 }
};

// Compact 3x5 glyphs: digits then uppercase alphabet.
const char* glyph(char c){
 static const char* d[]={"111101101101111","010110010010111","111001111100111","111001111001111","101101111001001","111100111001111","111100111101111","111001001001001","111101111101111","111101111001111"};
 static const char* a[]={"010101111101101","110101110101110","011100100100011","110101101101110","111100110100111","111100110100100","011100101101011","101101111101101","111010010010111","001001001101010","101101110101101","100100100100111","101111111101101","110101101101101","010101101101010","110101110100100","010101101111011","110101110101101","011100010001110","111010010010010","101101101101111","101101101101010","101101111111101","101101010101101","101101010010010","111001010100111"};
 if(c>='0'&&c<='9')return d[c-'0']; if(c>='A'&&c<='Z')return a[c-'A']; return nullptr;
}
void text(Canvas&g,int x,int y,const std::string&s,uint32_t col,int scale=1){for(char ch:s){if(ch==' '){x+=4*scale;continue;}auto q=glyph((char)toupper((unsigned char)ch));if(q)for(int yy=0;yy<5;++yy)for(int xx=0;xx<3;++xx)if(q[yy*3+xx]=='1')g.rect(x+xx*scale,y+yy*scale,scale,scale,col);x+=4*scale;}}

struct Enemy{V p,v;int hp=3;float hit=0,think=0;bool alive=true;int type=0;};
struct Drop{V p;bool got=false;};
struct Game{
 Canvas g; Texture heroSprite,slimeSprite,skeleton2Sprite,grassTile,chestSprite,chest2Sprite,wallsTile,doorSprite,woodTile,rock1,rock2,rock3,dustSprite; V hero{160,145},face{0,-1}; std::vector<Enemy> enemies;std::vector<Drop>drops;std::mt19937 rng{77};
 int hp=5,shards=0,map=0;float attack=0,dash=0,inv=0,time=0,teleport=0;bool chest=false,chest2=false,win=false,lose=false;std::array<bool,256>key{},prev{};
 Game(){reset();}
 bool loadAssets(){wchar_t path[MAX_PATH]{};GetModuleFileNameW(nullptr,path,MAX_PATH);std::wstring p(path);auto cut=p.find_last_of(L"\\/");p=(cut==std::wstring::npos?L"":p.substr(0,cut+1))+L"assets\\mysticwoods\\";bool ok=true;ok&=heroSprite.load(p+L"player.png");ok&=slimeSprite.load(p+L"slime.png");ok&=skeleton2Sprite.load(p+L"skeleton2.png");ok&=grassTile.load(p+L"grass.png");ok&=chestSprite.load(p+L"chest_01.png");ok&=chest2Sprite.load(p+L"chest_02.png");ok&=wallsTile.load(p+L"walls.png");ok&=doorSprite.load(p+L"wooden_door.png");ok&=woodTile.load(p+L"wooden.png");ok&=rock1.load(p+L"rock_01.png");ok&=rock2.load(p+L"rock_03.png");ok&=rock3.load(p+L"rock_05.png");ok&=dustSprite.load(p+L"dust_particles_01.png");return ok;}
 void reset(){hero={160,145};face={0,-1};hp=5;shards=0;map=0;attack=dash=inv=time=teleport=0;chest=chest2=win=lose=false;drops.clear();enemies={{{73,76}},{{114,104}},{{207,92}},{{244,125}},{{177,61}},{{85,126},{},5,0,0,true,3},{{236,45},{},5,0,0,true,3}};}
 void enterMoonRuins(){map=1;hero={160,148};face={0,-1};attack=dash=0;inv=1.f;teleport=1.f;drops.clear();enemies={{{55,55},{},4,0,0,true,1},{{104,88},{},4,0,0,true,1},{{215,72},{},4,0,0,true,1},{{265,116},{},4,0,0,true,1},{{160,48},{},6,0,0,true,2},{{78,132},{},5,0,0,true,3},{{248,142},{},5,0,0,true,3}};}
 bool pressed(int k){return key[k]&&!prev[k];}
 bool wall(V p){if(p.x<14||p.x>306||p.y<22||p.y>169)return true;if(map==0&&p.y<53&&(p.x<133||p.x>187))return true;if(map==1&&((p.x>115&&p.x<137&&p.y>63&&p.y<112)||(p.x>184&&p.x<206&&p.y>63&&p.y<112)))return true;return false;}
 void update(float dt){time+=dt;teleport=std::max(0.f,teleport-dt);if(win||lose){if(pressed('R'))reset();prev=key;return;} attack=std::max(0.f,attack-dt);dash=std::max(0.f,dash-dt);inv=std::max(0.f,inv-dt);
  V in{float(key['D']||key[VK_RIGHT])-float(key['A']||key[VK_LEFT]),float(key['S']||key[VK_DOWN])-float(key['W']||key[VK_UP])};if(len(in)>0){in=norm(in);face=in;}
  if((pressed('K')||pressed('X')||pressed(VK_SHIFT))&&dash<=0)dash=.24f;
  float speed=dash>0?150.f:63.f;V np=hero+in*(speed*dt);if(!wall({np.x,hero.y}))hero.x=np.x;if(!wall({hero.x,np.y}))hero.y=np.y;
  if((pressed('J')||pressed('Z')||pressed(VK_SPACE))&&attack<=0){attack=.25f;V hitp=hero+face*13.f;for(auto&e:enemies)if(e.alive&&dist(e.p,hitp)<18){e.hp--;e.hit=.15f;e.v=e.v+face*90.f;if(e.hp<=0){e.alive=false;if(e.type==0)drops.push_back({e.p});}}}
  for(auto&e:enemies)if(e.alive){e.hit=std::max(0.f,e.hit-dt);e.think-=dt;V to=hero-e.p;if(e.think<0){std::uniform_real_distribution<float>u(-1,1);float chase=e.type?34.f:25.f,wander=e.type?18.f:13.f;e.v=dist(e.p,hero)<(e.type?96.f:72.f)?norm(to)*chase:norm(V{u(rng),u(rng)})*wander;e.think=e.type?.5f:.7f;}e.v=e.v*.96f;V ep=e.p+e.v*dt;if(!wall(ep))e.p=ep;if(dist(e.p,hero)<(e.type?12.f:10.f)&&inv<=0&&dash<=0){hp--;inv=1.f;hero=hero+norm(hero-e.p)*13.f;if(hp<=0)lose=true;}}
  for(auto&d:drops)if(!d.got&&dist(d.p,hero)<10){d.got=true;shards++;}
  if(map==0&&(pressed('E')||pressed(VK_RETURN))&&dist(hero,{267,63})<22&&!chest){chest=true;hp=std::min(5,hp+2);}
  if(map==0&&(pressed('E')||pressed(VK_RETURN))&&dist(hero,{52,116})<22&&!chest2){chest2=true;hp=std::min(5,hp+1);}
  if(map==0&&(pressed('E')||pressed(VK_RETURN))&&dist(hero,{160,37})<24&&shards>=3)enterMoonRuins();
  if(map==1){bool any=false;for(const auto&e:enemies)if(e.alive)any=true;if(!any)win=true;}prev=key;
 }
 void tree(int x,int y,int kind=0){if(kind==3){g.rect(x-5,y-2,11,5,C(61,67,64));g.rect(x-3,y-4,7,5,C(104,109,98));g.p(x+2,y-3,C(151,150,125));return;}g.rect(x-2,y+4,5,10,C(83,55,38));g.rect(x-1,y+4,2,9,C(133,83,48));uint32_t edge=C(20,61,38),dark=C(29,91,48),mid=C(43,128,58),light=C(74,157,70);int w=kind==1?14:20,h=kind==2?17:14;g.rect(x-w/2,y-h+4,w,h-3,edge);g.rect(x-w/2+2,y-h+2,w-4,h-3,dark);g.rect(x-w/2+4,y-h+1,w-7,h-5,mid);g.rect(x-w/2+6,y-h+1,5,3,light);g.p(x-w/2+1,y-4,GRASS);g.p(x+w/2-2,y-5,GRASS);}
 void slime(const Enemy&e){int x=(int)e.p.x,y=(int)e.p.y;if(!slimeSprite.pixels.empty()){int dir=std::abs(e.v.x)>std::abs(e.v.y)?1:(e.v.y<0?2:0);int row=(e.hit>0?9:(len(e.v)>3?3:0))+dir;int count=e.hit>0?4:(row<3?4:6);int frame=(int)(time*(e.hit>0?14.f:8.f)+e.p.x*.03f)%count;slimeSprite.draw(g,x-16,y-23,frame*32,row*32,32,32,e.v.x<0);return;}g.rect(x-6,y-2,12,7,C(35,78,44));g.rect(x-5,y-5,10,9,C(91,190,63));}
 void wraith(const Enemy&e){int x=(int)e.p.x,y=(int)e.p.y+(int)std::sin(time*5.f+e.p.x)*2,sz=e.type==2?9:7;uint32_t edge=C(23,20,48),body=e.hit>0?C(235,224,255):(e.type==2?C(171,67,220):C(85,97,205)),glow=e.type==2?C(244,102,220):C(91,218,239);g.rect(x-sz,y-sz,sz*2+1,sz+7,edge);g.rect(x-sz+2,y-sz+1,sz*2-3,sz+5,body);g.rect(x-sz+3,y+4,3,4,body);g.rect(x+sz-5,y+4,3,4,body);g.p(x-3,y-2,glow);g.p(x+3,y-2,glow);g.p(x-2,y-1,WHITE);g.p(x+4,y-1,WHITE);for(int i=0;i<3;++i){float a=time*2.5f+i*2.09f;g.p(x+(int)(std::cos(a)*(sz+3)),y+(int)(std::sin(a)*(sz+3)),glow);}}
 void skeleton2(const Enemy&e){int frame=(int)(time*7.f+e.p.x*.02f)%6,row;if(e.hit>0)row=6;else if(dist(e.p,hero)<20)row=e.v.x<0?4:5;else if(std::abs(e.v.y)>std::abs(e.v.x))row=e.v.y<0?0:1;else row=e.v.x<0?2:3;int fw=157,fh=186,sx=frame*fw,sy=row*fh;if(sx+fw>skeleton2Sprite.w)sx=std::max(0,skeleton2Sprite.w-fw);if(sy+fh>skeleton2Sprite.h)sy=std::max(0,skeleton2Sprite.h-fh);skeleton2Sprite.drawScaledChroma(g,(int)e.p.x-18,(int)e.p.y-30,36,43,sx,sy,fw,fh);}
 void enemyHealthBar(const Enemy&e){
  int maxHp=e.type==2?6:(e.type==3?5:(e.type==1?4:3));
  int w=20,h=4,x=(int)e.p.x-w/2,y=(int)e.p.y-(e.type==3?34:18);
  int fill=std::clamp(e.hp,0,maxHp)*(w-2)/maxHp;
  uint32_t color=e.hp*2>maxHp?C(73,201,91):(e.hp*4>maxHp?C(239,181,55):C(226,68,68));
  g.rect(x,y,w,h,C(13,20,24));
  g.rect(x+1,y+1,w-2,h-2,C(78,39,43));
  if(fill>0)g.rect(x+1,y+1,fill,h-2,color);
 }
 void heroDraw(){int x=(int)hero.x,y=(int)hero.y;if(inv>0&&((int)(time*15)&1))return;if(!heroSprite.pixels.empty()){bool moving=key['A']||key['D']||key['W']||key['S']||key[VK_LEFT]||key[VK_RIGHT]||key[VK_UP]||key[VK_DOWN];int dir=std::abs(face.x)>std::abs(face.y)?1:(face.y<0?2:0);int row=(attack>0?6:(moving?3:0))+dir;int frame=(int)(time*(attack>0?14.f:9.f))%6;heroSprite.draw(g,x-24,y-31,frame*48,row*48,48,48,face.x<-.2f);return;}g.rect(x-4,y+5,3,5,C(36,57,76));g.rect(x+2,y+5,3,5,C(36,57,76));g.rect(x-6,y-2,12,9,C(24,116,133));g.rect(x-4,y-7,8,7,C(33,163,169));g.rect(x-2,y-4,5,5,C(232,181,139));g.p(x+2,y-2,DARK);}
 void renderMoonRuins(){g.clear(C(24,24,54));for(int ty=14;ty<SH;ty+=16)for(int tx=0;tx<SW;tx+=16){uint32_t h=hash2d(tx/16+90,ty/16+40);uint32_t c=(h%4==0)?C(38,39,79):C(31,32,68);g.rect(tx,ty,16,16,c);if(h%3==0)g.p(tx+(int)(h%13)+1,ty+(int)((h>>7)%13)+1,C(83,75,135));}
  // Broken moon temple, pillars and glowing cracks.
  g.rect(115,63,22,49,C(17,18,42));g.rect(118,66,16,43,C(74,72,105));g.rect(121,69,10,37,C(103,99,132));g.rect(184,63,22,49,C(17,18,42));g.rect(187,66,16,43,C(74,72,105));g.rect(190,69,10,37,C(103,99,132));
  for(int i=0;i<7;++i){int x=25+i*45;g.rect(x,24+(i%2)*4,18,5,C(88,84,120));g.rect(x+3,29+(i%2)*4,12,12,C(54,54,88));}
  g.line(13,94,76,91,C(75,84,179));g.line(76,91,92,105,C(99,205,229));g.line(307,134,252,126,C(75,84,179));g.line(252,126,235,138,C(99,205,229));
  // Arrival portal remains visible at the southern edge.
  int pulse=(int)(std::sin(time*4.f)*2);g.rect(149-pulse,153-pulse,23+pulse*2,8+pulse,C(32,25,70));g.rect(153-pulse,151-pulse,15+pulse*2,8+pulse*2,C(86,70,190));g.rect(157,151,7,5,C(113,230,247));
  for(const auto&e:enemies)if(e.alive){if(e.type==3)skeleton2(e);else wraith(e);enemyHealthBar(e);}heroDraw();if(dash>0&&!dustSprite.pixels.empty()){int frame=(int)(time*18)%4;dustSprite.draw(g,(int)hero.x-6-(int)face.x*8,(int)hero.y+5-(int)face.y*6,frame*12,0,12,12);}
  int sx=(int)hero.x-10,sy=(int)hero.y-10+(int)std::sin(time*5);g.rect(sx-1,sy-1,3,3,C(166,94,242));g.p(sx-3,sy,C(91,218,239));g.p(sx+3,sy,C(91,218,239));
  g.rect(0,0,SW,14,C(10,10,27));for(int i=0;i<5;i++){uint32_t c=i<hp?C(232,65,75):C(74,67,67);g.rect(7+i*10,4,7,6,c);g.p(8+i*10,3,c);g.p(12+i*10,3,c);}int left=0;for(const auto&e:enemies)if(e.alive)left++;text(g,67,5,"WISPS "+std::to_string(left),C(111,221,241));text(g,207,5,"MOONLIT RUINS",C(196,139,255));text(g,83,166,"DEFEAT THE VOID WISPS",WHITE);
  if(teleport>0){int inset=(int)((1.f-teleport)*160.f);g.rect(0,0,std::max(0,160-inset),SH,C(205,244,255));g.rect(160+inset,0,std::max(0,160-inset),SH,C(205,244,255));}
  if(win||lose){g.rect(55,58,210,63,C(12,12,31));g.box(58,61,204,57,C(29,26,58),C(171,89,220));text(g,win?92:112,73,win?"RUINS PURIFIED":"LIGHT FADES",win?C(111,221,241):C(239,87,81),2);text(g,102,101,"PRESS R TO RESTART",WHITE);}}
 void render(){if(map==1){renderMoonRuins();return;}g.clear(GRASS);if(!grassTile.pixels.empty())for(int y=0;y<SH;y+=16)for(int x=0;x<SW;x+=16)grassTile.draw(g,x,y,0,0,16,16);else for(int y=0;y<SH;y+=8)for(int x=0;x<SW;x+=8)if(((x*7+y*13)/8)%7==0)g.p(x+2,y+3,GRASS2);
   // --- Grass variation: tint ~20% tiles darker, ~10% warmer for natural variation ---
   for(int ty=0;ty<SH;ty+=16)for(int tx=0;tx<SW;tx+=16){uint32_t h=hash2d(tx/16,ty/16);int v=h%10;
     if(v<2)for(int yy=std::max(0,ty);yy<std::min(SH,ty+16);++yy)for(int xx=std::max(0,tx);xx<std::min(SW,tx+16);++xx){uint32_t c=g.px[yy*SW+xx];g.px[yy*SW+xx]=C(((c>>16)&255)*92/100,((c>>8)&255)*92/100,(c&255)*92/100);}
      else if(v==3)for(int yy=std::max(0,ty);yy<std::min(SH,ty+16);++yy)for(int xx=std::max(0,tx);xx<std::min(SW,tx+16);++xx){uint32_t c=g.px[yy*SW+xx];g.px[yy*SW+xx]=C(std::min(255,(int)((c>>16)&255)+6),std::min(255,(int)((c>>8)&255)+4),(int)(c&255));}
     if((h>>4)%5==0){int dx=tx+(int)((h>>8)%14)+1,dy=ty+(int)((h>>12)%14)+1;if(dx<SW&&dy<SH)g.p(dx,dy,GRASS2);}}
   // --- Forest edge: scatter dark pixels at map borders for dense forest feel ---
   for(int i=0;i<180;i++){uint32_t h=hash2d(i+500,i*7+333);int side=i%4,ex,ey;
     if(side==0){ex=15+(int)(h%16);ey=22+(int)((h>>8)%148);}
     else if(side==1){ex=292+(int)(h%14);ey=22+(int)((h>>8)%148);}
     else if(side==2){ex=14+(int)((h>>8)%292);ey=22+(int)(h%10);}
     else{ex=14+(int)((h>>8)%292);ey=160+(int)(h%10);}
     g.p(ex,ey,C(18+(int)(h%12),38+(int)((h>>4)%15),20+(int)((h>>8)%8)));if((h>>12)%3==0)g.p(ex+1,ey,C(20+(int)(h%10),40+(int)((h>>4)%12),22+(int)((h>>8)%8)));}
   // --- Dirt path from spawn to shrine, branching east to chest ---
   {const uint32_t DT=C(139,109,72),DD=C(112,86,58),DL=C(162,135,95);
   g.rect(156,50,9,98,DT);g.rect(155,50,1,98,DD);g.rect(165,50,1,98,DD);
   g.rect(157,49,7,1,DT);g.rect(157,148,7,1,DT);
   for(int px=164;px<258;px++){int py=82-((px-160)*19/98);for(int dy=-2;dy<=3;dy++)g.p(px,py+dy,DT);g.p(px,py-3,DD);g.p(px,py+4,DD);}
   for(int i=0;i<25;i++){uint32_t h=hash2d(i+100,i*3+200);int px=157+(int)(h%7),py=52+(int)((h>>8)%94);g.p(px,py,(h%2)?DL:DD);}
   for(int i=0;i<10;i++){uint32_t h=hash2d(i+700,i*11+800);int sd=(h%2)?155:165,py=55+(int)((h>>4)%90);g.p(sd,py,GRASS);}}
   // --- Small pond with animated water glint ---
   {const uint32_t WD=C(24,56,86),WM=C(38,88,128),WL=C(62,130,174),WG=C(120,192,222);
   g.rect(44,113,14,6,WD);g.rect(42,114,18,4,WD);g.rect(46,112,10,1,WM);g.rect(46,119,10,1,WM);
   g.rect(43,113,1,6,WM);g.rect(57,113,1,6,WM);g.rect(47,113,8,1,WL);g.rect(45,117,10,1,WL);
   {int gx=48+(int)(std::sin(time*2.5f)*3);g.p(gx,115,WG);g.p(gx+1,115,C(170,215,238));}
   g.p(41,114,C(28,68,35));g.p(41,117,C(28,68,35));g.p(42,119,C(28,68,35));g.p(59,114,C(28,68,35));g.p(58,118,C(28,68,35));
   g.p(40,113,C(45,95,42));g.p(40,112,C(52,110,48));g.p(40,111,C(55,120,50));g.p(60,115,C(45,95,42));g.p(60,114,C(52,110,48));}
  // A small wooden rest platform and stone shrine floor use clean standalone tiles.
  if(!woodTile.pixels.empty())for(int y=104;y<=120;y+=16)for(int x=36;x<=68;x+=16)woodTile.draw(g,x,y,0,0,16,16);
  // Stone shrine floor assembled from clean 16x16 wall tiles.
  g.rect(132,18,56,42,C(49,75,61));if(!wallsTile.pixels.empty()){for(int x=136;x<=168;x+=16){wallsTile.draw(g,x,20,0,0,16,16);wallsTile.draw(g,x,36,0,32,16,16);}wallsTile.draw(g,132,20,0,16,16,true);wallsTile.draw(g,172,20,0,16,16,16);}else g.rect(136,22,48,38,C(56,99,73));
  // The northern edge stays open; repeating the pack's vertical fence-post tile
  // here looked like a row of logs and did not match the intended perspective.
  std::array<V,12> trees{{{25,45},{45,95},{20,143},{294,44},{292,102},{278,150},{65,38},{104,36},{218,37},{250,35},{82,152},{230,154}}};for(size_t i=0;i<trees.size();++i)tree((int)trees[i].x,(int)trees[i].y,(int)(i%4));
  // Small original ground details; the free pack's decor atlases contain premium watermarks.
  for(auto p:std::array<V,8>{{{52,132},{94,72},{121,142},{202,137},{257,92},{276,119},{152,82},{37,72}}}){int x=(int)p.x,y=(int)p.y;g.p(x,y,C(213,221,154));g.p(x-1,y+1,C(236,190,76));g.p(x+1,y+1,C(236,190,76));g.p(x,y+2,C(34,101,47));}
  // Shrine and crystal
  g.rect(145,49,30,5,C(101,102,89));g.rect(149,44,22,7,C(142,140,111));g.rect(157,25,7,23,C(24,112,158));g.rect(159,22,3,25,C(74,211,240));g.p(160,20,C(202,255,255));
  if(!doorSprite.pixels.empty())doorSprite.draw(g,152,14,(shards>=3?1:0)*16,0,16,16);
  // chest
  int cx=267,cy=63;if(!chestSprite.pixels.empty())chestSprite.draw(g,cx-8,cy-9,(chest?3:0)*16,0,16,16);else {g.box(cx-8,cy-5,16,11,chest?C(92,68,39):C(176,104,31),C(67,43,29));g.rect(cx-1,cy-4,3,9,C(241,189,61));}
  int cx2=52,cy2=116;if(!chest2Sprite.pixels.empty())chest2Sprite.draw(g,cx2-8,cy2-9,(chest2?3:0)*16,0,16,16);
  if(!rock1.pixels.empty()){rock1.draw(g,103,66,0,0,16,16);rock2.draw(g,215,122,0,0,16,16);rock3.draw(g,274,143,0,0,16,16);}
  for(auto&d:drops)if(!d.got){int y=(int)d.p.y+(int)(sin(time*6+d.p.x)*2);g.rect((int)d.p.x-2,y-4,5,8,C(28,112,181));g.rect((int)d.p.x-1,y-3,3,6,C(72,221,245));}
  for(const auto&e:enemies)if(e.alive){if(e.type==3)skeleton2(e);else slime(e);enemyHealthBar(e);}heroDraw();
  if(dash>0&&!dustSprite.pixels.empty()){int frame=(int)(time*18)%4;dustSprite.draw(g,(int)hero.x-6-(int)face.x*8,(int)hero.y+5-(int)face.y*6,frame*12,0,12,12);}
  // spirit companion
  int sx=(int)hero.x-10,sy=(int)hero.y-10+(int)sin(time*5);g.p(sx,sy,C(255,246,174));g.rect(sx-1,sy-1,3,3,C(255,178,42));g.p(sx-3,sy,C(255,208,65));g.p(sx+3,sy,C(255,208,65));
  g.rect(0,0,SW,14,C(15,27,35));for(int i=0;i<5;i++){uint32_t c=i<hp?C(232,65,75):C(74,67,67);g.rect(7+i*10,4,7,6,c);g.p(8+i*10,3,c);g.p(12+i*10,3,c);}
  text(g,66,5,"SHARDS "+std::to_string(shards)+" 3",C(111,221,241));text(g,217,5,"LUMINA QUEST",C(244,198,77));
  if(shards<3)text(g,78,166,"DEFEAT THE MONSTERS",WHITE);else text(g,67,166,"PRESS E AT THE CRYSTAL",WHITE);
  if(dist(hero,{267,63})<22&&!chest)text(g,248,76,"E OPEN",WHITE);if(dist(hero,{160,37})<24&&shards>=3)text(g,146,56,"E LIGHT",WHITE);
  if(dist(hero,{52,116})<22&&!chest2)text(g,35,129,"E OPEN",WHITE);
  if(win||lose){g.rect(55,58,210,63,C(17,27,38));g.box(58,61,204,57,C(27,48,58),C(232,184,67));text(g,win?100:112,73,win?"SHRINE RESTORED":"LIGHT FADES",win?C(89,224,240):C(239,87,81),2);text(g,102,101,"PRESS R TO RESTART",WHITE);}
 }
};
Game game; BITMAPINFO bmi{}; bool running=true;
LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l){switch(m){case WM_DESTROY:running=false;PostQuitMessage(0);return 0;case WM_KEYDOWN:if(w<256)game.key[w]=true;if(w==VK_ESCAPE)DestroyWindow(h);return 0;case WM_KEYUP:if(w<256)game.key[w]=false;return 0;case WM_KILLFOCUS:game.key.fill(false);return 0;case WM_ERASEBKGND:return 1;case WM_PAINT:{PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);int cw=r.right-r.left,ch=r.bottom-r.top;int scale=std::max(1,std::min(cw/SW,ch/SH));int dw=SW*scale,dh=SH*scale,ox=(cw-dw)/2,oy=(ch-dh)/2;
  // Compose the complete frame off-screen, then copy it in one operation.
  // This prevents the background fill and game image from becoming visible separately.
  HDC backDC=CreateCompatibleDC(dc);HBITMAP backBitmap=CreateCompatibleBitmap(dc,cw,ch);HGDIOBJ oldBitmap=SelectObject(backDC,backBitmap);RECT backRect{0,0,cw,ch};FillRect(backDC,&backRect,(HBRUSH)GetStockObject(BLACK_BRUSH));SetStretchBltMode(backDC,COLORONCOLOR);StretchDIBits(backDC,ox,oy,dw,dh,0,0,SW,SH,game.g.px.data(),&bmi,DIB_RGB_COLORS,SRCCOPY);BitBlt(dc,0,0,cw,ch,backDC,0,0,SRCCOPY);SelectObject(backDC,oldBitmap);DeleteObject(backBitmap);DeleteDC(backDC);EndPaint(h,&ps);return 0;}}return DefWindowProc(h,m,w,l);}
}

int WINAPI wWinMain(HINSTANCE hi,HINSTANCE,LPWSTR,int){ULONG_PTR gdipToken{};Gdiplus::GdiplusStartupInput gdipInput;Gdiplus::GdiplusStartup(&gdipToken,&gdipInput,nullptr);game.loadAssets();WNDCLASS wc{};wc.style=CS_OWNDC;wc.lpfnWndProc=WndProc;wc.hInstance=hi;wc.lpszClassName=L"LuminaQuestWindow";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);RegisterClass(&wc);RECT r{0,0,960,540};AdjustWindowRect(&r,WS_OVERLAPPEDWINDOW,FALSE);HWND h=CreateWindow(wc.lpszClassName,L"Lumina Quest - Original C++ Pixel Adventure",WS_OVERLAPPEDWINDOW|WS_VISIBLE,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,nullptr,nullptr,hi,nullptr);bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bmi.bmiHeader.biWidth=SW;bmi.bmiHeader.biHeight=-SH;bmi.bmiHeader.biPlanes=1;bmi.bmiHeader.biBitCount=32;bmi.bmiHeader.biCompression=BI_RGB;auto last=std::chrono::steady_clock::now();double acc=0;MSG msg{};game.render();InvalidateRect(h,nullptr,FALSE);while(running){while(PeekMessage(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessage(&msg);}auto now=std::chrono::steady_clock::now();acc+=std::chrono::duration<double>(now-last).count();last=now;acc=std::min(acc,.1);bool frameReady=false;while(acc>=1.0/60){game.update(1.f/60);acc-=1.0/60;frameReady=true;}if(frameReady){game.render();InvalidateRect(h,nullptr,FALSE);UpdateWindow(h);}Sleep(1);}Gdiplus::GdiplusShutdown(gdipToken);return 0;}
