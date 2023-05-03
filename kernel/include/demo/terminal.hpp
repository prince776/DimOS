#pragma once
#include <kernel/cpp/string.hpp>
#include <kernel/filesystem/vfs.h>
#include <kernel/filesystem/ramdisk.h>
#include <kernel/devices/keyboard.h>

extern vfs::VFS globalVFS;

namespace demo {

    struct Command {
        String<> bin;
        Vector<String<>> args;
    };

    class Terminal {
    public:
        Terminal(String<> username)
            : currDir(""), username(username), vfs(globalVFS) {}

        void run() {
            printf("\n\e[1;34m----------------------------------------------------------------------------------------------------------------------------------------------\n");
            printf("\t\t\t\t\t\t\tLanding into kernel shell!!\n");
            printf("----------------------------------------------------------------------------------------------------------------------------------------------\n\e[0m");
            while (true) {
                printf("\e[1;31m%s @ %s/ > \e[0m", username.c_str(), currDir.c_str());
                auto line = readLine(true);
                auto cmd = parseCommand(line);

            }
        }

        Command parseCommand(const String<>& cmd) {
            auto tokens = cmd.split(' ');
            return Command{
                .bin = tokens[0],
                .args = tokens,
            };
        }

        Keyboard::KeyCode getch() {
            auto& keyboard = Keyboard::get();
            auto key = keyboard.getLastKeyCode();
            while (key == Keyboard::KEY_UNKNOWN) {
                key = keyboard.getLastKeyCode();
            }
            keyboard.discardLastKeyCode();
            return key;
        }

        char getChar() {
            auto& keyboard = Keyboard::get();
            while (true) {
                auto key = getch();
                auto ch = keyboard.keyCodeToASCII(key);
                if (ch) {
                    return ch;
                }
            }
            return 0;
        }

        String<> readLine(bool print = false) {
            String<> line = "";
            Keyboard::KeyCode key;
            auto& keyboard = Keyboard::get();
            do {
                key = getch();
                auto c = keyboard.keyCodeToASCII(key);
                if (c && c != '\n') {
                    if (c != '\n') {
                        printf("%c", c);
                        line += c;
                    }
                }
            } while (key != Keyboard::KEY_RETURN);
            if (print) {
                printf("\n");
            }
            return line;
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


        static fs::RamDisk createRamdisk(int nodeCount, int blockCount) {
            int dataBitsetSize = (blockCount + 7) / 8;
            int size = sizeof(fs::RamDisk::Super) + sizeof(fs::RamDisk::Node) * nodeCount + dataBitsetSize + fs::RamDisk::BlockSize * blockCount;

            void* ptr = malloc(size);
            memset(ptr, 0, size);

            ((fs::RamDisk::Super*)ptr)->blockCnt = blockCount;
            ((fs::RamDisk::Super*)ptr)->iNodeCnt = nodeCount;
            ((fs::RamDisk::Super*)ptr)->dataBitsetSize = dataBitsetSize;

            auto rdfs = fs::RamDisk(ptr, 1);
            auto tempNode = vfs::Node(nullptr);
            rdfs.create(tempNode, vfs::NodeType::DIRECTORY);
            return fs::RamDisk(ptr, 1);
        }

    private:
        String<> currDir{};
        String<> username;
        vfs::VFS& vfs;
    };

}
