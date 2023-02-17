#ifndef THREEDSSKEY_HTTP_HPP_
#define THREEDSSKEY_HTTP_HPP_

#include <curl/curl.h>
#include <malloc.h>
#include <string>
#include <3ds.h>
#include <citro2d.h>
#include <png.h>
#include "json.hpp"

#define FILE_ALLOC_SIZE 0x60000
#define USER_AGENT "3dsskey/0.0.0"

using json = nlohmann::json;

class HTTP
{
  private:
    CURL *CurlHandle = nullptr;
    static int curlProgress(CURL*, curl_off_t, curl_off_t, curl_off_t, curl_off_t);
    static size_t handleBinaryData(char*, size_t, size_t, std::vector<unsigned char>*);
    static size_t handleHeaders(char*, size_t, size_t, void*);
    std::vector<uint8_t> response;
    std::vector<char> headerData;
    int request(const std::string&, const json&, bool isPost);
  public:
    int get(const std::string&);
    int post(const std::string&);
    int post(const std::string&, const json&);
    json parse(void);
    C2D_Image image(void);
    long httpCode;
    std::string accessed;
    CURLcode result;
    std::string contentType;
};

#endif
