#pragma once

#include <kernel/filesystem/vfs.h>

namespace gfx {
struct Color {
    uint8_t r{}, g{}, b{}, a{};

    static Color red, green, blue, black, white;
};

struct Vec2 {
    int x{}, y{};
};

class FrameBuffer {
  public:
    FrameBuffer(int width, int height);

    void clscr(const Color& color = Color::black);

    void putPixel(const Color& color);

    void drawRect(const Vec2& pos, const Vec2& dimension, const Color& color);

    void fillRect(const Vec2& pos, const Vec2& dimension, const Color& color);

    void render();

  private:
    vfs::VFS& vfs;
    vfs::Node* fbFile;
    Vector<Color> buffer;
    int width, height;
};
} // namespace gfx
