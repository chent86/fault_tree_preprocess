/*
 *Author: Tao Chen
 *Organization: SYSU
 */

#include "simplify.hpp"
#include "find_modules.hpp"
#include "json.hpp"
using json = nlohmann::json;

map<string, string> config = {
    {"INPUT_DIR", "example/input"},
    {"OUTPUT_DIR", "example/output"},
    {"SAME_GATE", "true"},
    {"ONE_CHILD", "true"},
    {"SAME_TREE", "true"},
    {"NORMAL_PROCESS", "true"},
    {"LCC_PROCESS", "true"},
    {"SIMPLE_OUTPUT", "false"}
};

void structToJson(package& p, string name) {
    json j;
    j["origin_basic_event_num"] = p.originBasicEventNum;
    j["origin_gate_evnet_num"] = p.originGateEvnetNum;
    j["basic_event_num"] = p.basicEventNum;
    j["gate_event_num"] = p.gateEventNum;
    j["modules_num"] = p.modulesNum;
    j["modularized"] = p.modularized;
    j["simplify_time"] = p.simplifyTime;
    j["module_time"] = p.moduleTime;

    j["coherent_map"] = {};
    for(auto &i: p.coherentMap)
        j["coherent_map"][i.first] = i.second;

    j["root_map"] = {};
    for(auto &i: p.rootMap)
        j["root_map"][i.first] = i.second;

    j["module_var_index_map"] = {};
    for(auto &i: p.moduleVarIndexMap) {
        j["module_var_index_map"][i.first] = {};
        for(auto &k: i.second) {
            j["module_var_index_map"][i.first][k.first] = k.second;
        }
    }

    j["module_index_var_map"] = {};
    for(auto &i: p.moduleIndexVarMap) {
        j["module_index_var_map"][i.first] = {};
        for(auto &k: i.second) {
            j["module_index_var_map"][i.first][to_string(k.first)] = k.second;
        }
    }

    std::ofstream package_file(config["OUTPUT_DIR"] + "/" + name.substr(0, name.length()-4) + "/package");
    package_file << j.dump();
    package_file.close();
}

int main(int argc, char* argv[]) {
    if(argc > 1) {
        for(int i = 1; i < argc; i++) {
            if(string(argv[i]) == "-h") {
                cout << "For example: \n./main -INPUT_DIR=example/input -OUTPUT_DIR=example/output "
                << "-SAME_GATE=true -ONE_CHILD=true -SAME_TREE=true -NORMAL_PROCESS=true "
                << "-LCC_PROCESS=true -SIMPLE_OUTPUT=false -FILE_NAME=chinese.dag" << endl;
                return 0;
            }
            string param = argv[i];
            int p = param.find("=");
            if(p != param.npos) {
                config[param.substr(1, p-1)] = param.substr(p+1, param.length()-1);
            }
        }
    }
    DIR *d = opendir(config["INPUT_DIR"].c_str());
    dirent* entry;
    while((entry=readdir(d)) != NULL) {
        string name = entry->d_name;
        if(config.count("FILE_NAME"))
            name = config["FILE_NAME"]; // 处理单个例子
        if(name == "." || name == "..")
            continue;
        tree* t = new tree();
        t->parse(config["INPUT_DIR"] + "/" + name);
        int originBasicEventNum = t->basicNum;
        int originGateEventNum = t->gateNum;
        string cmd = "mkdir -p " + config["OUTPUT_DIR"] + "/" + name.substr(0, name.length()-4);
        system(cmd.c_str());
        int curTime = time(0);
        simplify* s = new simplify(name, t, config["OUTPUT_DIR"], config["SAME_GATE"]=="true", 
                                   config["ONE_CHILD"]=="true", config["SAME_TREE"]=="true");
        int simTime = time(0);
        delete t;
        t = new tree();
        t->parse(config["OUTPUT_DIR"] + "/" + name.substr(0, name.length()-4) + "/" + name.substr(0, name.length()-4) + ".sdag");
        findModules* f = new findModules(t, config["OUTPUT_DIR"], name.substr(0, name.length()-4), originBasicEventNum, originGateEventNum, 
                                        config["NORMAL_PROCESS"]=="true", config["LCC_PROCESS"]=="true", config["SIMPLE_OUTPUT"]=="true");
        int moTime = time(0);
        package p = f->getData();
        p.simplifyTime = simTime - curTime;
        p.moduleTime = moTime - simTime;
        structToJson(p, name);
        delete s;
        delete f;
        delete t;
        cout << name << " " << "【simplify time】" << simTime-curTime << "【module time】" << moTime-simTime << 
        "【module】" << p.modulesNum << endl;
        if(config.count("FILE_NAME"))
            break;
    }
    closedir(d);
}