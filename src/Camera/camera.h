
#pragma once
#include "../Noncopyable/noncopyable.h"
#include "cinder/Camera.h"


class GameCamera : public ly::Noncopyable {
private:
  std::unique_ptr<cinder::CameraPersp> camera;
  
  cinder::Vec2f angle;

  GameCamera() = default;

public:
  static GameCamera& getInstance();

  void create(const cinder::CameraPersp& cam);

  void update();

  cinder::CameraPersp& cam();
};
