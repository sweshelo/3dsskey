#ifndef __MISSKEY_HPP_
#define __MISSKEY_HPP_

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240
#define DISPLAY_TRANSFER_FLAGS \
  (GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
   GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
   GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

#include <3ds.h>
#include <citro2d.h>
#include <dirent.h>
#include <fstream>

#include "json.hpp"
#include "http.hpp"
#include "util.hpp"
#include "qrcodegen.hpp"
#include "Keyboard.hpp"

const u32 SPRINGGREEN = C2D_Color32(0x00, 0xFF, 0x7F, 0xFF);
const u32 GREENYELLOW = C2D_Color32(0xAD, 0xFF, 0x2F, 0xFF);
const u32 BLACK = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
const u32 WHITE = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);

using namespace qrcodegen;

class Misskey{
  private:
    C3D_RenderTarget *top, *bottom;
    HTTP http;
    nlohmann::json confjson;
    Keyboard kbd;
    std::ifstream conffile;
    std::string token = "";
    void drawQRCode(std::string, int, int, int, int);
    void auth();
    void verifyToken();
  public:
    Misskey();

    // 初期化後にこれを呼び出す
    void start();

    // 初期化後アカウント情報が見つからない場合
    void login();

    // main
    void timeline();
};

#endif
