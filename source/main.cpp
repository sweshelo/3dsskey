#include <3ds.h>
#include <citro2d.h>

#include "Misskey.hpp"

int main(int argc, char **argv)
{
  gfxInitDefault();
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
  C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
  C2D_Prepare();
  consoleInit(GFX_BOTTOM, NULL);

  Misskey msky;
  do{
    msky.login();
  }while(!msky.verifyToken());

  msky.timeline();

  gfxExit();
  return 0;
}
