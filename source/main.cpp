#include <3ds.h>
#include <string>
#include <array>
#include <iostream>
#include <fstream>
#include <random>
#include <citro2d.h>
#include <dirent.h>
#include <initializer_list>

#include "http.hpp"
#include "json.hpp"
#include "qrcodegen.hpp"
#include "Keyboard.hpp"

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

using namespace qrcodegen;

const u32 springgreen = C2D_Color32(0x00, 0xFF, 0x7F, 0xFF);
const u32 greenyellow = C2D_Color32(0xAD, 0xFF, 0x2F, 0xFF);
const u32 black = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
const u32 white = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);

std::string genUuid(){
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(0, 15);

  std::string uuid = "";
  for (int i = 0; i < 36; i++){
    if (i == 8 || i == 13 || i == 18 || i == 23){
      uuid += "-";
    }else{
      int j = dist(mt);
      uuid += j >= 10 ? 'A' + j - 10 : '0' + j;
    }
  }
  return uuid;
}

// Prints the given QrCode object to the console.
static void printQr(const QrCode &qr, C3D_RenderTarget* screen) {
  int pixel = 2;
  int border = 3;
  int marginX = (400/pixel - qr.getSize() - border*2) / 2;
  int marginY = 15;

  C2D_TextBuf gTextBuf = C2D_TextBufNew(4096);
  C2D_Text gText[2];
  C2D_TextParse(&gText[0], gTextBuf, "QRコードをスキャンしてログインしてください");
  C2D_TextParse(&gText[1], gTextBuf, "完了したらボタンを押して続行します");

  for(int i=0; i<2; i++)
    C2D_TextOptimize(&gText[i]);

  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
  C2D_SceneBegin(screen);
  C2D_TargetClear(screen, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
  C2D_DrawRectangle(0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, springgreen, greenyellow, greenyellow, springgreen);

  //Draw QrCode
  for (int y = -border; y < qr.getSize() + border; y++) {
    for (int x = -border; x < qr.getSize() + border; x++) {
      u32 color = qr.getModule(x,y) ? black : white;
      C2D_DrawRectangle((border+x+marginX)*pixel , (border+y+marginY)*pixel, 0, pixel, pixel, color, color, color, color);
    }
  }

  //Draw Info
  for(int i=0; i<2; i++)
    C2D_DrawText(&gText[i], C2D_AlignCenter, 200.0f, 180.0f + 12.0f * i, 0.5f, 0.5f, 0.5f);

  C3D_FrameEnd(0);
  while(aptMainLoop()){
    //HID
    hidScanInput();
    if(hidKeysDown() & KEY_A)
      break;
  }
}

int main(int argc, char **argv)
{
  gfxInitDefault();
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
  C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
  C2D_Prepare();
  consoleInit(GFX_BOTTOM, NULL);

  // Create screens
  C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
  C2D_SceneBegin(top);
  C2D_TargetClear(top, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
  C2D_DrawRectangle(0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, springgreen, greenyellow, greenyellow, springgreen);

  HTTP http;
  json res, req;
  std::string MISSKEY_DOMAIN = "";

  mkdir("sdmc:/3ds/3dsskey", 0777);
  std::ifstream f("sdmc:/3ds/3dsskey/config.json");
  std::string token, uuid, loginstate;

  if(!f){
    C2D_TextBuf gTextBuf = C2D_TextBufNew(4096);
    C2D_Text gText;
    C2D_TextParse(&gText, gTextBuf, "ログインするサーバのドメインを指定してください");
    C2D_DrawText(&gText, C2D_AlignCenter | C2D_AtBaseline, 200.0f, 120.0f, 0.5f, 0.5f, 0.5f);
    C3D_FrameEnd(0);

    Keyboard kbd;
    kbd.setHintText("misskey.io");
    kbd.setDictionary({
        "misskey.io",
        "misskey.cf",
        "sushi.ski",
        "newskey.cc",
        "misskey.neos.love"
    });
    MISSKEY_DOMAIN = kbd.input();
    std::cout << "BACKED" << std::endl;

    if(MISSKEY_DOMAIN == ""){
      return -1;
    }else{
      std::cout << "return: " << MISSKEY_DOMAIN << std::endl;
    }

    uuid = genUuid();
    std::string authUri = "https://" + MISSKEY_DOMAIN + "/miauth/" + uuid + "?name=3dsskey&permission=write:notes";
    QrCode qr0 = QrCode::encodeText(authUri.c_str(), QrCode::Ecc::MEDIUM);
    printQr(qr0, top);

    res = http.post("https://" + MISSKEY_DOMAIN + "/api/miauth/" + uuid + "/check", NULL);
    if (res.contains("token")){
      token = res["token"];
      std::ofstream ofstr("sdmc:/3ds/3dsskey/config.json", std::ios::out | std::ios::binary | std::ios_base::trunc);
      if(!ofstr){
        std::cout << "Error!: cannot open file" << std::endl;
      }
      ofstr.write( res.dump(2).c_str(), res.dump(2).size() );
      loginstate = "ログインに成功しました";
    }else{
      loginstate = "ログインに失敗しました";
    }
  }else{
    json data = json::parse(f);
    token = data["token"];
    loginstate = "ログインに成功しました";
  }

  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
  C2D_SceneBegin(top);
  C2D_TextBuf gTextBuf = C2D_TextBufNew(4096);
  C2D_Text gText;
  C2D_TextParse(&gText, gTextBuf, loginstate.c_str());
  C2D_TargetClear(top, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
  C2D_DrawRectangle(0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, springgreen, greenyellow, greenyellow, springgreen);
  C2D_DrawText(&gText, C2D_AlignCenter, 200.0f, 120, 0.5f, 0.6f, 0.6f);
  C3D_FrameEnd(0);

  req = {
    {"visibility", "home"},
    {"text", "ログインに成功しました"},
    {"localOnly", false},
    {"i", token},
  };

  //if(token != "")
  //  res = http.post("https://misskey.neos.love/api/notes/create", req);

  while (aptMainLoop())
  {
    hidScanInput();

    if (hidKeysDown() & KEY_START)
      break;

    //gfxFlushBuffers();
    //gfxSwapBuffers();
  }

  gfxExit();
  return 0;
}
