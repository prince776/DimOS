#pragma once
#include <stdint-gcc.h>
#include <kernel/cpp/unique-ptr.hpp>
#include <kernel/cpp/string.hpp>
#include <kernel/filesystem/resource.h>
#include <kernel/filesystem/fs.h>

namespace vfs {
    struct Node;
    enum class NodeType {
        INVALID,
        FILE,
        DIRECTORY,
    };

    struct Node {
        String name = "";
        NodeType type = NodeType::INVALID;
        UniquePtr<Resource> resource;
        FileSystem* fileSystem;
        Vector<Node*> children;
        Node* parent;
    };

    using FileNameStr = char[16];
    struct DirEntry {
        FileNameStr fileName;
        int inode;
    };

    class VFS {
    private:
        UniquePtr<Node> root;
    public:
        VFS(const String& name) {
            root = makeUnique<Node>();
            root->name = name;
        }

    };

}
