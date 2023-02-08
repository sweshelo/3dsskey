#include <3ds.h>
#include <string>
#include <array>
#include <iostream>
#include <fstream>

#include "http.hpp"
#include "json.hpp"


int main(int argc, char **argv)
{
  gfxInitDefault();
  consoleInit(GFX_TOP, NULL);

  std::ifstream f("sdmc:/3ds/3dsskey/config.json");
  std::string token = "";

  if(!f){
    std::cout << "missing config.json" << std::endl;
    while(aptMainLoop()){
      hidScanInput();
      if(hidKeysDown()){
        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxExit();
        return -1;
      };
    }
  }else{
    json data = json::parse(f);
    token = data["token"];
    std::cout << token << std::endl;
  }

  json res, req = {
    {"visibility", "home"},
    {"text", "Hello, Misskey!"},
    {"localOnly", false},
    {"i", token},
  };

  HTTP http;
  res = http.post("https://misskey.neos.love/api/notes/create", req);
  std::cout << "res: " << res << std::endl;

  while (aptMainLoop())
  {
    hidScanInput();

    if (hidKeysDown() & KEY_START)
      break;

    gfxFlushBuffers();
    gfxSwapBuffers();
  }

  gfxExit();
  return 0;
}
