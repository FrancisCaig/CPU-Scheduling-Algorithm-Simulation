//  -------------------------------------Operating System Project------------------------------------------
//      Team: Guanghan Cai, Gregorios Katsios, Fanfan Qian
// 
//
//      Update: 3/17/2020, FCFS (using list to store I/O), algo finished with statistics information (except the turnaround time)
//		Update: 3/21/2020, Code cosmetic changes + added some comments
//		Update: 3/22/2020, Added RR algorithm
//      Update: 3/29/2020, SJF, but still has context switch problem...
//      Update: 4/9/2020,  Fix context_switch counter bug + cosmetic changes
//		Update: 4/10/2020, Fix IO burst bug
//		Update: 4/11/2020, Fix context switch logic for corner case, added SRT algorithm
//      Update: 4/13/2020, basic output formation and ready to submit
//                            trying to fix
//      Update: 4/14/2020, SJF and print(999) fixed
//                            statistic still has problem, and it's worth 50 points
//      Update: 4/14/2020, Updated SRT and RR + cosmetic changes              
//      Update: 4/14/2020, fixed SRT, now there is only one problem: RR, the order and statictis results.  --- this version got 68 points
//      
//  -------------------------------------Operating System Project------------------------------------------

//-- library for C++
#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cmath>
#include <vector>
#include <cctype>
#include <queue>
#include <list>
#include <map>
#include <set>

//-- library for C
//#include <wait.h>
#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <iomanip>
#include <math.h>
#include <time.h>

using namespace std;

ofstream outfile("simout.txt");  //store the performance

// Function to get exponential distribution based process arrival times using upper bound and lambda
int arrival_time(double bound, double lambda)
{
    double res = bound + 100;

    // skip all values bigger than upper bound
    while (res > bound)
    {
        double r = drand48();
        res = floor(-log(r) / lambda);
    }
    return (int) res;
}

// Function to get the number of CPU bursts [returns values in range 1 - 100]
int num_burst(double bound)
{
    double res = bound + 100;
    while (res > bound)
    {
        double r = drand48();
        r = r * 100 + 1;
        res = floor(r);
    }
    return res;
}

// Function to get exponential distribution based burst times using upper bound and lambda
int exp_ceil_num(double bound, double lambda)
{
    double res = bound + 100;
    while (res > bound)
    {
        double r = drand48();
        res = ceil(-log(r) / lambda);
    }
    return (int) res;
}

// Function to set the CPU and IO burst times
void burst_time(vector<pair<int, int>> &v, double bound, int num, double lambda)
{
    for (int i = 0; i < num; i++)
    {
        if (i == num - 1)
        {
            double res = exp_ceil_num(bound, lambda);
            v.push_back(make_pair(res, 0));
        }

        else
        {
            double res1 = exp_ceil_num(bound, lambda);
            double res2 = exp_ceil_num(bound, lambda);
            v.push_back(make_pair(res1, res2));
        }
    }

    assert(num == (int) v.size());
}

class Process
{
public:

    // Process constructor 1: no initialization
    Process()
    {
        num = 0;
        pre = 0;
        run = false;
        current = 0;
        switch_in = 0;
        total_wait = 0;
        switch_out = 0;
        context_switch = 0;
    }

    // Process constructor 2: with default initialization
    Process(const string n, double bound, double lambda)
    {
        arrival = arrival_time(bound,lambda);
        num = num_burst(bound); // decide the number of CPU burst. After that, we can either check burst.size() or call num to get it.

        burst_time(burst, bound, num, lambda);
        copy(burst.begin(), burst.end(), back_inserter(burst_2));

        context_switch = 0;
        total_burst = 0;
        total_wait = 0;
        switch_out = 0;
        switch_in = 0;
        current = 0;
        run = false;
        name = n;
        pre = 0;
    }

    Process& operator = (const Process &p)
    {
        context_switch = p.context_switch;
        est_burst_2 = p.est_burst_2;
        total_burst = p.total_burst;   	// CPU burst, not include I/O
        switch_out = p.switch_out;
        total_wait = p.total_wait;      // once we have total_wait, adding CPU burst and I/O burst(apply the rule), then adding switch time===> Turnaround time
        est_burst = p.est_burst; 		// store the estimated burst time for SJF or SRF method
        switch_in = p.switch_in;
        burst_2 = p.burst_2;
        arrival = p.arrival;
        current = p.current;     		// store the current index in vector burst
        burst = p.burst;
        num = p.arrival;
        name = p.name;
        pre = p.pre;  					// preemptions
        run = p.run;

        return *this;
    }

    // Method to set estimates of arrival, CPU and IO burst times
    void set_estimate(float alpha, float lambda)
    {
        int tao = (int)ceil(1 / lambda);
        est_burst.push_back(make_pair(tao, burst[0].second));
        for (unsigned i = 0; i < burst.size() - 1; i++)
        {
            tao = (int) ceil(alpha * burst[i].first + (1 - alpha) * tao);
            est_burst.push_back(make_pair(tao, burst[i + 1].second));
        }

        // copy estimated burst to a second vector
        copy(est_burst.begin(), est_burst.end(), back_inserter(est_burst_2));
    }

    string name;
    vector<pair<int, int>> burst;       // store every pair{CPU burst , I/O burst}, for the last pair = {CPU burst, 0}
    vector<pair<int, int>> burst_2;     // copy of burst
    vector<pair<int, int>> est_burst;   // store the estimated burst time for SJF or SRF method
    vector<pair<int, int>> est_burst_2; // copy of est_burst
    // ---- if want to save space, we can just store burst time as vector<int>, not vector<pair>

    unsigned int current;   // store the current index in vector burst
    int context_switch;
    double total_wait;      // once we have total_wait, adding CPU burst and I/O burst(apply the rule), then adding switch time===> Turnaround time    
    int total_burst;        // CPU burst, not include I/O
    int switch_out;         // from Tst/2 to 0
    int switch_in;
    int arrival;
    bool run;
    int num;
    int pre;                //preemptions
};

// Function to generate all processes given user parameters
void generate_processes(vector<Process> &storage, double bound, double lambda, double alpha, int n)  //storage the original processes we created, n is number of processes given
{
    // wipe storage
    if (!storage.empty()) {
        storage.clear();
    }

    char temp = 'A'; //start from A, to Z
    for (int i = 0; i < n; i++)
    {
        string s(1, temp);
        Process p(s, bound, lambda);
        p.set_estimate(alpha, lambda);
        storage.push_back(p);
        temp++;
    }
}

// Function to sort the processes by order of arrival time
bool compare1(const Process &a, const Process &b)
{
    // same arrival, compare name
    if (a.arrival == b.arrival) return a.name < b.name;
    else return a.arrival < b.arrival;
}

// Function to sort the processes by alphabetical order
bool compare2(const Process &a, const Process &b)
{
    return a.name < b.name;
}

// Function to sort the processes by CPU burst completetion (tau)
bool compare3(const Process &a, const Process &b)
{
    if (a.est_burst[a.current].first == b.est_burst[b.current].first)
        return (a.name < b.name);
    else
        return (a.est_burst[a.current].first < b.est_burst[b.current].first);
}

// Function to print out final statistics
void statistic(const list<Process>  &result, int cont)   //get average of burst for every algo times to report
{
    int sum_of_context_switches = 0;
    float sum_turnaround_time = 0;
    float sum_cpu_burst_time = 0;
    int sum_of_preemptions = 0;
    float sum_wait_time = 0;
    int total = 0;  // count the total number for burst
    for (auto p: result)
    {
        sum_of_context_switches += p.context_switch;
        sum_turnaround_time     += (p.burst.size() * cont) + (p.pre * cont) + p.total_burst + p.total_wait;
        sum_cpu_burst_time      += p.total_burst;
        sum_of_preemptions      += p.pre;
        sum_wait_time           += p.total_wait;
        total                   += p.burst.size();
    }
    
    // calculate averages
    float avg_cpu_burst_time = (float) sum_cpu_burst_time / (float) total;
    float avg_turnaround = (float) sum_turnaround_time / (float) total;
    float avg_wait_time = (float) sum_wait_time / (float) total;

    outfile << fixed << setprecision(3);
    outfile << "-- average CPU burst time: " << avg_cpu_burst_time << " ms" << endl;
    outfile << "-- average wait time: " << avg_wait_time << " ms" << endl;
    outfile << "-- average turnaround time: " << avg_turnaround << " ms" << endl;
    outfile << "-- total number of context switches: " << sum_of_context_switches << endl;
    outfile << "-- total number of preemptions: " << sum_of_preemptions << endl;
}

// Function to print out the Q
void report_Q(const list<Process> &ready) {
    cout  <<  " [Q";
    if (ready.empty())  //nothing
        cout  <<  " <empty>]"  <<  endl;
    else {
        for (auto itr = ready.begin(); itr != ready.end(); itr++)  //Q has others
        {
            cout  <<  " "  <<  itr->name;
        }
        cout  <<  "]"  <<  endl;
    }
}

// Function to simulate the FCFS algorithm
void FCFS(vector<Process> storage, int cont) // try to check every millisecond. should use const or & (cuz we erase elements at end of loop)
{
    // report processes at begin
    for (auto i: storage) {
        cout << "Process " << i.name << " [NEW] (arrival time " << i.arrival << " ms) " << i.burst.size();
        if (i.burst.size() == 1) {
            cout << " CPU burst" << endl;
        } else {
            cout << " CPU bursts" << endl;
        }
    }
    // report start for algo
    cout << "time 0ms: Simulator started for FCFS [Q <empty>]" << endl;
    // though some processes arrive at 0, we still report at first Q is empty, see output4.txt
    
    list<Process> result; // store finished processes
    list<Process> ready;  // ready queue
    list<Process> io;     // store processes in I/O blocking
    int flag = 0;    
    Process cpu;          // track for the process in CPU burst

    for (int t = 0; t < 999 * 1000; t++) {
        // from the tie: CPU burst, I/O blocking, new arrival
        // CPU burst
        if (cpu.switch_out > 0) 
        {
            cpu.switch_out -= 1;
        } 
        else if (cpu.switch_out == 0 && flag == 0) 
        {
            if (cpu.run) //so this one is finished and switch_out is 0
            {
                cpu.run = false;
                io.push_back(cpu);
            }

            if (ready.empty()) {
                //do nothing
            } 

            else //select one process
            {
                cpu = ready.front();
                ready.pop_front();     //here we pretend the process has not entered into cpu, still in the ready Q
                flag = 1;
                cpu.run = true;
                cpu.context_switch += 1;      //////////////////////////////////////I'm not sure if it's right to add here
                cpu.switch_in = (cont / 2) - 1;
            }
        } 

        else if (cpu.switch_in > 0 && flag == 1) 
        {
            cpu.switch_in -= 1;
        }

        if (cpu.switch_in == 0 && flag != 0) 
        {
            if (flag == 1) 
            {
                flag = 2;  //so if flag == 2, don't report start

                if (t <= 999) 
                {
                    cout << "time " << t << "ms: Process " << cpu.name << " started using the CPU for " << cpu.burst[cpu.current].first << "ms burst";
                    report_Q(ready);
                }
            } 

            else  // switch time finished or running
            {
                cpu.burst[cpu.current].first -= 1;    // one second pass
                cpu.total_burst += 1;    // add one ms
                // burst completed, take care of
                // 1) if the whole process terminate,
                // 2) switching to I/O, remaining bursts to go
                // 3) report Q
                // 4) I/O time to get back to Q,add into I/O
                // 5) run becomes false;
                // 6) choose next one. If nothing in Q, do nothing, otherwise just pick first in Q
                //                                             [Not sure if needs to care context switch here?]
                if (cpu.burst[cpu.current].first == 0) 
                {
                    flag = 0;
                    cpu.current += 1;
                    cpu.switch_out = cont / 2;

                    if (cpu.current == cpu.burst.size())  // terminate
                    {
                        cout << "time " << t << "ms: Process " << cpu.name << " terminated";
                        report_Q(ready);
                        cpu.run = false;
                        result.push_back(cpu);
                    } 

                    else //next one
                    {
                        if (t <= 999) 
                        {
                            cout << "time " << t << "ms: Process " << cpu.name << " completed a CPU burst; " << cpu.burst.size() - cpu.current << " bursts to go";
                            report_Q(ready);

                            cout << "time " << t << "ms: Process " << cpu.name << " switching out of CPU; will block on I/O until time " << t + cpu.burst[cpu.current - 1].second + cont / 2 << "ms";
                            report_Q(ready);
                        }
                    }
                }
            }
        }

        // this part add wait time for processes in ready Q, because after here, back from I/O or new arrival will be added into Q           [should count for them?]
        for (auto itr = ready.begin(); itr != ready.end(); itr++) 
        {
            itr->total_wait += 1;
        }

        // I/O blocking
        if (io.empty()) 
        {
            // do nothing
        } 

        else  //  1) I/O time-- 2) if finished, add to queue, no switch here 3)report
        {
            vector<Process> back;
            for (auto itr = io.begin(); itr != io.end(); itr++) 
            {
                // need to use "current - 1" cuz we incremented it when the cpu-burst ended (in above).
                itr->burst[itr->current - 1].second -= 1;
                if (itr->burst[itr->current - 1].second <= 0) // go back to queue, less than than 0 because last I/O is 0, then -- becomes -1
                {
                    back.push_back(*itr);
                    itr = io.erase(itr);
                    itr--;
                }
            }

            // now back has all processes completed I/O just now, add them to ready Q by name order, so sort first
            sort(back.begin(), back.end(), compare2); // sort by name
            for (auto i: back) 
            {
                ready.push_back(i);
                if (t <= 999) 
                {
                    cout << "time " << t << "ms: Process " << i.name << " completed I/O; added to ready queue";
                    report_Q(ready);
                }

            }
        }
        //new arrival
        for (auto itr = storage.begin(); itr != storage.end(); itr++)   //1) add 2)report
        {
            if (itr->arrival == t) 
            {
                ready.push_back(*itr);
                if (t <= 999) 
                {
                    cout << "time " << t << "ms: Process " << itr->name << " arrived; added to ready queue";
                    report_Q(ready);
                }
                itr = storage.erase(itr);
                itr--;
            }
        }
        if (!cpu.run && storage.empty() && io.empty() && ready.empty())  //kill all, break the loop
        {
            cout << "time " << t + 2 << "ms: Simulator ended for FCFS [Q <empty>]" << endl;
            break;
        }

    }
    outfile << "Algorithm FCFS" << endl;
    statistic(result, cont);

}

// Function to simulate the SJF algorithm
void SJF(vector<Process> storage, int cont)
{
    // report processes at begin
    for (auto i: storage)
    {
        cout << "Process " << i.name << " [NEW] (arrival time " << i.arrival << " ms) " << i.burst.size();

        if (i.burst.size() == 1)   //just one, no s
        {
            cout<< " CPU burst (tau " << i.est_burst[0].first << "ms)" << endl;
        }

        else
        {
            cout<< " CPU bursts (tau " << i.est_burst[0].first << "ms)" << endl;
        }
    }
    // report start for algo
    cout << "time 0ms: Simulator started for SJF [Q <empty>]" << endl;    // though some processes arrive at 0, we still report at first Q is empty, see output4.txt

    list<Process> result; // store finished processes
    list<Process> ready;  // ready queue
    list<Process> io;     // store processes in I/O blocking
    int flag = 0;
    Process cpu;          // track for the process in CPU burst

    for (int t = 0; t < 999 * 1000; t++)
    {
        // from the tie: CPU burst, I/O blocking, new arrival
        // CPU burst
        if (cpu.switch_out > 0)
        {
            cpu.switch_out -= 1;
        }

        else if (cpu.switch_out == 0 && flag == 0)
        {
            if (cpu.run) //so this one is finished and switch_out is 0
            {
                cpu.run = false;
                io.push_back(cpu);
            }

            if (ready.empty())
            {
                //do nothing
            }

            else //select one process
            {
                cpu = ready.front();
                ready.pop_front();     //here we pretend the process has not entered into cpu, still in the ready Q
                flag = 1;
                cpu.run = true;
                cpu.context_switch += 1;      //////////////////////////////////////I'm not sure if it's right to add here
                cpu.switch_in = (cont / 2) - 1;
            }
        }
        else if (cpu.switch_in > 0 && flag == 1)
        {
            cpu.switch_in -= 1;
        }

        if (cpu.switch_in == 0 && flag != 0) // switch_in == 0 starts to run
        {
            if (flag == 1)
            {
                // ready.pop_front();
                if (t <= 999)
                {
                    cout << "time " << t << "ms: Process " << cpu.name << " (tau " << cpu.est_burst[cpu.current].first << "ms) started using the CPU for " << cpu.burst[cpu.current].first << "ms burst";
                    report_Q(ready);
                }

                flag = 2;  //so if flag == 2, don't report start
            }
            else
            {
                cpu.burst[cpu.current].first -= 1;    // one ms pass
                cpu.total_burst += 1;    // add one ms
                // burst completed, take care of
                // 1) if the whole process terminate,
                // 2) switching to I/O, remaining bursts to go
                // 3) report Q
                // 4) I/O time to get back to Q,add into I/O
                // 5) run becomes false;
                // 6) choose next one. If nothing in Q, do nothing, otherwise just pick first in Q
                //                                             [Not sure if needs to care context switch here?]
                if (cpu.burst[cpu.current].first == 0)
                {
                    flag = 0;
                    cpu.current += 1;
                    cpu.switch_out = cont / 2;

                    if (cpu.current == cpu.burst.size())  // terminate
                    {
                        cout << "time " << t << "ms: Process " << cpu.name << " terminated";
                        report_Q(ready);
                        cpu.run = false;
                        result.push_back(cpu);                        
                    }

                    else //next one
                    {
                        if (t <= 999)
                        {
                            cout << "time " << t << "ms: Process " << cpu.name << " (tau " << cpu.est_burst[cpu.current - 1].first << "ms) completed a CPU burst; " << cpu.burst.size() - cpu.current << " bursts to go";
                            report_Q(ready);
                            cout << "time " << t << "ms: Recalculated tau = " << cpu.est_burst[cpu.current].first << "ms for process " << cpu.name;
                            report_Q(ready);

                            cout << "time " << t << "ms: Process " << cpu.name << " switching out of CPU; will block on I/O until time " << t + cpu.burst[cpu.current-1].second + cont / 2 << "ms";
                            report_Q(ready);
                        }
                    }
                }
            }
        }

        // this part add wait time for processes in ready Q, because after here, back from I/O or new arrival will be added into Q           [should count for them?]
        for (auto itr = ready.begin(); itr != ready.end(); itr++) 
        {
            itr->total_wait += 1;    // we need to change here because it includes that when process is in CPU but switch in>0
        }
        // I/O blocking
        if (io.empty())
        {
            // do nothing
        }
        else  //  1) I/O time-- 2) if finished, add to queue, no switch here 3)report
        {
            vector<Process> back;
            for (auto itr = io.begin(); itr != io.end(); itr++)
            {
                // need to use "current - 1" cuz we incremented it when the cpu-burst ended (in above).
                itr->burst[itr->current - 1].second -= 1;
                if (itr->burst[itr->current - 1].second <= 0) // go back to queue, less than than 0 because last I/O is 0, then -- becomes -1
                {
                    back.push_back(*itr);
                    itr = io.erase(itr);
                    itr--;
                }
            }

            // now back has all processes completed I/O just now, add them to ready Q by name order, so sort first
            sort(back.begin(), back.end(), compare2); // sort by name
            for (auto i: back)
            {
                ready.push_back(i);
                ready.sort(compare3);
                if (t <= 999)
                {
                    cout << "time " << t << "ms: Process " << i.name << " (tau " << i.est_burst[i.current].first << "ms) completed I/O; added to ready queue";
                    report_Q(ready);
                }
            }
        }

        //new arrival
        for (auto itr = storage.begin(); itr != storage.end(); itr++)   //1) add 2)report
        {
            if (itr->arrival == t)
            {
                ready.push_back(*itr);
                ready.sort(compare3);
                if (t <= 999)
                {
                    cout << "time " << t << "ms: Process " << itr->name << " (tau " << itr->est_burst[itr->current].first << "ms) arrived; added to ready queue";
                    report_Q(ready);
                }

                itr = storage.erase(itr);
                itr--;
            }
        }

        if (!cpu.run && storage.empty() && io.empty() && ready.empty())
        {
            cout << "time " << t + 2 << "ms: Simulator ended for SJF [Q <empty>]" << endl;
            break;
        }
    }

        outfile << "Algorithm SJF" << endl;
        statistic(result,cont);
}

// Function to simulate the SRT algorithm
void SRT(vector<Process> storage, int cont)
{
    // report processes at begin
    for (auto i: storage)
    {
        cout << "Process " << i.name << " [NEW] (arrival time " << i.arrival << " ms) " << i.burst.size();
        if (i.burst.size() == 1)   //just one, no s
        {
            cout<< " CPU burst (tau " << i.est_burst_2[0].first << "ms)" <<endl;
        }

        else
        {
            cout<< " CPU bursts (tau " << i.est_burst_2[0].first << "ms)" <<endl;
        }
    }
    // report start for algo
    cout << "time 0ms: Simulator started for SRT [Q <empty>]" << endl;    // though some processes arrive at 0, we still report at first Q is empty, see output4.txt
    
    int preemption_flag = 0;
    int flag = 0;

    list<Process> result; // store finished processes
    list<Process> ready;  // ready queue
    list<Process> io;     // store processes in I/O blocking
    Process cpu;          // track for the process in CPU burst

    for (int t = 0; t < 999 * 1000; t++)
    {
        // from the tie: CPU burst, I/O blocking, new arrival
        // CPU burst
        if (cpu.switch_out > 0)
        {
            cpu.switch_out -= 1;
        }

        else if (cpu.switch_out == 0 && flag == 0)
        {
            if (cpu.run && preemption_flag == 0) // so this one is finished and switch_out is 0
            {
                cpu.run = false;
                io.push_back(cpu);
            }

            else if (cpu.run && preemption_flag == 1) // cpu.run == true and we have preemption, cpu must go to readyQ
            {
                cpu.pre += 1;
                cpu.run = false;
                preemption_flag = 0;
                ready.push_back(cpu);
                ready.sort(compare3);
            }

            if (ready.empty())
            {
                //do nothing
            }

            else //select one process
            {
                cpu = ready.front();
                ready.pop_front();     //here we pretend the process has not entered into cpu, still in the ready Q
                flag = 1;
                cpu.run = true;
                cpu.context_switch += 1;      //////////////////////////////////////I'm not sure if it's right to add here
                cpu.switch_in = (cont / 2) - 1;                
            }
        }

        else if (cpu.switch_in > 0 && flag == 1) 
        {
            cpu.switch_in -= 1;
        }

        if (cpu.switch_in == 0 && flag != 0) // switch_in == 0 starts to run
        {
            if (flag == 1)
            {                
                if (t <= 999)
                {
                    cout << "time " << t << "ms: Process " << cpu.name << " (tau " << cpu.est_burst_2[cpu.current].first << "ms) started using the CPU with " << cpu.burst[cpu.current].first << "ms burst remaining";
                    report_Q(ready);
                }

                flag = 2;  //so if flag == 2, don't report start
            } 

            else 
            {
                cpu.est_burst[cpu.current].first -= 1;
                cpu.burst[cpu.current].first -= 1;
                cpu.total_burst += 1;    // add one ms
                // burst completed, take care of
                // 1) if the whole process terminate,
                // 2) switching to I/O, remaining bursts to go
                // 3) report Q
                // 4) I/O time to get back to Q,add into I/O
                // 5) run becomes false;
                // 6) choose next one. If nothing in Q, do nothing, otherwise just pick first in Q
                //                                             [Not sure if needs to care context switch here?]
                if (cpu.burst[cpu.current].first == 0)
                {
                    flag = 0;                    
                    cpu.current += 1;
                    preemption_flag = 0;
                    cpu.switch_out = cont / 2;

                    if (cpu.current == cpu.burst.size())  // terminate
                    {
                        cout << "time " << t << "ms: Process " << cpu.name << " terminated";
                        report_Q(ready);

                        cpu.run = false;
                        result.push_back(cpu);
                    }

                    else //next one
                    {
                        if (t <= 999)
                        {
                            cout << "time " << t << "ms: Process " << cpu.name << " (tau " << cpu.est_burst_2[cpu.current - 1].first << "ms) completed a CPU burst; " << cpu.burst.size() - cpu.current << " bursts to go";
                            report_Q(ready);

                            cout << "time " << t << "ms: Recalculated tau = " << cpu.est_burst_2[cpu.current].first << "ms for process " << cpu.name;
                            report_Q(ready);

                            cout << "time " << t << "ms: Process " << cpu.name << " switching out of CPU; will block on I/O until time " << t + cpu.burst[cpu.current - 1].second + cont / 2 << "ms";
                            report_Q(ready);
                        }
                    }
                }
            }
        }

        // hack fix for corner case preemption        
        if (!ready.empty() && ready.front().num != 0) {
            Process candidate = ready.front();

            if (flag >= 2 && candidate.est_burst[candidate.current].first < cpu.est_burst[cpu.current].first)
            {                    
                flag = 0;
                preemption_flag = 1;
                cpu.switch_out = cont / 2;                

                if (t <= 999)
                {
                    cout << "time " << t << "ms: Process " << candidate.name << " (tau " << candidate.est_burst_2[candidate.current].first << "ms) will preempt " << cpu.name;
                    report_Q(ready);
                }
            }
        }

        // this part add wait time for processes in ready Q, because after here, back from I/O or new arrival will be added into Q           [should count for them?]
        for (auto itr = ready.begin(); itr != ready.end(); itr++) 
        {
            itr->total_wait += 1;
        }

        // I/O blocking
        if (io.empty())
        {
            // do nothing
        }

        else  //  1) I/O time-- 2) if finished, add to queue, no switch here 3)report
        {
            vector<Process> back;
            for (auto itr = io.begin(); itr != io.end(); itr++)
            {
                // need to use "current - 1" cuz we incremented it when the cpu-burst ended (in above).
                itr->burst[itr->current - 1].second -= 1;
                if (itr->burst[itr->current - 1].second <= 0) // go back to queue, less than than 0 because last I/O is 0, then -- becomes -1
                {
                    back.push_back(*itr);
                    itr = io.erase(itr);
                    itr--;
                }
            }

            // now back has all processes completed I/O just now, add them to ready Q by name order, so sort first
            sort(back.begin(), back.end(), compare3); // sort by tau and resolve ties by name

            Process candidate;
            if (!back.empty()) 
            {                
                candidate = back.front();                
            }

            // add returning processes to ready queue
            for (auto p: back)
            {
                ready.push_back(p);
                ready.sort(compare3);

                if (p.name == candidate.name && p.name == ready.front().name // this process preempts the CPU
                    && flag >= 2 && p.est_burst[p.current].first < cpu.est_burst[cpu.current].first)
                {
                    flag = 0;
                    preemption_flag = 1;
                    cpu.switch_out = cont / 2;                     

                    if (t <= 999)
                    {
                        cout << "time " << t << "ms: Process " << p.name << " (tau " << p.est_burst_2[p.current].first << "ms) completed I/O; preempting " << cpu.name;
                        report_Q(ready);
                    }
                }

                else
                {
                    if (t <= 999)
                    {
                        cout << "time " << t << "ms: Process " << p.name << " (tau " << p.est_burst_2[p.current].first << "ms) completed I/O; added to ready queue";
                        report_Q(ready);
                    }
                }
            }
        }

        //new arrival
        vector<Process> new_arrivals;
        for (auto itr = storage.begin(); itr != storage.end(); itr++)   //1) add 2)report
        {
            if (itr->arrival == t)
            {
                new_arrivals.push_back(*itr);
                itr = storage.erase(itr);
                itr--;
            }
        }

        // new arraivals has all processes that come into the scheduler at time t
        sort(new_arrivals.begin(), new_arrivals.end(), compare3); // sort by tau and resolve ties by name   

        Process candidate;
        if (!new_arrivals.empty()) 
        {
            candidate = new_arrivals.front();
        }

        for (auto p: new_arrivals) 
        {
            ready.push_back(p);
            ready.sort(compare3);

            if (p.name == candidate.name // this process preempts the CPU
                && flag >= 2 && p.est_burst[p.current].first < cpu.est_burst[cpu.current].first)
            {
                flag = 0;
                preemption_flag = 1;
                cpu.switch_out = cont / 2;

                if (t <= 999)
                {
                    cout << "time " << t << "ms: Process " << p.name << " (tau " << p.est_burst_2[p.current].first << "ms) arrived; preempting " << cpu.name;
                    report_Q(ready);
                }
            }

            else
            {                
                if (t <= 999)
                {
                    cout << "time " << t << "ms: Process " << p.name << " (tau " << p.est_burst_2[p.current].first << "ms) arrived; added to ready queue";
                    report_Q(ready);
                }
            }
        }

        if (!cpu.run && storage.empty() && io.empty() && ready.empty())
        {
            cout << "time " << t + 2 << "ms: Simulator ended for SRT [Q <empty>]" << endl;
            break;
        }
    }

    outfile << "Algorithm SRT" << endl;
    statistic(result, cont);
}

// Function to simulate the RR algorithm
void RR(vector<Process> storage, int cont, int slice, string order)
{
    // report processes at begining
    for (auto i: storage)
    {
        cout << "Process " << i.name << " [NEW] (arrival time " << i.arrival << " ms) " << i.burst.size();
        if (i.burst.size() == 1)   //just one, no s
        {
            cout<< " CPU burst" << endl;
        }

        else
        {
            cout<< " CPU bursts" << endl;
        }
    }

    // report start for algo
    cout << "time 0ms: Simulator started for RR [Q <empty>]" << endl;
    int preemption_flag = 0;
    int slice_to_go = slice;  // track how many ms of CPU time a process has left before preemption
    int flag = 0;

    list<Process> result; // store finished processes
    list<Process> ready;  // ready queue
    list<Process> io;     // store processes in I/O blocking
    Process cpu;          // track for the process in CPU burst

    for (int t = 0; t < 999 * 1000; t++)
    {
        // from the tie: CPU burst, I/O blocking, new arrival
        // CPU burst
        if (cpu.switch_out > 0)
        {
            cpu.switch_out -= 1;
        }

        else if (cpu.switch_out == 0 && flag == 0)
        {
            if (cpu.run && preemption_flag == 0) // so this one is finished and switch_out is 0
            {
                cpu.run = false;
                io.push_back(cpu);
            }

            else if (cpu.run && preemption_flag == 1) // cpu.run == true and we have preemption, cpu must go to readyQ
            {
                cpu.pre += 1;
                cpu.run = false;
                preemption_flag = 0;
                ready.push_back(cpu);
            }

            if (ready.empty())
            {
                //do nothing
            }

            else //select one process
            {
                cpu = ready.front();
                ready.pop_front();     //here we pretend the process has not entered into cpu, still in the ready Q
                flag = 1;
                cpu.run = true;
                slice_to_go = slice;
                cpu.context_switch += 1;
                cpu.switch_in = (cont / 2) - 1;
            }
        }

        else if (cpu.switch_in > 0 && flag == 1) 
        {
            cpu.switch_in -= 1;
        }

        if (cpu.switch_in == 0 && flag != 0) // switch_in == 0 starts to run
        {
            if (flag == 1)
            {                
                if (t <= 999)
                {
                    if (cpu.burst[cpu.current].first < cpu.burst_2[cpu.current].first)
                    {
                        //time 512ms: Process B started using the CPU with 70ms burst remaining [Q <empty>]
                        cout << "time " << t << "ms: Process " << cpu.name << " started using the CPU with " << cpu.burst[cpu.current].first << "ms burst remaining";
                    } 

                    else 
                    {
                        cout << "time " << t << "ms: Process " << cpu.name << " started using the CPU for " << cpu.burst[cpu.current].first << "ms burst";
                    }

                    report_Q(ready);
                }

                flag = 2;  //so if flag == 2, don't report start
            } 

            else  // switch time finished or running
            {
                cpu.burst[cpu.current].first -= 1;    // one millisecond pass
                cpu.total_burst += 1;    // add one ms
                slice_to_go -= 1;

                // burst completed, take care of
                // 1)if the whole process terminate,
                // 2) switching to I/O, remaining bursts to go
                // 3) report Q
                // 4) I/O time to get back to Q,add into I/O
                // 5) run becomes false;
                // 6) choose next one. If nothing in Q, do nothing, otherwise just pick first in Q
                //                                             [Not sure if needs to care context switch here?]

                if (cpu.burst[cpu.current].first == 0) // process reached the end of a burst, no preemption
                {
                    flag = 0;
                    cpu.current += 1;
                    preemption_flag = 0;
                    cpu.switch_out = cont / 2;

                    if (cpu.current == cpu.burst.size())  // terminate
                    {
                        cout << "time " << t << "ms: Process " << cpu.name << " terminated";
                        report_Q(ready);

                        cpu.run = false;
                        result.push_back(cpu);
                        
                    }

                    else //next one
                    {
                        if (t <= 999)
                        {
                            cout << "time " << t << "ms: Process " << cpu.name << " completed a CPU burst; " << cpu.burst.size() - cpu.current << " bursts to go";
                            report_Q(ready);

                            cout << "time " << t << "ms: Process " << cpu.name << " switching out of CPU; will block on I/O until time " << t + cpu.burst[cpu.current - 1].second + cont / 2 << "ms";
                            report_Q(ready);
                        }
                    }
                }

                // process used up all the t_slice and the ready queue is not empty, we have preemption
                else if (slice_to_go == 0 && !ready.empty())
                {
                    flag = 0;                   
                    preemption_flag = 1;
                    cpu.switch_out = cont / 2;

                    if (t <= 999)
                    {
                        cout << "time " << t << "ms: Time slice expired; process " << cpu.name << " preempted with " << cpu.burst[cpu.current].first << "ms to go";
                        report_Q(ready);
                    }
                }

                // process used up all the t_slice BUT the ready queue IS empty, we DO NOT have preemption
                else if (slice_to_go == 0 && ready.empty())
                {
                    slice_to_go = slice;
                    preemption_flag = 0;

                    if (t <= 999)
                    {
                        cout << "time " << t << "ms: Time slice expired; no preemption because ready queue is empty [Q <empty>]" << endl;
                    }

                    // and do nothing else
                }
            }
        }

        // this part add wait time for processes in ready Q, because after here, back from I/O or new arrival will be added into Q           [should count for them?]
        for (auto itr = ready.begin(); itr != ready.end(); itr++) 
        {
            itr->total_wait += 1;            
        }

        // I/O blocking
        if (io.empty())
        {
            // do nothing
        }

        else  //  1) I/O time -- 2) if finished, add to queue, no switch here 3)report
        {
            vector<Process> back;
            for (auto itr = io.begin(); itr != io.end(); itr++)
            {
                // need to use "current - 1" cuz we incremented it when the cpu-burst ended (in above).
                itr->burst[itr->current - 1].second -= 1;
                if (itr->burst[itr->current - 1].second == 0) // go back to queue, less than than 0 because last I/O is 0, then -- becomes -1
                {
                    back.push_back(*itr);
                    itr = io.erase(itr);
                    itr--;
                }
            }

            // now back has all processes completed I/O just now, add them to ready Q by name order, so sort first
            sort(back.begin(), back.end(), compare2); // sort by name

            // check ordering parameter
            if ((order.compare("END")) == 0)
            {
                for (auto i: back)
                {
                    ready.push_back(i);
                    if (t <= 999)
                    {
                        cout << "time " << t << "ms: Process " << i.name << " completed I/O; added to ready queue";
                        report_Q(ready);
                    }
                }
            }

            else if ((order.compare("BEGINNING")) == 0)
            {
                for (auto i: back)
                {
                    ready.push_front(i);
                    if (t <= 999)
                    {
                        cout << "time " << t << "ms: Process " << i.name << " completed I/O; added to ready queue";
                        report_Q(ready);
                    }

                }
            }
        }

        vector<Process> new_arrivals;
        for (auto itr = storage.begin(); itr != storage.end(); itr++)
        {
            if (itr->arrival == t)
            {
                new_arrivals.push_back(*itr);
                itr = storage.erase(itr);
                itr--;
            }
        }

        sort(new_arrivals.begin(), new_arrivals.end(), compare2); // sort by name

        // new process arrival and check ordering parameter
        if ((order.compare("END")) == 0)
        {
            for (auto i: new_arrivals)
            {
                ready.push_back(i);
                if (t <= 999)
                {
                    cout << "time " << t << "ms: Process " << i.name << " arrived; added to ready queue";
                    report_Q(ready);
                }
            }
        }

        else if ((order.compare("BEGINNING")) == 0)
        {
            for (auto i: new_arrivals)
            {
                ready.push_front(i);
                if (t <= 999)
                {
                    cout << "time " << t << "ms: Process " << i.name << " completed I/O; added to ready queue";
                    report_Q(ready);
                }

            }
        }

        if (!cpu.run && storage.empty() && io.empty() && ready.empty())
        {
            cout << "time " << t + 2 << "ms: Simulator ended for RR [Q <empty>]" << endl;
            break;
        }
    }

    outfile << "Algorithm RR" << endl;
    statistic(result, cont);
}

// Entry point of our program
int main(int argc, char* argv[]) {

    int seed        = atoi(argv[1]);
    double lambda   = stof(argv[2]);
    int bound       = atoi(argv[3]);
    int num         = atoi(argv[4]);
    int cont        = atoi(argv[5]); // context switch time
    double alpha    = stof(argv[6]);
    int slice       = atoi(argv[7]); // for RR

    string order;
    if (argc == 9)
    {
        order = argv[8];
    }

    else
    {
        order = "END";
    }

    srand48(seed);

    vector<Process> storage;
    generate_processes(storage, bound, lambda, alpha, num);

    FCFS(storage, cont);
    cout<<endl;
    SJF(storage, cont);
    cout<<endl;
    SRT(storage, cont);
    cout<<endl;
    RR(storage, cont, slice, order);

    return EXIT_SUCCESS;
}
