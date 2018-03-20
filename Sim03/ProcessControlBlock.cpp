/*
	Bryce Monaco
	CS 446

	This class is a simple process control block

*/

#include <string>

using namespace std;

class ProcessControlBlock
{
	public:
		ProcessControlBlock();
		ProcessControlBlock (int sentPID, int sentML);
		~ProcessControlBlock();

		void SetPID (int sentValue);
		void SetMemoryLocation (int sentValue);
		bool SetState (int sentValue); //Returns false if the transition is not valid (see comment next to 'state')
		bool SetState (string sentString);

		int GetPID ();
		int GetMemoryLocation ();
		int GetState ();
		string GetStringState ();

		

	private:
		int pid;
		int memoryLocation;
		int state; 	//0 = New, 1 = Ready, 2 = Running, 3 = Waiting, 4 = Terminated
					/*
						Note:	0 -> 1
								1 -> 2
								2 -> 1, 3, 4
								3 -> 1
								4 - > None (End processes)

					*/

};

ProcessControlBlock::ProcessControlBlock ()
{
	pid = -1;
	memoryLocation = -1;
	state = 0;

}

ProcessControlBlock::ProcessControlBlock (int sentPID, int sentML)
{
	pid = sentPID;
	memoryLocation = sentML;
	state = 0;

}

ProcessControlBlock::~ProcessControlBlock ()
{
	//Nothing to destruct

}

void ProcessControlBlock::SetPID (int sentValue)
{
	pid = sentValue; //This function should probably check if pid has been set already (if it hasn't pid=-1)

	return;

}

void ProcessControlBlock::SetMemoryLocation (int sentValue)
{
	memoryLocation = sentValue;

}

bool ProcessControlBlock::SetState (int sentValue)
{
	if (state == 0 && sentValue == 1)
	{
		state = sentValue;

	} else if (state == 1 && sentValue == 2)
	{
		state = sentValue;

	} else if (state == 2 && (sentValue == 1 || sentValue == 3 || sentValue == 4))
	{
		state = sentValue;

	} else if (state == 3 && sentValue == 1)
	{
		state = sentValue;

	} else if (state == 4)
	{
		//Error: attempted to assign a state to an already terminated process
		return false;

	} else
	{
		//Error: Invalid state transition
		return false;

	}

	return true;

}

bool ProcessControlBlock::SetState (string sentString)
{
	int tempState;

	//Assuming strings spelled/cased correctly
	if (sentString == "New")
	{
		//Do nothing, technically a state cannot be set to new once it has been set to something else
		return false;

	} else if (sentString == "Ready")
	{
		tempState = 1;
		
	} else if (sentString == "Running")
	{
		tempState = 2;
		
	} else if (sentString == "Waiting")
	{
		tempState = 3;
		
	} else if (sentString == "Terminated")
	{
		tempState = 4;
		
	} else
	{
		//Error: Unrecognized state string

	}

	if (state == 0 && tempState == 1)
	{
		state = tempState;

	} else if (state == 1 && tempState == 2)
	{
		state = tempState;

	} else if (state == 2 && (tempState == 1 || tempState == 3 || tempState == 4))
	{
		state = tempState;

	} else if (state == 3 && tempState == 1)
	{
		state = tempState;

	} else if (state == 4)
	{
		//Error: attempted to assign a state to an already terminated process
		return false;

	} else
	{
		//Error: Invalid state transition
		return false;

	}

	return true;

}

int ProcessControlBlock::GetPID ()
{
	return pid;

}

int ProcessControlBlock::GetMemoryLocation ()
{
	return memoryLocation;

}

int ProcessControlBlock::GetState ()
{
	return state;

}

string ProcessControlBlock::GetStringState ()
{
	if (state == 0)
	{
		return string ("New");

	} else if (state == 1)
	{
		return string ("Ready");

	} else if (state == 2)
	{
		return string ("Running");

	}  else if (state == 3)
	{
		return string ("Waiting");

	}  else if (state == 4)
	{
		return string ("Terminated");

	} else
	{
		return string ("INVALID_STATE_ERR");

	} 
}