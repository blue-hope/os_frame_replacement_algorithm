//
//  main.cpp
//  os3
//
//  Created by bluehope on 26/03/2019.
//  Copyright © 2019 bluehope. All rights reserved.
//

#include <iostream>
#include <sys/time.h>
#include <cstring>
#include <cstdio>
#include <cmath>
using namespace std;

#define reference_byte_size 8
#define time_interval 8
#define phy_mem_size 32
#define page_table_size 64
#define INF 99999999

class PhysicalMemory{
    private:
        int* phy_mem;
        int phy_mem_available;
        string phy_mem_str;
        int* arr_for_FIFO;
        int* arr_for_LRU;
        int** arr_for_Sampled_LRU;
        int* arr_for_LFU;
        int* arr_for_MFU;
        int* arr_for_Optimal_pid;
        int* arr_for_Optimal_func;
        int* arr_for_Optimal_alloc_id;
        int* arr_for_Optimal_demand_pg;

    public:
        PhysicalMemory(int arg_process_num, int arg_command_num);
        ~PhysicalMemory();
        bool is_hit(int arg_alloc_id);
        string formatting_procedure(void);
        bool is_replace_needed(int arg_demand_pg_binary);
        int is_best_fit(int arg_demand_pg_binary, int arg_alloc_index);
        bool phy_mem_alloc(int arg_demand_pg_binary, int arg_alloc_id);
        int* pra_FIFO(int arg_demand_pg_binary, int arg_alloc_id);
        int* pra_LRU(int arg_demand_pg_binary, int arg_alloc_id);
        void shift_reference_byte(int arg_process_num);
        void free_reference_byte(int arg_alloc_id);
        int calculate_reference_byte(int arg_alloc_id);
        int* pra_Sampled_LRU(int arg_demand_pg_binary, int arg_alloc_id);
        int* pra_LFU(int arg_demand_pg_binary, int arg_alloc_id);
        int* pra_MFU(int arg_demand_pg_binary, int arg_alloc_id);
        void alloc_access_sequence(int arg_pid, int arg_func, int arg_alloc_id, int arg_demand_pg_binary);
        int* get_argu(int arg_i);
        int* pra_Optimal(int arg_i, int arg_command_num);
}; 
class PageTable{
    private:
        int* page_table;
        int* valid_flag;
        int page_table_available;
        string page_table_str;
        string valid_flag_str;
    
    public:
        PageTable();
        ~PageTable();
        string* formatting_procedure(void);
        bool is_valid_alloc_id_in_table(int arg_index, int arg_alloc_id);
        void page_table_alloc(int arg_demand_pg, int arg_alloc_id);
        void change_valid_frame(int arg_alloc_id);
};

//PhysicalMemory methods---------------------------------------------------------------------------------------------------------------------------------------
PhysicalMemory::PhysicalMemory(int arg_process_num, int arg_command_num){
    phy_mem_str = "";
    phy_mem = new int[phy_mem_size];
    arr_for_FIFO = new int[phy_mem_size];
    arr_for_LRU = new int[phy_mem_size];
    arr_for_Sampled_LRU = new int*[arg_process_num * page_table_size];
    arr_for_LFU = new int[arg_process_num * page_table_size];
    arr_for_MFU = new int[arg_process_num * page_table_size];
    arr_for_Optimal_pid = new int[arg_command_num];
    arr_for_Optimal_func = new int[arg_command_num];
    arr_for_Optimal_alloc_id = new int[arg_command_num];
    arr_for_Optimal_demand_pg = new int[arg_command_num];
    for(int i = 0; i < phy_mem_size; i++){
        phy_mem[i] = -1;
        arr_for_FIFO[i] = -1;
        arr_for_LRU[i] = -1;
    }
    for(int i = 0; i < arg_process_num * page_table_size; i++){
        arr_for_Sampled_LRU[i] = new int[reference_byte_size + 1];
        for(int j = 0; j < reference_byte_size + 1; j++){
            arr_for_Sampled_LRU[i][j] = 0;
        }
        arr_for_LFU[i] = 0;
        arr_for_MFU[i] = 0;
    }
    for(int i = 0; i < arg_command_num; i++){
        arr_for_Optimal_pid[i] = -1;
        arr_for_Optimal_func[i] = -1;
        arr_for_Optimal_alloc_id[i] = -1;
        arr_for_Optimal_demand_pg[i] = -1;
    }
    phy_mem_available = phy_mem_size;
}
PhysicalMemory::~PhysicalMemory(){
    delete[] phy_mem;
    delete[] arr_for_FIFO;
    delete[] arr_for_LRU;
    delete[] arr_for_Sampled_LRU;
    delete[] arr_for_LFU;
    delete[] arr_for_MFU;
    delete[] arr_for_Optimal_pid;
    delete[] arr_for_Optimal_func;
    delete[] arr_for_Optimal_alloc_id;
    delete[] arr_for_Optimal_demand_pg;
}
bool PhysicalMemory::is_hit(int arg_alloc_id){
    bool is_hit = false;
    for(int i = 0; i < phy_mem_size; i++){
        if(phy_mem[i] == arg_alloc_id){//hit
            arr_for_LRU[i] = 0;
            is_hit = true;
        } else {
            if(arr_for_LRU[i] != -1){
                arr_for_LRU[i]++;
            }
        }
    }
    arr_for_LFU[arg_alloc_id - 1]++;
    arr_for_MFU[arg_alloc_id - 1]++;
    arr_for_Sampled_LRU[arg_alloc_id - 1][0] = 1;
    return is_hit;
}
string PhysicalMemory::formatting_procedure(void){
    phy_mem_str = "|";
    for(int i = 0; i < phy_mem_size; i++){
        if(phy_mem[i] == -1){
            phy_mem_str += "-";
        }
        else{
            phy_mem_str += to_string(phy_mem[i]);
        }
        if(i % 4 == 3){
            phy_mem_str += "|";
        }
    }
    return phy_mem_str;
}
bool PhysicalMemory::is_replace_needed(int arg_demand_pg_binary){
    return phy_mem_available < arg_demand_pg_binary;
}
int PhysicalMemory::is_best_fit(int arg_demand_pg_binary, int arg_alloc_index){
    int fit = 1;
    while(arg_demand_pg_binary * pow(2, fit - 1) < phy_mem_size){
        int operand_xor = arg_demand_pg_binary * pow(2, fit - 1);
        int operand_and = phy_mem_size - operand_xor;
        int start_index = (arg_alloc_index ^ operand_xor) & operand_and;
        int end_index = start_index + arg_demand_pg_binary * pow(2, fit - 1);
        for(int i = start_index; i < end_index; i++){
            if(phy_mem[i] != -1){
                return fit;
            } else {
                fit++;
            }
        }
    }
    return fit;
}
bool PhysicalMemory::phy_mem_alloc(int arg_demand_pg_binary, int arg_alloc_id){
    int alloc_index;
    int tmp_alloc_index;
    bool phy_mem_alloc_avail = false;
    int fit = INF;
    for(int i = 0; i < phy_mem_size / arg_demand_pg_binary; i++){
        if(phy_mem[i * arg_demand_pg_binary] == -1){
            tmp_alloc_index = i * arg_demand_pg_binary;
            bool full = false;
            for(int i = tmp_alloc_index; i < tmp_alloc_index + arg_demand_pg_binary; i++){
                if(phy_mem[i] != -1){
                    full = true;
                    break;
                }
            }
            if(!full){
                phy_mem_alloc_avail = true;
                int tmp_fit = this->is_best_fit(arg_demand_pg_binary, tmp_alloc_index);
                if(tmp_fit < fit){
                    fit = tmp_fit;
                    alloc_index = tmp_alloc_index;
                } 
                else {
                    continue;
                }
            }
        }
    }
    if(!phy_mem_alloc_avail){
        return false;
    } else {
        for(int i = 0; i < phy_mem_size; i++){//for FIFO && LRU
            if(arr_for_FIFO[i] != -1){
                arr_for_FIFO[i] += 1;
            }
            if(arr_for_LRU[i] != -1){
                arr_for_LRU[i] += 1;
            }
        }
        for(int i = alloc_index; i < alloc_index + arg_demand_pg_binary; i++){
            phy_mem[i] = arg_alloc_id;
            arr_for_FIFO[i] = 0;
            arr_for_LRU[i] = 0;
        }
        phy_mem_available -= arg_demand_pg_binary;

        return true;
    }
}
int* PhysicalMemory::pra_FIFO(int arg_demand_pg_binary, int arg_alloc_id){
    int oldest;
    int oldest_alloc_id;
    int cnt = 0;
    int* replace_alloc_id = new int[phy_mem_size];
    for(int i = 0; i < phy_mem_size; i++){
        replace_alloc_id[i] = -1;
    }
    
    while(true){
        oldest = -1;
        oldest_alloc_id = -1;
        for(int i = 0; i < phy_mem_size; i++){
            if(oldest < arr_for_FIFO[i]){
                oldest = arr_for_FIFO[i];
                oldest_alloc_id = phy_mem[i];
            }
            else if(oldest == arr_for_FIFO[i]){
                if(phy_mem[i] < oldest_alloc_id){
                    oldest_alloc_id = phy_mem[i];
                }
            }
        }
        replace_alloc_id[cnt] = oldest_alloc_id;
        cnt++;
        for(int i = 0; i < phy_mem_size; i++){
            if(oldest_alloc_id == phy_mem[i]){
                phy_mem[i] = -1;
                arr_for_FIFO[i] = -1;
                phy_mem_available++;
            }
        }
        if(this->phy_mem_alloc(arg_demand_pg_binary, arg_alloc_id)){
            //frame alloc
            break;
        }
    }

    return replace_alloc_id;
}
int* PhysicalMemory::pra_LRU(int arg_demand_pg_binary, int arg_alloc_id){
    int oldest;
    int oldest_alloc_id; 
    int cnt = 0;
    int* replace_alloc_id = new int[phy_mem_size];
    for(int i = 0; i < phy_mem_size; i++){
        replace_alloc_id[i] = -1;
    }
    
    while(true){
        oldest = -1;
        oldest_alloc_id = -1;
        for(int i = 0; i < phy_mem_size; i++){
            if(oldest < arr_for_LRU[i]){
                oldest = arr_for_LRU[i];
                oldest_alloc_id = phy_mem[i];
            }
            else if(oldest == arr_for_LRU[i]){
                if(phy_mem[i] < oldest_alloc_id){
                    oldest_alloc_id = phy_mem[i];
                }
            }
        }
        replace_alloc_id[cnt] = oldest_alloc_id;
        cnt++;
        for(int i = 0; i < phy_mem_size; i++){
            if(oldest_alloc_id == phy_mem[i]){
                phy_mem[i] = -1;
                arr_for_LRU[i] = -1;
                phy_mem_available++;
            }
        }
        if(this->phy_mem_alloc(arg_demand_pg_binary, arg_alloc_id)){
            //frame alloc
            break;
        }
    }

    return replace_alloc_id;
}
void PhysicalMemory::shift_reference_byte(int arg_process_num){
    for(int i = 0; i < arg_process_num * page_table_size; i++){
        for(int j = reference_byte_size; j > 0; j--){
            arr_for_Sampled_LRU[i][j] = arr_for_Sampled_LRU[i][j - 1];
        }
        arr_for_Sampled_LRU[i][0] = 0;
    }
}
void PhysicalMemory::free_reference_byte(int arg_alloc_id){
    for(int i = 0; i < reference_byte_size + 1; i++){
        arr_for_Sampled_LRU[arg_alloc_id - 1][i] = 0;
    }
}
int PhysicalMemory::calculate_reference_byte(int arg_alloc_id){
    int sum = 0;
    for(int i = 1; i < reference_byte_size + 1; i++){
        sum += pow(2, 8 - i) * arr_for_Sampled_LRU[arg_alloc_id - 1][i];
    }
    return sum;
}
int* PhysicalMemory::pra_Sampled_LRU(int arg_demand_pg_binary, int arg_alloc_id){
    int fewest;
    int fewest_alloc_id;
    int cnt = 0;
    int tmp_cnt;
    int* tmp_phy_mem_assigned = new int[phy_mem_size];
    int* replace_alloc_id = new int[phy_mem_size];
    for(int i = 0; i < phy_mem_size; i++){
        replace_alloc_id[i] = -1;
    }
    
    while(true){
        tmp_cnt = 0;
        for(int i = 0; i < phy_mem_size; i++){
            tmp_phy_mem_assigned[i] = -1;
        }
        for(int i = 0; i < phy_mem_size; i++){
            if(phy_mem[i] != -1){
                tmp_phy_mem_assigned[0] = phy_mem[i];
                break;
            }
        }
        for(int i = 0; i < phy_mem_size; i++){
            if(phy_mem[i] != -1){
                if(tmp_phy_mem_assigned[tmp_cnt] == phy_mem[i]){
                    continue;
                } else {
                    tmp_phy_mem_assigned[tmp_cnt + 1] = phy_mem[i];
                    tmp_cnt++;
                }
            }
        }
        tmp_cnt++;

        fewest = this->calculate_reference_byte(tmp_phy_mem_assigned[0]);
        fewest_alloc_id = tmp_phy_mem_assigned[0];

        for(int i = 0; i < tmp_cnt; i++){
            if(this->calculate_reference_byte(tmp_phy_mem_assigned[i]) < fewest){
                fewest = this->calculate_reference_byte(tmp_phy_mem_assigned[i]);
                fewest_alloc_id = tmp_phy_mem_assigned[i];
            }
            else if(this->calculate_reference_byte(tmp_phy_mem_assigned[i]) == fewest){
                if(tmp_phy_mem_assigned[i] < fewest_alloc_id){
                    fewest_alloc_id = tmp_phy_mem_assigned[i];
                }
            }
        }
        replace_alloc_id[cnt] = fewest_alloc_id;
        cnt++;

        for(int i = 0; i < phy_mem_size; i++){
            if(fewest_alloc_id == phy_mem[i]){
                phy_mem[i] = -1;
                phy_mem_available++;
            }
        }
        free_reference_byte(fewest_alloc_id);
        if(this->phy_mem_alloc(arg_demand_pg_binary, arg_alloc_id)){
            //frame alloc
            break;
        }
    }
    delete[] tmp_phy_mem_assigned;
    return replace_alloc_id;
}
int* PhysicalMemory::pra_LFU(int arg_demand_pg_binary, int arg_alloc_id){
    int fewest;
    int fewest_alloc_id;
    int cnt = 0;
    int tmp_cnt;
    int* tmp_phy_mem_assigned = new int[phy_mem_size];
    int* replace_alloc_id = new int[phy_mem_size];
    for(int i = 0; i < phy_mem_size; i++){
        replace_alloc_id[i] = -1;
    }
    
    while(true){
        tmp_cnt = 0;
        for(int i = 0; i < phy_mem_size; i++){
            tmp_phy_mem_assigned[i] = -1;
        }
        for(int i = 0; i < phy_mem_size; i++){
            if(phy_mem[i] != -1){
                tmp_phy_mem_assigned[0] = phy_mem[i];
                break;
            }
        }
        for(int i = 0; i < phy_mem_size; i++){
            if(phy_mem[i] != -1){
                if(tmp_phy_mem_assigned[tmp_cnt] == phy_mem[i]){
                    continue;
                } else {
                    tmp_phy_mem_assigned[tmp_cnt + 1] = phy_mem[i];
                    tmp_cnt++;
                }
            }
        }
        tmp_cnt++;

        fewest = arr_for_LFU[tmp_phy_mem_assigned[0]-1];
        fewest_alloc_id = tmp_phy_mem_assigned[0];

        for(int i = 0; i < tmp_cnt; i++){
            if(arr_for_LFU[tmp_phy_mem_assigned[i]-1] < fewest){
                fewest = arr_for_LFU[tmp_phy_mem_assigned[i]-1];
                fewest_alloc_id = tmp_phy_mem_assigned[i];
            }
            else if(arr_for_LFU[tmp_phy_mem_assigned[i]-1] == fewest){
                if(tmp_phy_mem_assigned[i] < fewest_alloc_id){
                    fewest_alloc_id = tmp_phy_mem_assigned[i];
                }
            }
        }
        replace_alloc_id[cnt] = fewest_alloc_id;
        cnt++;

        for(int i = 0; i < phy_mem_size; i++){
            if(fewest_alloc_id == phy_mem[i]){
                phy_mem[i] = -1;
                phy_mem_available++;
            }
        }
        arr_for_LFU[fewest_alloc_id - 1] = 0;
        if(this->phy_mem_alloc(arg_demand_pg_binary, arg_alloc_id)){
            //frame alloc
            break;
        }
    }
    delete[] tmp_phy_mem_assigned;
    return replace_alloc_id;
}
int* PhysicalMemory::pra_MFU(int arg_demand_pg_binary, int arg_alloc_id){
    int frequent;
    int frequent_alloc_id;
    int cnt = 0;
    int tmp_cnt;
    int* tmp_phy_mem_assigned = new int[phy_mem_size];
    int* replace_alloc_id = new int[phy_mem_size];
    for(int i = 0; i < phy_mem_size; i++){
        replace_alloc_id[i] = -1;
    }
    
    while(true){
        tmp_cnt = 0;
        for(int i = 0; i < phy_mem_size; i++){
            tmp_phy_mem_assigned[i] = -2;
        }
        for(int i = 0; i < phy_mem_size; i++){
            if(phy_mem[i] != -1){
                tmp_phy_mem_assigned[0] = phy_mem[i];
                break;
            }
        }
        for(int i = 0; i < phy_mem_size; i++){
            if(phy_mem[i] != -1){
                if(tmp_phy_mem_assigned[tmp_cnt] == phy_mem[i]){
                    continue;
                } else {
                    tmp_phy_mem_assigned[tmp_cnt + 1] = phy_mem[i];
                    tmp_cnt++;
                }
            }
        }
        tmp_cnt++;

        frequent = arr_for_MFU[tmp_phy_mem_assigned[0]-1];
        frequent_alloc_id = tmp_phy_mem_assigned[0];

        for(int i = 0; i < tmp_cnt; i++){
            if(frequent < arr_for_MFU[tmp_phy_mem_assigned[i]-1]){
                frequent = arr_for_MFU[tmp_phy_mem_assigned[i]-1];
                frequent_alloc_id = tmp_phy_mem_assigned[i];
            }
            else if(frequent == arr_for_MFU[tmp_phy_mem_assigned[i]-1]){
                if(tmp_phy_mem_assigned[i] < frequent_alloc_id){
                    frequent_alloc_id = tmp_phy_mem_assigned[i];
                }
            }
        }
        replace_alloc_id[cnt] = frequent_alloc_id;
        cnt++;

        for(int i = 0; i < phy_mem_size; i++){
            if(frequent_alloc_id == phy_mem[i]){
                phy_mem[i] = -1;
                phy_mem_available++;
            }
        }
        arr_for_MFU[frequent_alloc_id - 1] = 0;
        if(this->phy_mem_alloc(arg_demand_pg_binary, arg_alloc_id)){
            //frame alloc
            break;
        }
    }

    return replace_alloc_id;
}
void PhysicalMemory::alloc_access_sequence(int arg_pid, int arg_func, int arg_alloc_id, int arg_demand_pg_binary){
    int i = 0;
    while(true){
        if(arr_for_Optimal_alloc_id[i] == -1){
            arr_for_Optimal_pid[i] = arg_pid;
            arr_for_Optimal_func[i] = arg_func;
            arr_for_Optimal_alloc_id[i] = arg_alloc_id;
            arr_for_Optimal_demand_pg[i] = arg_demand_pg_binary;
            break;
        }
        i++;
    }
}
int* PhysicalMemory::get_argu(int arg_i){
    int* argu = new int[4];
    argu[0] = arr_for_Optimal_pid[arg_i];
    argu[1] = arr_for_Optimal_func[arg_i];
    argu[2] = arr_for_Optimal_alloc_id[arg_i];
    argu[3] = arr_for_Optimal_demand_pg[arg_i];
    return argu;
}
int* PhysicalMemory::pra_Optimal(int arg_i, int arg_command_num){
    int target_left;
    int target_alloc_id;
    int cnt = 0;
    int tmp_cnt;
    int* tmp_phy_mem_assigned = new int[phy_mem_size];
    int* replace_alloc_id = new int[phy_mem_size];
    for(int i = 0; i < phy_mem_size; i++){
        replace_alloc_id[i] = -1;
    }

    while(true){
        tmp_cnt = 0;
        for(int i = 0; i < phy_mem_size; i++){
            tmp_phy_mem_assigned[i] = -1;
        }
        for(int i = 0; i < phy_mem_size; i++){
            if(phy_mem[i] != -1){
                tmp_phy_mem_assigned[0] = phy_mem[i];
                break;
            }
        }
        for(int i = 0; i < phy_mem_size; i++){
            if(phy_mem[i] != -1){
                if(tmp_phy_mem_assigned[tmp_cnt] == phy_mem[i]){
                    continue;
                } else {
                    tmp_phy_mem_assigned[tmp_cnt + 1] = phy_mem[i];
                    tmp_cnt++;
                }
            }
        }
        tmp_cnt++;
        int* tmp_phy_mem_assigned_left = new int[tmp_cnt];

        for(int i = 0; i < tmp_cnt; i++){
            tmp_phy_mem_assigned_left[i] = INF;
        }

        for(int i = 0; i < tmp_cnt; i++){
            int j = arg_i + 1;
            while(j < arg_command_num){
                if(arr_for_Optimal_alloc_id[j] == tmp_phy_mem_assigned[i]){
                    tmp_phy_mem_assigned_left[i] = j - arg_i;
                    break;
                }
                j++;
            }
        }

        //find target
        target_left = -1;
        target_alloc_id = -1;
        for(int i = 0; i < tmp_cnt; i++){
            if(target_left < tmp_phy_mem_assigned_left[i]){
                target_left = tmp_phy_mem_assigned_left[i];
                target_alloc_id = tmp_phy_mem_assigned[i];
            }
            else if(target_left == tmp_phy_mem_assigned_left[i]){
                if(tmp_phy_mem_assigned[i] < target_alloc_id){
                    target_alloc_id = tmp_phy_mem_assigned[i];
                }
            }
        }
        replace_alloc_id[cnt] = target_alloc_id;
        cnt++;

        for(int i = 0; i < phy_mem_size; i++){
            if(target_alloc_id == phy_mem[i]){
                phy_mem[i] = -1;
                phy_mem_available++;
            }
        }
        int demand_pg_binary = 1;
        for(int j = 0; j < 5; j++){
            if(pow(2, j) < arr_for_Optimal_demand_pg[arg_i] && arr_for_Optimal_demand_pg[arg_i] <= pow(2, j+1)){
                demand_pg_binary = pow(2, j+1);
                break;
            }
        }
        if(this->phy_mem_alloc(demand_pg_binary, arr_for_Optimal_alloc_id[arg_i])){
            //frame alloc
            break;
        }
        delete[] tmp_phy_mem_assigned_left;
    }
    delete[] tmp_phy_mem_assigned;
    return replace_alloc_id;
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//PageTable methods--------------------------------------------------------------------------------------------------------------------------------------------
PageTable::PageTable(){
    page_table_str = "";
    page_table = new int[page_table_size];
    valid_flag = new int[page_table_size];
    for(int i = 0; i < page_table_size; i++){
        page_table[i] = -1;
        valid_flag[i] = -1;
    }
    page_table_available = page_table_size;
}
PageTable::~PageTable(){
    delete[] page_table;
    delete[] valid_flag;
}
string* PageTable::formatting_procedure(void){
    page_table_str = "|";
    valid_flag_str = "|";
    
    for(int i = 0; i < page_table_size; i++){
        if(page_table[i] == -1){
            page_table_str += "-";
        }
        else{
            page_table_str += to_string(page_table[i]);
        }
        if(i % 4 == 3){
            page_table_str += "|";
        }
    }

    for(int i = 0; i < page_table_size; i++){
        if(valid_flag[i] == -1){
            valid_flag_str += "-";
        }
        else{
            valid_flag_str += to_string(valid_flag[i]);
        }
        if(i % 4 == 3){
            valid_flag_str += "|";
        }
    }
    
    string* str = new string[2];
    str[0] = page_table_str;
    str[1] = valid_flag_str;
    return str;
}
bool PageTable::is_valid_alloc_id_in_table(int arg_index, int arg_alloc_id){
    return page_table[arg_index] == arg_alloc_id;
}
void PageTable::page_table_alloc(int arg_demand_pg, int arg_alloc_id){
    for(int i = page_table_size - page_table_available; i < page_table_size - page_table_available + arg_demand_pg; i++){
        //allocate
        page_table[i] = arg_alloc_id;
        valid_flag[i] = 0;
    }
    page_table_available -= arg_demand_pg;
}
void PageTable::change_valid_frame(int arg_alloc_id){
    for(int i = 0; i < page_table_size; i++){
        if(page_table[i] == arg_alloc_id){
            valid_flag[i] ^= 1;
        }
    }
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------



//functions----------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------

int main(int argc, const char * argv[]) {
    //get init line
    string str;

    getline(cin, str);
    int algorithm_type = stoi(str);

    getline(cin, str);
    int process_num = stoi(str);

    getline(cin, str);
    int command_num = stoi(str);
    int command_access_cnt = 0;
    int command_cnt = 0;

    //valiable init
    PhysicalMemory phy_mem(process_num, command_num);
    PageTable* page_table = new PageTable[process_num];
    int page_fault = 0;

    //iterating command
    for(int i = 0; i < command_num; i++){
        getline(cin, str);
        int pid = stoi(str.substr(0, str.find("\t")));
        str = str.substr(str.find("\t") + 1);
        int func = stoi(str.substr(0, str.find("\t")));
        str = str.substr(str.find("\t") + 1);
        int alloc_id = stoi(str.substr(0, str.find("\t")));
        str = str.substr(str.find("\t") + 1);
        int demand_pg;
        if(func == 1){
            demand_pg = stoi(str.substr(0, str.find("\t")));
        } else {
            demand_pg = 0;
            for(int j = 0; j < page_table_size; j++){
                if(page_table[pid].is_valid_alloc_id_in_table(j, alloc_id)){
                    demand_pg++;
                }
            }
        }
        
        string func_str = "";
        if(func == 1){
            func_str = "ALLOCATION";
        } else {
            func_str = "ACCESS";
        }


        //process with new command
        if(func == 1){//allocation
            page_table[pid].page_table_alloc(demand_pg, alloc_id);
        } else {//access
            int demand_pg_binary = 1;
            for(int j = 0; j < 5; j++){
                if(pow(2, j) < demand_pg && demand_pg <= pow(2, j+1)){
                    demand_pg_binary = pow(2, j+1);
                    break;
                }
            }

            if(algorithm_type == 5){
                phy_mem.alloc_access_sequence(pid, func, alloc_id, demand_pg);
            }
            
            if(!phy_mem.is_hit(alloc_id)){
                if(!phy_mem.is_replace_needed(demand_pg_binary)){//no replace
                    if(algorithm_type != 5){
                        phy_mem.phy_mem_alloc(demand_pg_binary, alloc_id);
                        page_table[pid].change_valid_frame(alloc_id);
                    }
                } else {//need replace
                    if(algorithm_type == 0){
                        int* replaced_alloc_id = phy_mem.pra_FIFO(demand_pg_binary, alloc_id);
                        for(int j = 0; j < process_num; j++){
                            for(int k = 0; k < phy_mem_size; k++){
                                if(replaced_alloc_id[k] != -1){
                                    page_table[j].change_valid_frame(replaced_alloc_id[k]);
                                }
                            }
                        }
                        delete[] replaced_alloc_id;
                        page_table[pid].change_valid_frame(alloc_id);
                    } else if(algorithm_type == 1){
                        int* replaced_alloc_id = phy_mem.pra_LRU(demand_pg_binary, alloc_id);
                        for(int j = 0; j < process_num; j++){
                            for(int k = 0; k < phy_mem_size; k++){
                                if(replaced_alloc_id[k] != -1){
                                    page_table[j].change_valid_frame(replaced_alloc_id[k]);
                                }
                            }
                        }
                        page_table[pid].change_valid_frame(alloc_id);
                    } else if(algorithm_type == 2){
                        int* replaced_alloc_id = phy_mem.pra_Sampled_LRU(demand_pg_binary, alloc_id);
                        for(int j = 0; j < process_num; j++){
                            for(int k = 0; k < phy_mem_size; k++){
                                if(replaced_alloc_id[k] != -1){
                                    page_table[j].change_valid_frame(replaced_alloc_id[k]);
                                }
                            }
                        }
                        page_table[pid].change_valid_frame(alloc_id);
                    } else if(algorithm_type == 3){
                        int* replaced_alloc_id = phy_mem.pra_LFU(demand_pg_binary, alloc_id);
                        for(int j = 0; j < process_num; j++){
                            for(int k = 0; k < phy_mem_size; k++){
                                if(replaced_alloc_id[k] != -1){
                                    page_table[j].change_valid_frame(replaced_alloc_id[k]);
                                }
                            }
                        }
                        page_table[pid].change_valid_frame(alloc_id);
                    } else if(algorithm_type == 4){
                        int* replaced_alloc_id = phy_mem.pra_MFU(demand_pg_binary, alloc_id);
                        for(int j = 0; j < process_num; j++){
                            for(int k = 0; k < phy_mem_size; k++){
                                if(replaced_alloc_id[k] != -1){
                                    page_table[j].change_valid_frame(replaced_alloc_id[k]);
                                }
                            }
                        }
                        page_table[pid].change_valid_frame(alloc_id);
                    } else if(algorithm_type == 5){
                    }                    
                }
                if(algorithm_type != 5){
                    page_fault += 1;
                }
            }
            if(algorithm_type == 2){
                if(command_cnt % time_interval ==  time_interval - 1){
                    phy_mem.shift_reference_byte(process_num);
                }
            }
        }


        //formatting
        if(algorithm_type != 5 || func == 1){
            if(i != 0){
                printf("\n");
            }
            printf("* Input : Pid [%d] Function [%s] Alloc ID [%d] Page Num[%d]\n", pid, func_str.c_str(), alloc_id, demand_pg);

            string phy_mem_str = phy_mem.formatting_procedure();
            string* page_table_str = new string[process_num];
            string* valid_flag_str = new string[process_num];
            for(int j = 0; j < process_num; j++){
                string* tmp_str = page_table[j].formatting_procedure();
                page_table_str[j] = tmp_str[0];
                valid_flag_str[j] = tmp_str[1];
                delete[] tmp_str;
            }

            //output
            printf("%-30s", ">> Physical Memory : "); printf("%s", phy_mem_str.c_str());
            printf("\n");

            for(int j = 0; j < process_num; j++){
                printf(">> pid(%d) %-20s",j, "Page Table(AID) : "); printf("%s", page_table_str[j].c_str());
                printf("\n");
                printf(">> pid(%d) %-20s",j, "Page Table(Valid) : "); printf("%s", valid_flag_str[j].c_str());
                printf("\n");
            }

            delete[] page_table_str;
            delete[] valid_flag_str;
        }


        if(func == 0){
            command_access_cnt++;
        }
        command_cnt++;
    }

    if(algorithm_type == 5){//optimal
        for(int i = 0; i < command_access_cnt; i++){
            int* argu = phy_mem.get_argu(i);
        }
        for(int i = 0; i < command_access_cnt; i++){
            int* argu = phy_mem.get_argu(i);
            int demand_pg_binary = 1;
            for(int j = 0; j < 5; j++){
                if(pow(2, j) < argu[3] && argu[3] <= pow(2, j+1)){
                    demand_pg_binary = pow(2, j+1);
                    break;
                }
            }
            if(!phy_mem.is_hit(argu[2])){
                if(!phy_mem.is_replace_needed(demand_pg_binary)){//no replace
                    phy_mem.phy_mem_alloc(demand_pg_binary, argu[2]);
                    page_table[argu[0]].change_valid_frame(argu[2]);
                } else {//need replace
                    int* replaced_alloc_id = phy_mem.pra_Optimal(i, command_access_cnt);
                    for(int j = 0; j < process_num; j++){
                        for(int k = 0; k < phy_mem_size; k++){
                            if(replaced_alloc_id[k] != -1){
                                page_table[j].change_valid_frame(replaced_alloc_id[k]);
                            }
                        }
                    }
                    page_table[argu[0]].change_valid_frame(argu[2]);
                    delete[] replaced_alloc_id;

                }
                page_fault++;
            }

            //formatting
            printf("\n");
            string func_str = "";
            if(argu[1] == 1){
                func_str = "ALLOCATION";
            } else {
                func_str = "ACCESS";
            }
            printf("* Input : Pid [%d] Function [%s] Alloc ID [%d] Page Num[%d]\n", argu[0], func_str.c_str(), argu[2], argu[3]);
            
            delete[] argu;

            string phy_mem_str = phy_mem.formatting_procedure();
            string* page_table_str = new string[process_num];
            string* valid_flag_str = new string[process_num];
            for(int j = 0; j < process_num; j++){
                string* tmp_str = page_table[j].formatting_procedure();
                page_table_str[j] = tmp_str[0];
                valid_flag_str[j] = tmp_str[1];
                delete[] tmp_str;
            }

            //output
            printf("%-30s", ">> Physical Memory : "); printf("%s", phy_mem_str.c_str());
            printf("\n");

            for(int j = 0; j < process_num; j++){
                printf(">> pid(%d) %-20s",j, "Page Table(AID) : "); printf("%s", page_table_str[j].c_str());
                printf("\n");
                printf(">> pid(%d) %-20s",j, "Page Table(Valid) : "); printf("%s", valid_flag_str[j].c_str());
                printf("\n");
            }

            delete[] page_table_str;
            delete[] valid_flag_str;
        }
    }
    printf("\n");
    printf("page fault = %d\n", page_fault);

    delete[] page_table;
}

