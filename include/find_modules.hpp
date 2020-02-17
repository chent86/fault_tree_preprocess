#include "tools.hpp"

struct package {
    int originBasicEventNum;
    int originGateEvnetNum;
    int basicEventNum;
    int gateEventNum;
    map<string, bool> coherentMap;
    int modulesNum;
    map<string, map<string, int>> moduleVarIndexMap;
    map<string, map<int, string>> moduleIndexVarMap;
    bool modularized;
    map<string, int> rootMap;
    int simplifyTime;
    int moduleTime;
};

class findModules {
private:
    tree* m_tree;
    bool NORMAL_PROCESS;
    bool LCC_PROCESS;
    bool SIMPLE_OUTPUT;
    int levelCount = 1;
    map<string, set<pair<node*, int>>> connection_list; // childName: [(parentNode, AEG)]
    set<string> bothAEG;
    set<string> result; // 已经得到的model的root的name
    int lccCount = 1; // 为LCC阶段新增的门
    map<string, string> moduleDict; // { name : module_name } 不替换原有节点名称的代替品
    map<string, pair<string, set<string>>> sdag; // { root_name : (output, printed_set) } printed_set用于保证在一个module中不重复定义
    map<string, map<string, int>> moduleVarIndexMap;
    map<string, map<int, string>> moduleIndexVarMap;
    int gateNum = 0;
    int basicNum = 0;
    int originBasicEventNum;
    int originGateEventNum;
    map<string, bool> coherentMap;
    map<string, int> rootMap;
public:
    findModules(tree* t, string outputDir, string fileName, int originBasicEventNum, int originGateEventNum,
                bool NEW_MODULE=true, bool LCC_PROCESS=true, bool SIMPLE_OUTPUT=false);
    ~findModules();
    node* getNode(const string& name);
    void init_level(node* cur);
    int getAEG(const char& gateType, const bool& sign);
    void init_connection_list(node* cur);
    void run();
    string get_top_level_node(set<pair<node*, int>>& s);
    void PC_check();
    void CC_helper(node* cur, set<string>& expandSet, set<string>& connectionSet, set<string>& basicOrTop);
    void CC_check(node* cur);
    void LCC_check(node* cur, set<string>& basicOrTop);
    void get_sdag(node* curNode, string curRootName);
    string getRealName(string name);
    void writeFile(const string& path, const string& data);
    void output_sdag(string outputDir, string fileName);
    void get_cnf(string outputDir, string fileName, bool SIMPLE_OUTPUT);
    void devide_node(set<string>& parentSet, set<string>& childSet);
    void get_gate_and_basic_num();
    void check_coherent();
    bool coherent_helper(node* cur);
    bool checkModuleHelper();
    package getData();
};
