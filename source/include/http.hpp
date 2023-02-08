#ifndef THREEDSSKEY_HTTP_HPP_
#define THREEDSSKEY_HTTP_HPP_

#include <curl/curl.h>
#include <malloc.h>
#include <3ds.h>
#include "json.hpp"

#define FILE_ALLOC_SIZE 0x60000
#define USER_AGENT "3dsskey/0.0.0"

using json = nlohmann::json;

class HTTP
{
  private:
    CURL *CurlHandle = nullptr;
    static int curlProgress(CURL*, curl_off_t, curl_off_t, curl_off_t, curl_off_t);
    static size_t buffer_writer(char*, size_t, size_t, void*);
  public:
    json post(const std::string&, const json&);
    long httpCode;
};

#endif
