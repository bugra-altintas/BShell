#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "parser.h"
using namespace std;
typedef struct process_bundle{
    string bundle_name;
    vector<vector<string>> processes; // every element is a process with optional arguments
} bundle;

//This function converts a vector of strings to array of strings. It is implemented for giving proper inputs to exec() functions.
void converter(vector<string>& arr, char**& char_arr){
    char_arr = (char **) calloc(arr.size(),sizeof(char*));
    char_arr[arr.size()-1] = NULL;
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
    vector<bundle> registered_bundles;
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
                if(input.command.bundle_count == 1){ //no pipelining
                    string bundle_name(input.command.bundles[0].name);
                    bundle to_be_executed;
                    bundle_execution *execution = input.command.bundles; //in this case, we use only bundles[0] 
                    bool found = false;
                    for(bundle b : registered_bundles){//FIND THE BUNDLE
                        if(bundle_name.compare(b.bundle_name) == 0){
                            to_be_executed = b;
                            found = true;
                            break;
                        }
                    }
                    if(found){
                        pid_t bpid = fork(); // create a process for bundle
                        int cs;
                        if(bpid == 0){ // execute the bundle here
                            cout << "Executing bundle in child process, " << getpid() << " in parent " << getppid() << endl;
                            int nump = to_be_executed.processes.size();
                            pid_t pid[nump];
                            int child_status;

                            if(execution[0].output){//if there is a output redirection
                                char* out = execution[0].output;
                                int fd_out = open(out, O_WRONLY | O_APPEND | O_CREAT);
                                dup2(fd_out,1); //stdout redirected to 
                            }
                            for(int i=0;i<nump;i++){// FORK NUMBER OF PROCESSES TIME AND EXECUTE EACH PROCESS IN ONE CHILD
                                if((pid[i] = fork()) == 0){ //child process
                                    //execute i'th process in the child.
                                    //convert vector<string> process to char**, call execv()
                                    if(execution[0].input){// if there is a input redirection
                                        char* in = execution[0].input;
                                        int fd_in = open(in, O_RDONLY);
                                        dup2(fd_in,0);
                                    }
                                    char** process;
                                    converter(to_be_executed.processes[i],process);
                                    execv(process[0],process);
                                    return 1;
                                }
                            }
                            for(int i=0;i<nump;i++) //REAP ALL CHILDS
                                wait(&child_status);
                            return 1;
                        }
                        wait(&cs);    
                    }
                    else{
                        cout << "Bundle " << bundle_name << " couldnt be founded!" << endl;
                    }

                }
                else{ //pipelining here
                    bundle_execution *bundles = input.command.bundles;
                    for(int i=0;i<input.command.bundle_count-1;i++){
                        pid_t spid = fork();
                        if(spid == 0){ //sender process for bundle1
                            int fd_pred[2];
                            pid_t *sender_pids;
                            int cs;
                            int nump;
                            pipe(fd_pred);
                            int rep_status;
                            int finish = 0;
                            pid_t rep_pid = fork();
                            //create a pipe here with fd_pred
                            if(rep_pid > 0){ //sender process, execute bundles[i], writer

                                close(fd_pred[0]); //close read end
                                dup2(fd_pred[1],1); //redirect output to fd_pred[1]
                                //find the bundle
                                int rep_status;
                                string bundle_name(input.command.bundles[i].name);
                                bundle to_be_executed; 
                                bool found = false;
                                for(bundle b : registered_bundles){//FIND THE BUNDLE
                                    if(bundle_name.compare(b.bundle_name) == 0){
                                        to_be_executed = b;
                                        found = true;
                                        break;
                                    }
                                }
                                if(found){
                                    //fork and exec for each
                                    nump = to_be_executed.processes.size();
                                    sender_pids = (pid_t*) calloc(nump,sizeof(pid_t));
                                    int child_status;
                                    for(int j=0;j<nump;j++){// FORK NUMBER OF PROCESSES TIME AND EXECUTE EACH PROCESS IN ONE CHILD
                                        if((sender_pids[j] = fork()) == 0){ //child process
                                            //execute i'th process in the child.
                                            //convert vector<string> process to char**, call execv()
                                            if(i==0 && bundles[i].input){// if there is a input redirection, for only the first bundle
                                                char* in = bundles[i].input;
                                                int fd_in = open(in, O_RDONLY);
                                                dup2(fd_in,0);
                                            }
                                            char** process;
                                            converter(to_be_executed.processes[j],process);
                                            execv(process[0],process);
                                        }
                                    }
                                    for(int k=0;k<nump;k++){
                                        //waitpid(sender_pids[k],&child_status,0);
                                        wait(&child_status);
                                    } //REAP ALL CHILDS
                                }
                                else{
                                    cout << "Bundle " << bundle_name << " can not be found" << endl;
                                }
                            }
                            else{//repeater process *****************************************************************
                                string bundle_name(input.command.bundles[i+1].name);
                                bundle to_be_executed; 
                                bool found = false;
                                for(bundle b : registered_bundles){//FIND THE BUNDLE
                                    if(bundle_name.compare(b.bundle_name) == 0){
                                        to_be_executed = b;
                                        found = true;
                                        break;
                                    }
                                }
                                int nump = to_be_executed.processes.size();
                                int pipes[nump][2];
                                for(int p = 0;p<nump;p++)
                                    pipe(pipes[p]);
                                pid_t recv_pid = fork();
                                if(recv_pid == 0){ //receiver process
                                    pid_t recv_pids[nump];
                                    for(int t = 0;t<nump;t++){
                                        int* p = pipes[t];
                                        if((recv_pids[t] = fork()) == 0){ //child process
                                            //execute i+1'th process in the child.
                                            close(p[1]); //close write end
                                            dup2(p[0],0); //redirect stdin to read end
                                            close(p[0]); //close read end, its duplicated
                                            char** process;
                                            converter(to_be_executed.processes[t],process);
                                            execv(process[0],process);
                                        }
                                    }

                                }
                                else{//repeater process
                                    close[fd_pred[1]]; //close write end
                                    char line[10];
                                    while(1){
                                        int n = read(fd_pred[0],line,10);
                                        //write to every pipe
                                        for(int t = 0;t<nump;t++){
                                            write(pipes[t][1],line,10);
                                        }
                                    }
                                }
                                return 1;                              
                            }//***************************************************************************************
                        }

                    }
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
                        registered_bundles.push_back(*new_bundle); //store
                        delete new_bundle; //deallocate
                        new_bundle = nullptr; // make it nullptr again
                        is_bundle_creation = 0;
                        printBundles(registered_bundles);
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