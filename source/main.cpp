#include <3ds.h>
#include <string>
#include <array>
#include <iostream>
#include <fstream>
#include <random>
#include <citro2d.h>

#include "http.hpp"
#include "json.hpp"
#include "qrcodegen.hpp"

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

using namespace qrcodegen;

const std::string MISSKEY_DOMAIN = "misskey.neos.love";

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
  u32 springgreen = C2D_Color32(0x00, 0xFF, 0x7F, 0xFF);
  u32 greenyellow = C2D_Color32(0xAD, 0xFF, 0x2F, 0xFF);
  u32 black = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
  u32 white = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
  int border = 4;
  int marginX = (100 - qr.getSize() - border*2) / 2;
  int marginY = 1;

  C2D_TextBuf gTextBuf = C2D_TextBufNew(4096);
  C2D_Text gText[3];
  C2D_TextParse(&gText[0], gTextBuf, "ログイン情報が見つかりません");
  C2D_TextParse(&gText[1], gTextBuf, "QRコードをスキャンしてログインしてください");
  C2D_TextParse(&gText[2], gTextBuf, "完了したらボタンを押して続行します");

  for(int i=0; i<3; i++)
    C2D_TextOptimize(&gText[i]);

  while(aptMainLoop()){
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_SceneBegin(screen);
    C2D_TargetClear(screen, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectangle(0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, springgreen, greenyellow, greenyellow, springgreen);

    //Draw QrCode
    for (int y = -border; y < qr.getSize() + border; y++) {
      for (int x = -border; x < qr.getSize() + border; x++) {
        u32 color = qr.getModule(x,y) ? black : white;
        C2D_DrawRectangle((border+x+marginX)*4 , (border+y+marginY)*4, 0, 4, 4, color, color, color, color);
      }
    }

    //Draw Info
    for(int i=0; i<3; i++)
      C2D_DrawText(&gText[i], C2D_AlignCenter, 200.0f, 200.0f + 12.0f * i, 0.5f, 0.4f, 0.4f);

    //HID
    hidScanInput();
    if(hidKeysDown() & KEY_A)
      break;

    gfxFlushBuffers();
    gfxSwapBuffers();
    C3D_FrameEnd(0);
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

  HTTP http;
  json res, req;

  std::ifstream f("sdmc:/3ds/3dsskey/config.json");
  std::string token = "";
  std::string uuid = "";

  if(!f){
    uuid = genUuid();
    std::string authUri = "https://" + MISSKEY_DOMAIN + "/miauth/" + uuid + "?name=3dsskey&permission=write:notes";
    QrCode qr0 = QrCode::encodeText(authUri.c_str(), QrCode::Ecc::MEDIUM);
    printQr(qr0, top);

    res = http.post("https://" + MISSKEY_DOMAIN + "/api/miauth/" + uuid + "/check", NULL);
    if ( http.httpCode == 200 ){
      token = res["token"];
    }else{
      std::cout << res.dump(2) << std::endl;
    }
  }else{
    json data = json::parse(f);
    token = data["token"];
  }

  req = {
    {"visibility", "home"},
    {"text", "ログインに成功しました"},
    {"localOnly", false},
    {"i", token},
  };

  if(token != "")
    res = http.post("https://misskey.neos.love/api/notes/create", req);

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
