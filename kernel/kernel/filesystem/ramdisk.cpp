#include <kernel/filesystem/ramdisk.h>

namespace fs {

    RamDisk::RamDisk(void* base, int deviceID): deviceID(deviceID) {
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

    void RamDisk::create(vfs::Node& vfsNode, vfs::NodeType type) {
        auto freeINode = getFreeINode();
        Node& node = nodes[freeINode];
        node.blocksInUse = 0;
        node.inode = freeINode;
        node.size = 0;
        node.type = type;
        populateVFSNode(vfsNode, freeINode);
    }

    void RamDisk::remove(vfs::Node& vfsNode) {
        auto inode = vfsNode.resource.inode;
        auto& node = nodes[inode];
        for (int i = 0; i < node.blocksInUse; i++) {
            auto blockNum = getDataBlockNumber(node.dataBlocks[i]);
            freeDataBlock(blockNum);
        }
        freeINode(inode);
    }

    int RamDisk::read(const Resource& resource, uint32_t offset, uint32_t size, uint8_t* buffer) const {
        auto* node = getNode(resource.inode);
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

    int RamDisk::write(Resource& resource, uint32_t offset, uint32_t size, uint8_t* buffer) {
        auto* node = getNode(resource.inode);
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
        resource.size = node->size;
        return bytesWritten;
    }

    Vector<vfs::DirEntry> RamDisk::readDir(const vfs::Node& vfsNode) const {
        auto inode = vfsNode.resource.inode;
        auto* node = getNode(inode);

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

    void RamDisk::writeDir(vfs::Node& vfsNode, const Vector<vfs::DirEntry>& entries) {
        auto inode = vfsNode.resource.inode;
        auto* node = getNode(inode);

        auto data = &entries[0];
        auto dataSize = entries.size() * sizeof(vfs::DirEntry);

        write(vfsNode.resource, node->size, dataSize, (uint8_t*)data);
    }

    void RamDisk::populateVFSNode(vfs::Node& vfsNode, int inode) {
        auto* node = getNode(inode);

        vfsNode.fileSystem = this;
        vfsNode.type = node->type;
        vfsNode.resource.deviceID = deviceID;
        vfsNode.resource.inode = node->inode;
        vfsNode.resource.size = node->size;
    }

    int RamDisk::getFreeDataBlock()  noexcept {
        for (int i = 0; i < super->blockCnt; i++) {
            if (dataBitset[i] != 1) {
                return i;
            }
        }
        return -1;
    }

    int RamDisk::getFreeINode() noexcept {
        for (int i = 0; i < super->iNodeCnt; i++) {
            if (nodes[i].type == vfs::NodeType::INVALID) {
                return i;
            }
        }
        return -1;
    }

    void RamDisk::freeINode(int inode) {
        nodes[inode].type = vfs::NodeType::INVALID;
    }

    void RamDisk::freeDataBlock(int block) {
        dataBitset[block] = 0;
    }
}