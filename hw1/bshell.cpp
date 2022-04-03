#include <iostream>
#include <vector>
#include <unistd.h>
#include "parser.h"
using namespace std;
typedef struct process_bundle{
    string bundle_name;
    vector<vector<string>> processes; // every element is a process with optional arguments
} bundle;
void printBundles(vector<bundle>& bundles){
    cout << "Bundles are:" << endl;
        for(int j=0;j<bundles.size();j++){
            cout << "\tBundle " << bundles[j].bundle_name << ":" << endl;
            for(vector<string> p : bundles[j].processes){
                cout << "\t\t";
                for(string s:p){
                    cout << s << " ";
                }
                cout << endl;
            }
        }
}
int main(){
    int execution = 0; // what shell is doing right now
    string inp;
    int is_bundle_creation = 0;
    bundle *new_bundle = nullptr;
    vector<bundle> bundles;
    while(true){
        getline(cin,inp);
        inp.push_back(' ');         // ADJUSTING INPUT
        inp.push_back('\0');
        char line[inp.length()];
        for (int i = 0; i < sizeof(line); i++)
            line[i] = inp[i];       // ADJUSTING INPUT
        parsed_input input;
        parse(line,is_bundle_creation,&input);
        if(!is_bundle_creation){ //bundle creating or executing or quitting
            // input will be 'pbc' or 'execution' or 'quit' 
            if(input.command.type == QUIT){ // finish the shell
                break;
            }
            else if(input.command.type == PROCESS_BUNDLE_EXECUTION){
                //execute bundle
            }
            else{ // bundle creation is started, bundle_name set.
                string name(input.command.bundle_name);
                new_bundle = new bundle;
                new_bundle->bundle_name = name;
                cout << "Bundle creation is started. Bundle name: " << new_bundle->bundle_name << endl;
                is_bundle_creation = 1;
            }
            
        }
        else{ // bundle creation process is going on, input will be 'pbs' or a process
            try{ // check whether it is a special command
                if(input.command.type == PROCESS_BUNDLE_STOP){ //store created bundle, delete new_bundle and new_bundle = nullptr
                        cout << "Bundle creation is stopped. Registering the bundle " << new_bundle->bundle_name << endl;
                        bundles.push_back(*new_bundle); //store
                        delete new_bundle; //deallocate
                        new_bundle = nullptr; // make it nullptr again
                        is_bundle_creation = 0;
                        printBundles(bundles);
                }
                else
                    throw 0;
                }
                catch(int error){ // if an error happens, it means that this is a process. register the process.
                    cout << "Adding new process: ";
                    int index = 0;
                    vector<string> process;
                    while(1){
                        if(input.argv[index] == NULL)
                            break;
                        string arg(input.argv[index]);
                        cout << input.argv[index] << " ";
                        process.push_back(arg);
                        index++;
                    }
                    cout << endl;
                    new_bundle->processes.push_back(process);
                }

        }

    }
}