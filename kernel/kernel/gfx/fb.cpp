#include <kernel/gfx/fb.hpp>

extern vfs::VFS globalVFS;
namespace gfx {

Color Color::red = {255, 0, 0, 255};
Color Color::green = {0, 255, 0, 255};
Color Color::blue = {0, 0, 255, 255};
Color Color::black = {0, 0, 0, 255};
Color Color::white = {255, 255, 255, 255};

FrameBuffer::FrameBuffer(int width, int height) : vfs(globalVFS), width(width), height(height) {
    fbFile = vfs.openNode("/dev/fb");
    buffer = Vector<Color>(width * height);
}

void FrameBuffer::clscr(const Color& color) {
    buffer.fill(color);
    render();
}

void FrameBuffer::render() {
    uint8_t* buf = (uint8_t*)&buffer[0];
    uint32_t size = buffer.size() * sizeof(Color);
    printf("size of color: %d\n", sizeof(Color));
    fbFile->write(0, size, buf);
}

} // namespace gfx
