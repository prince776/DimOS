#pragma once
#include <stdint-gcc.h>
#include <kernel/cpp/unique-ptr.hpp>
#include <kernel/cpp/string.hpp>
#include <kernel/filesystem/resource.h>
#include <kernel/filesystem/fs.h>

namespace vfs {
    struct Node;
    struct DirEntry;
    using FileNameStr = char[16];
}

class FileSystem {
public:
    virtual int read(Resource* resource, uint32_t offset, uint32_t size, uint8_t* buffer) const = 0;
    virtual int write(Resource* resource, uint32_t offset, uint32_t size, uint8_t* buffer) = 0;
    virtual Vector<vfs::DirEntry> readDir(const vfs::Node& node) const = 0;
    virtual void populateVFSNode(vfs::Node& node, int inode) = 0;
};


namespace vfs {
    enum class NodeType {
        INVALID,
        FILE,
        DIRECTORY,
    };

    struct Node {
        String<> name;
        NodeType type = NodeType::INVALID;
        FileSystem* fileSystem;
        Resource* resource;
        Vector<Node*> children;
        Node* parent;

        void populate(FileSystem* fs, int inode) {
            fs->populateVFSNode(*this, inode);
        }
    };

    struct DirEntry {
        FileNameStr fileName;
        int inode;
    };

    class VFS {
    private:
        Node* root;
    public:
        VFS(FileSystem* fs, int inode = 0, const String<>& name = "") {
            root = new Node();
            root->name = name;
            root->populate(fs, inode);
        }

    };

}
