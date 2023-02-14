#include "Misskey.hpp"
#include <iostream>

Misskey::Misskey(){
  top = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
  C3D_RenderTargetSetOutput(top, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
  bottom = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
  C3D_RenderTargetSetOutput(bottom, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
  C2D_SceneBegin(top);
  C2D_TargetClear(top, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
  C2D_DrawRectangle(0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SPRINGGREEN, GREENYELLOW, GREENYELLOW, SPRINGGREEN);
  C3D_FrameEnd(0);

  mkdir("sdmc:/3ds/3dsskey", 0777);
  conffile.open("sdmc:/3ds/3dsskey/config.json");

  kbd.setHintText("misskey.io");
  kbd.setDictionary({
      "misskey.io",
      "misskey.cf",
      "sushi.ski",
      "newskey.cc",
      "misskey.neos.love",
      });
}

void Misskey::drawQRCode(std::string url, int pixel, int border, int offsetX, int offsetY){
  QrCode qr = QrCode::encodeText(url.c_str(), QrCode::Ecc::MEDIUM);
  int marginX = (400/pixel - qr.getSize() - border*2) / 2;
  int marginY = 15;

  //Draw QrCode
  for (int y = -border; y < qr.getSize() + border; y++) {
    for (int x = -border; x < qr.getSize() + border; x++) {
      u32 color = qr.getModule(x,y) ? C2D_Color32(0x00, 0x00, 0x00, 0xFF) : C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
      C2D_DrawRectangle((border+x+marginX)*pixel+offsetX , (border+y+marginY)*pixel+offsetY, 0, pixel, pixel, color, color, color, color);
    }
  }
}

void Misskey::login(){
  // 設定ファイルからログイン情報を読み出す
  if (!conffile.is_open()) auth();
  confjson = json::parse(conffile);
  if (confjson.find("token") != confjson.end()){
    token = confjson["token"];
    domain = confjson["domain"];
  }
  if(token=="") auth();
}

bool Misskey::verifyToken(){
  res = http.post("https://" + domain + "/api/i", {{
      "i", token
  }});
  return (http.httpCode == 200);
}

void Misskey::auth(){
  C2D_TextBuf gTextBuf = C2D_TextBufNew(4096);
  C2D_Text gText[2];

  std::string guideString = "ログインするMisskeyサーバのドメインを入力します";

  do{
    // Enter URL
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_SceneBegin(top);
    C2D_TargetClear(top, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
    C2D_DrawRectangle(0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SPRINGGREEN, GREENYELLOW, GREENYELLOW, SPRINGGREEN);
    C2D_TextParse(&gText[0], gTextBuf, guideString.c_str());
    C2D_TextOptimize(&gText[0]);
    C2D_DrawText(&gText[0], C2D_AlignCenter, 200.0f, 120.0f, 0.5f, 0.5f, 0.5f);
    C3D_FrameEnd(0);
    domain = kbd.input();

    //check domain
    res = http.post("https://" + domain + "/api/meta", NULL);

    guideString = "ドメイン名を正しく入力してください";
  }while(!(domain.length() > 0) || !http.httpCode);

  // Show QR
  C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
  C2D_SceneBegin(top);
  C2D_TargetClear(top, C2D_Color32(0x00, 0x00, 0x00, 0xFF));
  C2D_DrawRectangle(0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SPRINGGREEN, GREENYELLOW, GREENYELLOW, SPRINGGREEN);
  std::string uuid = genUuid();
  std::string authUri = "https://" + domain + "/miauth/" + uuid + "?name=3dsskey&permission=write:notes";
  drawQRCode(authUri, 2, 2, 0, 0);
  C2D_TextParse(&gText[0], gTextBuf, "QRコードをスキャンしてログインしてください");
  C2D_TextParse(&gText[1], gTextBuf, "完了したらボタンを押して続行します");
  for(int i=0; i<2; i++)
    C2D_TextOptimize(&gText[i]);

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

  res = http.post("https://" + domain + "/api/miauth/" + uuid + "/check", NULL);
  if (http.httpCode==200 && res.contains("token")){
    token = res["token"];
    std::ofstream ofstr("sdmc:/3ds/3dsskey/config.json", std::ios::out | std::ios::binary | std::ios_base::trunc);
    if(!ofstr){
      std::cout << "Error!: cannot open file" << std::endl;
    }
    //データ成形
    confjson = {
      { "token", token },
      { "domain", domain },
    };
    ofstr.write( confjson.dump(2).c_str(), confjson.dump(2).size() );
  }else{
    std::cout << "Something went wrong." << std::endl;
  }
}

void Misskey::timeline(){
  while(aptMainLoop()){
    hidScanInput();
    u32 kDown = hidKeysDown();
    if(kDown & KEY_A){
      break;
    }
    if(kDown & KEY_X){
      this->createPost();
    }
  }
}

void Misskey::createPost(){
  std::string content = kbd.input();
  req = {
    {"text", content},
    {"i", token}
  };
  res = http.post("https://" + domain + "/api/notes/create", req);
}
