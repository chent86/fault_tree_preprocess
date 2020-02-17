#include "find_modules.hpp"

findModules::findModules(tree* t, string outputDir, string fileName, int originBasicEventNum, int originGateEventNum,
                         bool NORMAL_PROCESS, bool LCC_PROCESS, bool SIMPLE_OUTPUT)
:m_tree(t), originBasicEventNum(originBasicEventNum), originGateEventNum(originGateEventNum),
 NORMAL_PROCESS(NORMAL_PROCESS), LCC_PROCESS(LCC_PROCESS), SIMPLE_OUTPUT(SIMPLE_OUTPUT){
    this->moduleDict["r1"] = "m0";
    if(NORMAL_PROCESS) {
        this->init_level(getNode("r1"));
        this->init_connection_list(getNode("r1"));
        this->run();
    } else {
        this->result.insert("r1");
    }
    this->get_sdag(getNode("r1"), "r1");
    this->output_sdag(outputDir + "/" + fileName + "/", fileName);
    this->get_cnf(outputDir + "/" + fileName + "/", fileName, SIMPLE_OUTPUT);
    this->get_gate_and_basic_num();
    this->check_coherent();
}

findModules::~findModules() {}

node* findModules::getNode(const string& name) {
    return this->m_tree->create_node(name);
}

void findModules::init_level(node* cur) {
    if(cur->children.size() == 0)
        return;
    for(auto& i:cur->children)
        init_level(getNode(i.first));
    cur->level = levelCount;
    this->levelCount++;
}

void findModules::init_connection_list(node* cur) {
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

void findModules::run() {
    this->PC_check();
}

string findModules::get_top_level_node(set<pair<node*, int>>& s) {
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

void findModules::PC_check() {
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

void findModules::CC_helper(node* cur, set<string>& expandSet, set<string>& connectionSet, set<string>& basicOrTop) {
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

void findModules::CC_check(node* cur) {
    set<string> expandSet;  // 当前model包括的节点
    set<string> connectionSet;  // 所有节点的connection_list包含的节点
    set<string> basicOrTop; // expand中的basic和module top, 用于LCC阶段 name
    expandSet.insert(cur->name);
    if(cur->name == "g23")
        cout << "hello" << endl;
    CC_helper(cur, expandSet, connectionSet, basicOrTop);
    bool flag = true;
    for(const auto&i: connectionSet) {
        if(expandSet.count(i) == 0) {
            flag = false;
            break;
        }
    }
    if(flag) {
        cout << cur->name << endl;
        LCC_check(cur, basicOrTop);
    }
}

void findModules::LCC_check(node* cur, set<string>& basicOrTop) {
    if(this->moduleDict.find(cur->name) == this->moduleDict.end()) {
        this->moduleDict[cur->name] = "m" + to_string(this->lccCount);
        this->lccCount++;
    }
    this->result.insert(cur->name);
    if(!this->LCC_PROCESS)
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
            if(connectionFirst.size() == connectionSecond.size()) { // 判断集合相等
                vector<node*> v;
                set_intersection(connectionFirst.begin(), connectionFirst.end(), connectionSecond.begin(),
                                 connectionSecond.end(), back_inserter(v));
                if(v.size() == connectionFirst.size()) {
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

void findModules::get_sdag(node* curNode, string curRootName) {
    if(curNode->name == curRootName) {
        if(this->sdag.find(curRootName) != this->sdag.end()) {
            return;
        } else {
            this->sdag[curRootName] = {"", {}};
        }
    }
    if(curNode->children.size() == 0)
        return;
    if(this->sdag[curRootName].second.count(curNode->name) != 0)
        return;
    string line = "";
    if(curNode->name == curRootName)
        line += "r1";
    else
        line += curNode->name;
    line += " := (";
    for(auto& i: curNode->children) {
        if(!i.second)
            line += "-";
        if(this->moduleDict.find(i.first) != this->moduleDict.end())
            line += this->moduleDict[i.first] + " " + curNode->gateType + " ";
        else
            line += i.first + " " + curNode->gateType + " ";
    }
    line = line.substr(0, line.length()-3) + ");\n";
    this->sdag[curRootName].first += line;
    this->sdag[curRootName].second.insert(curNode->name);
    for(auto& i:curNode->children) {
        if(this->result.count(i.first) == 0)
            get_sdag(getNode(i.first), curRootName); // 不是一个新的模块
        else
            get_sdag(getNode(i.first), i.first);
    }
}

string findModules::getRealName(string name) {
    if(this->moduleDict.find(name) != this->moduleDict.end())
        return this->moduleDict[name];
    return name;
}

void findModules::writeFile(const string& path, const string& data) {
    std::ofstream file(path);
    file << data;
    file.close(); 
}

void findModules::output_sdag(string outputDir, string fileName) {
    string path = "";
    for(auto& i: this->sdag) {
        if(i.first == "r1")
            path = fileName + "_m0.sdag";
        else
            path = fileName + "_" + getRealName(i.first) + ".sdag";
        this->writeFile(outputDir + path, i.second.first);
    }
}

void findModules::devide_node(set<string>& parentSet, set<string>& childSet) {
    for(auto& i: this->m_tree->nodeDict) {
        node* cur = i.second;
        if(cur->children.size() == 0 || this->result.count(cur->name))
            childSet.insert(cur->name);
        else
            parentSet.insert(cur->name);
    }
}

void findModules::get_cnf(string outputDir, string fileName, bool SIMPLE_OUTPUT) {
    for(auto& name: this->result) {
        string cnfResult = "";
        node* cur = getNode(name);
        set<string> parentSet, childSet;
        this->devide_node(parentSet, childSet);
        map<string, int> numDict; // name : num
        map<int, string> invNumDict; // num : name
        int index = 1;
        for(auto& name: childSet) {
            string realName = getRealName(name);
            numDict[realName] = index;
            invNumDict[index] = realName;
            index++;
        }
        string realModuleName = getRealName(cur->name);
        numDict[realModuleName] = index;
        invNumDict[index] = realModuleName;
        index++;
        for(auto& name: parentSet) {
            string realName = getRealName(name);
            numDict[realName] = index;
            invNumDict[index] = realName;
            index++;
        }
        this->moduleVarIndexMap[realModuleName] = numDict;
        this->moduleIndexVarMap[realModuleName] = invNumDict;
        parentSet.insert(cur->name);

        int totalLineNum = 1;
        map<int, pair<int, int>> lineScope; // {num : [from, to]}
        string line = "";
        for(auto& name: parentSet) {
            lineScope[numDict[getRealName(name)]] = {totalLineNum, 0};
            node* cur = getNode(name);
            if(cur->gateType == '&') {
                for(auto& i: cur->children) {
                    line += "-" + to_string(numDict[getRealName(cur->name)]) + " ";
                    if(!i.second)
                        line += "-";
                    line += to_string(numDict[getRealName(i.first)]) + " 0\n";
                    totalLineNum++;
                }
                line += to_string(numDict[getRealName(cur->name)]) + " ";
                for(auto& i: cur->children) {
                    if(i.second)
                        line += "-";
                    line += to_string(numDict[getRealName(i.first)]) + " ";
                }
                line += "0\n";
                totalLineNum++;
            } else {
                for(auto& i: cur->children) {
                    line += to_string(numDict[getRealName(cur->name)]) + " ";
                    if(i.second)
                        line += "-";
                    line += to_string(numDict[getRealName(i.first)]) + " 0\n";
                    totalLineNum++;
                }
                line += "-" + to_string(numDict[getRealName(cur->name)]) + " ";
                for(auto& i: cur->children) {
                    if(!i.second)
                        line += "-";
                    line += to_string(numDict[getRealName(i.first)]) + " ";
                }
                line += "0\n";
                totalLineNum++;
            }
            lineScope[numDict[getRealName(name)]].second = totalLineNum - 1;
        }
        if(SIMPLE_OUTPUT) {
            cnfResult += "c n orig vars " + to_string(childSet.size()) + "\n";
        } else {
            for(auto& i: numDict) {
                if(i.first == realModuleName)
                    cnfResult += "c " + to_string(i.second) + " = r1\n";
                else
                    cnfResult += "c " + to_string(i.second) + " = " + i.first + "\n";
            }
            cnfResult += "c\n";
            cnfResult += "n " + to_string(childSet.size()) + "\n";
            cnfResult += "c\n";
            for(auto& i: lineScope)
                cnfResult += "b " + to_string(i.first) + " " + to_string(i.second.first) + " " + to_string(i.second.second) + "\n";
            cnfResult += "c\n";
        }
        int totalNodeNum = childSet.size() + parentSet.size();
        cnfResult += "p cnf " + to_string(totalNodeNum) + " " + to_string(totalLineNum) + "\n";
        string p_cnfResult = cnfResult + to_string(numDict[getRealName(name)]) + " 0\n" + line;
        string n_cnfResult = cnfResult + "-" + to_string(numDict[getRealName(name)]) + " 0\n" + line;
        string p_path = "";
        string n_path = "";
        if(name == "r1") {
            p_path = fileName + "_m0-p.cnf";
            n_path = fileName + "_m0-n.cnf";
        } else {
            p_path = fileName + "_" + getRealName(name) + "-p.cnf";
            n_path = fileName + "_" + getRealName(name) + "-n.cnf";
        }
        this->writeFile(outputDir + p_path, p_cnfResult);
        this->writeFile(outputDir + n_path, n_cnfResult);
    }
}

void findModules::get_gate_and_basic_num() {
    for(auto& i: m_tree->nodeDict) {
        if(i.second->children.size() != 0)
            this->gateNum++;
        else
            this->basicNum++;
    }
}

bool findModules::coherent_helper(node* cur) {
    for(auto& i: cur->children) {
        if(!i.second)
            return false;
        if(getNode(i.first)->children.size() != 0 && !this->result.count(i.first)) {
            if(!coherent_helper(getNode(i.first)))
                return false;
        }
    }
    return true;
}

void findModules::check_coherent() {
    for(auto& name: this->result) {
        node* root = getNode(name);
        this->coherentMap[getRealName(name)] = coherent_helper(root);
    }
}

bool findModules::checkModuleHelper() {
    vector<set<string>> nodeList;
    for(auto& i: this->moduleVarIndexMap) {
        set<string> s;
        for(auto& j: i.second)
            s.insert(j.first);
        nodeList.push_back(s);
    }
    int size = nodeList.size();
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            if(i != j) {
                vector<string> v;
                set_intersection(nodeList[i].begin(), nodeList[i].end(), nodeList[j].begin(), nodeList[j].end(), back_inserter(v));
                if(v.size() != 1)
                    return false;
            }
        }
    }
    return true;
}

package findModules::getData() {
    for(auto& i: moduleVarIndexMap) {
        string key = i.first;
        map<string, int>& mapDict = i.second;
        for(auto& j: mapDict) {
            string nodeName = j.first;
            int& index = j.second;
            if(nodeName == key) {
                mapDict["r1"] = index;
                this->rootMap[key] = index;
                mapDict.erase(nodeName);
                break;
            }
        }
    }
    for(auto& i: moduleIndexVarMap) {
        string key = i.first;
        map<int, string>& mapDict = i.second;
        for(auto& j: mapDict) {
            string& nodeName = j.second;
            int index = j.first;
            if(nodeName == key) {
                mapDict[index] = "r1";
                break;
            }
        }
    }
    package p;
    p.originBasicEventNum = this->originBasicEventNum;
    p.originGateEvnetNum = this->originGateEventNum;
    p.basicEventNum = this->basicNum;
    p.gateEventNum = this->gateNum;
    p.coherentMap = this->coherentMap;
    p.modulesNum = this->result.size();
    p.moduleVarIndexMap = this->moduleVarIndexMap;
    p.moduleIndexVarMap = this->moduleIndexVarMap;
    p.modularized = this->checkModuleHelper();
    p.rootMap = this->rootMap;
    p.simplifyTime = 0;
    p.moduleTime = 0;
    return p;

}