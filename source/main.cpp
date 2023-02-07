#include <3ds.h>
#include <string>
#include <cstring>
#include <array>
#include <curl/curl.h>
#include <iostream>
#include <malloc.h>

#include "json.hpp"

#define FILE_ALLOC_SIZE 0x60000
#define USER_AGENT "3dsskey/0.0.0"

using json = nlohmann::json;

CURL *CurlHandle = nullptr;

static int curlProgress(CURL *hnd,
    curl_off_t dltotal, curl_off_t dlnow,
    curl_off_t ultotal, curl_off_t ulnow)
{
  return 0;
}

size_t buffer_writer(char *ptr, size_t size, size_t nmemb, void *userdata) {
  std::string *stream = static_cast<std::string*>(userdata);
  stream->append(ptr, size * nmemb);
  return size * nmemb;
}

json httpGet(const std::string &url, const json &body) {
  std::string response, request = body.dump(0);
  int res;

  void *socubuf = memalign(0x1000, 0x100000);
  if (!socubuf) {
      return json({"error", "memalign error"});
  }

  res = socInit((u32 *)socubuf, 0x100000);
  if (R_FAILED(res)) {
    return json({"error", "socInit error"});
  }

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "Accept: application/json");

  CurlHandle = curl_easy_init();
  curl_easy_setopt(CurlHandle, CURLOPT_BUFFERSIZE, FILE_ALLOC_SIZE);
  curl_easy_setopt(CurlHandle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(CurlHandle, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(CurlHandle, CURLOPT_POST, 1L);
  curl_easy_setopt(CurlHandle, CURLOPT_POSTFIELDS, request.c_str());
  curl_easy_setopt(CurlHandle, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(CurlHandle, CURLOPT_USERAGENT, USER_AGENT);
  curl_easy_setopt(CurlHandle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(CurlHandle, CURLOPT_ACCEPT_ENCODING, "gzip");
  curl_easy_setopt(CurlHandle, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(CurlHandle, CURLOPT_XFERINFOFUNCTION, curlProgress);
  curl_easy_setopt(CurlHandle, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
  curl_easy_setopt(CurlHandle, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(CurlHandle, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(CurlHandle, CURLOPT_STDERR, stdout);
  curl_easy_setopt(CurlHandle, CURLOPT_WRITEFUNCTION, buffer_writer);
  curl_easy_setopt(CurlHandle, CURLOPT_WRITEDATA, &response);

  CURLcode curlResult = curl_easy_perform(CurlHandle);
  curl_easy_cleanup(CurlHandle);
  CurlHandle = nullptr;

  if (curlResult != CURLE_OK) {
    return json::parse(response);
  }

  if (socubuf) free(socubuf);
  socExit();

  if (response != ""){
    return json::parse(response);
  }else{
    return json::object();
  }

}

int main(int argc, char **argv)
{
  gfxInitDefault();
  consoleInit(GFX_TOP, NULL);

  json res, req = {
    {"visibility", "home"},
    {"text", input},
    {"localOnly", false},
    {"i", ""},
  };

  res = httpGet("https://misskey.neos.love/api/notes/create", req);
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
