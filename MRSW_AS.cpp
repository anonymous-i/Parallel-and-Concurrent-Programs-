/*
********************************************************************************************* 
 Atomic MRSW snapshot implementation
 This program implements the Atomic MRSW snapshot wait-free algorithm.
 
 Submitted by Tahir Ahmed Shaik, CS20MTECH14007, Indian Institute of Technology, Hyderabad
 
 Date : November 2020
 
 Course : CS5300 - Parallel And Concurrent Programming
 
 *********************************************************************************************
 */
 
#include<iostream>
#include<thread>
#include<fstream>
#include<string>
#include<cstdlib>
#include<atomic>
#include<vector>
#include<time.h>
#include<unistd.h>
#include<chrono>
#include <iomanip>
#include <random>
using namespace std;

time_t t= time(NULL);
tm* tPtr= localtime(&t);

ifstream inp_file("inp-parms.txt");
ofstream out_file("WFA_MRSW_THREADS_INFO.txt");
ofstream out_file_snap("WFA_MRSW_SNAPS.txt");


int num_threads,k,t1,t2;
int M=20;


class Snap_Captures                                 //This class captures the snapshots in the local space
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

class Thread_Local                                   //Class used to store thread information in its Local Space
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

vector<Thread_Local> thread_objects;


class StampedSnapshot                                              //Class For the timestamped Snapshots used in the Algorihtm
{
public:
	atomic<long> t_stamp;
	atomic<int> value;
	int *snap;
	
	
	StampedSnapshot(){}
	
	StampedSnapshot(const StampedSnapshot&)
	{
		
	} 
	StampedSnapshot(int value)
	{
		this->t_stamp.store(0);
		this->value.store(value);
		this->snap = nullptr;
	}
	
	StampedSnapshot(long stamp,int value,int *snap)
	{
		this->t_stamp.store(stamp);
		this->value.store(value);
		this->snap = snap;
	}
	
	StampedSnapshot& operator=(const StampedSnapshot&s)
	{
		
		this->t_stamp.store(s.t_stamp.load());
		this->value.store(s.value.load());
		this->snap = snap;
	}
	 
};

class WFA_MRSW_SNAPSHOT                                         // Main Class For the Algorithm
{
private:
	StampedSnapshot *a_table;

public:
	WFA_MRSW_SNAPSHOT(int cap,int initial_value)
	{
		a_table = new StampedSnapshot[cap];
		for(int i=0;i<cap;i++)
		{
			a_table[i] = StampedSnapshot(initial_value);
		}

	}

	StampedSnapshot* collect()
	{
		StampedSnapshot *copy = new StampedSnapshot[M];
		
		for(int j=0;j<M;j++)
		{
			copy[j] = a_table[j];
		}
		
		return copy;
	}

	void update(int th_id,int value)
	{   
	    
		int *snap = scan();
		StampedSnapshot oldSnap = a_table[th_id];
		StampedSnapshot newSnap = StampedSnapshot(oldSnap.t_stamp.load()+1,value,snap);
		a_table[th_id] = newSnap;
	}
	
	int *scan()
	{   
		StampedSnapshot *oldCopy;
		StampedSnapshot *newCopy;
		
		bool *moved = new bool[M];
		
		oldCopy = collect();
		  
COLLECTION_PHASE:while(1)                                              // Collection Phase that is repeated
		            {
			newCopy = collect();
			for(int j=0;j<M;j++)
			{
				if(oldCopy[j].t_stamp.load() != newCopy[j].t_stamp.load())
				{
					if(moved[j])
					{
						return newCopy[j].snap;
					}
					else
					{
						moved[j] = true;
						oldCopy = newCopy;
						goto COLLECTION_PHASE;
					}
				}
			}
			int *res = new int[M];
			for(int i=0;i<M;i++)
			{
				res[i] = newCopy[i].value.load();
			}
			return res;
		}
	}
};


WFA_MRSW_SNAPSHOT MRSW_Object = WFA_MRSW_SNAPSHOT(M,0);                        // Object of the Algorithm Class
bool term = false;

//The Following are the testing methods for writer and the snapshot threads

void writer(int thread_id)
{
	int tid = thread_id;
	int i=1;
	int value, t1;
	
	while(!term)
	{
		int value = rand()%100;
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
		
		MRSW_Object.update(tid,value);
	    
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
	collected_scan = MRSW_Object.scan();
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
	term = true;
	
	auto timeCompleted = stop_clock -start_clock;
	
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
		out_file_snap<<"\n";
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
	}
	
	cout<<"\n\n"<<"Total Elapsed Time :"<<setprecision(6)<<fixed<<chrono::duration <double,micro> (timeCompleted).count()/1e+6<<"\n";
	cout<<"Average Time For Snapshots : "<<setprecision(6)<<fixed<<total_time/k<<" Seconds\n";
	out_file_snap<<"Average Time For Snapshots : "<<setprecision(6)<<fixed<<total_time/k<<" Seconds\n";
	cout<<"Worst Case Time For Snapshots : "<<setprecision(6)<<fixed<<max_time<<" Seconds\n";
	out_file_snap<<"Worst Case Time For Snapshots : "<<setprecision(6)<<fixed<<max_time<<" Seconds\n";
	cout<<"\nFile Written In Directory , Please Check Directory\n";
	
	out_file.close();
	out_file_snap.close();
	inp_file.close();
	return 0;
			
}



