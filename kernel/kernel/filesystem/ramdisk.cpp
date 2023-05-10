#include <kernel/filesystem/ramdisk.h>

namespace fs {

RamDisk::RamDisk(void* base, int deviceID) : deviceID(deviceID) {
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

    // Allocate a block to this new inode. Zero out this memory.
    // If it's a directory, the entryCount will be set to 0.
    uint8_t buffer[BlockSize];
    memset(buffer, 0, BlockSize);
    write(vfsNode.resource, 0, BlockSize, buffer);
    node.size = 0;
    if (type == vfs::NodeType::DIRECTORY) {
        node.size = sizeof(DirHeader);
    }
    vfsNode.resource.size = node.size;
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
    if (offset > node->size) {
        printf("Can't read from offset > size, inode: %d\n", resource.inode);
        return 0;
    }
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
    if (offset > MaxDataBlocks * BlockSize) {
        printf("Can't write from offset > max file size, inode: %d\n", resource.inode);
        return 0;
    }
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
            dataBitset[newBlock] = 1;
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

// Only the first data block contains the header for Directory data.
Vector<vfs::DirEntry> RamDisk::readDir(const vfs::Node& vfsNode) const {
    auto inode = vfsNode.resource.inode;
    auto* node = getNode(inode);

    assert(node->type == vfs::NodeType::DIRECTORY, "Can't readDir on non dir type inode");

    void* buffer = malloc(node->size);
    read(vfsNode.resource, 0, node->size, (uint8_t*)buffer);

    int entryCount = ((DirHeader*)buffer)->entryCnt;
    if (!entryCount) {
        return {};
    }

    Vector<vfs::DirEntry> res(((DirHeader*)buffer)->entryCnt);
    buffer += sizeof(DirHeader);

    int realEntries = 0;
    for (int i = 0; i < res.size(); i++) {
        auto fileName = ((vfs::DirEntry*)buffer)->fileName;
        for (int j = 0; j < 16; j++) {
            res[realEntries].fileName[j] = fileName[j];
        }
        res[realEntries].inode = ((vfs::DirEntry*)buffer)->inode;
        buffer += sizeof(vfs::DirEntry);

        if (res[realEntries].inode != -1) {
            realEntries++;
        }
    }
    while (res.size() > realEntries) {
        res.pop_back();
    }
}

void RamDisk::writeDir(vfs::Node& vfsNode, const Vector<vfs::DirEntry>& entries) {
    auto inode = vfsNode.resource.inode;
    auto* node = getNode(inode);

    auto data = &entries[0];
    auto dataSize = entries.size() * sizeof(vfs::DirEntry);

    write(vfsNode.resource, node->size, dataSize, (uint8_t*)data);
}

// TODO: FIX WRITING and REMOVING DIR ENTRY

void RamDisk::removeDirEntry(vfs::Node& vfsNode, const vfs::DirEntry& entry) {
    auto inode = vfsNode.resource.inode;
    auto node = getNode(inode);

    for (int block = 0; block < node->blocksInUse; block++) {
        Dir* dirData = (Dir*)node->dataBlocks[block];
        for (int i = 0; i < dirData->header.entryCnt; i++) {
            if (dirData->entries[i].inode == entry.inode) {
                dirData->entries[i].inode = -1;
            }
        }
    }
}

void RamDisk::populateVFSNode(vfs::Node& vfsNode, int inode) {
    auto* node = getNode(inode);

    vfsNode.fileSystem = this;
    vfsNode.type = node->type;
    vfsNode.resource.deviceID = deviceID;
    vfsNode.resource.inode = node->inode;
    vfsNode.resource.size = node->size;
}

int RamDisk::getFreeDataBlock() noexcept {
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

void RamDisk::freeINode(int inode) noexcept { nodes[inode].type = vfs::NodeType::INVALID; }

void RamDisk::freeDataBlock(int block) noexcept { dataBitset[block] = 0; }
} // namespace fs
