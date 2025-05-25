#ifndef TYPE_H
#define TYPE_H

#include <iostream>
#include <vector>
#include <queue>
#include <limits>

using namespace std;
struct process{
    int identification;   
    int arrival_time;   
    int burst_time;   
    int priority;   
    int execution_time;
    process(int id, int arrive, int burst, int priority){
        this->identification = id;
        this->arrival_time = arrive;
        this->burst_time = burst;
        this->priority = priority;
        this->execution_time = 0;
    }
    process(){
        this->identification = -1;
        this->arrival_time = -1;
        this->burst_time = -1;
        this->priority = -1;
        this->execution_time = -1;
    }
};

struct prioritycompare {
    bool operator()(const process& a, const process& b) {
        return a.priority > b.priority;
    }
};
struct burstcompare {
    bool operator()(const process& a, const process& b) {
        return a.burst_time > b.burst_time;
    }
};
struct arrivalcompare {
    bool operator()(const process& a, const process& b) {
        return a.arrival_time > b.arrival_time;
    }
};
struct arrivalcompare_sort {
    bool operator()(const process& a, const process& b) {
        return a.arrival_time < b.arrival_time;
    }
};

template<typename T>
struct SCargs{
    vector<process> &process_list;
    priority_queue<process, vector<process>, T> &process_queue;
    SCargs(vector<process> &process_list, priority_queue<process, vector<process>, T> &queue) : process_list(process_list), process_queue(queue){}
};


#endif