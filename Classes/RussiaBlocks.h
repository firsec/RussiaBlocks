#include "cocos2d.h"
#include <string>

USING_NS_CC;

//Singlton service to control science creatation and game logic going on.


class ScreenBlock: public Sprite {
  public:
    CREATE_FUNC(ScreenBlock);
  protected:
};

#define DEFAULT_BLOCK_COLOR Color3B(0xBF,0x8A,0x30)
class ScreenImpl: public ScreenBlock {
  public:
    //ScreenImpl(int w, int h):width(w),height(h) {}
    //720x0.8 = 32x18, 1080x0.8 = 32x27, so block is 32x32
    static inline ScreenImpl* createWithArgs(
        Point &playOrig, Size &playSize, int width=18, int height=27); 
    void colorClickedBlock(Vec2 &loc);
    void setToInitialColor() {
      log("clearing screen");
      for(auto i : blockArr_) {
        i->setColor(DEFAULT_BLOCK_COLOR);
      }
    }
  protected:
    ScreenImpl(Point &playOrig, Size &playSize, int width=18, int height=27):
      playOrig_(playOrig), playSize_(playSize), width_(width), height_(height) {
        blockSize_.width = playSize_.width/width_;
        blockSize_.height = playSize_.height/height_;
        CCASSERT(blockSize_.width == blockSize_.height, "Not width==height block!\n");
      }
    virtual bool init();
    Vec2 getBlockPosByPixLoc(Vec2 loc) {
      int i = int((loc.x - playOrig_.x)/blockSize_.width);
      int j = int((loc.y - playOrig_.y)/blockSize_.height);
      return Vec2(i,j);
    }

//  j ^
//    |
//    |
//    |-------> i
//    0 the first row and colum is index 0
    ScreenBlock* getBlock(int i, int j) {
      return blockArr_[ width_*j + i ];
    }
    //colums and rows of blocks.
    int width_;
    int height_;
    //playing area size, unit px.
    Size playSize_;
    Point playOrig_;
    Size blockSize_;
    std::vector<ScreenBlock*> blockArr_;

};

class Runner: public Ref {
  public:
    enum {
      TOSTART = 0, //to start whole game. 
      TOGENSHAP = 1, //to generate a new shap.
      DROPPING = 2, //shap is dropping.
    };
    Runner(ScreenImpl* screenArr):screenArr_(screenArr), state_(TOSTART) {
      screenArr_->retain();
    }
    ~Runner() { 
      if(screenArr_) screenArr_->release(); 
    }
    static Runner* createWithArgs(ScreenImpl* screenArr) {
      Runner *runner = new Runner(screenArr);
      if(runner && runner->init()) {
        runner->autorelease();
        return runner;
      }
      CC_SAFE_DELETE(runner);
      return nullptr;
    }

    void run();
    void runClkForScheduler(float tmp);
    void clearScreen() const {
      screenArr_->setToInitialColor();
    }
  protected:
    bool init() { return true;}
    void moveLeft();
    void moveRight();
    void drop();
    void dropToBottom();

    ScreenImpl* screenArr_;
    int state_;
};

class Player {

  public:
    static Player& getInstance() {
      static Player player;
      return player;
    }

    ~Player() { 
      if(runner_) runner_->release(); 
    }
    Scene* createStartScene();
    void play(Scene* scene, ScreenImpl* screenArr);
    Runner* getRunner() const { return runner_; };
  private:
    Player():runner_(nullptr) {}
    Player(Player &Player) {}
    bool operator =(Player &Player) { return true; }
    Runner *runner_;

};
