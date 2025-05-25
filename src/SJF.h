#ifndef SJF_CPP
#define SJF_CPP

#include <iostream>
#include <vector>
#include <iomanip>
#include <queue>
#include <chrono>
#include <pthread.h>
#include <mutex>
#include "type.h"
using namespace std;

namespace SJFNS{
    static std::mutex queue_mtx; 
    static std::mutex list_mtx; 
    struct push_args{
        chrono::high_resolution_clock::time_point &start_time;
        vector<process> *process_list;
        priority_queue<process, vector<process>, burstcompare> *process_queue;
        push_args(vector<process> *l, priority_queue<process, vector<process>, burstcompare> *q,
            chrono::high_resolution_clock::time_point &s) :
                process_list(l), process_queue(q), start_time(s) {}
    };
    void* push_work(void* args){
        push_args* arg = (push_args*)args;
        vector<process>* process_list = arg->process_list;
        chrono::high_resolution_clock::time_point start_time = arg->start_time;
        priority_queue<process, vector<process>, burstcompare>* process_queue_ptr = arg->process_queue;
        while(true){
            for (auto it = process_list->begin(); it != process_list->end();) {
                if (it->arrival_time  <= (chrono::high_resolution_clock::now()-start_time).count()) {
                    {   
                        std::lock_guard<std::mutex> lock_q(queue_mtx);
                        std::lock_guard<std::mutex> lock_l(list_mtx);
                        process_queue_ptr->push(*it);
                        it = process_list->erase(it);
                    }
                    
                } else {
                    break;
                }
            }
            bool list_is_empty;
            {
                std::lock_guard<std::mutex> lock_l(list_mtx);
                list_is_empty = process_list->empty();
            }
            if (list_is_empty){
                return NULL;
            }
            usleep(5000);
        }
    }
    vector<vector<long long>> run_work(SCargs<burstcompare> args){
        priority_queue<process, vector<process>, burstcompare> process_queue = args.process_queue;
        vector<process> process_list = args.process_list;
        chrono::high_resolution_clock::time_point start_time = chrono::high_resolution_clock::now();
        vector<vector<long long>> process_timeline;
        pthread_t tid, tid2;
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
            
            if (!queue_is_empty){
                process current_process;
                {
                    std::lock_guard<std::mutex> lock_q(queue_mtx);
                    current_process = process_queue.top();
                    process_queue.pop();
                }
                if (current_process.identification != -1){
                    long long duration = 0;
                    chrono::high_resolution_clock::time_point exe_start = chrono::high_resolution_clock::now();
                    while(current_process.burst_time <= process_queue.top().burst_time &&
                        current_process.execution_time + (chrono::high_resolution_clock::now() - exe_start).count()< current_process.burst_time){
                        duration = (chrono::high_resolution_clock::now() - exe_start).count();
                    };
                    //cout << "time " << (chrono::high_resolution_clock::now() - start_time).count() <<"  Process " << current_process.identification << " executed for " << duration << " ns." << endl;
                    if (duration <= 0){
                        continue;
                    }
                    current_process.execution_time += duration;
                    {
                        std::lock_guard<std::mutex> lock_q(queue_mtx);
                        if (current_process.execution_time < current_process.burst_time){
                            process_queue.push(current_process);
                        }
                    }
                    process_timeline.push_back(vector<long long>{(exe_start-start_time).count(), current_process.identification, duration});
                    
                }
            }
        }
        pthread_join(tid, NULL);
        return process_timeline;
    }
}
#endif