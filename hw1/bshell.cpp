#include <iostream>
#include <vector>
#include <unistd.h>
#include "parser.h"
using namespace std;
typedef struct process_bundle{
    string bundle_name;
    vector<vector<string>> processes; // every element is a process with optional arguments
} bundle;

int main(){
    int execution = 0; // what shell is doing right now
    //char* line;
    string inp;
    parsed_input input;
    bundle *new_bundle = nullptr;
    vector<bundle> bundles;
    while(true){
        getline(cin,inp);
        cout <<"change" << endl;
        inp.push_back('\0');
        char line[inp.length()];
        for (int i = 0; i < sizeof(line); i++) 
            line[i] = inp[i];
        execution = parse(line,execution,&input);
        if(!execution){ //bundle creating or quitting
            if(!new_bundle){ // input will be 'pbc' or 'quit'
                if(input.command.type == QUIT){ // finish the shell
                    break;
                }
                else{ // bundle creation is started, bundle_name set.
                    string name(input.command.bundle_name);
                    new_bundle = new bundle;
                    new_bundle->bundle_name = name;
                    cout << "Bundle creation is started. Bundle name: " << new_bundle->bundle_name << endl;
                }
            }
            else{ // bundle creation process is going on, input will be 'pbs' or process //CHECKPOINT
                cout << "it is here" << endl;
                try{ // check whether it is a special command
                    if(input.command.type == PROCESS_BUNDLE_STOP){ //store created bundle, delete new_bundle and new_bundle = nullptr
                        cout << "Bundle creation is stopped. Registering the bundle " << new_bundle->bundle_name << endl;
                        bundles.push_back(*new_bundle); //store
                        delete new_bundle; //deallocate
                        new_bundle = nullptr; // make it nullptr again
                    }
                    //else{
                    //}
                }
                catch(...){ // if an error happens, it means that this is a process. register the process.
                    cout << "Adding new process: ";
                    int index = 0;
                    vector<string> process;
                    while(!input.argv[index]){
                        string arg(input.argv[index]);
                        cout << arg << " ";
                        process.push_back(arg);
                        index++;
                    }
                    cout << endl;
                    new_bundle->processes.push_back(process);
                }
            }
        }
        else{

        }

    }
}