#include "tools.hpp"
#include <sys/types.h>
#include <dirent.h>
#include <algorithm>

class simplify {
private:
    tree* m_tree;
    bool r1;  // 合并相邻相同的门
    bool r2;  // 合并单输入门
    bool r3;  // 合并相同子树
public:
    simplify(string name, bool r1=true, bool r2=true, bool r3=true);
    ~simplify();
    void run();
    void simplify_helper(node* curNode, node* parentNode, set<string>& visitedSet);
};
