/*
Topic : Implementing and analysing the performance between locking algorithms between
        1. Filter Lock
        2. Peterson Tree Lock
Author : Tahir Ahmed Shaik
Date : 
*/
#include<iostream>
#include<thread> //The standard thread library for C++ for creation and handling of multiple threads
#include<fstream> // File management library for input and output of data in files
#include<time.h>
#include<chrono> //Used for defining the delay for threads in the order of seconds.
#include<vector> // This library provided the dynamic storage spaces 
#include<stdlib.h>
#include<cmath>
#include <unistd.h>

using namespace std;

int n,k,lambda1,lambda2;

ifstream InFile("inp-parms.txt");
/* 
   Global Variables declaration
   n = number of threads
   k = number of Critical Section accesses of each thread
   lambda1,lamda2 = delay time in seconds for making threads to wai, for emulating computaion operations
                    by the threads.
   OutFile = Output File Stream Object for streaming the output toh the file in the directory.
*/

/* The time object and the time structure type used for getting the timestamps for each thread operation */
time_t t= time(NULL);
tm* tPtr= localtime(&t);

/* Filter lock Algorithm Space */

/* This function is used for storing the thread Ids used in the waiting loop in the lock() function
   for fetching the thread ids to identify threads at various levels */

int thread_ids[100],pointer=-1;
void pushThread(int item)
{
 thread_ids[++pointer] = item;	
}

/* The Filter Lock Class Which implements the Filter Locking Algorithm, Which basically works on the 
   principle of filtering threads at each level, until a single thread access the Critical section. This
   Algorithm works for 'n' threads.
   
   lock() - This function locks the section from multiple access by the threads running concurrently
   unlock() - This function unlocks the section and allows access to other waiting threads.
*/

class FilterLock
{
private:
	
	   int* level;  // The Level array that maintins level information for each thread
	   int* victim;	// Victim variable that identifies the victim at each level 
public:
	FilterLock() 
	{
		this->level = new int[n];
		this->victim = new int[n];
		for(int i=0;i<n;i++)
		this->level[i] = 0;
	}
	
	void lock(int id)
	{
		int thread_id = id;
		for(int i=1;i<n;i++)
		{
			this->level[thread_id] = i;
			this->victim[i] = thread_id;
			CHECK_THREAD: for(int k : thread_ids)
			            {
					        if(k!=thread_id && this->level[k]>=i&& this->victim[i]==thread_id)
					        goto CHECK_THREAD;
				        }
		}
	}
	
	void unlock(int id)
	{
		int thread_id = id;
		this->level[thread_id] = 0;
		
	}
};

//End of Filter Lock Space

/* Peterson Tree Lock Algorithm Space */
   
/* 
    Peterson Lock Class
    This class is a standard Peterson implementation for each instance of the Peterson Lock Node, of
    the Peterson tree. This class acts for every two threads, and multiple such two threaded locks
    are used as the leaf nodes of the Peterson tree structure for n threads

*/

class PetersonLock
{
private:
	int lockn;
	int numThreads = 2;
	bool flag[2];
	int victim;
public:
	PetersonLock()
	{
		for(int i=0;i<2;i++)
		flag[i] = false;
	}
	
	void lock(int threadId)
	{
		int i= threadId%2;
		int j = 1-i;
		flag[i] = true;
		victim = i;
		while(flag[j]&& victim ==i){};
	}
	
	void unlock(int threadId)
	{
		flag[threadId%2] = false;
	}
};

/* 
   Peterson Lock Tree Class
   This is the main Class of the PTL algorithm which builds the peterson tree with every instance of
   the two thread peterson lock as its leaves as defined above. Each leaf node is attached with two threads.
   This class has two functions
   
   lock() - the lock() function allows a thread to locks the node as it moves above the level from the
            leaf to the root. During this it locks the path from its leaf to root.
   
   unlock() - This function is used by the thread to unlock the path and the nodes that were locked by the
              thread during its locking phase.
*/
class PetersonLockTree
{
private:
	int numThreads;
	vector<PetersonLock> pTree;  // The Lock instance object array of the peterson lock type
	int numLock;                

public:
	PetersonLockTree(int numofthreads)
	{
		this->numThreads = numofthreads;
		this->numLock = (int) numThreads/2;
		for(int i=0;i<sizeof(pTree);i++)
		{
			pTree.push_back(std::move(PetersonLock()));
		}
	}
	
	void lock(int tid)
	{
		vector<int> lockway;      // The Lock Path array that keeps track of the path traversed by the thread from leaf to root
		int i = tid;
		int lockAccuire = floor(i/2);   
		
		while(lockAccuire>=1)       // Wait loop where each thread waits as it still accuires the locks 
		{
			pTree[lockAccuire].lock(i);
			lockway.push_back(std::move(lockAccuire));
			i= lockAccuire % 2;
			lockAccuire /=2;
		}
	}
	
	void unlock(int thid)
	{
		vector<int> lockway;
		int j = thid;
		int lp = this->numLock - 1;
		
		while(lp>=0)          //All Locks held are unlocked and lockpaths are removed
		{
			int unlocked = lockway.at(lp);
			if(lp == 0)
			{
				pTree[unlocked].unlock(j%2);
				break;
			}
			else
			{
				pTree[unlocked].unlock((lockway.at(lp-1))%2);
				lp--;
			}
		}
	}
	
};

/* End of PetersonTreeLock space */

FilterLock fl = FilterLock();    // Filter Lock Object
PetersonLockTree pt = PetersonLockTree(n);  // PetersonTreeLock object

void testPTLock(int id)     // Peterson Tree Lock testing subroutine
{
int thread_id = id;
for(int i=0;i<k;i++)
{   
    t = time(0);
	tPtr = localtime(&t);	
	auto start_clock = chrono::steady_clock::now();
	//cout<<i<<" th CS Entry Request at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<"\n";
	pt.lock(thread_id);
	auto stop_clock = chrono::steady_clock::now();
	t = time(0);
	tPtr = localtime(&t);
	auto diff_sec = stop_clock - start_clock;
	cout<<i<<" th CS Entry at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" Time taken "<<chrono::duration <double,micro> (diff_sec).count()/1e+6<<"\n";
	std::this_thread::sleep_for(std::chrono::seconds(lambda1));
	t = time(0);
	tPtr = localtime(&t);
	cout<<i<<" th CS Exit request at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<"\n";
	auto start_clock_exit = chrono::steady_clock::now();
	t = time(0);
	tPtr = localtime(&t);
	pt.unlock(thread_id);
	auto stop_clock_exit = chrono::steady_clock::now();
	auto diff_sec_exit = stop_clock_exit - start_clock_exit;
	//cout<<i<<" th CS Exit at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" Time taken "<<chrono::duration <double,micro> (diff_sec_exit).count()/1e+6<<"\n";
	std::this_thread::sleep_for(std::chrono::seconds(lambda2));
}
}

void testFLock(int id)     // Filter Lock testing subroutine
{
int thread_id = id;
for(int i=0;i<k;i++)
{   
   
    t = time(0);
	tPtr = localtime(&t);
	auto start_clock = chrono::steady_clock::now();	
	cout<<i<<" th CS Requested at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<"\n";
	fl.lock(thread_id);
	auto stop_clock = chrono::steady_clock::now();
	t = time(0);
	tPtr = localtime(&t);
	auto diff_sec = stop_clock - start_clock;
	cout<<i<<" th CS Thread Entry at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" Time taken "<<chrono::duration <double,micro> (diff_sec).count()/1e+6<<"\n";
	std::this_thread::sleep_for(std::chrono::seconds(lambda1));
		t = time(0);
	tPtr = localtime(&t);
	cout<<i<<" th CS Exit request at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<"\n";
	auto start_clock_exit = chrono::steady_clock::now();
	t = time(0);
	tPtr = localtime(&t);
	fl.unlock(thread_id);
	auto stop_clock_exit = chrono::steady_clock::now();
	auto diff_sec_exit = stop_clock_exit - start_clock_exit;
	cout<<i<<" th CS Exit at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<" by thread "<<thread_id<<" Time taken "<<chrono::duration <double,micro> (diff_sec_exit).count()/1e+6<<"\n";
	std::this_thread::sleep_for(std::chrono::seconds(lambda2));
}
}


int main()
{
	std::vector<std::thread> threads;
	string param1,param2,param3,param4;
	getline(InFile,param1);
	getline(InFile,param2);
	getline(InFile,param3);
	getline(InFile,param4);
	n = stoi(param1);
	k = stoi(param2);
	lambda1 = stoi(param3);
	lambda2 = stoi(param4);
	cout<<"------->Filter Lock Algorithm<----------------\n";	
	for(int i=0;i<n;i++)
	{
		pushThread(i);
		thread t(testFLock,i);
		threads.push_back(std::move(t));
	}	
	
	for(auto &t: threads)
	{
    	t.join();
	}
	
	threads.clear();
	
	usleep(5000);   //Sleeping to prevent overwriting of unlocked writes to the file
	cout<<"\n\n------->Peterson Tree Lock Algorithm<----------------\n";	
	for(int i=0;i<n;i++)
	{
		thread t1(testPTLock,i);
		threads.push_back(std::move(t1));
	}	
	
	for(auto &t1: threads)
	{
    	t1.join();
	}
	
	threads.clear();
	
	cout<<"Done!!\n";
}
