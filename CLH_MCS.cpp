/*
Assignment 4 : implementation and Comparison of CLH and MCS locks

Course : CS5300- Parallel and Concurrent Programming

Submitted by : Tahir Ahmed Shaik, CS20MTECH14007, Indian Institute of Technology Hyderabad

Date : 01/12/2020
*/

#include<iostream>
#include<thread>
#include<fstream>
#include<chrono>
#include<atomic>
#include<vector>
#include<time.h>
#include<unistd.h>
#include<fstream>

using namespace std;

ofstream OutFile("output.txt");
ifstream InFile("inp-params.txt");

time_t t= time(NULL);
tm* tPtr= localtime(&t);

vector<double> entryTimes;
vector<double> exitTimes;

//Thread Local Class to save each threads local space information at particular instance in the test procedure
class Thread_Local
{
public:
	int tid,iter;
	int entryReqHr,entryReqMin,entryReqSec,exitReqHr,exitReqMin,exitReqSec,entryHr,entryMin,entrySec,exitHr,exitMin,exitSec;
	double entryTime,exitTime;
};

vector<Thread_Local> threads_info;
int n,k,lambda1,lambda2;

/* The CLH Lock is defined below in the CLH class, the node taken up by each thread is defined by the internal
   nested node class CLHQnode. This Lock node is the core structure of the locking algorithm which
   defines a single boolean vairable called locked, this defines if a thread has hold on the critical
   section to the predessor threads in the queue.
   
   The CLH Lock main class defines two thread local variables my_predessor and my_node and has a global atomic
   referenced tail that points to the new nodes entering the queue.
*/
class CLH
{
private:
	class CLHQnode
	{
	public:
	CLHQnode()noexcept{}
	CLHQnode(const CLHQnode &s)
	{	
	this->locked = s.locked;
	}
	bool locked;
	};
	
	atomic<CLHQnode> tail;
public:	
	CLHQnode my_predessor;
	CLHQnode my_node;
	
	CLH(){}
	
	CLH(const CLH &){}
	
	void lock()
	{
		CLHQnode qnode = my_node;
		qnode.locked = true;
		static CLHQnode pred = tail.exchange(qnode);
		my_predessor = pred;
		while(pred.locked){}
	}
	void unlock()
	{
		static CLHQnode qnode = my_node;
		my_node.locked = false;
		my_node = my_predessor;
	}
	
};
//End of CLH lock space

/*
  The Following space defines the MCS Lock algorithm, this lock on contrast with the CLH lock defines a 
  explicit linked list for the thread nodes. The node structure for each thread is defined by the
  internal nested class MSCNode. This class defines the two varibales, the Locked varibale true indicates a 
  node hoilding the lock and false indicating the critical section to be free.
  The explicit linked list is maintained by the pointer varable next, that points to the next successor node in the list.
  Each thread spins on its own node and waits for its successor node to change it to false.
*/

	class MCSNode
	{
	public:
		bool locked;
		MCSNode *next;
	};
	
	atomic<MCSNode> tail;
class MCS
{

public:
	MCSNode my_node;
MCS(){}   
MCS (const MCS &){}
void lock()
{
	static auto qnode = MCSNode();
	static auto predessor = tail.exchange(qnode);
	if(&predessor != nullptr)
	{
		qnode.locked = true;
		predessor.next = &qnode;
			
		while(!qnode.locked){}
	}
}
	
void unlock()
{
static MCSNode qnode = my_node;
if(qnode.next == nullptr)
{
	if(tail.compare_exchange_strong(qnode,MCSNode()))
		return;
			
	while(qnode.next == nullptr){}
}
	qnode.next->locked = false;
	qnode.next = nullptr;
}
};

//End of MCS Lock Space

//Global Lock Objects, Uncomment/Comment 148 and 150 to test each lock

//CLH Test = CLH();

MCS Test = MCS();

/* 
   The Testing Procedure which is invoked by each thread to write to the stdout, it is iterated for
   K times provided by the input. Also each threads information at instance is saved into a thread local space
   using a class defined above called Thread_Local. This is used to later output to the output file,
   Commenting out the lines 161 and 197 is required for verification of the threads accesses.
*/

void testCS(int thread_id)
{
	Thread_Local tl = Thread_Local();
	for(int i=0;i<k;i++)
	{
		tl.tid = thread_id;
		tl.iter = i;
		t = time(0);
	    tPtr = localtime(&t);
		//cout<<i<<"th CS entry request at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<"by thread "<<thread_id<<endl;
		tl.entryReqHr = tPtr->tm_hour;
		tl.entryReqMin = tPtr->tm_min;
		tl.entryReqSec = tPtr->tm_sec;
		auto start_clock = chrono::steady_clock::now();
		Test.lock();
		auto stop_clock = chrono::steady_clock::now();
		t = time(0);
	    tPtr = localtime(&t);
	    auto diff_sec = stop_clock - start_clock;
	    double entrytm = chrono::duration <double,micro> (diff_sec).count()/1e+6;
	    tl.entryTime = entrytm;
	    entryTimes.push_back(std::move(entrytm));
		cout<<i<<"th CS entry at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<"by thread "<<thread_id<<"Entry Time taken "<<chrono::duration <double,micro> (diff_sec).count()/1e+6<<endl;
		tl.entryHr = tPtr->tm_hour;
		tl.entryMin = tPtr->tm_min;
		tl.entrySec = tPtr->tm_sec;
		this_thread::sleep_for(std::chrono::seconds(lambda1));
		t = time(0);
	    tPtr = localtime(&t);
	    cout<<i<<"th CS exit request at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<"by thread "<<thread_id<<endl;
	    tl.exitReqHr = tPtr->tm_hour;
		tl.exitReqMin = tPtr->tm_min;
		tl.exitReqSec = tPtr->tm_sec;
		auto start_clock_exit = chrono::steady_clock::now();
		Test.unlock();
		auto stop_clock_exit = chrono::steady_clock::now();
	    t = time(0);
	    tPtr = localtime(&t);
	    tl.exitHr = tPtr->tm_hour;
		tl.exitMin = tPtr->tm_min;
		tl.exitSec = tPtr->tm_sec;
	    auto diff_sec_exit = stop_clock_exit - start_clock_exit;
	    double exittm = chrono::duration <double,micro> (diff_sec_exit).count()/1e+6;
	    exitTimes.push_back(std::move(exittm));
	    tl.exitTime = exittm;
	    //cout<<i<<"th CS exit at "<<tPtr->tm_hour<<":"<<tPtr->tm_min<<":"<<tPtr->tm_sec<<"by thread "<<thread_id<<" Exit Time taken "<<chrono::duration <double,micro> (diff_sec_exit).count()/1e+6<<endl;
	    threads_info.push_back(std::move(tl));
		this_thread::sleep_for(std::chrono::seconds(lambda2));
 }
}

int main()
{
	string param1,param2,param3,param4;
	getline(InFile,param1);
	getline(InFile,param2);
	getline(InFile,param3);
	getline(InFile,param4);
	n = stoi(param1);
	k = stoi(param2);
	lambda1 = stoi(param3);
	lambda2 = stoi(param4);
	
	vector<thread> threads;
	
	for(int i=0;i<n;i++)
	{
		thread t(testCS,i);
		threads.push_back(move(t));
	}
	
	for(auto &t : threads)
	{
		t.join();
	}
	
	double total,averageEntry,averageExit;
	
	for(int i=0;i<entryTimes.size();i++)
	{
		total += entryTimes[i]; 
	}
	
	//Calculating average entry and exit times through the time information stored in the thread entry and exit time vectors
	averageEntry = total/entryTimes.size();
	
	total = 0;
	for(int i=0;i<exitTimes.size();i++)
	{
		total += exitTimes[i]; 
	}
	
	averageExit = total/exitTimes.size();
	
	cout<<"\n Average Entry Time :"<<averageEntry<<endl;
	cout<<"\n Average Exit Time :"<<averageExit<<endl;
	
	//Printing the thread local information to output file "Output.txt"
	OutFile<<"\t\t\t*********Threads Information*********************\n\n";
	for(Thread_Local t : threads_info)
	{
		OutFile<<t.iter<<" th Entry Request by "<<t.tid<<" Thread at "<<t.entryReqHr<<":"<<t.entryReqMin<<":"<<t.entryReqSec<<endl;
		OutFile<<t.iter<<" th Entry by "<<t.tid<<" Thread at "<<t.entryHr<<":"<<t.entryMin<<":"<<t.entrySec<<" Time Taken : "<<t.entryTime<<endl;
		OutFile<<t.iter<<" th Exit Request by "<<t.tid<<" Thread at "<<t.exitReqHr<<":"<<t.exitReqMin<<":"<<t.exitReqSec<<endl;
		OutFile<<t.iter<<" th Exit by "<<t.tid<<" Thread at "<<t.exitHr<<":"<<t.exitMin<<":"<<t.exitSec<<" Time Taken "<<t.exitTime<<endl;
	}
	
	OutFile<<"\n**************************************************************";
	OutFile<<"\n\tAverage Entry Time : "<<averageEntry<<" Seconds"<<endl;
	OutFile<<"\n\tAverage Exit Time : "<<averageExit<<" Seconds"<<endl;
	OutFile<<"\n**************************************************************";
	cout<<"\n Output written to file (output.txt)"<<endl;
	OutFile.close();
	
	return 0;
}
