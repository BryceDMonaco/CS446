#include <iostream>
#include <chrono>
#include "Clock.cpp"

Clock thisClock;
chrono::steady_clock::time_point systemStart;

using namespace std;

void WaitForMicroSeconds (unsigned int sentTime);
float CountToSeconds (unsigned int sentCount);

int main ()
{	
	systemStart = chrono::steady_clock::now ();

	WaitForMicroSeconds (1000000); //1 second
	WaitForMicroSeconds (500000); //0.5 seconds
	WaitForMicroSeconds (100000); //0.1 seconds

	return 0;

}

void WaitForMicroSeconds (unsigned int sentTime)
{
	chrono::steady_clock::time_point timer;

	cout << "Waiting for " << CountToSeconds (sentTime) << " seconds..." << endl;

	auto dur = thisClock.WaitForMicroSeconds (sentTime).time_since_epoch (). count ();

	cout << "Done!" << endl;

	timer = chrono::steady_clock::now ();

	auto durs = chrono::duration_cast<chrono::microseconds>(timer-systemStart);

	cout << "dur count = " << CountToSeconds (durs.count ()) << endl;

}

float CountToSeconds (unsigned int sentCount)
{
	return (float) sentCount / (float) 1000000;

}