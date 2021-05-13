/*
******************************************************************************************************

CS5300 : Parallel and Concurrent Programming

Course Project

Title : Implementation and analysis of Ticket Lock variant algorithms 

Submitted by : Tahir Ahmed Shaik,CS20MTECH14007, Indian Institute of Technology Hyderabad

Date: 23 December 2020
******************************************************************************************************
*/

#include<iostream>
#include<thread>
#include<vector>
#include<atomic>
#include<chrono>
#include<fstream>
#include<time.h>
#include<stdbool.h>
#include<math.h>
#include<string>

using namespace std;

int n,k,lambda1,lambda2; // Experimentation Parameters

ofstream tpFile("ThroughPuts.txt"); //Genberates the throughputs i.e number of locks and unlocks processed 
ofstream timeFile("Times.txt");
int threshold_limit = 1; // The thresh hold limt for TWA lock
time_t t= time(NULL);
tm* tPtr= localtime(&t);

/*
  The following is an helper class, that records the thread information, in the thread's local space
*/
class ThreadInfo
{
public:
	double entryTime,exitTime;
};

vector<ThreadInfo> tinfo;

/*The NaiveTicket Lock algorithm class
This uses the global, grant variable on which all threads spin.
Each thread also has its own ticket value, in the threads lock node.

*/

class NaiveTicketLock
{
private:
	typedef struct TicketNode
	{
		atomic<int> myTicket;
	};
	
	atomic<int> now_serving;
	TicketNode *nodes;
public:
	NaiveTicketLock(){}
	
	NaiveTicketLock(const NaiveTicketLock &){}
	
	NaiveTicketLock(int cap)
	{
		nodes = new TicketNode[cap];
		
		for(int i=0;i<cap;i++)
		{
			nodes[i].myTicket.store(0);
			now_serving.store(0);
		}
	}
	
	void lock(int thread_id)
	{
		while(nodes[thread_id].myTicket.fetch_add(1) != now_serving.load()){}
	}
	
	void unlock(int thread_id)
	{
		auto currentToken = now_serving.load();
		now_serving.store(currentToken+1);
	}
};

/*
   TWA lock algorithm implementation.
   A recent variant of the standard ticket locks, which uses the concept of local spinning, and defines the 
   routes for each thread to take based on contention conditions.
   Uses an augmented wating array that consists of the currently serving ticket values, which is shared between all threads.
*/
class TicketAugmentArrayLock
{
private:
	typedef struct TAANode
	{
		atomic<int> myTicket;
	};
	atomic<int> nowServing;
	std::atomic_size_t waitArray[100];
	TAANode *nodes;
public:
	TicketAugmentArrayLock(const TicketAugmentArrayLock &){}
	TicketAugmentArrayLock(int cap)
	{
		nodes = new TAANode[cap];
		
		for(int i=0;i<cap;i++)
		waitArray[i].store(0);
	}
	
	void lock(int thread_id)
	{
		auto static myTicket = nodes[thread_id].myTicket.fetch_add(1);
		auto static more = myTicket - nowServing.load();
		if(more == 0)
		return;
		
		if(more>threshold_limit)
		{
			while(1)
			{
				auto u = waitArray[thread_id].load();
				more = myTicket - nowServing.load();
				if(more<=threshold_limit)
				break;
				
				while(waitArray[thread_id].load() == u){}
			}
		}
		
		while(myTicket != nowServing.load()){}
	}
	
	void unlock(int thread_id)
	{
		nowServing.store(nowServing.load()+1);
		static int k = nowServing.load();
		
		waitArray[k+threshold_limit].fetch_add(1);
	}
};

/* Partitioned Ticket Lock (PTL) lock algorithm class.
   This lock defines the semi-local waiting, where each thread wait on a local defined array for each lock node instance of the thread.
*/
class PartitionedTicketLock
{
private:
	struct lockNode
	{
		atomic<int> myTicket;
		atomic_size_t nowServing[100];
	};
	
	lockNode *nodes;
public:
	PartitionedTicketLock(int cap)
	{
		nodes = new lockNode[cap];
		for(int i=0;i<cap;i++)
		{
			for(int j=0;j<cap;j++)
			nodes[i].nowServing[j].store(0);

		}
		
	}
	
	void lock(int thread_id)
	{
		static int current = nodes[thread_id].nowServing[thread_id].load();
		static int myTicket = nodes[thread_id].myTicket.fetch_add(1);
		while( myTicket != current){}
	}
	
	void unlock(int thread_id)
	{
		for(static int i=0;i<n;i++)
		{   
		    for(static int j=0;j<n;j++)
		    {
		    int current = nodes[i].nowServing[j].load();
			nodes[i].nowServing[j].store(current+1);
			}
		}
	}
};

//Defining the lock objects, uncomment/ comment to use each specific lock type.
TicketAugmentArrayLock TestObj = TicketAugmentArrayLock(n);

//NaiveTicketLock TestObj = NaiveTicketLock(n);

//PartitionedTicketLock TestObj = PartitionedTicketLock(n);

//A helper function for performance analysis, which defines the procedure for timing performance test.
void timingTest(int thread_id)
{
    ThreadInfo tin = ThreadInfo();
		for(int i=0;i<k;i++)
		{
		t = time(0);
		tPtr = localtime(&t);
		//cout<<"Lock request at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" for "<<i<<" th time"<<endl;
		timeFile<<"Lock request at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" for "<<i<<" th time"<<endl;
		auto start_clock = chrono::steady_clock::now();
		TestObj.lock(thread_id);
		auto stop_clock = chrono::steady_clock::now();
		auto diff_sec = stop_clock - start_clock;
	    double entrytm = chrono::duration <double,micro> (diff_sec).count()/1e+6;
	    tin.entryTime = entrytm;
		t = time(0);
		tPtr = localtime(&t);
		cout<<"Lock entry at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" for "<<i<<" th time"<<endl;
		timeFile<<"Lock entry at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" for "<<i<<" th time"<<endl;
		this_thread::sleep_for(chrono::seconds(lambda1));
		t = time(0);
		tPtr = localtime(&t);
		cout<<"Lock exit request at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" for "<<i<<" th time"<<endl;
		timeFile<<"Lock exit request at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" for "<<i<<" th time"<<endl;
		auto start_clock_exit = chrono::steady_clock::now();
		TestObj.unlock(thread_id);
		auto stop_clock_exit = chrono::steady_clock::now();
		auto diff_sec_exit = stop_clock_exit - start_clock_exit;
	    double exittm = chrono::duration <double,micro> (diff_sec_exit).count()/1e+6;
	    tin.exitTime = exittm;
		t = time(0);
		tPtr = localtime(&t);
		//cout<<"Lock exit at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" for "<<i<<" th time"<<endl;
		timeFile<<"Lock exit at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" for "<<i<<" th time"<<endl;
		tinfo.push_back(std::move(tin));
		this_thread::sleep_for(chrono::seconds(lambda2));
		}
}

static int numLocks,numUnlocks;

//Another helper function, that defines the throughput performance testing on the locking algorithms.
void throughPutTest(int thread_id)
{
	t = time(0);
	tPtr = localtime(&t);;
	TestObj.lock(thread_id);
	numLocks++;
	tpFile<<"Thread entry intitated Lock for "<<numLocks<<" time"<<endl;
	//this_thread::sleep_for(chrono::seconds());
	TestObj.unlock(thread_id);
	numUnlocks++;
	tpFile<<"Thread entry intitated UnLock for "<<numUnlocks<<" time"<<endl;
	tpFile<<"Time :"<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<endl;
	//this_thread::sleep_for(chrono::seconds());
}

int main()
{	
	vector<thread> threads;
    //--------------->>Uncoment/Comment this section for timing performance test<<----------------
	scanf("%d%d%d%d",&n,&k,&lambda1,&lambda2);
	for(int i=0;i<n;i++)
	{
		thread t(timingTest,i);
		threads.push_back(std::move(t));
	}
	
	for(auto &t : threads)
	{
		t.join();
	}
	
	double avgEntry,avgExit;
	for(ThreadInfo t : tinfo)
	{
		avgEntry += t.entryTime;
		avgExit += t.exitTime; 
	}
	
	avgEntry = avgEntry/tinfo.size();
	avgExit = avgExit/tinfo.size();
	
	cout<<"\nThreads\tAverageEntry Time\tAverage Exit Time\n";
	timeFile<<"\nThreads\tAverageEntry Time\tAverage Exit Time\n";
	cout<<n<<"\t"<<avgEntry<<"\t\t"<<avgExit<<endl;
	timeFile<<n<<"\t"<<avgEntry<<"\t\t"<<avgExit<<endl;
	
	threads.clear();
	//---------->>Comment/ Uncomment this section for throughput test<<--------------------
	/*
	while(true)
	{
		thread t(throughPutTest,0);
		threads.push_back(std::move(t));
	
	for(auto &t : threads)
	{
		t.join();
	}
	threads.clear();
	}
	cout<<"Num Locks: "<<numLocks<<" Num Unlocks: "<<numUnlocks<<endl;
	cout<<"Done";
	*/
}
