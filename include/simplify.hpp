#include "tools.hpp"
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>

class simplify {
private:
    tree* m_tree;
    bool isSameGate;  // 合并相邻相同的门
    bool isOneChild;  // 合并单输入门
    bool isSameTree;  // 合并相同子树
public:
    simplify(string name, tree* t, string outputDir,
             bool isSameGate=true, bool isOneChild=true, bool isSameTree=true);
    ~simplify();
    void run();
    void simplify_helper(node* curNode, node* parentNode, set<string>& visitedSet);
};
