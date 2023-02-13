#include <3ds.h>
#include <string>
#include <array>
#include <iostream>
#include <citro2d.h>
#include <initializer_list>

#include "http.hpp"
#include "json.hpp"
#include "qrcodegen.hpp"
#include "util.hpp"
#include "Keyboard.hpp"
#include "Misskey.hpp"

using namespace qrcodegen;

const u32 springgreen = C2D_Color32(0x00, 0xFF, 0x7F, 0xFF);
const u32 greenyellow = C2D_Color32(0xAD, 0xFF, 0x2F, 0xFF);
const u32 black = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
const u32 white = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);

int main(int argc, char **argv)
{
  gfxInitDefault();
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
  C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
  C2D_Prepare();
  consoleInit(GFX_BOTTOM, NULL);

  Misskey msky;
  msky.login();

  gfxExit();
  return 0;
}
