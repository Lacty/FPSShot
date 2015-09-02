
#pragma once


namespace ly {
  // このクラスを継承したクラスはコピー不可とする
  class Noncopyable {
  private:
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator = (const Noncopyable&) = delete;

  public:
    Noncopyable()  = default;
    ~Noncopyable() = default;
  };
}
