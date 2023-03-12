#pragma once
#include <stdint-gcc.h>
#include <kernel/cpp/unique-ptr.hpp>
#include <kernel/cpp/string.hpp>
#include <kernel/filesystem/resource.h>
#include <kernel/filesystem/fs.h>

namespace vfs {
    enum class NodeType {
        INVALID,
        FILE,
        DIRECTORY,
    };

    struct Node;

    using openFn_t = uint32_t(*)(Node* node);
    using openFn_t = uint32_t(*)(Node* node);

    struct FileSystem {
        openFn_t openFn{};
        writeFn_t writeFn{};
    };

    struct Node {
        String name = "";
        VFSNodeType type = VFSNodeType::INVALID;
        UniquePtr<Resource> resource;
        UniquePtr<FileSystem> fileSystem;
        Vector<Node*> children;
        Node* parent;
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
