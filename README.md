# 3dsskey

Misskey client for Nintendo 3DS

## build
### 必要なもの
- [devkitPro](https://devkitpro.org/)
- [nayuki/QR-Code-generator](https://github.com/nayuki/QR-Code-generator/tree/master/cpp)
  * qrcodegen.cpp と qrcodegen.hpp をそれぞれ source/ と source/include に置く
- [nlohmann/json](https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp) (Direct download)
  * source/include に置く

### build
```
make
```
