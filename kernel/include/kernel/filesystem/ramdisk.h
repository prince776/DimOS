#pragma once
#include <kernel/filesystem/vfs.h>
#include <kernel/cpp/string.hpp>
#include <kernel/memory/vmm.h>
#include <kernel/cpp/utility.hpp>
#include <kernel/cpp/bitset.hpp>
namespace fs {

    /**
     * @brief RAM Disk FS Format:
     * | Super Block | INodes | Data bitset | Data blocks |
     * Block Size: 512 bytes
     * Max data blocks per inode: 10
     *
     */
    class RamDisk: FileSystem {
    public:
        static constexpr int BlockSize = 512;
        static constexpr int MaxDataBlocks = 10;
        int deviceID{};

        RamDisk(void* base, int deviceID): deviceID(deviceID) {
            super = (Super*)base;
            nodes = (Node*)(base + sizeof(Super));
            Blk dataBitsetMemBlk = Blk{
                .ptr = (void*)(nodes + super->iNodeCnt),
                .size = super->dataBitsetSize,
            };
            DataBitsetAllocator alloc(dataBitsetMemBlk);
            dataBitset = Bitset<DataBitsetAllocator>(dataBitsetMemBlk.size, alloc);

            dataBlocks = (DataBlock*)(dataBitsetMemBlk.ptr + dataBitsetMemBlk.size);
        }
        // Data structures
        struct DataBlock {
            uint8_t data[BlockSize];
        };
        struct Super {
            int32_t iNodeCnt;
            int32_t dataBitsetSize;
            int32_t blockCnt;
        } __attribute__((packed));
        struct Node {
            int inode{};
            vfs::NodeType type{vfs::NodeType::INVALID};
            int blocksInUse{};
            uint32_t size{};
            DataBlock* dataBlocks[MaxDataBlocks];
        } __attribute__((packed));
        struct DirHeader {
            int entryCnt{};
        } __attribute__((packed));
        static constexpr int DirEntryCntPerBlock = (BlockSize - sizeof(DirHeader)) / sizeof(vfs::DirEntry);
        struct Dir {
            DirHeader header{};
            vfs::DirEntry entries[DirEntryCntPerBlock];
        } __attribute__((packed));

        // methods
        Vector<vfs::DirEntry> readDir(const vfs::Node& vfsNode) const override {
            auto inode = vfsNode.resource->stat.inode;
            auto* node = getNode(inode);

            assert(node->blocksInUse > 0, "Invalid dir inode");
            assert(node->type == vfs::NodeType::DIRECTORY, "Can't readDir on non dir type inode");

            Vector<vfs::DirEntry> res;
            for (int block = 0; block < node->blocksInUse; block++) {
                Dir* dirData = (Dir*)node->dataBlocks[block];
                for (int i = 0; i < dirData->header.entryCnt; i++) {
                    res.push_back(dirData->entries[i]);
                }
            }
            return res;
        }

        int read(Resource* resource, uint32_t offset, uint32_t size, uint8_t* buffer) const override {
            auto* node = getNode(resource->stat.inode);
            // Can't read more than possible.
            size = min(size, node->size - offset);

            int prevDoneInBlock = offset % BlockSize, bytesRead = 0;
            for (int i = offset / BlockSize; i < node->blocksInUse && bytesRead < size; i++) {
                for (int j = prevDoneInBlock; j < BlockSize && bytesRead < size; j++) {
                    buffer[bytesRead++] = node->dataBlocks[i]->data[j];
                }
                prevDoneInBlock = 0;
            }
            return bytesRead;
        }

        int write(Resource* resource, uint32_t offset, uint32_t size, uint8_t* buffer)  override {
            auto* node = getNode(resource->stat.inode);
            if (offset >= node->size) {
                printf("Invalid offset to write when size is : %d, offset: %d", node->size, offset);
                return 0;
            }
            // Can't write more than possible.
            size = min(size, MaxDataBlocks * BlockSize - offset);
            int prevDoneInBlock = offset % BlockSize, bytesWritten = 0;
            for (int i = offset / BlockSize; i < MaxDataBlocks && bytesWritten < size; i++) {
                if (i == node->blocksInUse) {
                    auto newBlock = getFreeDataBlock();
                    if (newBlock == -1) {
                        printf("Ram disk full, can't allocate new block");
                        break;
                    }
                    node->dataBlocks[node->blocksInUse] = (dataBlocks + newBlock);
                    node->blocksInUse++;
                }
                for (int j = prevDoneInBlock; j < BlockSize && bytesWritten < size; j++) {
                    node->dataBlocks[i]->data[j] = buffer[bytesWritten++];
                }
                prevDoneInBlock = 0;
            }
            node->size = max(node->size, offset + bytesWritten);
            resource->stat.size = node->size;
            return bytesWritten;
        }

        void populateVFSNode(vfs::Node& vfsNode, int inode)  override {
            auto* node = getNode(inode);

            vfsNode.fileSystem = this;
            vfsNode.type = node->type;
            vfsNode.resource->stat.deviceID = deviceID;
            vfsNode.resource->stat.inode = node->inode;
            vfsNode.resource->stat.size = node->size;
        }

        int getFreeDataBlock()  noexcept {
            for (int i = 0; i < super->blockCnt; i++) {
                if (dataBitset[i] != 1) {
                    return i;
                }
            }
            return -1;
        }

        const Node* getNode(int32_t inode) const {
            return &nodes[inode];
        }
        Node* getNode(int32_t inode) {
            return &nodes[inode];
        }

    private:
        Super* super;
        Node* nodes;

        class DataBitsetAllocator {
        private:
            Blk data;
        public:
            DataBitsetAllocator() = default;
            DataBitsetAllocator(Blk data)
                : data(data) {}
            Blk allocate(size_t) { return data; }
            void deallocate(const Blk&) {}
            bool owns(const Blk& blk) { return data.ptr == blk.ptr; }
        };

        Bitset<DataBitsetAllocator> dataBitset;
        DataBlock* dataBlocks;
    };

}
