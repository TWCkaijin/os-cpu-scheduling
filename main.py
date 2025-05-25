from src.build import main_cpp
import matplotlib.pyplot as plt

if __name__ == "__main__":

    fig, ax = plt.subplots(2,2,figsize=(15, 8))
    schedule_names = ["SJF", "RR", "MLQS", "Custom"]
    for index, name in enumerate(schedule_names, 0):
        x = main_cpp.scheduller(index+1)
        schedule_raw, arrive = x[0], x[1] 
        print(f"--- {name} Scheduler ---")
        
        tmp = schedule_raw.copy()
        schedule_processed = [] 
        total_context_switch_duration = 0        
        
        for i in range(len(tmp)):
            tmp[i][1] = str(f"P {tmp[i][1]}") 
            schedule_processed.append(tmp[i])
            if (i < len(tmp) - 1):
                
                context_switch_start_time = tmp[i][0] + tmp[i][2]
                context_switch_duration = tmp[i+1][0] - context_switch_start_time
                if context_switch_duration < 0: 
                    context_switch_duration = 0 
                data = [context_switch_start_time, "switch", context_switch_duration]
                total_context_switch_duration += data[2]
                schedule_processed.append(data)
        
        num_actual_processes_in_schedule = len(tmp)
        avg_context_switch_duration = total_context_switch_duration / (num_actual_processes_in_schedule - 1) if num_actual_processes_in_schedule > 1 else 0
        
        print(f"Average Context Switch Duration for {name}: {avg_context_switch_duration:.2f} ns")
        
        
        if not schedule_processed:
            print("No data to plot.")
        else:
            axes = ax[int(index/2)][index%2]
            
            unique_pids_for_plot = sorted(list(set(item[1] for item in schedule_processed)))
            pid_to_y = {pid: i for i, pid in enumerate(unique_pids_for_plot)}
            colors = plt.get_cmap('viridis', len(unique_pids_for_plot))

            total_waiting_time_ns = 0
            total_turnaround_time_ns = 0
            num_completed_processes = 0
        
            arrival_times_map = {} 
            burst_times_map = {}   
            for proc_info in arrive:
                pid_int = int(proc_info[1]) 
                arrival_times_map[pid_int] = proc_info[0] 
                burst_times_map[pid_int] = proc_info[2]   

            completion_times_map = {}
            
            for seg_start_time, seg_pid_str, seg_duration in schedule_raw:
                try:
                    
                    pid_int = int(str(seg_pid_str).replace("P ", ""))
                except ValueError:
                    continue 
                
                current_segment_completion_time = seg_start_time + seg_duration
                if pid_int not in completion_times_map or current_segment_completion_time > completion_times_map[pid_int]:
                    completion_times_map[pid_int] = current_segment_completion_time
            
            for process_id_int, completion_time_ns in completion_times_map.items():
                if process_id_int not in arrival_times_map or process_id_int not in burst_times_map:
                    continue

                arrival_time_ns = arrival_times_map[process_id_int]
                burst_time_ns = burst_times_map[process_id_int]

                turnaround_time_ns = completion_time_ns - arrival_time_ns
                waiting_time_ns = turnaround_time_ns - burst_time_ns
                
                if turnaround_time_ns < 0: turnaround_time_ns = 0 
                if waiting_time_ns < 0: waiting_time_ns = 0 

                total_turnaround_time_ns += turnaround_time_ns
                total_waiting_time_ns += waiting_time_ns
                num_completed_processes += 1

            avg_waiting_time_ns = total_waiting_time_ns / num_completed_processes if num_completed_processes > 0 else 0
            avg_turnaround_time_ns = total_turnaround_time_ns / num_completed_processes if num_completed_processes > 0 else 0
            
            print(f"Average Waiting Time for {name}: {avg_waiting_time_ns/1000/1000:.2f} ms")
            print(f"Average Turnaround Time for {name}: {avg_turnaround_time_ns/1000/1000:.2f} ms\n")

            legend_str = (f"wait:\n{avg_waiting_time_ns/1000/1000:.2f} ms\n"
                          f"exec:\n{avg_turnaround_time_ns/1000/1000:.2f} ms\n"
                          f"switch:\n{avg_context_switch_duration/1000:.2f} ns")
            
            for i in range(len(schedule_processed)):
                start_time, process_id_str_plot, duration = schedule_processed[i]
                
                
                if process_id_str_plot not in pid_to_y:
                    continue
                
                y_pos = pid_to_y[process_id_str_plot]
                
                bar_color = colors(pid_to_y[process_id_str_plot] % len(unique_pids_for_plot))
                if process_id_str_plot == "Switch":
                    bar_color = 'grey' 

                axes.barh(
                    y_pos,          
                    duration/1000/1000, 
                    left=start_time/1000/1000, 
                    height=0.6,
                    edgecolor='black',
                    color=bar_color, 
                    alpha=0.75,
                    label=legend_str if i == 0 else None 
                )

            axes.set_yticks(range(len(unique_pids_for_plot)))
            axes.set_yticklabels([f'{pid}' for pid in unique_pids_for_plot])
            axes.invert_yaxis()

            axes.set_xlabel("Time (ms)")
            axes.set_ylabel("Process")
            axes.set_title(f"{name} Gantt Chart") 

            axes.grid(True, axis='x', linestyle=':', alpha=0.7)        
            if num_completed_processes > 0 : 
                axes.legend(bbox_to_anchor=(1.02, 1), loc='upper left', borderaxespad=0.)
    
    plt.tight_layout()
    fig.subplots_adjust(right=0.85)
    plt.show()