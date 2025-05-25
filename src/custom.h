#ifndef CUSTOM_H
#define CUSTOM_H

#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
#include "type.h"
#include "chrono"


namespace CUSTOMNS{
    static std::mutex queue_mtx; 
    static std::mutex q1_mtx; 
    static std::mutex q2_mtx; 
    static std::mutex q3_mtx; 
    static std::mutex list_mtx; 
    static std::mutex custom_rules_mtx;
    struct push_args{
        chrono::high_resolution_clock::time_point &start_time;
        vector<process> *process_list;
        priority_queue<process, vector<process>, prioritycompare> *process_queue;
        priority_queue<process, vector<process>, prioritycompare> *q1;
        priority_queue<process, vector<process>, prioritycompare> *q2;
        priority_queue<process, vector<process>, burstcompare> *q3;
        int *cr;
        push_args(vector<process> *l, 
            priority_queue<process, vector<process>, prioritycompare> *q,
            priority_queue<process, vector<process>, prioritycompare> *q1,
            priority_queue<process, vector<process>, prioritycompare> *q2,
            priority_queue<process, vector<process>, burstcompare> *q3,
            int *cr,
            chrono::high_resolution_clock::time_point &s) :
                process_list(l), 
                process_queue(q), 
                q1(q1),
                q2(q2),
                q3(q3),
                cr(cr),
                start_time(s) {}
    };
    void* push_work(void* args){
        push_args* arg = (push_args*)args;
        vector<process>* process_list = arg->process_list;
        chrono::high_resolution_clock::time_point start_time = arg->start_time;
        priority_queue<process, vector<process>, prioritycompare>* process_queue_ptr = arg->process_queue;
        priority_queue<process, vector<process>, prioritycompare>* q1 = arg->q1;
        priority_queue<process, vector<process>, prioritycompare>* q2 = arg->q2;
        priority_queue<process, vector<process>, burstcompare>* q3 = arg->q3;
        while(true){
            for (auto it = process_list->begin(); it != process_list->end();) {
                if (it->arrival_time <= (chrono::high_resolution_clock::now()-start_time).count()) {
                    lock_guard<mutex> lock_q(queue_mtx);
                    lock_guard<mutex> lock_l(list_mtx);
                    process_queue_ptr->push(*it);
                    it = process_list->erase(it);
                } else {
                    break;
                }
            }
            bool process_queue_is_empty;
            {
                std::lock_guard<std::mutex> lock_q(queue_mtx);
                process_queue_is_empty = process_queue_ptr->empty();
            }
            bool process_list_is_empty;
            {
                std::lock_guard<std::mutex> lock_l(list_mtx);
                process_list_is_empty = process_list->empty();
            }
            if (!process_queue_is_empty){
                lock_guard<mutex> lock_q(queue_mtx);
                vector<process> temp_vec = process_queue_ptr->__get_container();
                for (const auto& item : temp_vec){
                    if (item.priority ==1){
                        lock_guard<mutex> lock_q1(q1_mtx);
                        q1->push(item);
                    }
                    else if (item.priority ==2){
                        lock_guard<mutex> lock_q2(q2_mtx);
                        q2->push(item);
                    }
                    else if (item.priority ==3){
                        lock_guard<mutex> lock_q3(q3_mtx);
                        q3->push(item);
                    }
                }
                while(!process_queue_ptr->empty()){
                    process_queue_ptr->pop();
                }
            }
            
            if (process_list_is_empty){
                return NULL;
            }
            usleep(100);
        }
    }

    void* custom_rule(void* args){
        push_args* arg = (push_args*)args;
        vector<process>* process_list = arg->process_list;
        chrono::high_resolution_clock::time_point start_time = arg->start_time;
        priority_queue<process, vector<process>, prioritycompare>* process_queue_ptr = arg->process_queue;
        priority_queue<process, vector<process>, prioritycompare>* q1 = arg->q1;
        priority_queue<process, vector<process>, prioritycompare>* q2 = arg->q2;
        priority_queue<process, vector<process>, burstcompare>* q3 = arg->q3;
        int *crq = arg->cr;

        while(true){
            int q1_total = 0, q2_total = 0, q3_total = 0; 
            for (int i=0;i<q1->size();i++){
                q1_total += q1->top().burst_time;
            }
            for (int i=0;i<q2->size();i++){
                q2_total += q2->top().burst_time;
            }
            for (int i=0;i<q3->size();i++){
                q3_total += q3->top().burst_time;
            }

            if (q1_total !=0 && q1_total <= q2_total * 2 && q1_total <= q3_total * 2){
                lock_guard<std::mutex> lock_cr(custom_rules_mtx);
                *crq = 1;
            }else if (q2_total!=0 && q2_total  <= q3_total * 2){
                lock_guard<std::mutex> lock_cr(custom_rules_mtx);
                *crq = 2;
            }else if (q3_total!=0){
                lock_guard<std::mutex> lock_cr(custom_rules_mtx);
                *crq = 3;
            }else if(q1_total !=0 && q1_total <= q2_total * 2){
                lock_guard<std::mutex> lock_cr(custom_rules_mtx);
                *crq = 1;
            }else if (q2_total!=0){
                lock_guard<std::mutex> lock_cr(custom_rules_mtx);
                *crq = 2;
            }else{
                lock_guard<std::mutex> lock_cr(custom_rules_mtx);
                *crq = 1;
            }
            if (process_list->empty() && process_queue_ptr->empty() && q1->empty() && q2->empty() && q3->empty()){
                return NULL;
            }
            usleep(100);
        }
    }


    vector<vector<long long >> run_work(SCargs<prioritycompare>& args){
        chrono::high_resolution_clock::time_point start_time = chrono::high_resolution_clock::now();

        priority_queue<process, vector<process>, prioritycompare> &process_queue = args.process_queue;
        vector<process> &process_list = args.process_list;
        process current_process;
        vector<vector<long long>> process_timeline;
        priority_queue<process, vector<process>, prioritycompare> q1,q2;
        priority_queue<process, vector<process>, burstcompare> q3;
        int qid = 0;
        pthread_t tid, tid2;
        push_args tmp(&process_list, &process_queue, &q1, &q2, &q3, &qid, start_time);
        pthread_create(&tid, NULL, push_work, (void*)&tmp);
        pthread_create(&tid2, NULL, custom_rule, (void*)&tmp);

        while(true){
            bool q1_is_empty;
            {
                std::lock_guard<std::mutex> lock_q(queue_mtx);
                q1_is_empty = q1.empty();
            }
            bool q2_is_empty;
            {
                std::lock_guard<std::mutex> lock_q(queue_mtx);
                q2_is_empty = q2.empty();
            }
            bool q3_is_empty;
            {
                std::lock_guard<std::mutex> lock_q(queue_mtx);
                q3_is_empty = q3.empty();
            }
            bool process_queue_is_empty;
            {
                std::lock_guard<std::mutex> lock_q(queue_mtx);
                process_queue_is_empty = process_queue.empty();
            }
            bool list_is_empty;
            {
                std::lock_guard<std::mutex> lock_l(list_mtx);
                list_is_empty = process_list.empty();
            }
            
            if (q1_is_empty && q2_is_empty && q3_is_empty && list_is_empty){
                break;
            }
            
            process current_process;
            long long  tick = 0;
            int tmp_cr = 0;
            {
                lock_guard<std::mutex> lock_cr(custom_rules_mtx);
                tmp_cr = qid;
            }
            if (!q1_is_empty && tmp_cr == 1){
                lock_guard<std::mutex> lock_q(q1_mtx);
                chrono::high_resolution_clock::time_point exe_start = chrono::high_resolution_clock::now();
                tick=5*1000*1000;
                current_process = q1.top();
                q1.pop();
                int time_left = current_process.burst_time - current_process.execution_time;
                current_process.execution_time += tick;
                if (time_left <= tick){
                    tick = time_left;
                }else{
                    q1.push(current_process);
                }
                process_timeline.push_back(vector<long long>{(chrono::high_resolution_clock::now() - start_time).count(), current_process.identification, tick});
                while( (chrono::high_resolution_clock::now()- exe_start).count()  < tick);
                //cout << "sleeping in q1 for "<< current_process.identification <<" with "<< tick << endl; 
            }
            else if (!q2_is_empty && tmp_cr == 2){
                lock_guard<std::mutex> lock_q2(q2_mtx);
                chrono::high_resolution_clock::time_point exe_start = chrono::high_resolution_clock::now();
                tick=10*1000*1000;
                current_process = q2.top();
                q2.pop();
                int time_left = current_process.burst_time - current_process.execution_time;
                current_process.execution_time += tick;
                if (time_left <= tick){
                    tick = time_left;
                }else{
                    q2.push(current_process);
                }
                process_timeline.push_back(vector<long long>{(chrono::high_resolution_clock::now() - start_time).count(), current_process.identification, tick});
                while( (chrono::high_resolution_clock::now()- exe_start).count()  < tick);
                //cout << "sleeping in q2 for: " << current_process.identification <<" with "<< tick << endl; 
            }
            else if (!q3_is_empty && tmp_cr == 3){
                lock_guard<std::mutex> lock_q3(q3_mtx);
                chrono::high_resolution_clock::time_point exe_start = chrono::high_resolution_clock::now();
                current_process = q3.top();
                tick=current_process.burst_time - current_process.execution_time;
                q3.pop();
                current_process.execution_time += tick;
                process_timeline.push_back(vector<long long>{(chrono::high_resolution_clock::now() - start_time).count(), current_process.identification, tick});
                while( (chrono::high_resolution_clock::now()- exe_start).count()  < tick);
                //cout << "sleeping in q3 for: " << current_process.identification <<" with "<< tick << endl; 
            }
            
        }
        pthread_join(tid, NULL);
        pthread_join(tid2, NULL);
        
        return process_timeline;
    }
}
#endif