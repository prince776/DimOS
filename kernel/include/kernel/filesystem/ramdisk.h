#pragma once
#include <kernel/filesystem/fs.h>
#include <kernel/cpp/string.hpp>
#include <kernel/memory/vmm.h>
#include <kernel/cpp/utility.hpp>

namespace fs {

    class RamDisk: FileSystem<RamDisk> {
    public:
        using RDFileName = char[16];
        static constexpr int BlockSize = 512;
        static constexpr int MaxDataBlocks = 10;

        RamDisk(void* base) {
            super = (Super*)base;
            void* nodeAddr = base + sizeof(base);
            for (int i = 0; i < super->iNodeCnt; i++) {
                nodes.push_back((Node*)nodeAddr);
                nodeAddr += sizeof(Node);
            }
        }

        // Data structures
        using Data = uint8_t[BlockSize];
        struct Super {
            int32_t iNodeCnt;
            int32_t blockCnt;
        };
        struct Node {
            int inode{};
            vfs::NodeType type{vfs::NodeType::INVALID};
            int blocksInUse{};
            Data data[MaxDataBlocks];
        };
        struct DirHeader {
            int entryCnt{};
        };
        struct DirEntry {
            RDFileName fileName;
            int inode;
        };
        static constexpr int DirEntryCntPerBlock = (BlockSize - sizeof(DirHeader)) / sizeof(DirEntry);
        struct Dir {
            DirHeader header{};
            DirEntry entries[DirEntryCntPerBlock];
        };

        // methods
        const Dir& readDir(Node* node) {
            assert(node->type == vfs::NodeType::DIRECTORY, "Can't readDir a non dir.");
            assert(node->blocksInUse > 0, "Invalid dir inode");
            Vector<DirEntry> res;
            for (int)
        }

    private:
        Super* super;
        Vector<Node*> nodes;
    };

}
