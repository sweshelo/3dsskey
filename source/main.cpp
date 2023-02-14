#include <3ds.h>
#include <citro2d.h>

#include "Misskey.hpp"

#define DEBUG false

int main(int argc, char **argv)
{
  gfxInitDefault();
  romfsInit();
  C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
  C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
  C2D_Prepare();

  Misskey msky;
  do{
    if(DEBUG) break;
    msky.login();
  }while(!msky.verifyToken());

  msky.timeline();

  gfxExit();
  return 0;
}
