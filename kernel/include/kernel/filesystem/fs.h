#pragma once
#include <kernel/filesystem/resource.h>
#include <kernel/filesystem/vfs.h>
namespace fs {

    template<typename Concrete>
    class FileSystem {
    public:
        vfs::Node toVFSNode(const Concrete::Node& node) {
            return (static_cast<Concrete*>(this))->toVFSNode_impl(node);
        }
    };
}
