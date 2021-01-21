/*
*********************************************************************************************
Implementation of the Atomic MRMW Snapshot

Submitted by Tahir Ahmed Shaik, CS20MTECH14007, Indian Institue of Technology Hyderabad

Course : CS5300 Parallel And Concurrent Programming

Date : November 2020

*********************************************************************************************
*/
#include<iostream>
#include<thread>
#include<fstream>
#include<string>
#include<cstdlib>
#include<atomic>
#include<time.h>
#include<unistd.h>
#include<chrono>
#include <iomanip>
#include<vector>

using namespace std;


time_t t= time(NULL);
tm* tPtr= localtime(&t);

ifstream inp_file("inp-parms.txt");
ofstream out_file("WFA_MRMW_THREADS_INFO.txt");
ofstream out_file_snap("WFA_MRMW_SNAPS.txt");

int num_threads=0;
int k,t1,t2;
int M=20;

class Thread_Local                    //Class used to store thread information in its Local Space
{
public:
	int thread_id;
	int value;
	long stamp_hr,stamp_min,stamp_sec;
	
	Thread_Local(){}
	
	Thread_Local(int tid,int value,long stamp_hr,long stamp_min,long stamp_sec)
	{
		this->thread_id = tid;
		this->value = value;
		this->stamp_hr = stamp_hr;
		this->stamp_min = stamp_min;
		this->stamp_sec = stamp_sec;
	}
};

class Snap_Captures                   //This class captures the snapshots in the local space
{
public:
	int *snap_shots;
	int timeStamp_hr,timeStamp_min, timeStamp_sec;
	double timeTaken;
	Snap_Captures(){}
	Snap_Captures(int *s,int timeStamp_hr,int timeStamp_min,int timeStamp_sec,double timeTaken)
	{
		this->snap_shots = s;
		this->timeStamp_hr = timeStamp_hr;
		this->timeStamp_min = timeStamp_min;
		this->timeStamp_sec = timeStamp_sec;
		this->timeTaken = timeTaken;
		
	}
};

Snap_Captures snaps[100];

vector<Thread_Local> thread_objects;


class StampedValue                                              //Class For the timestamped Snapshots used in the Algorihtm
{
public:
	atomic<long> sequenceNo;
	atomic<int> value;
	atomic<int> pid;

	StampedValue(){}
	
	StampedValue(const StampedValue&s){} 
	
	StampedValue(int value)
	{
		this->sequenceNo.store(0);
		this->value.store(value);
		this->pid.store(0);
	}
	StampedValue(long sn,int value,int pid)
	{
		this->sequenceNo.store(sn);
		this->value.store(value);
		this->pid.store(pid);
	}
	
	StampedValue& operator=(const StampedValue&s)
	{
		
		this->sequenceNo.store(s.sequenceNo.load());
		this->value.store(s.value.load());
		this->pid.store(s.pid.load());
	}
};

class WFA_MRMW_SNAPSHOT                                             // Main Class For the Algorithm
{
public:
	StampedValue *reg;
	int *can_help;
	int **help_snap;
	
	WFA_MRMW_SNAPSHOT(int cap,int init)
	{
		help_snap = new int*[num_threads];
		
		for(int i=0;i<num_threads;i++)
		help_snap[i] = new int[M];
		
		reg = new StampedValue[cap];
		for(int i=0;i<cap;i++)
		{
			reg[i] = StampedValue(init);
		}
		
		can_help = new int[cap];
		
	}
	
	StampedValue* collect()
	{
		StampedValue *copy = new StampedValue[M];
		
		for(int i=0;i<M;i++)
		{
			copy[i] = reg[i];
		}
		
		return copy;
	}
	
	void update(int th_id,int loc,int value)
	{
		int *snap = scan();
		
		StampedValue oldSnap = reg[loc];
		StampedValue newSnap = StampedValue(oldSnap.sequenceNo.load()+1,value,th_id);
		reg[loc] = newSnap;
		help_snap[th_id] = scan();
	}
	
	bool isPresent(int id)
	{
		bool flag = false;
		for(int i=0;i<M;i++)
		{
			if(can_help[i] == id)
			{
			flag = true;
			break;
		    }
		}
		
		return flag;
	}
	
	int *scan()
	{   
	   StampedValue *oldCopy;
	   StampedValue *newCopy;

	   oldCopy = collect();

COLLECTION_SCOPE:                                 // Collection Phase that is repeated 
       while(true)
       {
       	newCopy = collect();
       	
       	for(int j=0;j<M;j++)
       	{
       		if(oldCopy[j].sequenceNo.load() != newCopy[j].sequenceNo.load())
       		{
       			int pid = newCopy[j].pid.load();
       			
       			if(isPresent(pid))
       			{
       				return this->help_snap[pid];
				}
				else
				{
					for(int k=0;k<M;k++)
					{
						if(can_help[k] == 0)
						{
							can_help[k] = pid;
							break;
						}
					}
				}
			}
       		
		}
		
		oldCopy = newCopy;
		
		int *res = new int[M];
		for(int i=0;i<M;i++)
		{
			res[i] = newCopy[i].value.load();
		}
		
		return res;
	   }
	   
	}
	
};

WFA_MRMW_SNAPSHOT MRMW_Object = WFA_MRMW_SNAPSHOT(M,0);              // Object of the Algorithm Class
bool term = false;


//The Following are the testing methods for writer and the snapshot threads
void writer(int thread_id)
{
	int tid = thread_id;
	int i=1;
	int value, t1,loc;
	while(!term)
	{
		value = rand()%100;
		loc = rand()%M;
		t = time(0);
	    tPtr = localtime(&t);
	    if(i==1)
	    {
		cout<<"Thread "<<tid<<" Write of "<<value<<" at "<<tPtr->tm_hour<<" : "<<tPtr->tm_min<<" : "<<tPtr->tm_sec<<"\n";
		thread_objects.push_back(std::move(Thread_Local(tid,value,tPtr->tm_hour,tPtr->tm_min,tPtr->tm_sec)));
		}
		i=i+1;
		
		if(i==10)
		i=1;
		
		MRMW_Object.update(tid,loc,value);
	    
	    //cout<<"Completed Working";
		this_thread::sleep_for(chrono::microseconds(t1));
	}
}

void snapshot()
{
	//cout<<"in Scan";
	int *collected_scan;
	int i=0;
	while(i<k)
	{
	t = time(0);
	tPtr = localtime(&t);
	
	auto begin_collect = chrono::steady_clock::now();
	collected_scan = MRMW_Object.scan();
	auto end_collect = chrono::steady_clock::now();
	
	auto timeElapsed = end_collect - begin_collect;
	//cout<<"Snapshot Of Thread's---> ";
	//cout<<"Time";
	snaps[i] = Snap_Captures(collected_scan,tPtr->tm_hour,tPtr->tm_min,tPtr->tm_sec,chrono::duration <double,micro> (timeElapsed).count()/1e+6);
	this_thread::sleep_for(chrono::microseconds(t2));
	i++;
    }
}

int main()
{
string parm1,parm2,parm3,parm4,parm5;
getline(inp_file,parm1);
getline(inp_file,parm2);
getline(inp_file,parm3);
getline(inp_file,parm4);
getline(inp_file,parm5);
M = stoi(parm1);
num_threads = stoi(parm2);
k = stoi(parm3);
t1 = stoi(parm4);
t2 = stoi(parm5);
std::vector<std::thread> threads;

  for(int i=0;i<num_threads;i++)
    {   
		thread t(writer,i);
		threads.push_back(std::move(t));
	}
	auto start_clock = chrono::steady_clock::now();
	
    thread t2(snapshot);
    t2.join();
    
	auto stop_clock = chrono::steady_clock::now();
	
	auto timeCompleted = stop_clock -start_clock;
	
	term = true;
	
	for(auto &t:threads)
	{
		t.join();
	}
	
	
	cout<<"Threads Local\n";
		
	for(int j=0;j<thread_objects.size();j++)
	{
	cout<<"Thread "<<thread_objects[j].thread_id<<" Write of "<<thread_objects[j].value<<" at "<<thread_objects[j].stamp_hr<<" : "<<thread_objects[j].stamp_min<<" : "<<thread_objects[j].stamp_sec<<"\n";
	out_file<<"Thread "<<thread_objects[j].thread_id<<" Write of "<<thread_objects[j].value<<" at "<<thread_objects[j].stamp_hr<<" : "<<thread_objects[j].stamp_min<<" : "<<thread_objects[j].stamp_sec<<"\n";	
    }
    
	cout<<"\n";
	
	cout<<"Snapshots Taken:\n";
	
	int count = 0;
	double max_time = snaps[0].timeTaken,total_time;
	for(Snap_Captures sc : snaps)
	{   
	    count=count +1;
		if(count>k)
		break;
		cout<<"\n";
		cout<<"Snap Shots of Threads"<<" at "<<sc.timeStamp_hr<<":"<<sc.timeStamp_min<<":"<<sc.timeStamp_sec<<"  Time ELapsed : "<<setprecision(6)<<fixed<<sc.timeTaken<<" Seconds "<<"\n\n";
        out_file_snap<<"Snap Shots of Threads"<<" at "<<sc.timeStamp_hr<<":"<<sc.timeStamp_min<<":"<<sc.timeStamp_sec<<"  Time ELapsed : "<<setprecision(6)<<fixed<<sc.timeTaken<<" Seconds "<<"\n\n";
		
		total_time = total_time + sc.timeTaken;
		if(sc.timeTaken > max_time)
		max_time = sc.timeTaken;
		
		for(int i=0;i<num_threads;i++)
        {
        	cout<<sc.snap_shots[i]<<" ";
        	out_file_snap<<sc.snap_shots[i]<<" ";
		}
		cout<<"\n\n";
		out_file_snap<<"\n\n";
	}
	
	cout<<"\n\n"<<"Total Elapsed Time :"<<setprecision(6)<<fixed<<chrono::duration <double,micro> (timeCompleted).count()/1e+6;
	cout<<"\nAverage Time For Snapshots : "<<setprecision(6)<<fixed<<total_time/k<<" Seconds\n";
	out_file_snap<<"\nAverage Time For Snapshots : "<<setprecision(6)<<fixed<<total_time/k<<" Seconds\n";
	cout<<"Worst Case Time For Snapshots : "<<setprecision(6)<<fixed<<max_time<<" Seconds\n";
	out_file_snap<<"Worst Case Time For Snapshots : "<<setprecision(6)<<fixed<<max_time<<" Seconds\n";
	cout<<"\nFile Written In Directory , Please Check Directory\n";
	
	out_file.close();
	out_file_snap.close();
	inp_file.close();
	
	return 0;			
}
