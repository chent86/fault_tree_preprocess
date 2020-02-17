#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ctime>

using namespace std;

struct node {
    string name;
    char gateType;
    int level;
    map<string, bool> children; // name: sign, 符号true为正, false为负
    node(string name) {
        this->name = name;
        this->gateType = 'X';
        this->level = 0;  // 用于find modules步骤
    }
};

class tree {
public:
    map<string, node*> nodeDict;
    int gateNum = 0;
    int basicNum = 0;
private:
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
    void get_gate_and_basic_num();
};

#endif