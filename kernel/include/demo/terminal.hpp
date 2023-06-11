#pragma once
#include <demo/binaries.hpp>
#include <demo/common.hpp>
#include <kernel/cpp/string.hpp>
#include <kernel/devices/keyboard.h>
#include <kernel/filesystem/ramdisk.h>
#include <kernel/filesystem/vfs.h>

extern vfs::VFS globalVFS;

namespace demo {
class Terminal {
  public:
    Terminal(String<> username) : currDir(""), username(username), vfs(globalVFS) {
        binPath = "/bin";
        vfs.mkdir(binPath);
    }

    void run() {
        printf("\n\e[1;34m-------------------------------------------------------------------------"
               "---------------------------------------------------------------------\n");
        printf("\t\t\t\t\t\t\tLanding into kernel shell!!\n");
        printf("-----------------------------------------------------------------------------------"
               "-----------------------------------------------------------\n\e[0m");

        char buffer[DataChunkSize + 1] = {0};
        // memset((void*)buffer, 0, DataChunkSize + 1);

        while (true) {
            // Print prompt.
            // printf("curr dir is: %s\n", currDir.c_str());
            // printf("username is: %s\n", username.c_str());
            printf("\e[1;31m%s @ %s/ > \e[0m", username.c_str(), currDir.c_str());

            // Input.
            auto line = readLine(true);
            auto cmd = parseCommand(line);

            // Execute.
            execute(cmd);

            // Output.
            auto& stdoutFD = kernel::thisThread().fileDescriptors[1];
            while (stdoutFD.canReadXbytes(1)) {
                auto bytesRead = stdoutFD.read(DataChunkSize, (uint8_t*)buffer);

                buffer[bytesRead] = '\0';
                for (int i = 0; i < bytesRead; i++) {
                    printf("%c", buffer[i]);
                }
            }
            printf("\n");
        }
    }

    // TODO: Ideally we wanna read from binPath, check for executable with correct name and spawn a
    // sub process So for that we need to build some more abstractions around process (kthread):
    // - Passing argc, argv to process.
    // For now we can just if else for correct function.
    void execute(Command& cmd) {
        auto& bin = cmd.bin;
        if (bin == "cd") {
            cd(cmd, currDir);
        } else if (bin == "pwd") {
            echo(formPath(currDir, ""));
        } else if (bin == "echo") {
            echo(cmd.args[1]);
        } else if (bin == "ls") {
            ls(cmd, currDir);
        } else if (bin == "touch") {
            touch(cmd, currDir);
        } else if (bin == "mkdir") {
            mkdir(cmd, currDir);
        } else if (bin == "cat") {
            cat(cmd, currDir);
        } else if (bin == "fprint") {
            fprint(cmd, currDir);
        } else if (bin == "help") {
            help(cmd, currDir);
        } else if (bin == "stat") {
            stat(cmd, currDir);
        } else if (bin == "snake") {
            snake(cmd, currDir);
        } else {
            echo("Command doesn't exist, type help to get help");
        }
    }

    Command parseCommand(const String<>& cmd) {
        auto tokens = cmd.split(' ');
        return Command{
            .bin = tokens[0],
            .args = tokens,
        };
    }

    // TODO: clear the stdin file at some threshold.
    Keyboard::KeyCode getch() {
        auto& stdinFileDescriptor = kernel::thisThread().fileDescriptors[0];
        Keyboard::KeyCode key;
        while (!stdinFileDescriptor.canReadXbytes(sizeof(key))) {
            // spin
        }

        stdinFileDescriptor.read(sizeof(key), (uint8_t*)&key);
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

        auto isPrintableASCII = [](auto ch) { return ch >= 32 && ch < 127; };
        do {
            key = getch();
            auto c = keyboard.keyCodeToASCII(key);
            if (isPrintableASCII(c)) {
                printf("%c", c);
                line += c;
            } else if (key == Keyboard::KEY_BACKSPACE) {
                printf("%c", c);
                line.pop_back();
            }
        } while (key != Keyboard::KEY_RETURN);
        if (print) {
            printf("\n");
        }
        return line;
    }

    static fs::RamDisk createRamdisk(int deviceID, int nodeCount, int blockCount) {
        int dataBitsetSize = (blockCount + 7) / 8;
        int size = sizeof(fs::RamDisk::Super) + sizeof(fs::RamDisk::Node) * nodeCount + dataBitsetSize +
                   fs::RamDisk::BlockSize * blockCount;

        void* ptr = malloc(size);
        memset(ptr, 0, size);

        ((fs::RamDisk::Super*)ptr)->blockCnt = blockCount;
        ((fs::RamDisk::Super*)ptr)->iNodeCnt = nodeCount;
        ((fs::RamDisk::Super*)ptr)->dataBitsetSize = dataBitsetSize;

        auto rdfs = fs::RamDisk(ptr, 1);
        auto tempNode = vfs::Node(nullptr);
        rdfs.create(tempNode, vfs::NodeType::DIRECTORY);
        return fs::RamDisk(ptr, deviceID);
    }

  private:
    String<> currDir{};
    String<> username;
    vfs::VFS& vfs;
    String<> binPath;

    static constexpr int DataChunkSize = 2048;
};

} // namespace demo
