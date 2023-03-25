#pragma once
#include <kernel/filesystem/fs.h>
#include <kernel/cpp/string.hpp>
#include <kernel/memory/vmm.h>
#include <kernel/cpp/utility.hpp>

namespace fs {

    class RamDisk: FileSystem {
    public:
        using RDFileName = char[16];
        static constexpr int BlockSize = 512;
        static constexpr int MaxDataBlocks = 10;
        int deviceID{};

        RamDisk(void* base, int deviceID): deviceID(deviceID) {
            super = (Super*)base;
            nodes = (Node*)(base + sizeof(Super));
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
            uint32_t size{};
            Data data[MaxDataBlocks];
        };
        struct DirHeader {
            int entryCnt{};
        };
        static constexpr int DirEntryCntPerBlock = (BlockSize - sizeof(DirHeader)) / sizeof(vfs::DirEntry);
        struct Dir {
            DirHeader header{};
            vfs::DirEntry entries[DirEntryCntPerBlock];
        };

        // methods
        Vector<vfs::DirEntry> readDir(const vfs::Node& vfsNode) const override {
            auto inode = vfsNode.resource->stat.inode;
            auto* node = getNode(inode);
            assert(node->blocksInUse > 0, "Invalid dir inode");
            Vector<vfs::DirEntry> res;
            for (int block = 0; block < node->blocksInUse; block++) {
                Dir* dirData = (Dir*)node->data[block];
                for (int i = 0; i < dirData->header.entryCnt; i++) {
                    res.push_back(dirData->entries[i]);
                }
            }
            return res;
        }

        int read(Resource* resource, uint32_t offset, uint32_t size, uint8_t* buffer) const override {
            auto* node = getNode(resource->stat.inode);
            int prevDoneInBlock = offset % BlockSize, bytesRead = 0;
            for (int i = offset / BlockSize; i < node->blocksInUse && bytesRead < size; i++) {
                int bufEnd = BlockSize;
                if (i == node->blocksInUse) {
                    bufEnd = node->size % BlockSize;
                    if (bufEnd == 0) bufEnd = BlockSize;
                }
                for (int j = prevDoneInBlock; j < bufEnd && bytesRead < size; j++) {
                    buffer[bytesRead++] = node->data[i][j];
                }
                prevDoneInBlock = 0;
            }
            return bytesRead;
        }

        int write(Resource* resource, uint32_t offset, uint32_t size, uint8_t* buffer)  override {
            auto* node = getNode(resource->stat.inode);
            if (offset > node->size) {
                printf("Invalid offset to write when size is : %d, offset: %d", node->size, offset);
                return 0;
            }
            int bytesWritten = 0;
            int blockSt = offset / BlockSize;
            offset %= BlockSize;
            if (offset == 0) blockSt++;
            for (int i = offset / BlockSize; i < MaxDataBlocks && bytesWritten < size; i++) {
                if (i > node->blocksInUse) {

                }
                for (int j = prevDoneInBlock; j < bufEnd && bytesRead < size; j++) {
                    buffer[bytesRead++] = node->data[i][j];
                }
                prevDoneInBlock = 0;
            }
            return bytesRead;
        }
        // void addResource(vfs::Node& vfsNode) override {
        //     vfsNode.resource->stat.deviceID = deviceID;
        //     vfsNode.resource->stat.inode = 
        // }


        const Node* getNode(int32_t inode) const {
            return &nodes[inode];
        }

    private:
        Super* super;
        Node* nodes;
    };

}
