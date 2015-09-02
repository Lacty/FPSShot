
#include "camera.h"
#include "../Monitor/monitor.h"
#include "../Mouse/mouse.h"
#include "cinder/app/AppNative.h"

#define PI 3.14159265358979


GameCamera& GameCamera::getInstance() {
  static GameCamera camera;
  return camera;
}

void GameCamera::create(const cinder::CameraPersp& cam) {
  camera = std::make_unique<cinder::CameraPersp>(cam);
  angle = cinder::Vec2f(PI, PI);
}

void GameCamera::update() {
  using namespace cinder;
  using namespace cinder::app;
  
  cinder::Vec2f difference;
  
  // カメラ横振り
  difference.x = cinder::app::AppBasic::getMousePos().x - Monitor::getInstance().getCenter().x;
  
  angle.x -= difference.x * 0.001;
  
  Quatf orientation = Quatf(Vec3f::yAxis(), angle.x);
  camera->setOrientation(orientation);
  
  
  // カメラ縦振り
  difference.y = Monitor::getInstance().getCenter().y - AppBasic::getMousePos().y;
  
  angle.y -= difference.y * 0.001;
  
  orientation = Quatf(Vec3f::yAxis().cross(camera->getViewDirection()), angle.y);
  camera->setOrientation(orientation);
  
  
  // カメラを上むきにする
  camera->setWorldUp(Vec3f::yAxis());
}

cinder::CameraPersp& GameCamera::cam() {
  return *camera;
}
