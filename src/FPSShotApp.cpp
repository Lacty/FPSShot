
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


class Transform {
public:
  Vec3f pos;
  Vec3f scale;
  Vec3f rotation;
};

class Triangle {
public:
  Vec3f v1, v2, v3;
  Color color;
  Transform transform;
  
  Triangle();
  Triangle(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3,
           const Transform& transform) {
    this->v1        = v1;
    this->v2        = v2;
    this->v3        = v3;
    this->transform = transform;
  }
};

class HitTriangle {
public:
  bool operator < (const HitTriangle& polygon) const {
    return this->dist < polygon.dist;
  }
  
  HitTriangle();
  HitTriangle(Triangle& triangle, const float& dist,
              const Vec3f& intersect, const bool& is_hit) {
    this->triangle      = &triangle;
    this->dist          = dist;
    this->intersect     = intersect;
    this->is_hit        = is_hit;
    this->hit_bullet_id = 0;
  }
  Triangle* triangle;
  float     dist;
  Vec3f     intersect;
  bool      is_hit;
  int       hit_bullet_id;
};

class FPSShotApp : public AppNative {
private:
  Font font;
  
  list<Triangle>    triangles;
  list<HitTriangle> hit_polygon;

  int bullet_id;
  struct Bullet {
    int   id;
    bool  is_hit;
    Vec3f pos;
    Vec3f last;
    Vec3f size;
    Vec3f direction;
    float speed;
    Bullet(const int&   id,
           const Vec3f& pos,
           const Vec3f& size,
           const Vec3f& direction,
           const float& speed) {
      this->id        = id;
      this->is_hit    = false;
      this->pos       = pos;
      this->last      = pos;
      this->size      = size;
      this->direction = direction;
      this->speed     = speed;
    }
  };
  std::list<Bullet> bullets;
  
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
  
  triangles.push_back({ Vec3f(0, 40, 0), Vec3f(40, -20, 0), Vec3f(-40, -20, 0),
                      { Vec3f(300, 0, 400), Vec3f(1, 1, 1), Vec3f(0, 0, 0) } });
  
  triangles.push_back({ Vec3f(0, 40, 0), Vec3f(40, -20, 0), Vec3f(-40, -20, 0),
                      { Vec3f(350, 0, 410), Vec3f(1, 1, 1), Vec3f(0, 0, 0) } });
  
  for(auto& polygon : triangles) {
    hit_polygon.push_back({ polygon, 0.0f, Vec3f::zero(), false });
  }
  
  bullet_id = 0;
  
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
  hideCursor();

  GameCamera::getInstance().update();

  // 弾とpolygonの当たり判定
  for (auto& bullet : bullets) {
    bullet.last = bullet.pos;
    bullet.pos += bullet.direction * bullet.speed;
    for (auto& polygon : hit_polygon) {
      if (isCollision(polygon.triangle->v1 + polygon.triangle->transform.pos,
                      polygon.triangle->v2 + polygon.triangle->transform.pos,
                      polygon.triangle->v3 + polygon.triangle->transform.pos,
                      bullet.pos, bullet.last, polygon.intersect)) {
        bullet.is_hit         = true;
        polygon.hit_bullet_id = bullet.id;
        polygon.dist          = Vec3f(polygon.intersect - bullet.last).length();
      }
    }
  }

  // dist順にそーと
  hit_polygon.sort();
  
  // hitした弾を削除
  auto bullet_itr = bullets.begin();
  while(bullet_itr != bullets.end()) {
    if (bullet_itr->is_hit) {
      for (auto& polygon : hit_polygon) {
        if (polygon.hit_bullet_id == bullet_itr->id) {
          polygon.is_hit = true;
          bullet_itr = bullets.erase(bullet_itr);
        }
      }
    } else {
      bullet_itr++;
    }
  }
  
  // hitしたポリゴンの色を変更
  for (auto& polygon : hit_polygon) {
    if (polygon.is_hit) {
      polygon.triangle->color = Color::white();
    } else {
      polygon.triangle->color = Color(1, 0, 0);
    }
  }
  
  // カーソルを移動
  Mouse::getInstance().warpMousePos(Monitor::getInstance().getCenter());

  if (!Mouse::getInstance().Left().isPush) return;
  bullet_id++;
  bullets.emplace_back((Bullet(bullet_id, GameCamera::getInstance().cam().getEyePoint(), Vec3f(10, 10, 10), GameCamera::getInstance().cam().getViewDirection().normalized(), 300)));
}

void FPSShotApp::draw() {
  gl::clear(Color(0.4, 0.4, 0.4));
  gl::setMatrices(GameCamera::getInstance().cam());

  gl::pushModelView();
  gl::translate(Vec3f(0, 200, 500));
  gl::rotate(Vec3f(180, 180, 0));
  gl::drawStringCentered("FPSShot", Vec2f::zero(), Color(1, 0.6, 0), font);
  gl::popModelView();

  for (auto polygon : triangles) {
    gl::pushModelView();
    gl::translate(polygon.transform.pos);
    gl::color(polygon.color);
    gl::drawSolidTriangle(polygon.v1.xy(), polygon.v2.xy(), polygon.v3.xy());
    gl::popModelView();
  }
  
  for (auto bullet : bullets) {
    gl::color(Color::white());
    gl::drawCube(bullet.pos, bullet.size);
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
