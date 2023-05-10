#include <kernel/concurrency/locks.hpp>
#include <kernel/filesystem/vfs.h>

namespace vfs {
VFS::VFS(FileSystem* fs, int inode, const String<>& name) {
    root = new Node(fs);
    root->populate(fs, name, inode);
}

Vector<Node*> VFS::readDir(const String<>& path) {
    ScopedLock lock(mutex);
    Node* dirNode = resolvePath(path);
    if (dirNode == nullptr) {
        printf("Directory doesn't exist: %s\n", path.c_str());
        return Vector<Node*>();
    }
    return dirNode->children;
}

Node* VFS::mkdir(const String<>& path) {
    ScopedLock lock(mutex);
    auto parentPath = path;
    String dirName = "";
    while (parentPath.back() != '/') {
        dirName.push_back(parentPath.back());
        parentPath.pop_back();
    }
    dirName.reverse();
    Node* parentNode = resolvePath(parentPath);
    if (!parentNode) {
        return nullptr;
    }
    Node* dirNode = new Node(parentNode->fileSystem);
    dirNode->mkdir();
    dirNode->name = dirName;
    dirNode->parent = parentNode;
    parentNode->children.push_back(dirNode);

    parentNode->writeDir(Vector<DirEntry>(1, DirEntry::fromStringName(dirName, dirNode->resource.inode)));
    return dirNode;
}

Node* VFS::mnt(const String<>& path, FileSystem* fs, int inode) {
    auto node = openNode(path);
    if (node) {
        printf("Node already exists, can't mount here: '%s'\n", path.c_str());
        return nullptr;
    }
    ScopedLock lock(mutex);

    auto parentPath = path;
    String dirName = "";
    while (parentPath.back() != '/') {
        dirName.push_back(parentPath.back());
        parentPath.pop_back();
    }
    dirName.reverse();
    Node* parentNode = resolvePath(parentPath);
    if (!parentNode) {
        printf("Parent node not found: '%s'\n", parentPath.c_str());
        return nullptr;
    }

    Node* dirNode = new Node(fs);
    dirNode->populate(fs, dirName, inode);
    dirNode->parent = parentNode;
    parentNode->children.push_back(dirNode);
    return dirNode;
}

Node* VFS::mkfile(const String<>& path) {
    ScopedLock lock(mutex);
    auto parentPath = path;
    String fileName = "";
    while (parentPath.back() != '/') {
        fileName.push_back(parentPath.back());
        parentPath.pop_back();
    }
    fileName.reverse();
    Node* parentNode = resolvePath(parentPath);
    if (!parentNode) {
        return nullptr;
    }
    Node* fileNode = new Node(parentNode->fileSystem);
    fileNode->mkFile();
    fileNode->name = fileName;
    fileNode->parent = parentNode;
    parentNode->children.push_back(fileNode);

    parentNode->writeDir(Vector<DirEntry>(1, DirEntry::fromStringName(fileName, fileNode->resource.inode)));
    return fileNode;
}

void VFS::rmdir(const String<>& path) {
    ScopedLock lock(mutex);
    auto node = resolvePath(path);
    if (node->children.size() != 0) {
        printf("Can't remove non empty directory: %s", path.c_str());
        return;
    }
    if (node->type != NodeType::DIRECTORY) {
        printf("Can't rmdir a non directory: %s", path.c_str());
        return;
    }
    auto removedInode = node->resource.inode;
    node->remove();

    auto parent = node->parent;
    parent->children.fastErase(parent->children.find(node));

    parent->removeDirEntry(DirEntry{.inode = removedInode});

    delete node;
}

void VFS::rmfile(const String<>& path) {
    ScopedLock lock(mutex);
    auto node = resolvePath(path);
    if (node->type != NodeType::FILE) {
        printf("Can't rmfile a non file: %s", path.c_str());
        return;
    }
    auto removedInode = node->resource.inode;
    node->remove();

    auto parent = node->parent;
    parent->children.fastErase(parent->children.find(node));

    parent->removeDirEntry(DirEntry{.inode = removedInode});

    delete node;
}

void VFS::read(const String<>& path, uint32_t offset, uint32_t size, uint8_t* buffer) {
    ScopedLock lock(mutex);
    auto node = resolvePath(path);
    if (!node || node->type == NodeType::INVALID) {
        printf("Can't read invalid node: %s\n", path.c_str());
        return;
    }
    node->read(offset, size, buffer);
}

void VFS::write(const String<>& path, uint32_t offset, uint32_t size, uint8_t* buffer) {
    ScopedLock lock(mutex);
    auto node = resolvePath(path);
    if (!node || node->type == NodeType::INVALID) {
        printf("Can't write invalid node: %s\n", path.c_str());
        return;
    }
    node->write(offset, size, buffer);
}

Node* VFS::openNode(const String<>& path) {
    ScopedLock lock(mutex);
    auto node = resolvePath(path);
    if (!node) {
        return nullptr;
    }
    return node;
}

void VFS::addChildren(Node* parent) {
    auto dirEntries = parent->readDir();

    for (auto& entry : dirEntries) {
        Node* node = new Node(parent->fileSystem);
        node->populate(parent->fileSystem, entry.fileName, entry.inode);
        node->parent = parent;
        parent->children.push_back(node);
    }
}

Node* VFS::resolvePath(const String<>& path) {
    auto pathEntries = path.split('/');
    if (pathEntries[0] != root->name) {
        return nullptr;
    }
    return resolvePathUtil(root, pathEntries, 1);
}

Node* VFS::resolvePathUtil(Node* curr, const Vector<String<>>& pathEntries, int idx) {
    if (idx == pathEntries.size()) {
        return curr;
    }
    if (!curr->children.size()) {
        addChildren(curr);
    }
    for (auto child : curr->children) {
        if (child->name == pathEntries[idx]) {
            return resolvePathUtil(child, pathEntries, idx + 1);
        }
    }
    return nullptr;
}
} // namespace vfs
