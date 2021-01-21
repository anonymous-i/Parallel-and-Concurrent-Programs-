

#include<iostream>
#include<pthread.h>
#include<mutex>
#include<fstream>
#include<math.h>
#include <chrono> 

using namespace std::chrono; 
using namespace std;

std::mutex mtx;
/*Global Variables section
  Descriptions:
  n,m : are the respective number range and the number of threads to be created supplied 
        in a file named "input-parms.txt"
  
  encountered : this variable keeps track of the total number of numbress processed by the 
                SAM1 Algorithm.
  DAMfile     : Output file consisting of the primes found by the DAM algorithm.
  
  SAMfile     : Output file consisting of the primes found by the SAM1 algorithm.
*/
int n,m;
long long int encountered;
ofstream DAMfile("Primes-DAM.txt");
ofstream SAMfile("Primes-SAM1.txt");
ofstream TimeFile("Times.txt",ofstream::app);
//The function that finds the given number to be prime or not.
bool isPrime(long long int number)
{
	bool flag = true;
	for(long long int i=2;i<=number/2;i++)
	{
		if(number%i==0)
		{
			flag= false;
			break;
		}
	}
	
	return flag;
	
}

/*Counter class is used by the DAM algorithm which allows the multiple parallel threads to take 
multiple numbers for processing dynamically 
This class getAndIncrement() function returns the next number available for the next concurrent 
thread.

The initilz() function is used to initialize the global or local thread value to the most recent
updated value.
*/
class Counter
{
	public:
		long long int num;
		//Counter(){}
		Counter(long long int num)
		{   
			this->num = num;
			
		}
		
		void getAndIncrement()
		{
			mtx.lock();
			this->num++;
			mtx.unlock();		
		}
		
		void Initlzr(long long int num)
		{
			//mtx.lock();
			this->num = num;
			//mtx.unlock();
		}
};

Counter global_count = Counter(1); //Initializes the counter to 1.
Counter thread_counter[50]=Counter(1); //Individual thread Conters local to each thread

/* The DAM Algorithm that uses a shared counter object which provides threads to take up a number 
dynamically for processing as prime. This algorithm follows the principle of mutual exclusion 
which prevents other threads from manipulating the shared object at the same time which might lead 
to inconsistencies. */
void *FindAndWritePrimeDAM(void* index)
{
	long long int t_index = (long long int)index;
	thread_counter[t_index].Initlzr(global_count.num);
	thread_counter[t_index].getAndIncrement();
	mtx.lock();
	global_count.Initlzr(thread_counter[t_index].num);
	mtx.unlock();
	bool flag = isPrime(thread_counter[t_index].num);
	if(flag)
	{
		DAMfile<<thread_counter[t_index].num<<" ";
		
	}
}
 
/*The SAM1 Algorithm which uses a static approach of parallalism, where the entire range of numbers
are divided into a set of domain subsets of numners, where each thread gets the sub domain of numbers
for processing */
void *FindAndWritePrimeSAM1(void* threadId)
{   
    
	for(static long long int i= (long long int)threadId*(int)pow(10,n)+1;i<=((long long int)threadId+1*(int)pow(10,n));i++)
    {  
        
		encountered++;
	
		if(isPrime(i))
		{   
			SAMfile<<i<<" ";
			
		}
		
	}
	
}


/* A Menu driven program, where a choice can be made between the DAM and SAM1 Algorithms for 
analysis */
int main()
{
	cout<<"Hello";
	ifstream InputFile("inp-parms.txt");
	string param1,param2;
    getline(InputFile,param1);
    getline(InputFile,param2);
	n = stoi(param1);
	m = stoi(param2);
	int m_dup = 0;
	pthread_t threads[m];
	int opt;
	char choice;
	do
	{
	system("cls");
	cout<<"********Please Choose An Algorithm***********\n";
	cout<<"1.DAM (Dynamic Allocation Method\n";
	cout<<"2.SAM1 (Static Allocation Method\n";
	cout<<"Enter an option:\n";
	cin>>opt;
	
	switch(opt)
	{
		case 1:
			{
			auto start_DAM = chrono::steady_clock::now();
			for(int i=0;i<=m;i++)
			{
			if(global_count.num>=(long long int)pow(10,n))
			break;
				
			if(i==m)
			{
			 i = 0;
			}
		    //Creating and calling multiple threads and invoing the DAM algorithm
			pthread_create(&threads[i],NULL,FindAndWritePrimeDAM,(void *)i);		 
			}
			auto stop_DAM = chrono::steady_clock::now();
			DAMfile.close();
		    auto diff = stop_DAM - start_DAM;
		    auto diff_sec = chrono::duration_cast<chrono::seconds>(diff);
		    TimeFile<<chrono::duration <double,micro> (diff).count()/1e+6<<" "; 
			TimeFile.close();		
			cout<<"Done! Check output files";
			
			break;
		    }
			
	    case 2:
			{
			auto start_SAM1 = chrono::steady_clock::now();
			while(1)
			{
			
			if(encountered >= pow(10,n))
			{
			break;
		    }
			
			if(m_dup==n)
			{
			m_dup=0;
		    }
			
			//Creating the multiple threads to invoke the SAM1 Algorithm
			pthread_create(&threads[m_dup++],NULL,FindAndWritePrimeSAM1,(void *)m_dup);
			}
			auto stop_SAM1 = chrono::steady_clock::now();
			auto diff = stop_SAM1 - start_SAM1;
		    auto diff_sec = chrono::duration_cast<chrono::seconds>(diff);
		    TimeFile<<chrono::duration <double,micro> (diff).count()/1e+6<<" "; 
			TimeFile.close();		
			cout<<"Done! Check output files";
			SAMfile.close();
			pthread_exit(NULL);
			break;
	        }
		default:
			{
			
			cout<<"Make a correct choice!\n";
			break;
		    }
    }
    cout<<"Want to continue?\n";
    cin>>choice;
}while(choice=='y'||choice== 'Y');

}


