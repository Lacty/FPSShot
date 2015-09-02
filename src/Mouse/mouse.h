
#pragma once
#include "cinder/app/MouseEvent.h"
#include "../Noncopyable/noncopyable.h"
#include <set>


class Mouse : public ly::Noncopyable {
public:
  struct MouseStatus {
    bool isPush;
    bool isPress;
    bool isPull;
  };

  static Mouse& getInstance();

  void flashInput();

  cinder::Vec2i Pos();
  MouseStatus   Left();
  MouseStatus   Right();

  void MoveEvent(cinder::app::MouseEvent event);
  void PushEvent(cinder::app::MouseEvent event);
  void PullEvent(cinder::app::MouseEvent event);
  
  void warpMousePos(const cinder::Vec2i& pos);

private:
  Mouse();

  cinder::Vec2i pos;
  MouseStatus   left, right;
};
