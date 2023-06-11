#include "kernel/cpp/utility.hpp"
#include <kernel/gfx/fb.hpp>
#include <stdint.h>

extern vfs::VFS globalVFS;
namespace gfx {

Color Color::red = {0, 0, 255, 255};
Color Color::green = {0, 255, 0, 255};
Color Color::blue = {255, 0, 0, 255};
Color Color::black = {0, 0, 0, 255};
Color Color::white = {255, 255, 255, 255};

static int grid = 64;

FrameBuffer::FrameBuffer(int width, int height) : vfs(globalVFS), width(width), height(height) {
    fbFile = vfs.openNode("/dev/fb");
}

uint32_t FrameBuffer::getOffset(const Vec2& pos) const { return (pos.y * width + pos.x) * sizeof(Color); }

void FrameBuffer::putPixel(const Vec2& pos, const Color& color) {
    fbFile->write(getOffset(pos), sizeof(Color), (uint8_t*)&color);
}

void FrameBuffer::clscr(const Color& color) {
    for (int i = 0; i < height; i += grid) {
        Vector<Color> row(width * grid, color);
        fbFile->write(getOffset({0, i}), row.size() * sizeof(Color), (uint8_t*)&row[0]);
        // for (int j = 0; j < width; j++) {
        //     Vec2 pos = {j, i};
        //     putPixel(pos, color);
        // }
    }
}

void FrameBuffer::fillRect(const Vec2& pos, const Vec2& dimension, const Color& col) {
    int y1 = pos.y, y2 = min(pos.y + dimension.y - 1, height - 1);
    int x1 = pos.x, x2 = min(pos.x + dimension.x - 1, width - 1);
    for (int i = y1; i <= y2; i++) {
        Vector<Color> row(x2 - x1 + 1, col);
        fbFile->write(getOffset({x1, i}), row.size() * sizeof(Color), (uint8_t*)&row[0]);
        // for (int j = x1; j <= x2; j++) {
        //     putPixel({j, i}, col);
        // }
    }
}

} // namespace gfx
