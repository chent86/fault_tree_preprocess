/*
 *Author: Tao Chen
 *Organization: SYSU
 */

#include "simplify.hpp"
#include "find_modules.hpp"
#include "config.hpp"

int main() {
    DIR *d = opendir(INPUT_DIR);
    dirent* entry;
    while((entry=readdir(d)) != NULL) {
        string name = entry->d_name;
        if(name == "." || name == "..")
            continue;
        tree* t = new tree();
        t->parse(string(INPUT_DIR) + "/" + name);
        int originBasicEventNum = t->basicNum;
        int originGateEventNum = t->gateNum;
        string cmd = "mkdir -p " + string(OUTPUT_DIR) + "/" + name.substr(0, name.length()-4);
        system(cmd.c_str());
        int curTime = time(0);
        simplify* s = new simplify(name, t, OUTPUT_DIR, SAME_GATE, ONE_CHILD, SAME_TREE);
        int simTime = time(0);
        delete t;
        t = new tree();
        t->parse(string(OUTPUT_DIR) + "/" + name.substr(0, name.length()-4) + "/" + name.substr(0, name.length()-4) + ".sdag");
        findModules* f = new findModules(t, OUTPUT_DIR, name.substr(0, name.length()-4), originBasicEventNum, originGateEventNum, 
                                        NORMAL_PROCESS, LCC_PROCESS, SIMPLE_OUTPUT);
        int moTime = time(0);
        package p = f->getData();
        p.simplifyTime = simTime - curTime;
        p.moduleTime = moTime - simTime;
        delete s;
        delete f;
        delete t;
        cout << name << " " << "【simplify time】" << simTime-curTime << "【module time】" << moTime-simTime << endl;
    }
    closedir(d);
}