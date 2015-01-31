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
class ScreenImpl: public ScreenBlock {
  public:
    enum {
      TSHAP,
      LSHAPL,
      LSHAPR,
      ZSHAPL,
      ZSHAPR,
      ISHAP,
      OSHAP,
      SHAPNR,
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
    void moveLR2Point(Vec2 point) {
      Vec2 pos = getBlockPosByPixLoc(point);      
      while(!(curShap_->blocks_[3].x > pos.x && curShap_->blocks_[0].x < pos.x)) {
        if(curShap_->blocks_[0].x > pos.x) {
          moveShapLeft();
        } else if(curShap_->blocks_[3].x < pos.x) {
          moveShapRight();
        }
      }
    }
    void eraseFullLine();
    void genShap();
    bool moveShapRight();
    bool moveShapLeft();
    void rotateShap();
    // return true means drop onestep successfully
    // false means shap connected to the top of other shap and added to the DropedBlocksArr_
    bool dropShap(bool isToBottom); 
    void shapFollowPointMovement(Vec2 oldPoint, Vec2 curPoint);
    void refreshCentorPointForTouch() {
      shapCentorPointOnTouchBegin_ = 
        getBlock(curShap_->centor_.x, curShap_->centor_.y)->getPosition();
    }

  protected:
    ScreenImpl(Point &playOrig, Size &playSize, int width=18, int height=27):
      playOrig_(playOrig), playSize_(playSize), width_(width), height_(height) {
        blockSize_.width = playSize_.width/width_;
        blockSize_.height = playSize_.height/height_;
        //CCASSERT(blockSize_.width == blockSize_.height, "Not width==height block!\n");
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
    bool isLegal(Vec2 &&);
    bool isLegal(Vec2 &);
    //colums and rows of blocks.
    int width_;
    int height_; //playing area size, unit px.
    Size playSize_;
    Point playOrig_;
    Size blockSize_;
    std::vector<ScreenBlock*> blockArr_;
    Shap* curShap_;//contain current shap and next shap.
    Shap* nextShap_;
    Vec2 shapCentorPointOnTouchBegin_;

}; //class ScreenImpl



class Runner: public Ref {
  public:
    enum {
      TOSTART = 0, //to start whole game. 
      TOGENSHAP = 1, //to generate a new shap.
      DROPPING = 2, //shap is dropping.
    };
    Runner(ScreenImpl* screenArr):screenArr_(screenArr), state_(TOSTART), movedTag_(false), isLockDrop_(false){
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
    void rotate();
    void moveLeft();
    void moveRight();
    void drop();
    void dropToBottom();
    void shapFollowPoint(Vec2 point) {
      screenArr_->moveLR2Point(point);
    }
    //only move left or right by touch screen, set it and endtouch clear it.
    bool processMoved(Vec2 oldPoint, Touch *t) {
      if(state_ != DROPPING) 
        return false;
      Vec2 delta = t->getDelta();
      Vec2 curPoint = t->getLocation();
      if(curPoint.y - oldPoint.y < -30 && abs(curPoint.x - oldPoint.x) < 30) {
        //directly drop to bottom.
        if(!isLockDrop_ && !screenArr_->dropShap(true)) {
        //erasefullline and  set state_ to genshap and .
          screenArr_->eraseFullLine();
          state_ = TOGENSHAP;
          isLockDrop_ = true;//set to false when touch end.
        }
        return true;
      } else if(abs(delta.y) < 30) {
        //follow right or left.
        if(screenArr_) { 
          screenArr_->shapFollowPointMovement(oldPoint, t->getLocation());
        }
        return false;
      }
      return false;
    }
    bool checkRotateTouchEvent(Vec2 startPoint, Vec2 endPoint) {
      if(abs(startPoint.x - endPoint.x) < 1 && abs(startPoint.y - endPoint.y) < 1) {
        rotate();
      }
      return false;
    }
    void clearMoveEventTags() {
      isLockDrop_ = false;
    }
    void refreshScreenArrShapCentorPointForTouch() {
      screenArr_->refreshCentorPointForTouch();
    }
  protected:
    bool init() { return true;}
    void genShap();

    ScreenImpl* screenArr_;
    int state_;
    bool movedTag_;
    bool isLockDrop_;
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
    bool processOnTouchMoved(Touch *t) { 
      if(runner_) {
        return runner_-> processMoved(pointOnTouchBegin_, t);
      }
      return false;
    }
    void saveOnTouchBeginPoint(Vec2 point) { 
      pointOnTouchBegin_ = point; 
      if(runner_) {
        runner_->refreshScreenArrShapCentorPointForTouch();
      } 
    }
    void processOnTouchEnd(Vec2 endPoint) {
      if(runner_) {
        runner_->checkRotateTouchEvent(pointOnTouchBegin_, endPoint);
        runner_->clearMoveEventTags();
      }
    }
    void followPoint(Vec2 point) {
      if(runner_) runner_->shapFollowPoint(point);
    } 
    void rescheduleTimer(Scene* scene, bool isUp);
  private:
    Player():runner_(nullptr), timerT_(0.5){}
    Player(Player &Player) {}
    bool operator =(Player &Player) { return true; }
    Runner *runner_;
    Vec2 pointOnTouchBegin_; 
    float timerT_;
}; //Player
