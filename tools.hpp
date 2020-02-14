#include <string>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

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
    set<string> printedSet;
    const set<char> charSet = {'(', ' ', '=', '@'};
    const set<char> ops = {'&', '|', '#'};
    int conNum = 0;
    int xoNum = 0;
    int alNum = 0;
public:
    tree();
    ~tree();
    node* create_node(string name);
    void parse(string fileName);
    void at_least_helper(vector<string> negList, vector<string> posList, int count, node* curNode, int lastPos);
    string quick_format();  // 输出结点map
    void dfs_format(string name, string& output);  // 输出树上的结点
};