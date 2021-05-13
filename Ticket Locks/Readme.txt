Title : Implementation and Analysis of Ticket lock variants
Course : CS5300 - Parallel and Concurrent Programming

Date : 23 December 2020

Submitted by : Tahir Ahmed Shaik, CS20MTECH14007
________________________________________________________________________________________

1. Specifications :

Operating System : Windows 10

Programming Language : C++ Std11

CPU : Intel Core i3

IDE : Dev C++ Ver. 5.11


2. Instructions : 

-->Run the file "project_source.cpp" to execute the program. There are several sections that can be commented/ Uncommented for choosing the lock type and the testing type such as timing test or throughput test. These sections are mentioned in the comments specifically in the program.

--> For the timing test, the input parameters include in the order such as n-number of threads, k- no iterations per thread, lambda1- time delay parameter, lambda2- time delay parameter.

-->The output files for both the tests are generated in same directory, as "Timing.txt" and "Throughput.txt" Respectively.


The ticket lock is one of the spin lock algorithms, in the context of the synchronization 
problem to solve the critical section problem. The main scope of the synchronization 
is to allow and control the thread accesses and execution of the critical section, which 
consists of several global shared common resources. These ticket locks are queue based 
and offer the FIFO order correctness for the threads. The basic concept of these locks 
is the usage of logical tickets, to allow the threads the critical section. The background 
working strategy of these locks if similar to the ticketing process used in the railway
reservation or bus reservation system, where each thread takes a certain ticket value 
upon its arrival and there exists a certain scope of presenting the ticket value that is 
being currently served. The thread compares its ticket value with the currently serving 
ticket number or the granted ticket number, if it finds they are equal the thread gets 
into the critical section.

![image](https://user-images.githubusercontent.com/55399754/118124097-bc825080-b412-11eb-945f-3b5d9b820e86.png)

Naïve Ticket Lock
This is the most traditional and the foundation lock algorithm, this is the fundamental 
algorithm upon which the other two advanced variations are discussed in the project. 
These locks were in the past employed in the linux kernels systems in early 2008. These 
locks were the re-employed in the Red hat linux systems [3]. The basic functioning is 
very simple and concise for this lock algorithm, it can be related to classical example 
of railway ticket reservation system, or a bakery, food take aways where each person 
is given a unique ticket, the person gets his turn when the system starts to serve his 
ticket number. This same principle has been implied upon for these locks. Any new 
thread that wishes to enter the critical section, basically fetches a unique ticket value 
and increments the value atomically for the next thread, on other hand there exists 
another variable that denotes the ticket value, that is being served currently in the 
critical section, all the other threads spin until the current serving ticket value is not 
incremented to next threads ticket value by the thread that is present in the critical 
section upon its exit. As soon as the spinning thread, sees the current serving ticket 
value is equal to its ticket value, it gets the critical section. The practical definition 
and the defining of these can be clearly understood in the further sections that follow. 
It can be intuitively seen, that as threads emerge in the system trying to acquire the 
locks for the critical section, it creates an implicit queue structure of the threads in 
the system. Hence, this locking algorithm promotes the queue based spin lock 
algorithm




