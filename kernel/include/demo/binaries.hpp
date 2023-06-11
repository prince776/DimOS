#pragma once
#include "kernel/cpp/vector.hpp"
#include "kernel/gfx/fb.hpp"
#include <demo/common.hpp>
#include <kernel/cpp/string.hpp>
#include <kernel/devices/keyboard.h>
#include <kernel/filesystem/ramdisk.h>
#include <kernel/filesystem/vfs.h>
#include <kernel/process/kthread.h>

extern vfs::VFS globalVFS;

namespace {

static constexpr int chunkSize = 2048;

inline String<> formPath(const String<>& currDir, const String<>& relPath) {
    if (relPath[0] == '/')
        return relPath;
    return currDir + "/" + relPath;
}

inline void echo(const String<>& data) {
    auto& stdoutFD = kernel::thisThread().fileDescriptors[1];
    stdoutFD.write(data.size(), (uint8_t*)data.c_str());
}

inline void echo(uint8_t* buffer, uint32_t size) {
    auto& stdoutFD = kernel::thisThread().fileDescriptors[1];
    stdoutFD.write(size, buffer);
}

inline void cd(const Command& cmd, String<>& currDir) {
    if (cmd.args.size() != 2) {
        echo("Bad arguments to command. Usage: cd <dirname>");
        return;
    }

    auto path = formPath(currDir, cmd.args[1]);

    auto node = globalVFS.openNode(path);
    if (!node || node->type != vfs::NodeType::DIRECTORY) {
        echo("Directory not found.");
        return;
    }
    currDir = path;
    while (currDir.size() && currDir.back() == '/') {
        currDir.pop_back();
    }
    echo("Changed dir");
}

inline void touch(const Command& cmd, const String<>& currDir) {
    if (cmd.args.size() < 2) {
        echo("Please enter file name(s) to create");
        return;
    }
    for (int i = 1; i < cmd.args.size(); i++) {
        auto path = formPath(currDir, cmd.args[i]);
        globalVFS.mkfile(path);
        echo(String("Created file: ") + path + "\n");
    }
}

inline void mkdir(const Command& cmd, const String<>& currDir) {
    if (cmd.args.size() < 2) {
        echo("Please enter dir name(s) to create");
        return;
    }
    for (int i = 1; i < cmd.args.size(); i++) {
        auto path = formPath(currDir, cmd.args[i]);
        globalVFS.mkdir(path);
        echo(String("Created dir: ") + path + "\n");
    }
}

inline void cat(const Command& cmd, const String<>& currDir) {
    if (cmd.args.size() != 2) {
        echo("Bad arguments to command. Usage: cat <filename>");
        return;
    }

    auto path = formPath(currDir, cmd.args[1]);

    uint8_t buffer[chunkSize] = {};
    auto fileNode = globalVFS.openNode(path);
    auto fileDesc = kernel::FileDescriptor(fileNode);

    while (fileDesc.canReadXbytes(1)) {
        int bytesRead = fileDesc.read(chunkSize, buffer);
        echo(buffer, bytesRead);
    }
}

inline void ls(const Command& cmd, const String<>& currDir) {
    auto nodes = globalVFS.readDir(currDir + "/");
    String<> sep = " ";
    for (int i = 0; i < nodes.size(); i++) {
        auto& node = nodes[i];
        auto name = node->name;
        if (node->type == vfs::NodeType::DIRECTORY) {
            name = String<>("\e[1;34m") + name + "\e[0m";
        }
        echo(name);
        echo(sep);
    }
}

inline void fprint(const Command& cmd, const String<>& currDir) {
    if (cmd.args.size() < 3) {
        echo("Bad arguments. Usage: fprint <filename> <data>");
        return;
    }
    auto path = formPath(currDir, cmd.args[1]);
    auto node = globalVFS.openNode(path);
    if (!node) {
        echo("File not present");
        return;
    }

    String<> data = "";
    for (int i = 2; i < cmd.args.size(); i++) {
        data += cmd.args[i];
        if (i != cmd.args.size() - 1) {
            data += " ";
        }
    }
    auto fd = kernel::FileDescriptor(node);
    node->write(0, data.size(), (uint8_t*)data.c_str());
}

inline void help(const Command& cmd, const String<>& currDir) {
    echo("Welcome to DimOS, will have a hard time finding a worse OS\n");
    echo("Following commands are supported in this fake kernel space shell:\n");
    echo("snake, ls, cd, help, pwd, cat, fprint, echo, mkdir, touch\n");
}

inline void stat(const Command& cmd, const String<>& currDir) {
    if (cmd.args.size() != 2) {
        echo("Bad arguments. Usage: stat <file/dir name>");
        return;
    }
    auto path = formPath(currDir, cmd.args[1]);
    auto node = globalVFS.openNode(path);

    if (!node) {
        echo("File/dir not found");
        return;
    }

    echo("Type: ");
    if (node->type == vfs::NodeType::DIRECTORY) {
        echo("directory");
    } else {
        echo("file");
    }
    echo("\n");

    echo("inode: ");
    echo(stoi(node->resource.inode));
    echo("\n");

    echo("device:");
    echo(stoi(node->resource.deviceID));
    echo("\n");

    echo("size: ");
    echo(stoi(node->resource.size));
}

Keyboard::KeyCode getchNoWait() {
    auto& stdinFileDescriptor = kernel::thisThread().fileDescriptors[0];
    Keyboard::KeyCode key = Keyboard::KEY_UNKNOWN;
    if (stdinFileDescriptor.canReadXbytes(sizeof(Keyboard::KeyCode))) {
        stdinFileDescriptor.read(sizeof(key), (uint8_t*)&key);
    }

    return key;
}

inline void snake(const Command& cmd, const String<>& currDir) {
    Keyboard::KeyCode input = Keyboard::KEY_UNKNOWN;

    Vector<gfx::Vec2> snake(1, {80, 80});
    gfx::Vec2 apple = {200, 200};
    gfx::Vec2 vel = {0, 0};
    int gridSize = 40;

    gfx::Vec2 dim = {gridSize, gridSize};

    gfx::FrameBuffer graphics(1280, 800);

    while (input != Keyboard::KEY_ESCAPE) {
        // game loop
        graphics.clscr(gfx::Color::white);

        // tick
        input = getchNoWait();
        switch (input) {
        case Keyboard::KEY_W:
            vel.x = 0;
            vel.y = -gridSize;
            break;
        case Keyboard::KEY_S:
            vel.x = 0;
            vel.y = gridSize;
            break;
        case Keyboard::KEY_A:
            vel.x = -gridSize;
            vel.y = 0;
            break;
        case Keyboard::KEY_D:
            vel.x = gridSize;
            vel.y = 0;
            break;
        default:
            break;
        }

        snake.reverse();
        auto head = snake.back();
        snake.push_back({head.x + vel.x, head.y + vel.y});

        head = snake.back();
        bool del = true;
        if (head.x == apple.x && head.y == apple.y) {
            del = false;
            apple.x = (apple.x + snake.size() * 500) % 1280;
            apple.x = (apple.x / gridSize) * gridSize;
            apple.y = (apple.y + snake.size() * 500) % 800;
            apple.y = (apple.y / gridSize) * gridSize;
        }
        snake.reverse();
        if (del) {
            snake.pop_back();
        }

        // render
        graphics.fillRect(apple, dim, gfx::Color::red);
        graphics.fillRect(snake[0], dim, gfx::Color(0, 165, 255));
        for (size_t i = 1; i < snake.size(); i++) {
            graphics.fillRect(snake[i], dim, gfx::Color::green);
        }

        for (volatile int i = 0; i < 5000000; i++)
            ;
    }
    graphics.clscr(gfx::Color::black);
}

} // namespace
