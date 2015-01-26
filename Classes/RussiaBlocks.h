#include "cocos2d.h"
#include <string>

USING_NS_CC;

class ScreenBlock: public Sprite {
  public:
    enum {
      FREE = 0,
      FIXED = 1,
    };
    bool init() {
      setTag(FREE);
      return initWithTexture(nullptr, Rect::ZERO );
    }
    CREATE_FUNC(ScreenBlock);
  protected:
};



#define DEFAULT_BLOCK_COLOR Color3B(0xBF,0x8A,0x30)
#define SELECTED_BLOCK_COLOR Color3B::BLACK
#define SHAPNR 6
class ScreenImpl: public ScreenBlock {
  public:
    enum {
      TSHAP = 0,
      LSHAPL = 1,
      LSHAPR = 2,
      ZSHAPL = 3,
      ZSHAPR = 4,
      ISHAP = 5,
    };
    enum {
      CENTOR = 0,
      TOP = 1,
      LEFT = 2,
      RIGHT = 3,
      BOTTOM = 4,
    };
    class Shap: public Ref {
      public:
        CREATE_FUNC(Shap);
        bool init() {
          return true;
        }
        //data members
        int rotation_;
        Vec2 centor_;
        //all is ref to rotation 0
        //sorted at any time.
        Vec2 blocks_[4];
    };
    //ScreenImpl(int w, int h):width(w),height(h) {}
    //720x0.8 = 32x18, 1080x0.8 = 32x27, so block is 32x32
    static inline ScreenImpl* createWithArgs(
        Point &playOrig, Size &playSize, int width=18, int height=27); 
    void colorBlock(Vec2 &&loc, Color3B color = SELECTED_BLOCK_COLOR);
    void colorBlock(Vec2 &loc, Color3B color = SELECTED_BLOCK_COLOR);
    void colorClickedBlock(Vec2 &loc);
    void setToInitialColorAndClearTag() {
      log("clearing screen");
      for(auto i : blockArr_) {
        i->setColor(DEFAULT_BLOCK_COLOR);
        i->setTag(FREE);
      }
    }
    void genShap();
    void moveShapRight();
    void moveShapLeft();
    // return true means drop onestep successfully
    // false means shap connected to the top of other shap and added to the DropedBlocksArr_
    bool dropShap(bool isToBottom); 
  protected:
    ScreenImpl(Point &playOrig, Size &playSize, int width=18, int height=27):
      playOrig_(playOrig), playSize_(playSize), width_(width), height_(height) {
        blockSize_.width = playSize_.width/width_;
        blockSize_.height = playSize_.height/height_;
        CCASSERT(blockSize_.width == blockSize_.height, "Not width==height block!\n");
        curShap_ = Shap::create();
        curShap_->retain();
        nextShap_ = Shap::create();
        nextShap_->retain();
      }
    ~ScreenImpl() {
      curShap_->release();
      nextShap_->release();
      for(auto i: blockArr_) {
        i->release();
      }
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
    Vec2 getMidTopPos() {
      return Vec2(width_/2, height_-1);
    }
    bool isStuck(int);
    //colums and rows of blocks.
    int width_;
    int height_;
    //playing area size, unit px.
    Size playSize_;
    Point playOrig_;
    Size blockSize_;
    std::vector<ScreenBlock*> blockArr_;
    Shap* curShap_;//contain current shap and next shap.
    Shap* nextShap_;

}; //class ScreenImpl



class Runner: public Ref {
  public:
    enum {
      TOSTART = 0, //to start whole game. 
      TOGENSHAP = 1, //to generate a new shap.
      DROPPING = 2, //shap is dropping.
    };
    Runner(ScreenImpl* screenArr):screenArr_(screenArr), state_(TOSTART), movedTag_(false) {
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
      screenArr_->setToInitialColorAndClearTag();
    }
    void moveLeft();
    void moveRight();
    void drop();
    void dropToBottom();
    //only move left or right by touch screen, set it and endtouch clear it.
    void processMoved(Vec2 delta) {
      if(delta.x < -30) {
        moveLeft();
      } else if(delta.x > 30){
        moveRight();
      } else if(delta.y < -30) {
        screenArr_->dropShap(true);
      }
    }
  protected:
    bool init() { return true;}
    void genShap();

    ScreenImpl* screenArr_;
    int state_;
    bool movedTag_;
}; //class Runner

//Singlton service to control science creatation and game logic going on.
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
    void processOnTouchMoved(Vec2 delta) { if(runner_) runner_-> processMoved(delta);}
    void saveOnTouchBeginPoint(Vec2 point) { onTouchBegin_ = point; }
    void processOnTouchEnd(Vec2 endPoint) {
      if(runner_) processOnTouchMoved(endPoint-onTouchBegin_);
    }
  private:
    Player():runner_(nullptr) {}
    Player(Player &Player) {}
    bool operator =(Player &Player) { return true; }
    Runner *runner_;
    Vec2 onTouchBegin_; 

};
