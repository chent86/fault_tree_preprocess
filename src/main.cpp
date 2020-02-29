/*
 *Author: Tao Chen
 *Organization: SYSU
 */

#include "simplify.hpp"
#include "find_modules.hpp"

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

int main(int argc, char* argv[]) {
    if(argc > 1) {
        for(int i = 1; i < argc; i++) {
            if(string(argv[i]) == "-h") {
                cout << "For example: \n./main -INPUT_DIR=example/input -OUTPUT_DIR=example/output "
                << "-SAME_GATE=true -ONE_CHILD=true -SAME_TREE=true -NORMAL_PROCESS=true "
                << "-LCC_PROCESS=true -SIMPLE_OUTPUT=false" << endl;
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
        std::ofstream package_file(config["OUTPUT_DIR"] + "/" + name.substr(0, name.length()-4) + "/package", ofstream::binary);
        package_file.write((char*)&p, sizeof(package));
        package_file.close();
        delete s;
        delete f;
        delete t;
        cout << name << " " << "【simplify time】" << simTime-curTime << "【module time】" << moTime-simTime << 
        "【module】" << p.modulesNum << endl;
    }
    closedir(d);
}