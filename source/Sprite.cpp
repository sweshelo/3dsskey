#include "Sprite.hpp"

void Sprite::set(C2D_SpriteSheet sheet, int index){
  C2D_SpriteFromSheet(&sprite, sheet, index);
}

void Sprite::set(C2D_Image img){
  C2D_SpriteFromImage(&sprite, img);
}

void Sprite::pos(float x, float y, float z){
  px = x;
  py = y;
  pz = z;
  C2D_SpriteSetPos(&sprite, px, py);
}

void Sprite::draw(){
  C2D_DrawSprite(&sprite);
}
