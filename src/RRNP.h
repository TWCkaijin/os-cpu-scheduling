#ifndef RRNP_H
#define RRNP_H

#include <iostream>
#include <vector>
#include <iomanip>
#include <queue>
#include <chrono>
#include <pthread.h>
#include <mutex>
#include "type.h"

namespace RRNPNS{
    static std::mutex queue_mtx; 
    static std::mutex list_mtx; 
    struct push_args{
        chrono::high_resolution_clock::time_point &start_time;
        vector<process> *process_list;
        priority_queue<process, vector<process>, prioritycompare> *process_queue;
        push_args(vector<process> *l, priority_queue<process, vector<process>, prioritycompare> *q,
            chrono::high_resolution_clock::time_point &s) :
                process_list(l), process_queue(q), start_time(s) {}
    };
    void* push_work(void* args){
        push_args* arg = (push_args*)args;
        vector<process>* process_list = arg->process_list;
        chrono::high_resolution_clock::time_point start_time = arg->start_time;
        priority_queue<process, vector<process>, prioritycompare>* process_queue_ptr = arg->process_queue;
        while(true){
            for (auto it = process_list->begin(); it != process_list->end();) {
                if (it->arrival_time <= (chrono::high_resolution_clock::now()-start_time).count()) {
                    {
                        lock_guard<mutex> lock_q(queue_mtx);
                        lock_guard<mutex> lock_l(list_mtx);
                        process_queue_ptr->push(*it);
                        it = process_list->erase(it);
                    }
                    
                } else {
                    break;
                }
            }
            if (process_list->size() == 0){
                return NULL;
            }
        }
    }
    
    vector<vector<long long>> run_work(int time_quantum, SCargs<prioritycompare> args){
        time_quantum *= 1000 * 1000;
        priority_queue<process, vector<process>, prioritycompare> process_queue = args.process_queue;
        vector<vector<long long>> process_timeline;
        chrono::high_resolution_clock::time_point start_time = chrono::high_resolution_clock::now();
        vector<process> process_list = args.process_list;
        pthread_t tid;
        push_args tmp(&process_list, &process_queue ,start_time);
        pthread_create(&tid, NULL, push_work, (void*)&tmp);
        while(true){
            bool queue_is_empty;
            {
                std::lock_guard<std::mutex> lock_q(queue_mtx);
                queue_is_empty = process_queue.empty();
            }
            bool list_is_empty;
            {
                std::lock_guard<std::mutex> lock_l(list_mtx);
                list_is_empty = process_list.empty();
            }

            if (queue_is_empty && list_is_empty){
                //cout << "All processes completed." << endl << endl << endl << endl;
                break;
            }
            int tick = 0;
            if (!queue_is_empty){
                chrono::high_resolution_clock::time_point exe_start = chrono::high_resolution_clock::now();
                tick = time_quantum;
                process current_process = process_queue.top();
                process_queue.pop();
                int time_left = current_process.burst_time - current_process.execution_time;
                current_process.execution_time += tick;
                if (time_left <= tick){
                    tick = time_left;
                }else{
                    process_queue.push(current_process);
                }
                
                //cout << "Process " << current_process.identification << " is running for " << tick << "ms" << endl;
                process_timeline.push_back(vector<long long>{(exe_start - start_time).count(), current_process.identification, tick});
                while( (chrono::high_resolution_clock::now()- exe_start).count()  < tick);
            }
            
        }
        pthread_join(tid, NULL);
        return process_timeline;
    };

    
}


#endif