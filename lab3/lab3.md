# COL331 Lab 3: Tinkering with the Scheduler
The `xv6` scheduler implements a primitive round robin scheduling algorithm. Although this scheduling algorithm ensures that no process is starved of CPU time, there is no notion of "priority" or "types" of processes - all processes get equal CPU time irrespective of the importance of the tasks they perform. 

In this lab, you will implement another fairly simple scheduling algorithm, which will allow processes to specify whether they are "foreground" or "background", and they will be allocated CPU time accordingly. 

## Introduction

Broadly, processes can be classified into the following two types: 

- **Foreground Processes**: These are tasks which require a lot of user interaction, and are time-sensitive. For example, a music player on a phone, or a text editor like Vim running in the terminal. Such tasks need to be scheduled regularly for a smooth user experience.  

- **Background Processes**: These are tasks which are not very time sensitive. These may be long computational tasks which do not require much user interaction. For example, downloading large data from a server may be a background process. 

We would like to add a functionality using which a process can indicate to the operating system whether it is a foreground process or a background process. The scheduler must also be designed so that time slices are given to processes according to the preference each process indicates. 

## System Calls 
In the first part of the lab, you must implement two system calls. In `proc.h`, there is a new field (`int policy`) which need to be set and used by the below system calls:

- `int set_sched_policy(void)`: Using this system call, a process can indicate to the operating system whether it is a foreground or a background process. A foreground process would have `policy=0`, while a background process would have `policy=1`. The system call should return `0` if the policy for the process was set successfully, and `-22` in all other cases. 

- `int get_sched_policy(void)`: Using this system call, a process can get the scheduling policy for itself.

For reference, you may refer to the implementations of any of the other system calls in `syscall.h`. You may fill in the definitions of the system call functions in `sysproc.c` (a basic skeleton is provided).

## Scheduler 
The implementation of the `xv6` scheduler is in the `scheduler()` function in `proc.c`, which currently implements the round-robin scheduling algorithm. 

Your task is to modify the `scheduler` function so that it picks tasks for scheduling as per their scheduling policy, i.e. whether they are foreground or background. 

The scheduling algorithm is simple in principle: Overall, foreground processes must be allotted 90% of the CPU time, and background processes 10%. Within foreground and background processes, we may continue to schedule in a round-robin manner. 

You may implement this scheduling algorithm in any way that you like. However, you must ensure that processes are not starved of CPU time. If there are no background processes, you may switch back to running foreground processes, and vice-versa. 

## Testing 
To create multiple processes, as in `p18-sched`, we have called `pinit` twice in `main`. The `pinit` function has also been modified to take an argument `policy`, which will set the scheduling policy of the process. Currently, there is an infinite task in `init.c`, which prints to the display after a certain number of iterations. 

In the current setup, starting up `xv6` should produce an output like the following on the screen: 
```
Task done by process with sched policy 0
Task done by process with sched policy 0
Task done by process with sched policy 0
Task done by process with sched policy 0
Task done by process with sched policy 0
Task done by process with sched policy 0
Task done by process with sched policy 0
Task done by process with sched policy 0
Task done by process with sched policy 0
Task done by process with sched policy 1
...
```
i.e. the statement "Task done by process with sched policy 0" should be 9x as frequent as the statement "Task done by process with sched policy 1", as per the scheduling algorithm we have discussed. 

You can extend upon this technique to test the completeness and robustness of your implementation of your algorithm further. 

---

## Deliverables 
You will be required to submit the entire ```col331-lab3``` folder with your changes. The code should have the implementation of the system calls and the modified scheduling algorithm as described above. 

In the ```col331-lab3``` root directory, run the following commands:

```sh
make clean
tar czvf lab3_<entryNumber>.tar.gz *
``` 

This will create a tarball with the name, ```lab3_<entryNumber>.tar.gz``` in the same directory. Submit this tarball on Moodle. Entry number format: 2020CS10567 (All English letters will be in capitals in the entry number).  
For eg. If your entry number is 2023JCS2564, you will run the following command in the ```col331-lab3``` root directory:

```sh
make clean
tar czvf lab3_2023JCS2564.tar.gz *
```

## Auto-grader

Your submission will be evaluated by our autograder which is available [here](https://csciitd-my.sharepoint.com/:u:/g/personal/jcs232564_iitd_ac_in/EdqQ0HnXw6pPr8Bs8_TA9DMBAMdhkg6i3CEGRqTCgzjPUg?e=dmH4Q3). Follow the below instructions to run the autograder locally on your submission:

- Download and unzip the lab3_check_scripts.zip.
  ```bash
  unzip lab3_check_scripts.zip
  cd lab3_check_scripts
  ```
- Use the following command to run the auto-grader
  ```bash
  bash check.sh <path/to/lab3_<entryNumber>.tar.gz>
  ```
- The given sample script consists of 2 basic test cases. Note that your submission will be evaluated by hidden test cases.


# Logistics

- **Submission Deadline:** The final date for submitting your lab is March 9, 2024, at 11:59 PM.
- **Part-A Grading:** Part-A of the lab carries a total of 20 marks, contributing 5% to the overall course grade.
- **Part-B Grading:** Part-B of the lab is worth 4 marks and accounts for 1% of the total course grade.