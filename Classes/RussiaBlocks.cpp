#include "RussiaBlocks.h"
USING_NS_CC;

void ScreenImpl::colorBlock(Vec2 &pos, Color3B color) {
  //log("%g %g", pos.x, pos.y);
  auto block = getBlock(pos.x, pos.y);
  block->setColor(color);
}

void ScreenImpl::colorBlock(Vec2 &&pos, Color3B color) {
  //log("---%g %g", pos.x, pos.y);
  auto block = getBlock(pos.x, pos.y);
  block->setColor(color);
}

void ScreenImpl::colorClickedBlock(Vec2 &loc) {
  if(loc.x > playOrig_.x + playSize_.width || loc.y < playOrig_.y) { 
    return;
  }
  Vec2 pos = getBlockPosByPixLoc(loc);
  colorBlock(pos);
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
      block->retain();
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

bool ScreenImpl::isStuck(int checkWhichSide) {
  for(auto &i : curShap_->blocks_) {
    if(checkWhichSide == BOTTOM) {
      if(i.y - 1 < 0 || getBlock(i.x, i.y - 1)->getTag() == FIXED) {
        return true;
      }
    } else if(checkWhichSide == LEFT) {
      if(i.x - 1 < 0 || getBlock(i.x - 1, i.y)->getTag() == FIXED) { 
        return true;
      }
    } else {
      //check right
      if(i.x + 1 >= width_ || getBlock(i.x + 1, i.y)->getTag() == FIXED) {
        return true;
      }
    }
  }
  return false;
}

bool ScreenImpl::dropShap(bool isToBottom) {
  if(isToBottom) {
    //checkStuck()
    do {
      if(!dropShap(false)) {
        return false;
      }
    } while(1); 
  } else {
    //log("dropShap");
    //check stuck
    if(isStuck(BOTTOM)) {
      //fix curBlocks
      for(auto &i: curShap_->blocks_) {
        getBlock(i.x, i.y)->setTag(FIXED);
      }
      return false;
    } else {
      //drop cur blocks
//      int minY = height_;
//      for(int i = 0; i < 4; ++i) {
//        if(p[i].y < minY)
//          minY = p[i].y;
//      }
      //if(minY > 0) {
      auto p = curShap_->blocks_;
      curShap_->centor_.y -= 1;
        for(int i = 0; i < 4; ++i) {
          colorBlock(Vec2(p[i].x, p[i].y - 1));
          colorBlock(p[i], DEFAULT_BLOCK_COLOR);
          p[i].y -= 1;
        }
      //}
      return true;
    }
  }
}

bool ScreenImpl::moveShapRight() {
  auto p = curShap_->blocks_;
  //checkStuck()
  if(!isStuck(RIGHT)) {
    log("moveShapRight");
    curShap_->centor_.x += 1;
    for(int i = 3; i >= 0; --i) {
      colorBlock(Vec2(p[i].x+1, p[i].y));
      colorBlock(p[i], DEFAULT_BLOCK_COLOR);
      p[i].x += 1;
    }
    return true;
  }
  return false;
}

bool ScreenImpl::moveShapLeft() {
  auto p = curShap_->blocks_;
  //checkStuck()
  if(!isStuck(LEFT)) {
    log("moveSLeft");
    curShap_->centor_.x -= 1;
    for(int i = 0; i < 4; ++i) {
      colorBlock(Vec2(p[i].x-1, p[i].y));
      colorBlock(p[i], DEFAULT_BLOCK_COLOR);
      p[i].x -= 1;
    }
    return true;
  }
  return false;
}

//shap types: 0 is T shap.
void ScreenImpl::genShap() {
  srandom(time(0));
  curShap_->rotation_ = rand()%4;
  int shap = rand()%SHAPNR;
  Vec2 midTopPos = getMidTopPos();
  //here can be changed to vector<std::function<>>
  //shap == TSHAP
  if(shap == TSHAP) {// 0 
    curShap_->blocks_[0] = midTopPos;
    curShap_->blocks_[1] = Vec2(midTopPos.x, midTopPos.y - 1);
    curShap_->blocks_[2] = Vec2(midTopPos.x - 1, midTopPos.y - 1);
    curShap_->blocks_[3] = Vec2(midTopPos.x + 1, midTopPos.y - 1);
    curShap_->centor_ = curShap_->blocks_[1];
  } else if(shap == LSHAPL) { // 1
    curShap_->blocks_[0] = midTopPos;
    curShap_->blocks_[1] = Vec2(midTopPos.x, midTopPos.y - 1);
    curShap_->blocks_[2] = Vec2(midTopPos.x, midTopPos.y - 2);
    curShap_->blocks_[3] = Vec2(midTopPos.x - 1, midTopPos.y - 2);
    curShap_->centor_ = curShap_->blocks_[1];
  } else if(shap == LSHAPR) { // 2
    curShap_->blocks_[0] = midTopPos;
    curShap_->blocks_[1] = Vec2(midTopPos.x, midTopPos.y - 1);
    curShap_->blocks_[2] = Vec2(midTopPos.x, midTopPos.y - 2);
    curShap_->blocks_[3] = Vec2(midTopPos.x + 1, midTopPos.y - 2);
    curShap_->centor_ = curShap_->blocks_[1];
  } else if(shap == ZSHAPL) { // 3
    curShap_->blocks_[0] = midTopPos;
    curShap_->blocks_[1] = Vec2(midTopPos.x - 1, midTopPos.y);
    curShap_->blocks_[2] = Vec2(midTopPos.x, midTopPos.y - 1);
    curShap_->blocks_[3] = Vec2(midTopPos.x + 1, midTopPos.y - 1);
    curShap_->centor_ = curShap_->blocks_[0];
  } else if(shap == ZSHAPR) { // 4
    curShap_->blocks_[0] = midTopPos;
    curShap_->blocks_[1] = Vec2(midTopPos.x + 1, midTopPos.y);
    curShap_->blocks_[2] = Vec2(midTopPos.x, midTopPos.y - 1);
    curShap_->blocks_[3] = Vec2(midTopPos.x - 1, midTopPos.y - 1);
    curShap_->centor_ = curShap_->blocks_[0];
  } else if(shap == ISHAP) { // 5
    curShap_->blocks_[0] = midTopPos;
    curShap_->blocks_[1] = Vec2(midTopPos.x + 1, midTopPos.y);
    curShap_->blocks_[2] = Vec2(midTopPos.x - 1, midTopPos.y);
    curShap_->blocks_[3] = Vec2(midTopPos.x - 2, midTopPos.y);
    curShap_->centor_ = curShap_->blocks_[0];
  } else if(shap == OSHAP) {
    curShap_->blocks_[0] = midTopPos;
    curShap_->blocks_[1] = Vec2(midTopPos.x + 1, midTopPos.y);
    curShap_->blocks_[2] = Vec2(midTopPos.x, midTopPos.y - 1);
    curShap_->blocks_[3] = Vec2(midTopPos.x + 1, midTopPos.y - 1);
    curShap_->centor_ = midTopPos;
  } else { // ISHAP
    curShap_->blocks_[0] = midTopPos;
    curShap_->blocks_[1] = Vec2(midTopPos.x + 1, midTopPos.y);
    curShap_->blocks_[2] = Vec2(midTopPos.x - 1, midTopPos.y);
    curShap_->blocks_[3] = Vec2(midTopPos.x - 2, midTopPos.y);
    curShap_->centor_ = curShap_->blocks_[0];
  }
  for(auto &i : curShap_->blocks_) {
    colorBlock(i);
  }
  //ensure blocks_[0] is always the left block.
  //and blocks_[3] is always the right blocks.
  //less Nr is near bottom.
  std::sort(curShap_->blocks_, curShap_->blocks_ + 4);
  //fitRotation();
}
bool ScreenImpl::isLegal(Vec2 &&point) {
  if(point.x < 0 || point.x >= width_ 
      || point.y < 0 || point.y >= height_ 
      || getBlock(point.x, point.y)->getTag() == FIXED) {
    return false;
  }
  return true;
}
bool ScreenImpl::isLegal(Vec2 &point) {
  if(point.x < 0 || point.x >= width_ 
      || point.y < 0 || point.y >= height_ 
      || getBlock(point.x, point.y)->getTag() == FIXED) {
    return false;
  }
  return true;
}

void ScreenImpl::rotateShap() {
  //clear old color and rotate shap.
  auto &c = curShap_->centor_;
  if(c.x < 0 || c.y < 0)
    return;
  for(auto &i : curShap_->blocks_) {
    if(i != c) {
      //rotate 90 clockwise
      Vec2 newPoint(i.x - c.x, i.y - c.y);
      if(!isLegal(Vec2(c.x + newPoint.y, c.y - newPoint.x))) {
        return;
      }
    }
  }
  for(auto &i : curShap_->blocks_) {
    getBlock(i.x, i.y)->setColor(DEFAULT_BLOCK_COLOR);
    if(i != c) {
      //rotate 90 clockwise
      Vec2 newPoint(i.x - c.x, i.y - c.y); 
      i.x = c.x + newPoint.y;
      i.y = c.y - newPoint.x;
    }
  }
  for(auto &i : curShap_->blocks_) {
    //log("x %g y %g", i.x, i.y);
    getBlock(i.x, i.y)->setColor(SELECTED_BLOCK_COLOR);
  }
  std::sort(curShap_->blocks_, curShap_->blocks_ + 4);

}

void ScreenImpl::eraseFullLine() {
  for(int j = 0; j < height_; ++j) {
    bool isFullFixed = true;
    bool isFullFree = true;
    for(int i = 0; i < width_; ++i) {
      if(getBlock(i, j)->getTag() == FREE) {
        isFullFixed = false;
      } else {
        isFullFree = false;
      }
    }
    if(isFullFixed) {
      //drop all fixed blocks one step;
      for(int jj = j; jj < height_ - 1; ++jj) {
        for(int i = 0; i < width_; ++i) {
          getBlock(i, jj)->setTag(getBlock(i, jj + 1)->getTag());
          getBlock(i, jj)->setColor(getBlock(i, jj + 1)->getColor());
        }
      }
      --j;
    }
    if(isFullFree) {
      return;
    }
  }
}

void ScreenImpl::shapFollowPointMovement(Vec2 oldPoint, Vec2 curPoint) {
  Vec2 shapCentorOld = shapCentorPointOnTouchBegin_;
  double targetX = shapCentorOld.x + curPoint.x - oldPoint.x;
  log("old %g %g, cur %g %g, shapCentorOld %g %g, targetX %g", oldPoint.x, oldPoint.y, curPoint.x, curPoint.y,
      shapCentorOld.x, shapCentorOld.y, targetX);
  Vec2 shapCentorNew = getBlock(curShap_->centor_.x, curShap_->centor_.y)->getPosition();
  while(!(shapCentorNew.x <= targetX + blockSize_.width/2 
        && shapCentorNew.x >= targetX - blockSize_.width/2)) {
    if(shapCentorNew.x < targetX) {
      if(!moveShapRight()) 
        return;
    } else {
      if(!moveShapLeft())
        return;
    }
    shapCentorNew = getBlock(curShap_->centor_.x, curShap_->centor_.y)->getPosition();
  }
//  int i = (curPoint.x - oldPoint.x)/blockSize_.width;
//  int n = abs(i);
//  for(int j = 0; j < n; ++j) {
//    if(i < 0) {
//      moveShapLeft();
//    } else {
//      moveShapRight();
//    }
//  }
}

void Runner::rotate() {
  if(state_ == DROPPING) {
    screenArr_->rotateShap();
  }
}

void Runner::moveLeft() {
  if(state_ == DROPPING) {
    screenArr_->moveShapLeft();
  }
}
void Runner::moveRight() {
  if(state_ == DROPPING) {
    screenArr_->moveShapRight();
  }
}
void Runner::drop() {
  if(state_ == DROPPING) {
    if(!screenArr_->dropShap(false/*one step*/)) {
      screenArr_->eraseFullLine();
      state_ = TOGENSHAP;
    }
  }
}

void Runner::genShap() {
  screenArr_->genShap();
}

//need to lock before modify color of block in shift left, right and dropToBottom func.
//need a class to save shape??
void Runner::run() { 
  log("start to run");
  if(state_ == DROPPING) {
    //log("state dropping.");
    drop();
  } else if(state_ == TOGENSHAP) {
    log("state to gen shap.");
    genShap();
    refreshScreenArrShapCentorPointForTouch();
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
  scene->schedule(schedule_selector(Runner::runClkForScheduler), 0.2f);
  //to unschedule it.
  //unschedule(schedule_selector(Runner::run))
}

void Player::rescheduleTimer(Scene* scene, bool isUp) {
  //scene->unschedule(schedule_selector(Runner::runClkForScheduler));
  if(!isUp) {
    timerT_ += 0.04;  
  } else if(timerT_ > 0.05 ) {
    timerT_ -= 0.04;
  }
  if(getRunner()) {
    scene->schedule(schedule_selector(Runner::runClkForScheduler), timerT_);
  }
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
  auto screenArr = ScreenImpl::createWithArgs(playOrig, playSize, 12, 24);
  scene->addChild(screenArr, -3);

  //create touch listener.
  auto listener = EventListenerTouchOneByOne::create();
  listener->onTouchBegan = [this, scene, origin, screenArr](Touch *t, Event *e) {
    this->saveOnTouchBeginPoint(t->getLocation());
    //screenArr->colorClickedBlock(loc);
    //if want to process move and other event, need to return true.
    return true;
  };
  listener->onTouchMoved = [this, origin](Touch *t, Event *e) {
    return processOnTouchMoved(t);
  };
  //call any function must ensure the pointer is initialized.
  listener->onTouchEnded = [this](Touch *t, Event *e) {
    this->processOnTouchEnd(t->getLocation());
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
  auto speedUpItem = MenuItemFont::create("SpeedUp");
  auto speedDownItem = MenuItemFont::create("SpeedDown");
  speedDownItem->setCallback([scene](Ref *pSender){
      Player::getInstance().rescheduleTimer(scene, false);
      });
  speedUpItem->setCallback([scene](Ref *pSender){
      Player::getInstance().rescheduleTimer(scene, true);
      });
  speedUpItem->setFontNameObj("Marker Felt.ttf");
  speedDownItem->setFontNameObj("Marker Felt.ttf");
  exitMenuItem->setFontNameObj("Marker Felt.ttf");
  startMenuItem->setFontNameObj("Marker Felt.ttf");
  startMenuItem->setFontSizeObj(32);
  speedUpItem->setFontSizeObj(32);
  speedDownItem->setFontSizeObj(32);
  exitMenuItem->setFontSizeObj(32);
  exitMenuItem->setCallback([&](Ref *pSender) {
      Director::getInstance()->end();
      });
  startMenuItem->setCallback([this, scene, screenArr](Ref *pSender) {
      this->play(scene, screenArr);
      });
  auto menu = Menu::create(exitMenuItem, startMenuItem, speedUpItem, speedDownItem, NULL);
  menu->setAnchorPoint(Vec2(0,0));
  menu->setPosition(gameMenuOrig.x , 
                    gameMenuOrig.y);
  startMenuItem->setPosition(gameMenuSize.width/2, exitMenuItem->getContentSize().height/2);
  exitMenuItem->setPosition(gameMenuSize.width/2, exitMenuItem->getContentSize().height*3);
  speedUpItem->setPosition(gameMenuSize.width/2, exitMenuItem->getContentSize().height*7); 
  speedDownItem->setPosition(gameMenuSize.width/2, exitMenuItem->getContentSize().height*5); 
  
  menuNode->addChild(menu, 0);
  scene->addChild(menuNode, -4);
  // MessageBox("Hell","WW");

  return scene;

}
