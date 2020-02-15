#include "tools.hpp"

class findModules {
private:
    tree* m_tree;
    bool NEW_MODULE;
    int levelCount = 1;
    map<string, set<pair<node*, int>>> connection_list; // childName: [(parentNode, AEG)]
    set<string> bothAEG;
    set<string> result; // 已经得到的model的root的name
    int lccCount = 1; // 为LCC阶段新增的门
    map<string, string> moduleDict; // { name : module_name } 不替换原有节点名称的代替品

public:
    findModules(tree* t, bool NEW_MODULE=true)
    :m_tree(t), NEW_MODULE(NEW_MODULE){
        this->moduleDict["r1"] = "m0";
    }

    ~findModules() {
        delete m_tree;
    }

    node* getNode(const string& name) {
        return this->m_tree->create_node(name);
    }

    void init_level(node* cur) {
        if(cur->children.size() == 0)
            return;
        for(auto& i:cur->children)
            init_level(getNode(i.first));
        cur->level = levelCount;
        this->levelCount++;
    }

    void init_connection_list(node* cur) {
        int AEG = 0;
        for(auto& i: cur->children) {
            if(cur->gateType == '|' && i.second) {  // 1, 3 对应论文AEG = 1
                AEG = 1;
            } else if(cur->gateType == '&' && !i.second) {
                AEG = 3;
            } else if(cur->gateType == '|' && !i.second) {  // 2, 4 对应论文AEG = -1
                AEG = 2;
            } else if(cur->gateType == '&' && i.second) {
                AEG = 4;
            }
            if(this->connection_list.find(i.first) != this->connection_list.end()) {
                for(auto& j: this->connection_list[i.first]) {  // 判断connection_list中是否有两种AEG
                    if(j.second % 2 != AEG % 2) {  // 属于论文中描述的同一类
                        this->bothAEG.insert(i.first);
                        break;
                    }
                }
                bool visited = false;
                for(auto& j: this->connection_list[i.first]) {
                    if(j.first == cur) {
                        visited = true;
                        break;
                    }
                }
                if(!visited) {
                    this->connection_list[i.first].insert({cur, AEG});
                }
            } else {
                this->connection_list[i.first] = {{cur, AEG}};
            }
            if(getNode(i.first)->children.size() != 0) {
                init_connection_list(getNode(i.first));
            }
        }
    }

    void run() {
        this->PC_check();
    }

    string get_top_level_node(set<pair<node*, int>>& s) {
        int max = 0;
        string topNode = "";
        for(auto& i: s) {
            if(i.first->level > max) {
                max = i.first->level;
                topNode = i.first->name;
            }
        }
        return topNode;
    }

    void PC_check() {
        for(const auto& i: m_tree->nodeDict) {
            node* cur = i.second;
            if(cur->level != 0) {
                bool flag = true;
                for(const auto& j: cur->children) {
                    if(get_top_level_node(this->connection_list[j.first]) != cur->name) {
                        flag = false;
                        break;
                    }
                }
                if(flag) {
                    CC_check(cur);
                }
            }
        }
    }

    void CC_helper(node* cur, set<string>& expandSet, set<string>& connectionSet, set<string>& basicOrTop) {
        for(auto& i: cur->children) {
            expandSet.insert(i.first);
            for(auto& j: this->connection_list[i.first])
                connectionSet.insert(j.first->name);
            if(getNode(i.first)->children.size() == 0 || this->result.count(i.first)) {
                if(this->bothAEG.count(i.first) == 0) {  // 排除有两种AEG的节点
                    basicOrTop.insert(i.first);
                }
            } else {
                CC_helper(getNode(i.first), expandSet, connectionSet, basicOrTop);
            }
        }
    }

    void CC_check(node* cur) {
        set<string> expandSet;  // 当前model包括的节点
        set<string> connectionSet;  // 所有节点的connection_list包含的节点
        set<string> basicOrTop; // expand中的basic和module top, 用于LCC阶段 name
        expandSet.insert(cur->name);
        CC_helper(cur, expandSet, connectionSet, basicOrTop);
        bool flag = true;
        for(const auto&i: connectionSet) {
            if(expandSet.count(i) == 0) {
                flag = false;
                break;
            }
        }
        if(flag) {
            LCC_check(cur, basicOrTop);
        }
    }

    void LCC_check(node* cur, set<string>& basicOrTop) {
        if(this->moduleDict.find(cur->name) == this->moduleDict.end()) {
            this->moduleDict[cur->name] = "m" + to_string(this->lccCount);
            this->lccCount++;
        }
        this->result.insert(cur->name);
        if(!this->NEW_MODULE)
            return;
        vector<vector<string>> obtainedList;
        for(auto& i: basicOrTop)
            obtainedList.push_back({i});
        int size = obtainedList.size();
        for(int i = 0; i < size-1; i++) {  // 获取具有相同connection_list的节点
            if(obtainedList[i].size() == 0)
                continue;
            for(int j = i+1; j < size; j++) {
                if(obtainedList[j].size() == 0)
                    continue;
                // 只比较connection_list，不比较AEG
                set<node*> connectionFirst, connectionSecond;
                for(auto &k: this->connection_list[obtainedList[i][0]])
                    connectionFirst.insert(k.first);
                for(auto &k: this->connection_list[obtainedList[j][0]])
                    connectionSecond.insert(k.first);
                if(connectionFirst.size() == connectionSecond.size()) {
                    bool same = true;
                    for(auto& k: connectionFirst) {
                        if(connectionSecond.count(k) == 0) {
                            same = false;
                            break;
                        }
                    }
                    if(same) {
                        obtainedList[i].push_back(obtainedList[j][0]);
                        obtainedList[j].erase(obtainedList[j].begin());
                    }
                }               
            }
        }
        for(auto& v: obtainedList) {
            if(v.size() > 1) {
                auto& i = this->connection_list[v[0]];
                node* firstGateNode = i.begin()->first;
                int firstAEG = i.begin()->second;
                bool nodeExist = false;
                node* moduleNode;
                int moduleAEG = 0;
                for(auto& i: this->connection_list[v[0]]) {
                    if(i.first->children.size() == v.size()) {
                        nodeExist = true;
                        moduleNode = i.first;
                        moduleAEG = i.second;
                        break;
                    }
                }
                if(!nodeExist) {
                    node* newNode = this->m_tree->create_node("m" + to_string(this->lccCount));
                    this->lccCount++;
                    newNode->gateType = firstGateNode->gateType;
                    for(auto& name: v)
                        newNode->children[name] = firstGateNode->children[name];
                    for(auto& i: this->connection_list[v[0]]) {
                        for(auto& name: v)
                            i.first->children.erase(name);
                        if(i.second == firstAEG) {
                            i.first->children[newNode->name] = true;
                        } else {
                            i.first->children[newNode->name] = false;
                        }
                    }
                    this->result.insert(newNode->name);
                } else {
                    for(auto& i: this->connection_list[v[0]]) {
                        if(i.first->children.size() > moduleNode->children.size()) {
                            for(auto& name: v)
                                i.first->children.erase(name);
                            if(i.second == firstAEG) {
                                i.first->children[moduleNode->name] = true;
                            } else {
                                i.first->children[moduleNode->name] = false;
                            }
                        } else if(moduleNode != i.first &&
                                  i.first->children.size() == moduleNode->children.size()) {
                                      for(auto& j: this->connection_list[i.first->name]) {
                                            j.first->children.erase(i.first->name);
                                            if(i.second == moduleAEG) {
                                                j.first->children[moduleNode->name] = true;
                                            } else {
                                                j.first->children[moduleNode->name] = false;
                                            }
                                      }
                                }
                    }
                    this->result.insert(moduleNode->name);
                    if(this->moduleDict.find(moduleNode->name) == this->moduleDict.end()) {
                        this->moduleDict[moduleNode->name] = "m" + to_string(this->lccCount);
                        this->lccCount++;
                    }
                }
            }
        }
    }
};