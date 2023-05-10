#include "kernel/limine.h"
#include <kernel/devices/framebuffer.h>

static volatile limine_framebuffer_request framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

void FramebuferDevice::create(vfs::Node&, vfs::NodeType) {
    // nothing to do here.
}

void FramebuferDevice::remove(vfs::Node&) {
    // nothing to do here.
}

FramebuferDevice::FramebuferDevice(int _deviceID) : deviceID(_deviceID) {
    auto fbr = framebuffer_request.response;
    if (fbr == nullptr || fbr->framebuffer_count < 1) {
        printf("fb is null or count is 0");
    }
    fb = fbr->framebuffers[0];
    fbSize = fb->width * fb->height * sizeof(uint32_t);

    fb_ptr = (uint32_t*)fb->address;

    printf("framebuffer: w: %d, h: %d, pixelW: %d\n", fb->width, fb->height, fb->pitch / fb->width);
}

int FramebuferDevice::getIdx(int pixel) { return pixel * (fb->pitch / 4) + pixel; }

int FramebuferDevice::read(const Resource&, uint32_t offset, uint32_t size, uint8_t* buffer) const {
    auto len = min((uint64_t)size, fbSize - offset);
    auto ptr = (uint8_t*)fb_ptr;
    for (uint32_t i = offset; i < offset + len; i++) {
        buffer[i - offset] = ptr[i];
    }
    return len;
}

int FramebuferDevice::write(Resource&, uint32_t offset, uint32_t size, uint8_t* buffer) {
    auto len = min((uint64_t)size, fbSize - offset);
    auto ptr = (uint8_t*)fb_ptr;
    for (uint32_t i = offset; i < offset + len; i++) {
        ptr[i] = buffer[i - offset];
    }
    return len;
}

Vector<vfs::DirEntry> FramebuferDevice::readDir(const vfs::Node&) const { return Vector<vfs::DirEntry>(); }
void FramebuferDevice::writeDir(vfs::Node&, const Vector<vfs::DirEntry>&) {}
void FramebuferDevice::removeDirEntry(vfs::Node&, const vfs::DirEntry&) {}

void FramebuferDevice::populateVFSNode(vfs::Node& node, int inode) {
    node.fileSystem = this;
    node.type = vfs::NodeType::FILE;
    node.resource.deviceID = deviceID;
    node.resource.inode = inode;
    node.resource.size = fbSize;
}
