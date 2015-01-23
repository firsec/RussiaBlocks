#include "RussiaBlocks.h"
USING_NS_CC;

void ScreenImpl::colorClickedBlock(Vec2 &loc) {
  if(loc.x > playOrig_.x + playSize_.width || loc.y < playOrig_.y) { 
    return;
  }
  Vec2 pos = getBlockPosByPixLoc(loc);
  auto block = getBlock(pos.x, pos.y);
  block->setColor(Color3B::BLACK);
}

inline ScreenImpl* ScreenImpl::createWithArgs(
    Point &playOrig, Size &playSize, int width, int height) { 
  ScreenImpl *var = new ScreenImpl(playOrig, playSize, width, height);
  if (var && var->init()) {
    var->autorelease();
    return var;
  }
  CC_SAFE_DELETE(var);
  return nullptr;
}

bool ScreenImpl::init() {
  ScreenBlock *block = NULL;
  for(int i=0;i<width_*height_; ++i) {
    block = ScreenBlock::create();
    if(block) { 
      block->setContentSize(blockSize_);
      block->setAnchorPoint(Point::ZERO);
      block->setTextureRect(Rect(1, 1, blockSize_.width-1, blockSize_.height-1));
      block->setColor(DEFAULT_BLOCK_COLOR);
      blockArr_.push_back(block);
    }
  }
  for(int j=0; j<height_; ++j) {
    for(int i=0; i<width_; ++i) {
      block = getBlock(i,j);
      block->setPosition(Point(playOrig_.x + i*blockSize_.width, playOrig_.y + j*blockSize_.height));
      addChild(block);
    }
  }
  return true;
}


void Runner::moveLeft() {
}
void Runner::moveRight() {
}
void Runner::drop() {
}
void Runner::dropToBottom() {
}
//void Runner::genShap() {
//}

//need to lock before modify color of block in shift left, right and dropToBottom func.
//need a class to save shape??
void Runner::run() { 
  log("run");
  if(state_ == DROPPING) {
    log("state dropping.");
    //checkStuck();
    //drop();
  } else if(state_ == TOGENSHAP) {
    log("state to gen shap.");
//    genShap();
    state_ = DROPPING;
  } else if(state_ == TOSTART) {
    log("state to start.");
    clearScreen();
    state_ = TOGENSHAP;
  } 
}

void Runner::runClkForScheduler(float tmp) {
  Player::getInstance().getRunner()->run();
}

void Player::play(Scene* scene, ScreenImpl* screenArr) {
  runner_ = Runner::createWithArgs(screenArr);
  runner_->retain();

  //there are many types of macro to select call func type.
  //need deep analysis the schedule.  
  //in call back func. If you want to visit data. need to reget instance.
  //May be because of static_cast of the callBack function to Ref:: so it can't visit data??.
  //also can't use lamada because the type is not std::function??
  scene->schedule(schedule_selector(Runner::runClkForScheduler), 0.5f);
  //to unschedule it.
  //unschedule(schedule_selector(Runner::run))
}

Scene* Player::createStartScene() {
  auto scene = Scene::create();
  //prepare size and orig.
  Size visibleSize = Director::getInstance()->getVisibleSize();
  Vec2 origin = Director::getInstance()->getVisibleOrigin();
  Vec2 playOrig(origin.x, origin.y + visibleSize.height*0.2);
  Size playSize(visibleSize.width*0.8, visibleSize.height*0.8);
  auto colorPlayAreaBody = Color3B(200,200,255);
  auto colorPlayAreaOutLine = Color3B(255,50,50);
  Vec2 stripePlayArea[4] = {
    Vec2(playOrig.x, playOrig.y),
    Vec2(playOrig.x, playOrig.y + playSize.height),
    Vec2(playOrig.x + playSize.width, playOrig.y + playSize.height),
    Vec2(playOrig.x + playSize.width, playOrig.y)
  };
  //prepare playArea
  DrawNode *playAreaNode = DrawNode::create();
//  playAreaNode->setContentSize(playSize);
  playAreaNode->setAnchorPoint(Vec2(0,0));
  playAreaNode->drawPolygon(stripePlayArea, 4, Color4F(colorPlayAreaBody), 1, Color4F(colorPlayAreaOutLine));
  playAreaNode->setPosition(origin);
  scene->addChild(playAreaNode, -5);
  //create screen block arr.
  auto screenArr = ScreenImpl::createWithArgs(playOrig, playSize);
  scene->addChild(screenArr, -3);
  
  auto listener = EventListenerTouchOneByOne::create();
  listener->onTouchBegan = [scene, origin, screenArr](Touch *t, Event *e) {
    Vec2 loc = t->getLocation() - origin;
    log("orig %g %g", origin.x, origin.y);
    log("click %g %g",loc.x, loc.y);
    screenArr->colorClickedBlock(loc);
    return false;
  };
  Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener,scene);

  //create control pannel
  Vec2 playControlOrig(origin);
  Vec2 gameMenuOrig(playOrig.x + playSize.width, playOrig.y);
  Size gameMenuSize(visibleSize.width - playSize.width, playSize.height);

  auto menuNode = Node::create();
  auto exitMenuItem = MenuItemFont::create("Exit");
  auto startMenuItem = MenuItemFont::create("Start");
  exitMenuItem->setFontNameObj("Marker Felt.ttf");
  startMenuItem->setFontNameObj("Marker Felt.ttf");
  startMenuItem->setFontSizeObj(32);
  exitMenuItem->setFontSizeObj(32);
  exitMenuItem->setCallback([&](Ref *pSender) {
      Director::getInstance()->end();
      });
  startMenuItem->setCallback([this, scene, screenArr](Ref *pSender) {
      this->play(scene, screenArr);
      //Player* player = Player::getInstance();
      //sence->addChild(player);
      });
  auto menu = Menu::create(exitMenuItem, startMenuItem, NULL);
  menu->setAnchorPoint(Vec2(0,0));
  menu->setPosition(gameMenuOrig.x , 
                    gameMenuOrig.y);
  startMenuItem->setPosition(gameMenuSize.width/2, exitMenuItem->getContentSize().height/2);
  exitMenuItem->setPosition(gameMenuSize.width/2, exitMenuItem->getContentSize().height*3);
  menuNode->addChild(menu, 0);
  scene->addChild(menuNode, -4);
 // MessageBox("Hell","WW");

  return scene;

}
