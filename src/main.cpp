#include<iostream>
#include<algorithm>
#include<chrono>
#include<thread>
#include<vector>
#include<queue>
#include<pthread.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "type.h"
#include "SJF.h"
#include "RRNP.h"
#include "MLQS.h"
#include "custom.h"
namespace py=pybind11;

std::vector<process> processes = {
    process(0, 0, 18, 1),
    process(1, 0, 22, 3),
    process(2, 5, 8, 3),
    process(3, 5, 9, 2),
    process(4, 10, 15, 1),
    process(5, 10, 12, 2),
    process(6, 15, 7, 3),
    process(7, 15, 10, 2),
    process(8, 20, 25, 1),
    process(9, 20, 8, 3)
};
// u's
// process (id, arrival time, burst_time, priority)


vector<vector<long long>> SJF(std::vector<process> process_list){
    std::priority_queue<process, std::vector<process>, burstcompare> process_queue;
    SCargs<burstcompare> SJFarg(process_list, process_queue);
    auto timeline = SJFNS::run_work(SJFarg);
    return timeline;
}


vector<vector<long long>> RRNP(std::vector<process> process_list, int time_quantum){
    std::priority_queue<process, std::vector<process>, prioritycompare> process_queue;
    SCargs<prioritycompare> args(process_list, process_queue);
    auto result = RRNPNS::run_work(time_quantum, args);
    return result;
}


vector<vector<long long>> MLQS(std::vector<process> process_list){
    std::priority_queue<process, std::vector<process>, prioritycompare> process_queue;
    SCargs<prioritycompare> args(process_list, process_queue);
    auto result = MLQSNS::run_work(args);
    return result;
}

vector<vector<long long>> CUSTOM(std::vector<process> process_list){
    std::priority_queue<process, std::vector<process>, prioritycompare> process_queue;
    SCargs<prioritycompare> args(process_list, process_queue);
    auto result = CUSTOMNS::run_work(args);
    return result;
}

py::tuple main_caller(int method){
    processes = {};
    for(int i=0;i<40;i++){
        srand(time(0) + i); 
        long long r = rand();
        processes.push_back(process(i, (r%3)*5, r%22+6, rand()%3+1));
        processes[i].burst_time *= 1000*1000;
        processes[i].arrival_time *= 1000*1000;
    }
    vector<vector<long long>> arrival;
    for (auto &p : processes) {
        arrival.push_back(vector<long long>{p.arrival_time, p.identification, p.burst_time, p.priority});
    }
    std::sort(processes.begin(), processes.end(), arrivalcompare_sort());
    if (method == 1){
        std::cout << "SJF selected" << endl;
        auto result = SJF(processes);
        std::cout << "result size: " << result.size() << endl;
        return py::make_tuple(result,arrival);
    }else if (method == 2){
        std::cout << "RRNP selected" << endl;
        auto result = RRNP(processes, 5);
        std::cout << "result size: " << result.size() << endl;
        return py::make_tuple(result,arrival);
    }else if (method==3){
        std::cout << "MLQS selected" << std::endl;
        auto result = MLQS(processes);
        std::cout << "result size: " << result.size() << endl;
        return py::make_tuple(result,arrival);
    }else if(method==4){
        std::cout << "CUSTOM selected" << std::endl;
        auto result = CUSTOM(processes);
        std::cout << "result size: " << result.size() << endl;
        return py::make_tuple(result,arrival);
    }
    return py::make_tuple(vector<vector<long long >>{},arrival);
}

PYBIND11_MODULE(main_cpp, m) 
{
    m.doc() = "main caller of CPU scheduling algorithms";
    m.def("scheduller", &main_caller,
        py::arg("method")
    );
}