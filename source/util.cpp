#include "util.hpp"

std::string genUuid(){
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(0, 15);

  std::string uuid = "";
  for (int i = 0; i < 36; i++){
    if (i == 8 || i == 13 || i == 18 || i == 23){
      uuid += "-";
    }else{
      int j = dist(mt);
      uuid += j >= 10 ? 'A' + j - 10 : '0' + j;
    }
  }
  return uuid;
}

