#include <iostream>
#include <chrono>
#include "Clock.cpp"
#include <pthread.h>

Clock thisClock;
chrono::steady_clock::time_point systemStart;

using namespace std;

void* WaitForMicroSeconds (void* sentTime);
float CountToSeconds (unsigned int sentCount);
void* PrintValue (void* sentValue);

int main ()
{	
	systemStart = chrono::steady_clock::now ();

	pthread_t tid;
	pthread_attr_t attr;

	pthread_create(&tid, NULL, PrintValue, (void *) 69);
	pthread_join(tid, NULL);

	pthread_create(&tid, NULL, WaitForMicroSeconds, (void *) 3000000);
	pthread_join(tid, NULL);


	return 0;

}

void* PrintValue (void* sentValue)
{
	long a = (long) sentValue;

	cout << "Found " << a << endl;

	pthread_exit (0);

}

void* WaitForMicroSeconds (void* sentTime)
{
	long t = (long) sentTime;

	chrono::steady_clock::time_point timer;

	cout << "Waiting for " << CountToSeconds (t) << " seconds..." << endl;

	auto dur = thisClock.WaitForMicroSeconds (t).time_since_epoch (). count ();

	cout << "Done!" << endl;

	timer = chrono::steady_clock::now ();

	auto durs = chrono::duration_cast<chrono::microseconds>(timer-systemStart);

	cout << "dur count = " << CountToSeconds (durs.count ()) << endl;

	pthread_exit (0);

}

float CountToSeconds (unsigned int sentCount)
{
	return (float) sentCount / (float) 1000000;

}