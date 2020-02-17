#include "simplify.hpp"
#include "find_modules.hpp"

const string INPUT_DIR = "data/input";
const string OUTPUT_DIR = "data/output";
bool SAME_GATE = true;
bool ONE_CHILD = true;
bool SAME_TREE = true;

bool NORMAL_PROCESS = true;
bool LCC_PROCESS = true;

bool SIMPLE_OUTPUT = true;

int main() {
    set<string> avoid = {".", "..", "edf9206.dag", "das9701.dag", "elf9601.dag", "nus9601.dag"};
    DIR *d = opendir(INPUT_DIR.c_str());
    dirent* entry;
    while((entry=readdir(d)) != NULL) {
        string name = entry->d_name;
        // name = "das9209.dag";
        if(!avoid.count(name)) {
            try {
                tree* t = new tree();
                t->parse(INPUT_DIR + "/" + name);
                int originBasicEventNum = t->basicNum;
                int originGateEventNum = t->gateNum;
                string cmd = "mkdir -p " + OUTPUT_DIR + "/" + name.substr(0, name.length()-4);
                system(cmd.c_str());
                int curTime = time(0);
                simplify* s = new simplify(name, t, OUTPUT_DIR, SAME_GATE, ONE_CHILD, SAME_TREE);
                int simTime = time(0);
                delete t;
                t = new tree();
                t->parse(OUTPUT_DIR + "/" + name.substr(0, name.length()-4) + "/" + name.substr(0, name.length()-4) + ".sdag");
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
            } catch(int err) {
                cout << err << endl;
            }
        }
        // break;
    }
    closedir(d);
}