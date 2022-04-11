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
void dummy_2p(){
    pid_t dummy = fork();
    if(dummy==0){
        cout << "Predecessor2";
        return;
    }
    else{
        cout << "Predecessor1Predecessor1Predecessor1Predecessor1Predecessor1Predecessor1";
        return;
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
                bundle_execution *execution = input.command.bundles; 
                int bundle_count = input.command.bundle_count; 
                if(input.command.bundle_count == 1){ //no pipelining, //in this case, we use only bundles[0]
                    string bundle_name(input.command.bundles[0].name);
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
                                if(execution[0].input){// if there is a input redirection, it's in loop since file offset
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
                    string bundle_name1(input.command.bundles[0].name);
                    bundle to_be_executed1;
                    for(bundle b : registered_bundles){//FIND THE BUNDLE1
                        if(bundle_name1.compare(b.bundle_name) == 0){
                            to_be_executed1 = b;
                            break;
                        }
                    }
                    int child_stat[3];
                    //---create first pipe
                    int fd_1[2];
                    pipe(fd_1);
                    //---fork for bundle[0]
                    pid_t p1 = fork();
                    if(p1 == 0){// predecessor bundle
                        close(fd_1[0]); //close read end
                        dup2(fd_1[1],1); //REDIRECT THE OUTPUT TO WRITE END OF FIRST PIPE
                        close(fd_1[1]);
                        int nump1 = to_be_executed1.processes.size();
                        pid_t pid[nump1];
                        for(int p=0;p<nump1;p++){
                            if((pid[p] = fork()) == 0){
                                if(input.command.bundles[0].input){// if there is a input redirection
                                    char* in = input.command.bundles[0].input;
                                    int fd_in = open(in, O_RDONLY);
                                    dup2(fd_in,0);
                                }
                                char** process;
                                converter(to_be_executed1.processes[p],process);
                                execv(process[0],process);                                
                            }
                        }
                        return 1;
                    }                   
                    close(fd_1[1]); //outside the predecessor bundle, we dont need write end of first pipe
                    for(int i=1;i<bundle_count;i++){
                        string bundle_name2(input.command.bundles[i].name);
                        bundle to_be_executed2;
                        for(bundle b : registered_bundles){//FIND THE BUNDLE2
                            if(bundle_name2.compare(b.bundle_name) == 0){
                                to_be_executed2 = b;
                                break;
                            }
                        }
                        int nump2 = to_be_executed2.processes.size();
                        //---create pipes
                        int pipes[nump2][2];
                        for(int p = 0;p<nump2;p++) //create needed pipes for input broadcasting
                            pipe(pipes[p]);
                        //---fork for repeater
                        pid_t p2 = fork();
                        if(p2 == 0){//repeater process
                            for(int p=0;p<nump2;p++)
                                close(pipes[p][0]); //inside the repeater process, we dont need read end of pipes
                            
                            //REPLICATE INPUT
                            char c[2]; 
                            c[1]= '\0';
                            while(read(fd_1[0],c,1) != 0){
                                for(int p=0;p<nump2;p++) //write each recevied to each pipe
                                    write(pipes[p][1],c,1);                        
                                memset(c,0,1);
                            }
                            return 1;
                        }
                        close(fd_1[0]); //outside the repeater, we dont need read end of first pipe    
                        for(int p=0;p<nump2;p++)//outside the repeater, we dont need write end of pipes
                            close(pipes[p][1]);                    
                        //---if there is a successor, create a first pipe
                        if((i+1)<bundle_count) pipe(fd_1);
                        //---fork for bundle[i]
                        pid_t p3 = fork();
                        if(p3 == 0){// successor bundle
                            if((i+1)<bundle_count){//if there is a successor
                                //redirect output to fd_1 pipe
                                close(fd_1[0]);
                                dup2(fd_1[1],1);
                                close(fd_1[1]);
                            }
                            else if(input.command.bundles[i].output){//if there is a output redirection
                                //redirect output to file
                                char* out = input.command.bundles[i].output;
                                int fd_out = open(out, O_WRONLY | O_APPEND | O_CREAT);
                                dup2(fd_out,1); //stdout redirected to 
                            }
                            pid_t pid[nump2];
                            for(int p=0;p<nump2;p++){
                                if((pid[p] = fork()) == 0){
                                    //---add first pipe or output redirection here
                                    dup2(pipes[p][0],0);//REDIRECT THE INPUT TO CORRESPONDING PIPE'S READ END
                                    close(pipes[p][0]);
                                    char** process;
                                    converter(to_be_executed2.processes[p],process);
                                    execv(process[0],process);                                
                                }
                            }
                            return 1;
                        }
                        close(fd_1[1]);
                        for(int p=0;p<nump2;p++)//outside the successor, we dont need read end of pipes
                            close(pipes[p][0]);
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