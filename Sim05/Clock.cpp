#include <chrono>

using namespace std;

class Clock
{
	public:
		Clock();
		~Clock();

		chrono::steady_clock::time_point WaitForMicroSeconds (int sentValue);

	private:
		chrono::steady_clock::time_point lastEndPoint;

};

Clock::Clock ()
{


}

Clock::~Clock ()
{


}

chrono::steady_clock::time_point Clock::WaitForMicroSeconds (int sentValue)
{
	chrono::steady_clock::time_point end = chrono::steady_clock::now () + chrono::microseconds (sentValue);

	chrono::steady_clock::time_point now;

	do
	{
		now = chrono::steady_clock::now ();

	} while (now.time_since_epoch ().count () < end.time_since_epoch ().count ()); //Wait unit over

	lastEndPoint = now;

	return lastEndPoint;	



}