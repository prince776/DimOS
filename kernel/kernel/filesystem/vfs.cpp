#include <kernel/filesystem/vfs.h>

namespace vfs {
    VFS::VFS(FileSystem* fs, int inode = 0, const String<>& name = "") {
        root = new Node();
        root->populate(fs, name, inode);
    }

    void VFS::addChildren(Node* parent) {
        auto dirEntries = parent->readDir();

        for (auto& entry : dirEntries) {
            // bool present = false;
            // for (auto child : parent->children) {
            //     if (child->resource->inode == entry.inode) {
            //         present = true;
            //         break;
            //     }
            // }
            // if (present) {
            //     continue;
            // }
            Node* node = new Node();
            node->populate(parent->fileSystem, entry.fileName, entry.inode);
            parent->children.push_back(node);
        }
    }

    Vector<Node*> VFS::readDir(const String<>& path) {
        Node* dirNode = resolvePath(path);
        if (dirNode == nullptr) {
            printf("Directory doesn't exist: %s", path.c_str());
            return Vector<Node*>();
        }
        Vector<Node*> res;
        for (auto* child : dirNode->children) {
            res.push_back(child);
        }
        return res;
    }

    Node* VFS::mkdir(const String<>& path) {
        auto parentPath = path;
        while (parentPath.back() != '/') {
            parentPath.pop_back();
        }
        Node* parentNode = resolvePath(parentPath);
        if (!parentNode) {
            return nullptr;
        }
        Node* dirNode = new Node();
        dirNode->mkdir();
        parentNode->children.push_back(dirNode);
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
}
