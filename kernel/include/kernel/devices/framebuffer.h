#pragma once

#include <kernel/filesystem/vfs.h>

class FramebuferDevice : public FileSystem {
  public:
    FramebuferDevice(int deviceID);

    void create(vfs::Node& node, vfs::NodeType type) override;
    void remove(vfs::Node& node) override;
    int read(const Resource& resource, uint32_t offset, uint32_t size, uint8_t* buffer) const override;
    int write(Resource& resource, uint32_t offset, uint32_t size, uint8_t* buffer) override;
    [[nodiscard]] Vector<vfs::DirEntry> readDir(const vfs::Node& node) const override;
    void writeDir(vfs::Node& node, const Vector<vfs::DirEntry>& entries) override;
    void removeDirEntry(vfs::Node& node, const vfs::DirEntry& entry) override;
    void populateVFSNode(vfs::Node& node, int inode) override;

  private:
    int getIdx(int pixel);

  private:
    limine_framebuffer* fb{};
    uint32_t* fb_ptr;
    uint64_t fbSize{};
    int deviceID{};
};
