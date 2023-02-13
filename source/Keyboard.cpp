#include "Keyboard.hpp"

Keyboard::Keyboard(){
  swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, -1);
  swkbdSetInitialText(&swkbd, string);
  swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "キャンセル", false);
  swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "決定", true);
}

void Keyboard::setHintText(std::string hintText){
  swkbdSetHintText(&swkbd, hintText.c_str());
}

void Keyboard::setDictionary(std::initializer_list<std::string> words){
  for (auto&& word : words) {
    swkbdWords.emplace_back();
    swkbdSetDictWord(&swkbdWords.back(), word.substr(0, 2).c_str(), word.c_str());
  }
  swkbdSetDictionary(&swkbd, swkbdWords.data(), swkbdWords.size());
}

std::string Keyboard::input()
{
  swkbdSetStatusData(&swkbd, &swkbdStatus, false, true);
  swkbdSetLearningData(&swkbd, &swkbdLearning, false, true);
  swkbdSetFeatures(&swkbd, SWKBD_DARKEN_TOP_SCREEN | SWKBD_ALLOW_HOME | SWKBD_ALLOW_RESET | SWKBD_ALLOW_POWER | SWKBD_PREDICTIVE_INPUT);
  swkbdInputText(&swkbd, string, sizeof(string));
  std::string result(string);
  return result;
}

