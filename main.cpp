#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

class Register_File{
public:
    vector<int> table = {0,2,4,6,8};
    const int operator[](string reg)const{
        return table[reg[1] - '0' - 1];
    }
    int& operator[](string reg){
        return table[reg[1] - '0' - 1];
    }

    void show(){
        cout<<"      ┌──RF──┐"<<endl;
        for(int i = 0 ; i < table.size();i++){
            cout<<"  F"<<i + 1<<"  │"<<setw(5)<<table[i]<<" │"<<endl;
        }
        cout<<"      └──────┘"<<endl;
    }
};

class Register_Alias_Table{
public:
    vector<string> table = {"","","","",""};
    const string operator[](string reg)const{
        return table[reg[1] - '0' - 1];
    }
    string& operator[](string reg){
        return table[reg[1] - '0' - 1];
    }

    void show(){
        cout<<"      ┌──RAT─┐"<<endl;
        for(int i = 0 ; i < table.size();i++){
            cout<<"  F"<<i + 1<<"  │"<<setw(5)<<table[i]<<" │"<<endl;
        }
        cout<<"      └──────┘"<<endl;
    }
};

Register_File RF;
Register_Alias_Table RAT;

class Instruction{
public:
    Instruction(string name,string dest_reg,string source_reg1,string source_reg2){
        this->name = name;
        this->dest_reg = dest_reg;
        this->source_reg1 = source_reg1;
        this->source_reg2 = source_reg2;
        this->orig_dest = dest_reg;
    }
    Instruction(){}

    bool is_ready(){
        if(this->name == ""){
            return false;
        }

        return !(source_reg1[0] == 'R' || source_reg2[0] == 'R');
    }

    string name;
    string dest_reg;
    string source_reg1;
    string source_reg2;
    
    string orig_dest;
};

class Reservation_station{
public:
    vector<Instruction> add_sub_queue = vector<Instruction>(3);
    vector<Instruction> mul_div_queue = vector<Instruction>(2);

    int add_sub_process_index = -1;
    int mul_div_process_index = -1;

    int add_sub_remain_cycle = -1;
    int mul_div_remain_cycle = -1;

    const int ADD_SUB_TOTAL_CYCLE = 2;
    const int MUL_DIV_TOTAL_CYCLE = 10;

    vector<int> add_sub_dispatch_order = {0,1,2};
    vector<int> mul_div_dispatch_order = {0,1};


    void _broadcast(pair<string,int> reg_val){
        for(int i = 0 ; i < add_sub_queue.size();i++){
            if(add_sub_queue[i].source_reg1 == reg_val.first){
                add_sub_queue[i].source_reg1 = to_string(reg_val.second);
            }
            if(add_sub_queue[i].source_reg2 == reg_val.first){
                add_sub_queue[i].source_reg2 = to_string(reg_val.second);
            }
        }
        for(int i = 0 ; i < mul_div_queue.size();i++){
            if(mul_div_queue[i].source_reg1 == reg_val.first){
                mul_div_queue[i].source_reg1 = to_string(reg_val.second);
            }
            if(mul_div_queue[i].source_reg2 == reg_val.first){
                mul_div_queue[i].source_reg2 = to_string(reg_val.second);
            }
        }
    }
    
    void process_and_broadcast(){
        this->dispatch();
        if(add_sub_remain_cycle > 0){ // for add / sub
            add_sub_remain_cycle--;
        }else if(add_sub_remain_cycle != -1){ //broadcast

            Instruction process_inst = add_sub_queue[add_sub_process_index];
            
            // calculate result
            int result;
            if(process_inst.name == "SUB"){
                result = stoi(process_inst.source_reg1) - stoi(process_inst.source_reg2);
            }else{
                result = stoi(process_inst.source_reg1) + stoi(process_inst.source_reg2);
            }
            
            // broadcast result to RS's
            this->_broadcast(pair<string,int>("RS" + to_string(add_sub_process_index + 1),result));

            // write back to register file
            if(RAT[process_inst.orig_dest] == process_inst.dest_reg){
                RAT[process_inst.orig_dest] = "";
                RF[process_inst.orig_dest] = result;
            }

            // clear instruction space for next inst
            add_sub_queue[add_sub_process_index] = Instruction();
            add_sub_process_index = -1;
            add_sub_remain_cycle = -1;
        }

        if(mul_div_remain_cycle > 0){ // for mul / div
            mul_div_remain_cycle--;
        }else if(mul_div_remain_cycle != -1){ //broadcast

            Instruction process_inst = mul_div_queue[mul_div_process_index];
            
            // calculate result
            int result;
            if(process_inst.name == "DIV"){
                result = stoi(process_inst.source_reg1) / stoi(process_inst.source_reg2);
            }else{
                result = stoi(process_inst.source_reg1) * stoi(process_inst.source_reg2);
            }
            
            // broadcast result to RS's
            this->_broadcast(pair<string,int>("RS" + to_string(mul_div_process_index + 4),result));

            // write back to register file
            if(RAT[process_inst.orig_dest] == process_inst.dest_reg){
                RAT[process_inst.orig_dest] = "";
                RF[process_inst.orig_dest] = result;
            }

            // clear instruction space for next inst
            mul_div_queue[mul_div_process_index] = Instruction();
            mul_div_process_index = -1;
            mul_div_remain_cycle = -1;
        }

    }

    bool add_instruction(Instruction inst){
        this->issue_preprocess(inst);
        if(inst.name == "ADD" || inst.name == "ADDI" || inst.name == "SUB"){
            int avaliable_index = this->find_empty_space(this->add_sub_queue);
            if(avaliable_index == -1){
                return false;
            }

            inst.dest_reg = "RS" + to_string(avaliable_index + 1);
            RAT[inst.orig_dest] = inst.dest_reg;
            add_sub_queue[avaliable_index] = inst;
        }else{
            int avaliable_index = this->find_empty_space(this->mul_div_queue);
            if(avaliable_index == -1){
                return false;
            }

            inst.dest_reg = "RS" + to_string(avaliable_index + 4);
            RAT[inst.orig_dest] = inst.dest_reg;
            mul_div_queue[avaliable_index] = inst;
        }
        return true;
    }

    int find_empty_space(vector<Instruction> &que){
        for(int i = 0 ; i < que.size();i++){
            if(que[i].name == ""){
                return i;
            }
        }
        return -1;
    }

    bool is_empty(){
        bool is_empty = true;
        for(int i = 0 ; i < add_sub_queue.size(); i++){
            is_empty &= add_sub_queue[i].name == "";
        }

        for(int i = 0 ; i < mul_div_queue.size();i++){
            is_empty &= mul_div_queue[i].name == "";
        }
        return is_empty;
    }

    void issue_preprocess(Instruction& inst){
        if(RAT[inst.source_reg1] != ""){
            inst.source_reg1 = RAT[inst.source_reg1];
        }else{
            inst.source_reg1 = to_string(RF[inst.source_reg1]);
        }
        if(inst.source_reg2[0] == 'F'){
            if(RAT[inst.source_reg2] == ""){
                inst.source_reg2 = to_string(RF[inst.source_reg2]);
            }else{
                inst.source_reg2 = RAT[inst.source_reg2];
            }
        }

        
    }

    void dispatch(){
        if(add_sub_process_index == -1){
            for(int i = 0 ; i < add_sub_dispatch_order.size();i++){
                int index = add_sub_dispatch_order[i];
                if(add_sub_queue[index].is_ready()){
                    this->add_sub_process_index = index;
                    this->add_sub_remain_cycle = this->ADD_SUB_TOTAL_CYCLE;

                    add_sub_dispatch_order.erase(add_sub_dispatch_order.begin() + i);
                    add_sub_dispatch_order.push_back(index);

                    break;
                }
            }
        }

        if(mul_div_process_index == -1){
            for(int i = 0 ; i < mul_div_dispatch_order.size();i++){
                int index = mul_div_dispatch_order[i];
                if(mul_div_queue[index].is_ready()){
                    this->mul_div_process_index = index;
                    this->mul_div_remain_cycle = this->MUL_DIV_TOTAL_CYCLE;

                    mul_div_dispatch_order.erase(mul_div_dispatch_order.begin() + i);
                    mul_div_dispatch_order.push_back(index);
                    break;
                }
            }
        }
    }

    void show(){

        cout<<"      ┌───────┬───────┬───────┐               ┌───────┬───────┬───────┐"<<endl;

        for(int i = 0 ; i < 2;i++){
            cout<<"  RS"<<i + 1 <<" │ "<<setw(5)<<add_sub_queue[i].name<<" │ "<<setw(5)<<add_sub_queue[i].source_reg1<<" │ "<<setw(5)<<add_sub_queue[i].source_reg2<<" │ ";
            cout<<"         ";
            cout<<" RS"<<i+4<<" │ "<<setw(5)<<mul_div_queue[i].name<<" │ "<<setw(5)<<mul_div_queue[i].source_reg1<<" │ "<<setw(5)<<mul_div_queue[i].source_reg2<<" │"<<endl;
        }
        cout<<"  RS3 │ "<<setw(5)<<add_sub_queue[2].name<<" │ "<<setw(5)<<add_sub_queue[2].source_reg1<<" │ "<<setw(5)<<add_sub_queue[2].source_reg2<<" │ ";
        cout<<"        ";
        cout<<"      └───────┴───────┴───────┘"<<endl;
        cout<<"      └───────┴───────┴───────┘"<<endl;

        cout<<endl;

        cout<<"      ┌───────┬───────┬───────┐               ┌───────┬───────┬───────┐"<<endl;
        
        if(add_sub_process_index != -1){
            cout<<"  Buf │ "<<setw(5)<<add_sub_queue[add_sub_process_index].name;
            cout<<" │ "<<setw(5)<<add_sub_queue[add_sub_process_index].source_reg1;
            cout<<" │ "<<setw(5)<<add_sub_queue[add_sub_process_index].source_reg2<<" │ ";
        }else{
            cout<<"  Buf │       │       │       │ ";
        }

        cout<<"        ";
        if(mul_div_process_index != -1){
            cout<<"  Buf │ "<<setw(5)<<mul_div_queue[mul_div_process_index].name;
            cout<<" │ "<<setw(5)<<mul_div_queue[mul_div_process_index].source_reg1;
            cout<<" │ "<<setw(5)<<mul_div_queue[mul_div_process_index].source_reg2<<" │ "<<endl;
        }else{
            cout<<"  Buf │       │       │       │"<<endl;
        }
        cout<<"      └───────┴───────┴───────┘               └───────┴───────┴───────┘"<<endl;

        cout<<endl;
        
        cout<<"  Remain Cycle : ";
        if(this->add_sub_remain_cycle != -1){
            cout<<setw(4)<<left<<add_sub_remain_cycle;
        }else{
            cout<<"NaN ";
        }

        cout<<"                     Remain Cycle : ";
        if(this->mul_div_remain_cycle != -1){
            cout<<setw(4)<<left<<mul_div_remain_cycle;
        }else{
            cout<<"NaN ";
        }
        cout<<right<<endl;

    }
};

int main(int argc, char* argv[]){

    if(argc < 2){
        cout<<"input file not found"<<endl;
        exit(-1);
    }

    ifstream input_file(argv[1]);

    if(!input_file){
        cout<<"File could not be open"<<endl;
        exit(-1);
    }


    deque<Instruction> inst_queue;
    string input,tmp_str;
    getline(input_file,input);
    while(!input_file.eof()){
        
        stringstream ss(input);
        Instruction tmp_inst;
        ss>>tmp_inst.name;
        
        ss>>tmp_str;
        tmp_inst.dest_reg = tmp_str.substr(0,2);

        ss>>tmp_str;
        tmp_inst.source_reg1 = tmp_str.substr(0,2);

        ss>>tmp_str;
        tmp_inst.source_reg2 = tmp_str;

        tmp_inst.orig_dest = tmp_inst.dest_reg;

        inst_queue.push_back(tmp_inst);

        getline(input_file,input);
    }

    cout<<endl;

    Reservation_station RS;

    int cycle_count = 1;

    while(!inst_queue.empty() || !RS.is_empty()){
        cout<<"CYCLE : "<<cycle_count++<<endl;
        RS.process_and_broadcast();

        if(!inst_queue.empty()){
            if(RS.add_instruction(inst_queue.front())){
                inst_queue.pop_front();
            }
        }

        RF.show();

        cout<<endl;

        RAT.show();

        cout<<endl;

        RS.show();

        cout<<endl<<" ════════════════════════════════════════════════════════ "<<endl;

    }
    
}