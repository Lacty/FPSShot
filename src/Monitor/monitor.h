
#pragma once
#include "../Noncopyable/noncopyable.h"
#include "cinder/app/AppNative.h"
#include <ApplicationServices/ApplicationServices.h>


class Monitor : public ly::Noncopyable {
private:
  CGSize  size_cg;
  CGPoint center_cg;
  
  cinder::Vec2i size;
  cinder::Vec2i center;

public:
  static Monitor& getInstance();

  void setMonitorSize(const cinder::Vec2f& size);
  void setCenterPos(const cinder::Vec2i& center);
  
  CGSize  getSizeCG()   const;
  CGPoint getCenterCG() const;
  
  cinder::Vec2i getSize()   const;
  cinder::Vec2i getCenter() const;
};
