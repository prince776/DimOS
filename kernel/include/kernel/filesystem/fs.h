#pragma once
#include <kernel/filesystem/resource.h>
#include <kernel/filesystem/vfs.h>
class FileSystem {
public:
    virtual int read(Resource* resource, uint32_t offset, uint32_t size, uint8_t* buffer) const = 0;
    virtual int write(Resource* resource, uint32_t offset, uint32_t size, uint8_t* buffer) = 0;
    virtual Vector<vfs::DirEntry> readDir(const vfs::Node& node) const = 0;
    // virtual void addResource(vfs::Node& node) = 0;
};
