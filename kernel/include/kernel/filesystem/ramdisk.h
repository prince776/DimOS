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
        RamDisk() = default;
        RamDisk(void* base, int deviceID);
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
        void create(vfs::Node& node, vfs::NodeType type) override;
        void remove(vfs::Node& node) override;

        int read(const Resource& resource, uint32_t offset, uint32_t size, uint8_t* buffer) const override;
        int write(Resource& resource, uint32_t offset, uint32_t size, uint8_t* buffer)  override;

        Vector<vfs::DirEntry> readDir(const vfs::Node& vfsNode) const override;
        void writeDir(vfs::Node& node, const Vector<vfs::DirEntry>& entries) override;
        void removeDirEntry(vfs::Node& node, const vfs::DirEntry& entry) override;

        void populateVFSNode(vfs::Node& vfsNode, int inode)  override;
    private:
        int getFreeDataBlock()  noexcept;
        int getFreeINode() noexcept;
        const Node* getNode(int32_t inode) const { return &nodes[inode]; }
        Node* getNode(int32_t inode) { return &nodes[inode]; }

        void freeDataBlock(int block) noexcept;
        void freeINode(int inode) noexcept;
        int getDataBlockNumber(DataBlock* block) { return block - dataBlocks; }
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
        using DataBitsetAllocatorTest = AllocatorTester<DataBitsetAllocator>;
        Bitset<DataBitsetAllocator> dataBitset;
        DataBlock* dataBlocks;
    };

}
