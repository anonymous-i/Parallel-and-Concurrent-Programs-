Title : Implementation and Analysis of Ticket lock variants

**The Ticket Lock**

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

**Naïve Ticket Lock**

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
algorithm.

![image](https://user-images.githubusercontent.com/55399754/118124651-85606f00-b413-11eb-87fa-724c6098b0b1.png)

**Partitioned Ticket Lock Algorithm**

This lock is one of the advanced variant ticket lock based algorithm proposed to 
improve the demerits that were suffered by the naïve ticket locks described in the 
above section. This lock discussed here was proposed in the literature in 2011 by Dave 
Dice,Oracle Labs[1]
As seen in the naïve ticket lock, the major backdrop came into 
picture when multiple threads started to spin on a single global variable that denoted 
which thread is currently being served, this caused numerous cache invalidations, cache 
coherence traffic, this degraded the performance of the locks. In this locking algorithm 
approach, the central idea is to have an array of now-serving variables rather than a 
single global variable. This array is local to the threads local node, where each thread 
identifies its turn using the index of its thread id on the array. This idea generalises 
to the semi-local waiting of the threads, spinning on their semi-local locations rather 
than the global location, this reduces the number cache invalidations giving rise to 
performance improvements as compared to the naïve ticket lock. This can be seen in 
the following Fig

![image](https://user-images.githubusercontent.com/55399754/118124801-ba6cc180-b413-11eb-8442-39b2732f2e37.png)

The new thread that enters into the system and contends for a lock, first gets its ticket 
value from the dispenser variable atomically, i.e it performs the atomic fetchAndAdd() 
operation, thereby getting its ticket and incrementing the ticket value for the 
consequent thread that may enter the system. The difference in this locking algorithm 
comes in terms of the current serving denoting variable, each thread has array of the 
grant variable which denotes the current serving thread in the critical section. When 
a thread initially enters the critical section. The array indexes of all the waiting threads 
are updated with the thread value of the currently being served thread. As the current 
threads finishes its task in the critical section, it atomically updates the array locations
of spinning threads with the next thread value, the thread’s value that equals to the 
new updated ticket get the critical section.

**Ticket lock with augmented array (TWA) lock**

This lock here proposes the concept of sharing of the array between the threads. This 
algorithm was proposed in the literature most recently, making it the recent 
advancement to the ticket lock algorithm. This was proposed in 2019 by Dave Dice, 
Oracle Labs and Alex Kogan,Oracle Labs [2]. This locking algorithm is a more advanced 
variant to the naïve ticket lock, where it proposes to have global array of the grant or 
currently serving variable, which is shared across the threads indexed upon their 
respective thread ids. The TWA lock improves the performance by making all the 
waiting threads to spin on their local memory indexed locations on the augmented 
global waiting array. When looked at more depths, this algorithm also promotes a way 
of routing the threads across a shorter path and a longer path. The paths are chosen 
based on a defined threshold factor, in a real working scenario the threshold is fixed 
to 1, which denotes that when there is only one more thread in the system, the thread 
can take a longer path or route which is to wait on the global grant variable, if there 
is more contention in terms of number of threads, the threads wait or spin on the 
global shared array to spin on their local indexed locations. The TWA lock basically 
is to behave comparable to the naïve ticket lock under a lower contention, but is much 
more efficient under a high contention. This can be verified in the later 
experimentation section of this report. Consider at lower contention there exists only 
one more thread that need the lock, since the threshold remains at 1, the thread takes 
up the longer route, to spin or wait on the global grant variable as shown in the Fig

![image](https://user-images.githubusercontent.com/55399754/118124932-ebe58d00-b413-11eb-9466-b2e5a7f8862b.png)

For a much higher contention, as the number of threads increases more than the 
defined threshold, the threads spin on indexed local locations on the augmented waiting 
array, this is the longer route or path taken up by the threads, it makes a sense that 
under high contention making the threads to spin on the local memory locations would 
result in less coherence traffic leading to a lower cache invalidations. This scenario is 
shown in the following Fig

![image](https://user-images.githubusercontent.com/55399754/118124977-00298a00-b414-11eb-8204-eb6f4025d299.png)

We implement the discussed algorithms in C++ programming language, each lock 
algorithm is implements as an independent class, where each class generalizes to 
consisting of two main methods, the lock() and the unlock() methods. The atomic 
considerations are taken up by the std::atomic library in the C++ std11. The class 
diagram of the overall project program is defined below in Fig 4.1. The program also 
consists of a certain helper functions that define the performance measuring procedures 
for experimentation and performance analysis of each locking algorithm.

![image](https://user-images.githubusercontent.com/55399754/118125052-1c2d2b80-b414-11eb-8667-92d73a3ef3f1.png)

**Experimentation and Observations**

In this section, we intend to analyse the algorithms with respect to the standard 
performance measurements.

**Timing Performance**

We analyse the algorithms for timing performance, wherein we basically run the 
algorithms through a set of statements, while recording the lock acquiring and releasing 
times for each thread on each lock algorithm, we also define a set of time delay 
parameters to simulate a scenario of the threads performing a complex time taking 
tasks. This procedure is defined by the method called timingTest(). In this method we 
iterate n threads over a certain number of times defined by k. In each kth iteration of 
the thread we record the time taken for the thread to acquire the lock, and then the 
thread is delayed by a certain time delay parameter to simulate a certain complex task 
being done by the thread. We then also record the time taken for the thread to release 
the lock. This procedure is done for each algorithm for an increasing number of threads 
from 1 to 10 and a fixed value of k to be 5. The delay parameter are fixed at 3 and 1 
seconds for the experiment. These specific sets of experimentation parameters are 
performed upon each algorithm. The timing tests are recorded in the order of seconds.

![image](https://user-images.githubusercontent.com/55399754/118125165-41ba3500-b414-11eb-8bdd-c283a1d3f971.png)

![image](https://user-images.githubusercontent.com/55399754/118125202-4c74ca00-b414-11eb-917b-9d8c837bf136.png)

The following graph plot is obtained for the timing measurements recorded for each 
lock algorithm. The graph plot in Fig 5.4 shows the timing performance for the naïve 
ticket lock, the plot in Fig 5.5 shows the timing performance for the TWA and 
Partition locking algorithms.

![image](https://user-images.githubusercontent.com/55399754/118125258-5dbdd680-b414-11eb-8ca1-ad40c1c35e64.png)

![image](https://user-images.githubusercontent.com/55399754/118125297-6a422f00-b414-11eb-95ed-fc1c489d0910.png)

**Throughput Performance**

The next performance measurement for the locks is done with respect to the 
throughput performance of the algorithms, in this experimentation setup, a thread is 
made to continuously go through the series of locks and unlocks. These numbers of 
locking executions and unlock executions are recorded, and later we take the numbers 
at specific period of time intervals as shown below. The helper method 
throughPutTest() is used for this regard.

![image](https://user-images.githubusercontent.com/55399754/118125370-7fb75900-b414-11eb-96ec-a49a779607dc.png)

![image](https://user-images.githubusercontent.com/55399754/118125433-9493ec80-b414-11eb-8de1-b9e57c1d12a8.png)

**Conclusion**

Through this project we intended, to first implement the advanced variants of the 
ticket spin locks, that was initially employed in the linux system kernels, and also 
being reemployed into the popular linux distribution, the Red Hat linux kernels. We 
implemented the two proposed variants in literature, one of which was proposed earlier 
and the other being proposed in literature most recently. Through the experimentation, 
it is analysed and verified that the TWA lock algorithm which was the most recently 
proposed variant of the ticket lock did outperform the other lock algorithms and 
showed a definite performance boost. This project has provided a great foundation for 
understanding the synchronization problem in the parallel computing domain, leading 
to understand the insights to the queue based spin locks and their performance 
characteristics in a more practical manner.
