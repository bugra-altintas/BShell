#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "parser.h"
using namespace std;
typedef struct process_bundle{
    string bundle_name;
    vector<vector<string>> processes; // every element is a process with optional arguments
} bundle;

//This function converts a vector of strings to array of strings. It is implemented for giving proper inputs to exec() functions.
void converter(vector<string>& arr, char**& char_arr){
    char_arr = (char **) calloc(arr.size() + 1,sizeof(char*));
    char_arr[arr.size()] = NULL;
    for(int i=0;i<arr.size();i++){
        char_arr[i] = (char*) calloc(arr[i].size(),sizeof(char));
        strcpy(char_arr[i],arr[i].c_str());
    }
}

//This function prints all the bundles existing in the system.
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
            if(input.command.type == QUIT){ // finish the shell
                break;
            }
            else if(input.command.type == PROCESS_BUNDLE_EXECUTION){ // EXECUTE BUNDLE
                if(input.command.bundle_count == 1){
                    string bundle_name(input.command.bundles[0].name);
                    bundle to_be_executed;
                    bool found = false;
                    for(bundle b : bundles){//FIND THE BUNDLE
                        if(bundle_name.compare(b.bundle_name) == 0){
                            to_be_executed = b;
                            found = true;
                            break;
                        }
                    }
                    if(found){
                        int nump = to_be_executed.processes.size();
                        pid_t pid[nump];
                        int child_status;
                        for(int i=0;i<nump;i++){// FORK NUMBER OF PROCESSES TIME AND EXECUTE EACH PROCESS IN ONE CHILD
                            if((pid[i] = fork()) == 0){ //child process
                                // do the child's work here, execute i'th process in the child.
                                //convert vector<string> process to char**, call execv()
                                char ** process;
                                converter(to_be_executed.processes[i],process);
                                execv(process[0],process+1);
                                cout << "hello from " << getpid() << endl;
                                return 1;
                            }
                            else{ //parent

                            }
                        }
                        for(int i=0;i<nump;i++) //REAP ALL CHILDS
                            wait(&child_status);
                    }
                    else{
                        cout << "Bundle " << bundle_name << " couldnt be founded!" << endl;
                    }

                }
                else{

                }
                
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