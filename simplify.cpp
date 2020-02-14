#include "simplify.hpp"

const string INPUT_DIR = "data/input/";
const string OUTPUT_DIR = "data/output/";

simplify::~simplify() {
    delete m_tree;
}

simplify::simplify(string name, bool r1, bool r2, bool r3)
:r1(r1), r2(r2), r3(r3) {
    m_tree = new tree();
    m_tree->parse(INPUT_DIR + name);
    this->run();
    std::ofstream file(OUTPUT_DIR + name.substr(0, name.length()-4) + "/" + name.substr(0, name.length()-4) + ".sdag");
    string output = "";
    m_tree->dfs_format("r1", output);
    // output = m_tree->quick_format();
    file << output;
    file.close();
    // cout << output << endl; // TEST
}

void simplify::run() {
    if(this->r1 || this->r2 || this->r3) {
        node* root = m_tree->create_node("r1");
        set<string> visitedSet;
        this->simplify_helper(root, nullptr, visitedSet);
        if(root->children.size() == 1) {
            node* child = m_tree->create_node(root->children.begin()->first);
            root->children = child->children;
            root->gateType = child->gateType;
        }
    }
}

void simplify::simplify_helper(node* curNode, node* parentNode, set<string>& visitedSet) {
    if(curNode->children.size() == 0)
        return;
    vector<string> childList;
    for(auto& i: curNode->children)
        childList.push_back(i.first);
    int index = 0;
    while(index < childList.size()) {
        node* child = m_tree->create_node(childList[index]);
        // 合并单输入门
        if(this->r2 && child->children.size() == 1 && child->children.begin()->second) {
            curNode->children[child->children.begin()->first] = true;
            curNode->children.erase(child->name);
            childList.push_back(child->children.begin()->first);
            index++;
            continue;
        }
        // 合并相邻相同门
        // TODO 合并上来的可能存在冲突
        if(this->r1 && curNode->gateType == child->gateType
           && curNode->children[child->name]) {
            // 孩子为负不合并
            for(auto& i: child->children) {
                if(curNode->children.find(i.first) == curNode->children.end()) {
                    curNode->children[i.first] = i.second;
                    childList.push_back(i.first); // 合并上来后还会重新处理
                } else if(curNode->children[i.first] != i.second) {
                    cout << "ERROR" << endl; // TEST
                }
            }
            curNode->children.erase(child->name);
            index++;
            continue;
        }
        simplify_helper(child, curNode, visitedSet);
        // 合并相同子树
        if(this->r3 && child->children.size() != 0) {
            bool flag = true;
            for(auto& name: visitedSet) {
                if(name == child->name) {
                    flag = false;
                    break;
                }
                node* visitedNode = m_tree->create_node(name);
                if(visitedNode->gateType == child->gateType &&
                    visitedNode->children.size() == child->children.size()) {
                    bool same = true;
                    for(auto& i: child->children) {
                        if(visitedNode->children.find(i.first) == visitedNode->children.end() ||
                            visitedNode->children[i.first] != i.second) {
                            // 访问map会直接创建默认值(0)
                            same = false;
                            break;
                        }
                    }
                    if(same) {
                        // TODO 替换上来的visitedNode可能也有con
                        curNode->children[visitedNode->name] = curNode->children[child->name];
                        curNode->children.erase(child->name);
                        flag = false;
                        break;
                    }
                }
            }
            if(flag) {
                visitedSet.insert(child->name);
            }
        }
        index++;
    }
}

int main() {
    set<string> avoid = {".", "..", "das9701.dag", "elf9601.dag", "nus9601.dag"};
    DIR *d = opendir("data/input");
    dirent* entry;
    while((entry=readdir(d)) != NULL) {
        string name = entry->d_name;
        // name = "elf9601.dag";
        if(!avoid.count(name)) {
            string cmd = "mkdir -p " + OUTPUT_DIR + name.substr(0, name.length()-4);
            system(cmd.c_str());
            simplify* s = new simplify(name, 1, 1, 1);
            // delete s;
            cout << name << endl;
        }
        // break;
    }
    closedir(d);
}