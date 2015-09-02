
#include "monitor.h"


Monitor& Monitor::getInstance() {
  static Monitor monitor;
  return monitor;
}

void Monitor::setMonitorSize(const cinder::Vec2f& size) {
  this->size = size;
  size_cg.width  = size.x;
  size_cg.height = size.y;
}

void Monitor::setCenterPos(const cinder::Vec2i& center) {
  this->center = center;
  this->center_cg.x = center.x;
  this->center_cg.y = center.y;
}

CGSize Monitor::getSizeCG() const {
  return size_cg;
}

CGPoint Monitor::getCenterCG() const {
  return center_cg;
}

cinder::Vec2i Monitor::getSize() const {
  return size;
}

cinder::Vec2i Monitor::getCenter() const {
  return center;
}