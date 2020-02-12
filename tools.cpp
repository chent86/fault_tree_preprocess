#include "tools.hpp"

struct node {
    string name;
    char gateType;
    map<string, bool> children; // name: sign, 符号true为正, false为负
    node(string name) {
        this->name = name;
    }
};

class tree {
private:
    map<string, node*> nodeDict;
    const set<char> charSet = {'(', ' ', '=', '@'};
    const set<char> ops = {'&', '|', '#'};
    int conNum = 0;
    int xoNum = 0;
    int alNum = 0;
public:
    tree() {}

    node* create_node(string name) {
        if(name[0] == '-') {
            name = name.substr(1);
        }
        if(nodeDict.find(name) != nodeDict.end()) {
            return nodeDict[name];
        }
        node* newNode = new node(name);
        nodeDict[name] = newNode;
        return newNode;
    }

    void parse(string fileName) {
        std::ifstream file(fileName);
        string line;
        bool anno = false; // 标记多行注释
        bool multiLine = false; // 标记跨行
        string lastLine = ""; // 跨行累加
        while(getline(file, line)) {

            // 处理空行
            if(line.length() == 0) {
                continue;
            }

            // 处理注释
            if(line[0] == '/') {
                // 多行注释
                if(line[line.length()-1] != '/') {
                    anno = true;
                }
                continue;
            }
            if(anno) {
                if(line[line.length()-1] == '/') {
                    anno = false;
                }
                continue;
            }

            // 处理未在一行
            if(line[line.length()-1] != ';' && line[line.length()-2] != ';') { //最后一个字符可能是\n
                lastLine += line;
                multiLine = true;
                continue;
            }
            if(multiLine) {
                multiLine = false;
                line = lastLine + line;
                lastLine = "";
            }
            int index = 0;
            int length = line.length();
            string name = "";
            bool atleast = false;
            char gateType = ' ';
            

            // 获取父结点名称
            while(index < length) {
                if(line[index] == ' ' || line[index] == ':') {
                    break;
                }
                name += line[index];
                index++;
            }
            node* new_node = this->create_node(name);

            // 跳过无意义段(内注释)
            while(line[index] != '=') {
                index++;
            }
            while(charSet.count(line[index])) {
                if(line[index] == '@') {
                    atleast = true;
                }
                index++;
            }

            name = "";
            bool sign = true;
            // 不是aleast的情况
            if(!atleast) {
                // 构造子树
                while(index < length) {
                    if(ops.count(line[index]) || line[index] == ')' || line[index] == ';') {
                        if(ops.count(line[index])) {
                            new_node->gateType = line[index];    
                        }
                        if(new_node->children.find(name) == new_node->children.end() ||
                           new_node->children[name] == sign) {
                            new_node->children[name] = sign;
                            this->create_node(name);
                        } else {
                            // 特殊处理符号不同的相同孩子, 新增等价结点
                            this->conNum += 1;
                            node* conNode = this->create_node("con" + to_string(this->conNum));
                            conNode->gateType = '&';
                            conNode->children[name] = sign;
                            new_node->children[conNode->name] = 0;
                        }
                        name = "";
                        sign = true;
                        index += 1;
                        if(line[index] == ')' || line[index] == ';') {
                            break;
                        }
                        continue;                      
                    }
                    if(line[index] != ' ') {
                        if(line[index] == '-') {
                            sign = false;
                        } else {
                            name += line[index];
                        }
                    }
                    index += 1;
                }
                // 展开异或
                if(new_node->gateType == '#') {
                    node* curRoot = new node(new_node->name);
                    node* temp = curRoot;
                    index = 1;
                    length = new_node->children.size();
                    node* newXor;
                    for(auto& i: new_node->children) {
                        if(index == length-1) {
                            name = i.first;
                            sign = i.second;
                        } else {
                            if(index != length) {
                                this->xoNum += 1;
                                newXor = this->create_node("xo" + to_string(this->xoNum));
                            }
                            this->xoNum += 1;
                            node* xoFirst = this->create_node("xo" + to_string(this->xoNum));
                            xoFirst->gateType = '&';
                            xoFirst->children[i.first] = !i.second; // ~A & B
                            if(index == length) {
                                xoFirst->children[name] = sign;
                            } else {
                                xoFirst->children[newXor->name] = true;
                            }
                            this->xoNum += 1;
                            node* xoSecond = this->create_node("xo" + to_string(this->xoNum));
                            xoSecond->gateType = '&';
                            xoSecond->children[i.first] = i.second; // A & ~B
                            if(index == length) {
                                xoSecond->children[name] = !sign;
                            } else {
                                xoSecond->children[newXor->name] = false;
                            }

                            curRoot->gateType = '|';
                            curRoot->children[xoFirst->name] = true;
                            curRoot->children[xoSecond->name] = true;
                            
                            if(index != length) {
                                curRoot = newXor;
                            }
                        }
                        index++;
                    }
                    this->nodeDict[new_node->name] = temp;
                    delete new_node;
                }
            } else {
                // 处理atleast
                string count = ""; // atleast中至少几个为真
                vector<string> curList;
                name = "";
                new_node->gateType = '|';
                while(line[index] != ',' && line[index] != ' ') {
                    count += line[index];
                    index++;
                }
                while(line[index] == ',' || line[index] == ' ' || line[index] == '[')
                    index++;
                while(index < length) {
                    if(line[index] == ',' || line[index] == ']') {
                        this->create_node(name);
                        curList.push_back(name);
                        name = "";
                        if(line[index] == ']')
                            break;
                        index++;
                        continue;
                    }
                    if(line[index] != ' ') {
                        name += line[index];
                    }
                    index++;
                }
                for(int posNum = atoi(count.c_str()); posNum <= curList.size(); posNum++) {
                    this->at_least_helper(curList, {}, posNum, new_node, curList.size());
                }
            }
        }
        file.close();
        this->quick_format();
    }

    // 用lastPos记录，按照单个方向每次抽取一个，避免得到相同的组合
    void at_least_helper(vector<string> negList, vector<string> posList, int count, node* curNode, int lastPos) {
        if(count == 0) {
            this->alNum++;
            node* newNode = this->create_node("al" + to_string(this->alNum));
            newNode->gateType = '&';
            curNode->children[newNode->name] = true;
            for(auto& n: negList)
                newNode->children[n] = false;
            for(auto& n: posList)
                newNode->children[n] = true;
            return;
        }
        int curPos = negList.size();
        while(curPos > 0) {
            curPos--;
            if(curPos >= lastPos)
                continue;
            vector<string> newNegList,newPosList;
            for(auto& i: negList)
                newNegList.push_back(i);
            newNegList.erase(newNegList.begin()+curPos);
            for(auto& i: posList)
                newPosList.push_back(i);
            newPosList.push_back(negList[curPos]);
            this->at_least_helper(newNegList, newPosList, count-1, curNode, curPos);
        }
    }

    void quick_format() {
        set<string> printedSet;
        string output = "";
        for(auto& i: this->nodeDict) {
            node* cur = i.second;
            if(cur->children.size() != 0) {
                output += cur->name;
                output += " := (";
                for(auto& j: cur->children) {
                    if(!j.second)
                        output += "-";
                    output += j.first + " " + cur->gateType + " ";
                }
                output = output.substr(0, output.length()-3) + ");\n";
            }
        }
        cout << output << endl;
    }
};

int main() {
    tree t;
    t.parse("test.dag");
}