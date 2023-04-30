#pragma once
#include <kernel/cpp/string.hpp>
#include <kernel/filesystem/vfs.h>
#include <kernel/filesystem/ramdisk.h>

namespace demo {

    class Terminal {
    public:
        Terminal() {
            rdfs = createRamdisk(10, 10);
            currDir = "";
            vfs = vfs::VFS((FileSystem*)&rdfs, 0, "");
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
            printf("\n");
        }

        String<> pwd() {
            printf("Current directory: %s\n\n", currDir.c_str());
        }

        void mkdir(const String<>& name) {
            auto fullpath = currDir + "/" + name;
            printf("Creating dir: '%s'\n\n", fullpath.c_str());
            vfs.mkdir(fullpath);
        }

        void rmdir(const String<>& name) {
            auto fullpath = currDir + "/" + name;
            printf("Removing dir: '%s'\n\n", fullpath.c_str());
            vfs.rmdir(fullpath);
        }

        void cd(const String<>& name) {
            auto fullpath = currDir + "/" + name;
            if (vfs.openNode(fullpath)) {
                currDir = fullpath;
                printf("Directoy changed to: %s\n\n", currDir.c_str());
            }
        }

        void touch(const String<>& name) {
            auto fullpath = currDir + "/" + name;
            printf("Creating new file: %s\n\n", fullpath.c_str());
            vfs.mkfile(fullpath);
        }

        void rm(const String<>& name) {
            auto fullpath = currDir + "/" + name;
            printf("Removing file: %s\n\n", fullpath.c_str());
            vfs.rmfile(fullpath);
        }

        void writeFile(const String<>& name, uint8_t* buffer, uint32_t size) {
            auto fullpath = currDir + "/" + name;
            printf("Writing to file '%s': ", fullpath.c_str());
            printBuffer(buffer, size);
            printf("\n\n");

            vfs.write(fullpath, 0, size, buffer);
        }

        void readFile(const String<>& name, uint8_t* buffer, uint32_t size) {
            auto fullpath = currDir + "/" + name;

            vfs.read(fullpath, 0, size, buffer);

            printf("Reading file '%s': ", fullpath.c_str());
            printBuffer(buffer, size);
            printf("\n\n");
        }

        void printBuffer(uint8_t* buffer, int size) {
            for (int i = 0; i < size; i++) {
                printf("%c", (char)buffer[i]);
            }
        }

    private:

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
