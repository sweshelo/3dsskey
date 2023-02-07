#include <3ds.h>
#include <string>
#include <cstring>
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

json httpGet(const std::string &url) {
  Result retcode = 0;
  std::string response;
  int res;

  printf("Downloading from:\n%s\n", url.c_str());

  void *socubuf = memalign(0x1000, 0x100000);
  if (!socubuf) {
    retcode = -1;
    return "";
  }

  res = socInit((u32 *)socubuf, 0x100000);
  if (R_FAILED(res)) {
    retcode = res;
    return "";
  }

  CurlHandle = curl_easy_init();
  curl_easy_setopt(CurlHandle, CURLOPT_BUFFERSIZE, FILE_ALLOC_SIZE);
  curl_easy_setopt(CurlHandle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(CurlHandle, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(CurlHandle, CURLOPT_USERAGENT, USER_AGENT);
  curl_easy_setopt(CurlHandle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(CurlHandle, CURLOPT_FAILONERROR, 1L);
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
    retcode = -curlResult;
  }

  if (socubuf) free(socubuf);
  socExit();

  return json::parse(response);
}

int main(int argc, char **argv)
{
  gfxInitDefault();
  consoleInit(GFX_TOP, NULL);

  json res;
  res = httpGet("http://192.168.1.5/api/misskey?json=true");

  for(int i=0; i<5; i++){
    std::cout << res[i]["id"] << "[" << res[i]["user"]["name"] << "]: " << res[i]["text"] << std::endl;
  }

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
