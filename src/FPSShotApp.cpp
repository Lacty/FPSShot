
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

#include "cinder/Camera.h"
#include "cinder/Matrix.h"

#include "Camera/camera.h"
#include "Monitor/monitor.h"
#include "Mouse/mouse.h"

#include <ApplicationServices/ApplicationServices.h>

using namespace ci;
using namespace ci::app;
using namespace std;

class FPSShotApp : public AppNative {
private:
  Font font;

  struct Face {
    bool hit;
    Vec3f v1, v2, v3;
    Vec3f offset;
    Color color;
  };
  Face face;

  struct Gun {
    Vec3f pos;
    Vec3f last;
    Vec3f size;
    Vec3f direction;
    Gun(const Vec3f& pos,
        const Vec3f& size,
        const Vec3f& direction) {
      this->pos       = pos;
      this->size      = size;
      this->direction = direction;
    }
  };
  std::list<Gun> guns;
  
  bool isCollision(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3,
                   const Vec3f& r1, const Vec3f& r2, Vec3f& IntersectPos);

public:
  void setup() override;
  void prepareSettings(Settings* settings) override;

  void mouseDown(MouseEvent event) override {
    Mouse::getInstance().PushEvent(event);
  }
  void mouseUp(MouseEvent event) override {
    Mouse::getInstance().PullEvent(event);
  }
  void mouseMove(MouseEvent event) override {
    Mouse::getInstance().MoveEvent(event);
  }
  void keyDown(KeyEvent event) override;

  void update() override;
  void draw() override;
};

void FPSShotApp::setup() {
  Monitor::getInstance();
  GameCamera::getInstance();
  Mouse::getInstance();

  font = Font(loadAsset("rounded-l-mplus-1c-light.ttf"), 40);

  // フルスクに設定
  setFullScreen(true);

  // モニターのサイズを取得
  Vec2i monitor = Vec2i(CGDisplayPixelsWide(0),
                        CGDisplayPixelsHigh(0));

  Monitor::getInstance().setMonitorSize(monitor);
  Monitor::getInstance().setCenterPos(monitor * 0.5f);

  CameraPersp camera;
  Vec3f eye;
  Vec3f target;
  camera = CameraPersp(getWindowWidth(), getWindowHeight(), 60.0f, 0.5f, 10000.0f);
  eye    = Vec3f::zero();
  target = Vec3f(0, 0, 10000.0f);
  camera.lookAt(eye, target);
  camera.setWorldUp(Vec3f::yAxis());

  GameCamera::getInstance().create(camera);

  face.v1 = Vec3f(0, 40, 0);
  face.v2 = Vec3f(40, -20, 0);
  face.v3 = Vec3f(-40, -20, 0);
  face.offset = Vec3f(300, 0, 500);
  face.color = Color(1, 1, 0);

  gl::enableAlphaBlending();
  glBlendFunc(GL_SRC0_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl::enable(GL_CULL_FACE);
  gl::enableDepthRead();
  gl::enableDepthWrite();
}

void FPSShotApp::prepareSettings(Settings* settings) {
  settings->disableFrameRate();
}

void FPSShotApp::keyDown(KeyEvent event) {
  quit();
}

void FPSShotApp::update() {
  showCursor();
  // カーソルを非表示に設定
  //hideCursor();

  GameCamera::getInstance().update();

  Vec3f PIP;
  for (auto& it : guns) {
    it.last = it.pos;
    it.pos += it.direction * 150;
    if (isCollision(face.v1 + face.offset, face.v2 + face.offset, face.v3 + face.offset, it.pos, it.last, PIP)) {
      face.hit = face.hit ? false : true;
    }
  }
  
  if (face.hit) {
    face.color = Color::white();
  } else {
    face.color = Color(1, 0, 0);
  }
  
  // カーソルを移動
  Mouse::getInstance().warpMousePos(Monitor::getInstance().getCenter());

  if (!Mouse::getInstance().Left().isPush) return;
  guns.emplace_back((Gun(GameCamera::getInstance().cam().getEyePoint(), Vec3f(10, 10, 10), GameCamera::getInstance().cam().getViewDirection().normalized())));
  console() << GameCamera::getInstance().cam().getViewDirection().normalized() << endl;
}

void FPSShotApp::draw() {
  gl::clear(Color(0.4, 0.4, 0.4));
  gl::setMatrices(GameCamera::getInstance().cam());

  gl::pushModelView();
  gl::translate(Vec3f(0, 200, 500));
  gl::rotate(Vec3f(180, 180, 0));
  gl::drawStringCentered("FPSShot", Vec2f::zero(), Color(1, 0.6, 0), font);
  gl::popModelView();

  gl::pushModelView();
  gl::translate(face.offset);
  gl::color(face.color);
  gl::drawSolidTriangle(face.v1.xy(), face.v2.xy(), face.v3.xy());
  gl::popModelView();
  
  for (auto it : guns) {
    gl::color(Color::white());
    gl::drawCube(it.pos, it.size);
  }

  gl::color(Color::white());
  gl::drawCube(Vec3f(0, 0, 300), Vec3f(50, 50, 50));

  Mouse::getInstance().flashInput();
}


bool FPSShotApp::isCollision(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3,
                                const Vec3f& r1, const Vec3f& r2, Vec3f& IntersectPos)
{
  Vec3f Normal, Center, Intersect;
  
  // Find Triangle Normal
  Normal = (v3 - v1).cross(v2 - v1);
  Normal.normalize();
  
  // Find Triangle Center #1/3 = 0.33333333...
  Center = [&] {
    return (0.33333333f * v1) +
    (0.33333333f * v2) +
    (0.33333333f * v3);
  } ();
  
  // Find Distance from r1,r2 to the Plane Center
  Vec3f Dist1 = r1 - Center;
  Vec3f Dist2 = r2 - Center;
  
  // It Checks whether the line intersects the plane
  if (!(Dist1.dot(Normal) * Dist2.dot(Normal) <= 0.0f)) {
    return false;
  }
  
  // Find Vertical Distance from r1,r2 to the Plane
  float VerticalDist1 = [&] {
    return abs(Normal.dot(Vec3f(r1 - v1)));
  } ();
  
  float VerticalDist2 = [&] {
    return abs(Normal.dot(Vec3f(r2 - v1)));
  } ();
  
  // Find Internal Ratio
  float InternalRatio = [&] {
    return VerticalDist1 / (VerticalDist1 + VerticalDist2);
  } ();
  
  // Find Vector from v1 to Intersect Position
  Intersect = [&] {
    float m = InternalRatio;
    float n = 1.0f - InternalRatio;
    float x = (n * r1.x + m * r2.x) / (m + n);
    float y = (n * r1.y + m * r2.y) / (m + n);
    float z = (n * r1.z + m * r2.z) / (m + n);
    return Vec3f(x, y, z);
  } ();
  
  // It Checks whether the IntersectPos exist in the Polygon
  bool isExist = [&] {
    Vec3f cross1 = Vec3f(v3 - v1).cross(Vec3f(Intersect - v1)).normalized();
    Vec3f cross2 = Vec3f(v1 - v2).cross(Vec3f(Intersect - v2)).normalized();
    Vec3f cross3 = Vec3f(v2 - v3).cross(Vec3f(Intersect - v3)).normalized();
    
    double dot_12 = cross1.dot(cross2);
    double dot_13 = cross1.dot(cross3);
    
    if (dot_12 > 0 && dot_13 > 0) {
      return true;
    } else {
      return false;
    }
  } ();
  
  if (!isExist) return false;
  
  IntersectPos = Intersect;
  return true;
}

CINDER_APP_NATIVE(FPSShotApp, RendererGl)
