#include <3ds.h>
#include <string>
#include <vector>

static SwkbdState swkbd;
static SwkbdStatusData swkbdStatus;
static SwkbdLearningData swkbdLearning;

class Keyboard{
  private:
    SwkbdButton button = SWKBD_BUTTON_NONE;
    std::vector<SwkbdDictWord> swkbdWords;
    bool didit = false;
    char string[600];

  public:
    Keyboard();
    std::string input();
    void setHintText(std::string);
    void setDictionary(std::initializer_list<std::string>);
};

