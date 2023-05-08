#pragma once
#include <kernel/concurrency/primitives.h>
#include <kernel/filesystem/resource.h>
#include <stdint.h>

#include <kernel/cpp/string.hpp>
#include <kernel/cpp/unique-ptr.hpp>

namespace vfs {
struct Node;
struct DirEntry;
constexpr int FileNameStrLen = 16;
using FileNameStr = char[FileNameStrLen];

enum class NodeType {
    INVALID,
    FILE,
    DIRECTORY,
};
} // namespace vfs

class FileSystem {
  public:
    virtual void create(vfs::Node& node, vfs::NodeType type) = 0;
    virtual void remove(vfs::Node& node) = 0;

    virtual int read(const Resource& resource, uint32_t offset, uint32_t size,
                     uint8_t* buffer) const = 0;
    virtual int write(Resource& resource, uint32_t offset, uint32_t size, uint8_t* buffer) = 0;

    virtual Vector<vfs::DirEntry> readDir(const vfs::Node& node) const = 0;
    virtual void writeDir(vfs::Node& node, const Vector<vfs::DirEntry>& entries) = 0;
    virtual void removeDirEntry(vfs::Node& node, const vfs::DirEntry& entry) = 0;

    virtual void populateVFSNode(vfs::Node& node, int inode) = 0;
};

namespace vfs {

class Node {
  public:
    String<> name;
    NodeType type = NodeType::INVALID;
    FileSystem* fileSystem;
    Resource resource;
    Vector<Node*> children;
    Node* parent;

    Node(FileSystem* fs) : fileSystem(fs) {}

    void populate(FileSystem* fs, const String<>& name, int inode) {
        this->name = name;
        fs->populateVFSNode(*this, inode);
    }

    void create(NodeType type) { return fileSystem->create(*this, type); }
    void remove() { return fileSystem->remove(*this); }

    void mkdir() { return fileSystem->create(*this, NodeType::DIRECTORY); }
    void mkFile() { return fileSystem->create(*this, NodeType::FILE); }

    int read(uint32_t offset, uint32_t size, uint8_t* buffer) const {
        return fileSystem->read(resource, offset, size, buffer);
    }
    int write(uint32_t offset, uint32_t size, uint8_t* buffer) {
        return fileSystem->write(resource, offset, size, buffer);
    }

    Vector<vfs::DirEntry> readDir() { return fileSystem->readDir(*this); }
    void writeDir(const Vector<DirEntry>& entries) { return fileSystem->writeDir(*this, entries); }
    void removeDirEntry(const DirEntry& entry) { return fileSystem->removeDirEntry(*this, entry); }
};

struct DirEntry {
    FileNameStr fileName;
    int inode;

    static DirEntry fromStringName(const String<>& name, int inode) {
        if (name.size() > FileNameStrLen) {
            printf("'%s':Name will be truncated when being saved to Filesystem", name.c_str());
        }
        DirEntry result;
        result.inode = inode;
        int len = min((int)name.size(), FileNameStrLen);
        for (int i = 0; i < len; i++) {
            result.fileName[i] = name[i];
        }
        return result;
    }
} __attribute__((packed));

class VFS {
  private:
    Node* root;
    MutexLock mutex;

  public:
    VFS() = default;
    VFS(FileSystem* fs, int inode = 0, const String<>& name = "");

    // Read contents of a dir.
    Vector<Node*> readDir(const String<>& path);

    // Create a new dir. All parent directories should exist.
    Node* mkdir(const String<>& path);

    // Create a new file.
    Node* mkfile(const String<>& path);

    // Remove dirctory.
    void rmdir(const String<>& path);

    // Remove file.
    void rmfile(const String<>& path);

    // Read a node data.
    void read(const String<>& path, uint32_t offset, uint32_t size, uint8_t* buffer);

    // Write to node.
    void write(const String<>& path, uint32_t offset, uint32_t size, uint8_t* buffer);

    // Open a node. For now just returns vfs node to it.
    Node* openNode(const String<>& path);

    // Mount a different filesystem at a new path. Parent paths should be present.
    Node* mnt(const String<>& path, FileSystem* fs, int inode = 0);

  private:
    Node* resolvePath(const String<>& path);
    Node* resolvePathUtil(Node* curr, const Vector<String<>>& pathEntries, int idx);
    void addChildren(Node* node);
};
} // namespace vfs
