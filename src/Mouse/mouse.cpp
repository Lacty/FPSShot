
#include "mouse.h"
#include "../Monitor/monitor.h"


Mouse::Mouse() {
  pos   = cinder::Vec2i::zero();
  left  = { false, false, false };
  right = { false, false, false };
}


Mouse& Mouse::getInstance() {
  static Mouse mouse;
  return mouse;
}

void Mouse::flashInput() {
  left.isPush  = false;
  left.isPull  = false;
  right.isPush = false;
  right.isPull = false;
}

void Mouse::MoveEvent(cinder::app::MouseEvent event) {
  pos = event.getPos();
}

void Mouse::PushEvent(cinder::app::MouseEvent event) {
  left.isPush   = event.isLeftDown();
  left.isPress  = event.isLeft();
  right.isPush  = event.isRightDown();
  right.isPress = event.isRight();
}

void Mouse::PullEvent(cinder::app::MouseEvent event) {
  left.isPull = event.isLeft() ? true : false;
  right.isPull = event.isRight() ? true : false;

  left.isPress  = false;
  right.isPress = false;
}

void Mouse::warpMousePos(const cinder::Vec2i& pos) {
  CGAssociateMouseAndMouseCursorPosition(0);
  CGWarpMouseCursorPosition(Monitor::getInstance().getCenterCG());
  CGAssociateMouseAndMouseCursorPosition(1);
}

cinder::Vec2i Mouse::Pos() {
  return pos;
}

Mouse::MouseStatus Mouse::Left() {
  return left;
}

Mouse::MouseStatus Mouse::Right() {
  return right;
}
