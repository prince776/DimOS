#pragma once
#include <kernel/cpp/string.hpp>
#include <kernel/filesystem/vfs.h>
#include <kernel/filesystem/ramdisk.h>
#include <demo/testrd.hpp>

namespace demo {

    class Terminal {
    public:
        Terminal() {
            rdfs = createRamdisk(10, 10);
            currDir = "base";
            vfs = vfs::VFS((FileSystem*)&rdfs, 0, "base");
        }

        String<> ls() {
            printf("Listing directory: %s\n", currDir.c_str());
            auto dirEntries = vfs.readDir(currDir);
            for (auto entry : dirEntries) {
                String type;
                if (entry->type == vfs::NodeType::DIRECTORY) {
                    type = "Folder";
                }
                else {
                    type = "File";
                }
                printf("%s: %s, inode: %d, deviceID: %d\n", type.c_str(), entry->name.c_str(), entry->resource.inode, entry->resource.deviceID);
            }
            printf("LS END\n");
        }

        String<> pwd() {
            printf("Current directory: %s\n", currDir.c_str());
        }

        void mkdir(const String<>& name) {
            auto fullpath = currDir;
            fullpath += "/";
            fullpath += name;
            printf("Creating dir: '%s'\n", fullpath.c_str());
            vfs.mkdir(fullpath);
        }

        fs::RamDisk createRamdisk(int nodeCount, int blockCount) {
            int dataBitsetSize = (blockCount + 7) / 8;
            int size = sizeof(fs::RamDisk::Super) + sizeof(fs::RamDisk::Node) * nodeCount + dataBitsetSize + fs::RamDisk::BlockSize * blockCount;

            void* ptr = malloc(size);
            memset(ptr, 0, size);

            ((fs::RamDisk::Super*)ptr)->blockCnt = blockCount;
            ((fs::RamDisk::Super*)ptr)->iNodeCnt = nodeCount;
            ((fs::RamDisk::Super*)ptr)->dataBitsetSize = dataBitsetSize;

            auto node = (fs::RamDisk::Node*)(ptr + sizeof(fs::RamDisk::Super));
            node->type = vfs::NodeType::DIRECTORY;
            return fs::RamDisk(ptr, 1);
        }

    private:
        vfs::VFS vfs;
        String<> currDir{};
        fs::RamDisk rdfs;
    };

}
