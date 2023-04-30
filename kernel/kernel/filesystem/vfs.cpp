#include <kernel/filesystem/vfs.h>

namespace vfs {
    VFS::VFS(FileSystem* fs, int inode, const String<>& name) {
        root = new Node(fs);
        root->populate(fs, name, inode);
    }

    Vector<Node*> VFS::readDir(const String<>& path) {
        Node* dirNode = resolvePath(path);
        if (dirNode == nullptr) {
            printf("Directory doesn't exist: %s\n", path.c_str());
            return Vector<Node*>();
        }
        return dirNode->children;
    }

    Node* VFS::mkdir(const String<>& path) {
        auto parentPath = path;
        String dirName = "";
        while (parentPath.back() != '/') {
            dirName.push_back(parentPath.back());
            parentPath.pop_back();
        }
        dirName.reverse();
        Node* parentNode = resolvePath(parentPath);
        // printf("finding parent node for path: '%s', with parentPath: '%s'\n", path.c_str(), parentPath.c_str());
        if (!parentNode) {
            // printf("parent node not found for path: '%s', with parentPath: '%s'\n", path.c_str(), parentPath.c_str());
            return nullptr;
        }
        // printf("Dir name here is: %s\n", dirName.c_str());
        Node* dirNode = new Node(parentNode->fileSystem);
        dirNode->mkdir();
        dirNode->name = dirName;
        dirNode->parent = parentNode;
        parentNode->children.push_back(dirNode);
    }

    void VFS::rmdir(const String<>& path) {
        auto node = resolvePath(path);
        if (node->children.size() != 0) {
            printf("Can't remove non empty directory: %s", path.c_str());
            return;
        }
        node->remove();
        auto parent = node->parent;
        parent->children.fastErase(parent->children.find(node));
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
        // printf("Resolving path: %s ->", path.c_str());
        // for (auto x : pathEntries) {
        //     printf("'%s', ", x.c_str());
        // }
        // printf("end\n");
        // printf("Comparing '%s' of sz: '%d', with '%s of sz: '%d''\n", pathEntries[0].c_str(), pathEntries[0].size(), root->name.c_str(), root->name.size());
        if (pathEntries[0] != root->name) {
            // printf("Didn't get here");
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
        printf("Couldn't reolve path\n");
        return nullptr;
    }
}
