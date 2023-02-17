#include "http.hpp"
#include <iostream>

int HTTP::curlProgress(CURL *hnd,
    curl_off_t dltotal, curl_off_t dlnow,
    curl_off_t ultotal, curl_off_t ulnow)
{
  return 0;
}

size_t HTTP::handleBinaryData(char *ptr, size_t size, size_t nmemb, std::vector<unsigned char> *userdata){
  size_t index = userdata->size();
  size_t n = size * nmemb;
  userdata->resize(index + n);
  memcpy(&(*userdata)[index], ptr, n);
  return n;
}

size_t HTTP::handleHeaders(char* buffer, size_t size, size_t nitems, void* userdata) {
  std::string *content_type = static_cast<std::string*>(userdata);
  size_t length = nitems * size;
  if (std::string(buffer).find("Content-Type") != std::string::npos)
    *content_type = std::string(buffer).substr(13, length - 15);
  return length;
}

json HTTP::parse(void){
  if(!contentType.find("application/json")) return {};
  std::string text(response.begin(), response.end());
  return json::parse(text);
}

int HTTP::post(const std::string &url){
  return request(url, NULL, true);
}

int HTTP::post(const std::string &url, const json &body){
  return request(url, body, true);
}

int HTTP::get(const std::string &url){
  return request(url, NULL, false);
}

int HTTP::request(const std::string &url, const json &body, bool isPost) {
  std::string request = body.dump(0);

  response = {};
  headerData = {};
  accessed = url;
  httpCode = -1;

  int res;

  void *socubuf = memalign(0x1000, 0x100000);
  if (!socubuf) {
    return -1;
  }

  res = socInit((u32 *)socubuf, 0x100000);
  if (R_FAILED(res)) {
    return -2;
  }

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  //headers = curl_slist_append(headers, "Accept: application/json");

  CurlHandle = curl_easy_init();
  curl_easy_setopt(CurlHandle, CURLOPT_BUFFERSIZE, FILE_ALLOC_SIZE);
  curl_easy_setopt(CurlHandle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(CurlHandle, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(CurlHandle, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(CurlHandle, CURLOPT_USERAGENT, USER_AGENT);
  curl_easy_setopt(CurlHandle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(CurlHandle, CURLOPT_ACCEPT_ENCODING, "gzip");
  curl_easy_setopt(CurlHandle, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(CurlHandle, CURLOPT_XFERINFOFUNCTION, this->curlProgress);
  curl_easy_setopt(CurlHandle, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
  curl_easy_setopt(CurlHandle, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(CurlHandle, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(CurlHandle, CURLOPT_STDERR, stdout);
  curl_easy_setopt(CurlHandle, CURLOPT_WRITEFUNCTION, this->handleBinaryData);
  curl_easy_setopt(CurlHandle, CURLOPT_WRITEDATA, &(this->response));
  curl_easy_setopt(CurlHandle, CURLOPT_HEADERFUNCTION, this->handleHeaders);
  curl_easy_setopt(CurlHandle, CURLOPT_HEADERDATA, &(this->contentType));
  if(isPost){
    curl_easy_setopt(CurlHandle, CURLOPT_POST, 1L);
    curl_easy_setopt(CurlHandle, CURLOPT_POSTFIELDS, request.c_str());
  }

  result = curl_easy_perform(CurlHandle);
  curl_easy_getinfo(CurlHandle, CURLINFO_RESPONSE_CODE, &httpCode);
  curl_easy_cleanup(CurlHandle);
  CurlHandle = nullptr;

  if (socubuf) free(socubuf);
  socExit();

  return 1;
}

C2D_Image HTTP::image(){
  C2D_Image img;
  if(!contentType.find("image/png")) return img;
  png_image image;
  memset(&image, 0, sizeof(image));
  image.version = PNG_IMAGE_VERSION;
  png_image_begin_read_from_memory(&image, &response[0], response.size());
  image.format = PNG_FORMAT_RGBA;
  std::vector<u8> buffer(PNG_IMAGE_SIZE(image));
  png_image_finish_read(&image, nullptr, &buffer[0], 0, nullptr);

  img.tex = new C3D_Tex;
  img.subtex = new Tex3DS_SubTexture({image.width, image.height, 0.0f, 1.0f, image.width / 512.0f, 1.0f - (image.height / 512.0f)});

  C3D_TexInit(img.tex, 512, 512, GPU_RGBA8);
  C3D_TexSetFilter(img.tex, GPU_LINEAR, GPU_LINEAR);
  img.tex->border = 0xFFFFFFFF;
  C3D_TexSetWrap(img.tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);

  for (u32 x = 0; x < image.width && x < 512; x++) {
    for (u32 y = 0; y < image.height && y < 512; y++) {
      const u32 dstPos = ((((y >> 3) * (512 >> 3) + (x >> 3)) << 6) +
          ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
           ((x & 4) << 2) | ((y & 4) << 3))) * 4;

      const u32 srcPos = (y * image.width + x) * 4;
      ((uint8_t *)img.tex->data)[dstPos + 0] = buffer.data()[srcPos + 3];
      ((uint8_t *)img.tex->data)[dstPos + 1] = buffer.data()[srcPos + 2];
      ((uint8_t *)img.tex->data)[dstPos + 2] = buffer.data()[srcPos + 1];
      ((uint8_t *)img.tex->data)[dstPos + 3] = buffer.data()[srcPos + 0];
    }
  }

  return img;
}
