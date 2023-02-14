#ifndef __THREEDSSKEY_SPRITE_HPP_
#define __THREEDSSKEY_SPRITE_HPP_

#include <3ds.h>
#include <citro2d.h>

class Sprite{
  private:
    C2D_Sprite sprite;
    float px, py, pz, sx, sy, rx, ry;
  public:
    void set(C2D_SpriteSheet, int);
    void pos(float, float, float);
    void draw(void);
};

#endif
