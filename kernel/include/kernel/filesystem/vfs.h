#pragma once
#include <stdint-gcc.h>
#include <kernel/cpp/unique-ptr.hpp>
#include <kernel/cpp/string.hpp>
#include <kernel/filesystem/resource.h>

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
    virtual void makeDir(vfs::Node& node) = 0;
    virtual void populateVFSNode(vfs::Node& node, int inode) = 0;
};

namespace vfs {
    enum class NodeType {
        INVALID,
        FILE,
        DIRECTORY,
    };

    class Node {
    public:
        String<> name;
        NodeType type = NodeType::INVALID;
        FileSystem* fileSystem;
        Resource* resource;
        Vector<Node*> children;
        Node* parent;

        void populate(FileSystem* fs, const String<>& name, int inode) {
            this->name = name;
            fs->populateVFSNode(*this, inode);
        }
        Vector<vfs::DirEntry> readDir() { return fileSystem->readDir(*this); }
        void mkdir() { return fileSystem->makeDir(*this); }
        int read(Resource* resource, uint32_t offset, uint32_t size, uint8_t* buffer) const { return fileSystem->read(resource, offset, size, buffer); }
        int write(Resource* resource, uint32_t offset, uint32_t size, uint8_t* buffer) { return fileSystem->write(resource, offset, size, buffer); }
    };

    struct DirEntry {
        FileNameStr fileName;
        int inode;
    };

    class VFS {
    private:
        Node* root;
    public:
        VFS(FileSystem* fs, int inode = 0, const String<>& name = "");

        // Read contents of a dir.
        Vector<Node*> readDir(const String<>& path);
        // Create a new dir. All parent directories should exist.
        Node* mkdir(const String<>& path);
        // Open a file. For now just returns vfs node to it.
        Node* openNode(const String<>& path);

    private:
        Node* resolvePath(const String<>& path);
        Node* resolvePathUtil(Node* curr, const Vector<String<>>& pathEntries, int idx);
        void addChildren(Node* node);
    };

}
