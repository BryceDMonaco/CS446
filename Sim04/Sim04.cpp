/*
	Bryce Monaco
	CS 446
	Project 4

	Compile with: makefile ("make" within the Sim04 directory)

	Notes: 	This program is built on top of Sim03.
			

			(Sim01 Notes, Kept for reference)
			In the case of a fatal error the simulation will print directly to the console since
			the error might be output-related. 

			Most functions contain cout calls which are commented out, these were used for debug purposes 
			and are left as comments incase they are needed again as well as to show a bit of the debug process.

	TODO:	- Config file adds processor quantum (does nothing), scheduling code (picks one of three schedulers)

*/

#include <iostream>
#include <ostream>
#include <fstream>
#include <string>
#include <sstream> 	//Used to convert int memory address to hex
#include <vector>	//Not actually used in latest version
#include <chrono>	//Used for all of the timers
#include <cstdlib> 	//Used to generated random int for PCB memory location ()
#include <iomanip>	//Used with outputting the hex address to the correct size
#include <limits.h> //Used for INT_MAX for the random int
#include <pthread.h>
#include <time.h>	//NOT used for timers (see chrono), instead just used to seed random generator

#include "Clock.cpp"
#include "ConfigFile.cpp"
#include "ProcessControlBlock.cpp"

#include <stdio.h>	//Used primarily in ParseCommand for sscanf functionality

using namespace std;

struct thread_args
 {
    char deviceType; //'H', 'K', 'P', 'M', 'S', 'X'
    long count;
};

ofstream logFile; //Gets opened and closed whenever a new config file is scanned successfully
string currentLGFPath; //Stores the path to the current log file so that it doesn't need to be passed as an argument to multiple functions

ConfigFile currentConfFile; //Assuming ScanConfigFile () runs successfully, this contains all the information read in from the current config file
vector<ConfigFile> allConfigFiles; //Not used, currently once a config file is done it just gets overwritten in the currentConfFile

ProcessControlBlock currentPCB;
vector<ProcessControlBlock> allPCBs; //This won't get used in Sim02

Clock thisClock;
chrono::steady_clock::time_point systemStart;

//Locks for each IO device
pthread_mutex_t lock;			//Lock used by the timer thread if not an IO device
pthread_mutex_t lockHDD;		//Lock used by the hard drive thread 	(IO)
pthread_mutex_t lockProj;		//Lock used by the projector thread 	(O)
pthread_mutex_t lockMonitor;	//Lock used by the monitor thread 		(O)
pthread_mutex_t lockScanner;	//Lock used by the scanner thread 		(I)
pthread_mutex_t lockKeyboard;	//Lock used by the keyboard thread 		(I)

float stamp; //Used because I can't figure out how to get a pthread to return a float value
unsigned int memoryPosition = 0;
int onProjector = 0;
int onHardDrive = 0;

//The following are flag booleans to make sure that there are no extra/missing start or finish commands in the metadata
bool currentlyRunningSystem = false;
bool currentlyRunningApplication = false;

bool RunMetaDataFile ();										//Loops through the metadata file, calls ParseCommand () and ExecuteCommand ()
bool ScanConfigFile (string cfgFileName, ConfigFile& sentFile);	//Scans through the config file and imports all of the required data, fails if it can't find all
bool ParseCommand (string sentCommand);							//Takes a command as a string and parses it and then "executes" it
//bool ExecuteCommand (MetaDataObject sentCommand);				//Takes a MetaDataObject and "runs" it
string ScanNextLine (ifstream& sentStream);						//A generic IO function to scan a line from a file, made to clean up code
string ScanNextLine (ifstream& sentStream, char delimChar);		//That same generic function but also with a delim character
void OutputConfigFileData (bool toFile, bool toMonitor);		//Outputs the config file information to the relevant media in the required format
bool OutputToLog (string sentOutput, bool createNewLine);		//Hands output to the log file AND/OR the monitor
//float WaitForMicroSeconds (unsigned int sentTime);			//Replaced with pthread version below
void* WaitForMicroSeconds (void* sentTime);
float CountToSeconds (unsigned int sentCount);
float RunTimerThread (long sentTime, char sentDevice);							//Used to reduce duplicate code lines
//unsigned int GenerateRandomMemoryAddress ();					//Only used in Sim02, deprecated as of Sim03
unsigned int GetMemoryAddress (unsigned int sentSize);
void* GetProjectorNumber (void* sentArg);
void* GetHardDriveNumber (void* sentArg);
int RunHardDriveThread ();
int RunProjectorThread ();

int main (int argc, char* argv[])
{

	 if (argc < 2) 
	 {
        // Tell the user how to run the program
        cout << "Usage: " << argv[0] << " NAME_OF_CONFIG_FILE_WITH_EXTENSION (Multiple config files allowed with spaces between names)" << endl;

        return 0;

    } else
    {
    	srand (time(NULL)); //Just quickly seed the random generator, only use of a function from time.h
    	//cout << "Opening config file \"" << argv [1] << "\"..." << endl;

    	for (int i = 1; i < argc; i++) //A loop in case there is more than one config file in the arguments (command would be of the form "./Sim01 config1.conf config2.conf (etc)")
    	{

    		if (!ScanConfigFile (argv [i], currentConfFile)) //Function returns false if there was an error
	    	{
	    		cout << "FATAL ERROR: There was an error with the config file." << endl;

	    		return 0;

	    	}

	    	//From this point on, the config file has been read successfully

		    if (currentConfFile.ShouldLogToFile ())
		    {
		    	currentLGFPath = currentConfFile.GetLGFPath ();

		    	logFile.open (currentLGFPath);

		    }

		    /* The Configuration File Data header is no longer output in Sim02
		    if (!OutputToLog ("Configuration File Data", true)) //OutputToLog returns false if output is incorrectly configured
		    {
		    	cout << "FATAL ERROR: There was an output error. Closing the simulation." << endl;

		    	return 0;

		    }
		    */

		    systemStart = chrono::steady_clock::now ();

		    //Now the test output is the initial simulator start message
		    //Note:	If there are multiple config files this will be output at roughly 0 for each one (systemStart gets reset every config file from the line above)
		    //		If it is preferred that the time shown for Simulator program starting is based off of the start time of the original config then it can be moved out of the loop
		    //Waits for 1 uSecond because sometimes 0 uS causes some strange issues


		    if (!OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - Simulator program starting", true)) //OutputToLog returns false if output is incorrectly configured
		    {
		    	cout << "FATAL ERROR: There was an output error. Closing the simulation." << endl;

		    	return 0;

		    }

		    

		    //This doesn't get output in Sim02
		    //OutputConfigFileData (currentConfFile.ShouldLogToFile (), currentConfFile.ShouldLogToMonitor ());

		    RunMetaDataFile ();

		    OutputToLog(string ("(End of config file: ") + argv[i] + ")", true); //Not required output but this helps make the log easier to read with multiple config files

		    if ((i + 1) < argc) //True if there is at least one more config file
		    {
		    	OutputToLog ("\n", false); //Create an extra line between multiple config files, again not required output but it makes the log easier to read

		    } else //Last config file finished
		    {
		    	OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - Simulator program ending", true);

		    }

		    if (logFile.is_open ())
		    {

		    	logFile.close ();

		    }

    	}

    	

    }

	return 0;

}

//Returns true if the entire file is parsed correctly
//Essentially just scans in potential commands one at a time (assuming no typos) and sends them to ParseCommand () to see if they're actually commands
bool RunMetaDataFile ()
{
	ifstream mdfFile;
	mdfFile.open (currentConfFile.GetMDFPath ());
	string currentLine;

	//OutputToLog ("\nMeta-Data Metrics", true); //No longer used in Sim02
	//cout << endl << "Meta-Data Metrics" << endl;

	//Check if the file is empty
	if (mdfFile.eof () || !mdfFile.is_open ())
	{
		OutputToLog ("Error: Empty Meta Data File", true);
		cout << "File attempted to be opened \"" << currentConfFile.GetMDFPath () << "\"" << endl;
		//cout << "Error: Empty Meta Data File" << endl;

		mdfFile.close ();
		return false;

	}

	currentLine = ScanNextLine (mdfFile);

	//Check for start line Start Program Meta-Data Code:
	if (currentLine.find ("Start Program Meta-Data Code:") == string::npos) //True if not found
	{
		//First line does not contain start line
		OutputToLog ("Error: No start program command", true);
		OutputToLog (string ("Found: ") + currentLine + " in " + currentConfFile.GetMDFPath (), true);
		//cout << "Error: No start program command" << endl;



		mdfFile.close ();
		return false;

	}

	currentLine = ScanNextLine (mdfFile, ';');

	//Check for start command
	if (currentLine.find ("S{begin}0") == string::npos) //True if not found
	{
		//First line does not contain start command
		OutputToLog ("Error: No start command found", true);
		//cout << "Error: No start command found" << endl;

		mdfFile.close ();
		return false;

	} else
	{
		currentPCB = ProcessControlBlock (1, -1); //Defaulted to memory location -1 since it hasn't been assigned yet in mdf
		currentPCB.SetState (1);
		OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - OS: preparing process " + to_string (currentPCB.GetPID ()), true);

	}

	bool reachedEndOfFile = false;

	do
	{
		if (mdfFile.eof ())
		{
			OutputToLog ("Error: mdf file ended unexpectedly", true);
			//cout << "Error: mdf file ended unexpectedly" << endl;

			mdfFile.close ();
			return false;

		} else
		{
			//With ';' delim to read one command at a time
			currentLine = ScanNextLine (mdfFile, ';');

			//Check if the end of the file was reached
			if (currentLine.find ("End Program Meta-Data Code.") != string::npos)
			{
				reachedEndOfFile = true;

				OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - OS: removing process " + to_string (currentPCB.GetPID ()), true);

				currentPCB.SetState (4); //Terminate

				//cout << "Found EOF" << endl;

			} else
			{	
				//cout << "Parsing Command: " << currentLine << endl;
 				//S=OS, A=Application, P=Process, I=Input, O=Output 
				if (!ParseCommand (currentLine))
				{
					OutputToLog (string ("There was an error with the command \"" + currentLine + "\""), true);
					//cout << "There was an error with the command \"" << currentLine << "\"" << endl;

					mdfFile.close ();
					return false;

				}

			}

		}

		



	} while (!reachedEndOfFile);

	return true;

}

//Returns true if successful, false otherwise
//Attempts to find all 13 lines required for the config file. If even one is missing the function returns false and the program closes.
//Note: This function assumes that the values will not always be in the same order so it must scan in a line and then figure out which line it has
bool ScanConfigFile (string cfgFileName, ConfigFile& sentFile)
{
	//Open the config fire
	ifstream cfgFile;
	cfgFile.open (cfgFileName);
	string currentLine;

	//Temp versions of the config variables, in case of invalid config no global vars will be assigned
	float versionNumberTEMP;		//Float to allow 1.XX

	string metaDataFilePathTEMP;
	string logFilePathTEMP;	

	int monitorDispTimeTEMP;		//msec
	int processorCycleTimeTEMP;		//msec
	int scannerCycleTimeTEMP;		//msec
	int hardDriveCycleTimeTEMP;		//msec
	int keyboardCycleTimeTEMP;		//msec
	int memoryCycleTimeTEMP;		//msec
	int projectorCycleTimeTEMP;		//msec
	int systemMemoryTEMP;			//KB
	int projectorQuantityTEMP;
	int harddriveQuantityTEMP;
	int memoryBlockSizeTEMP;		//KB

	bool shouldLogToFileTEMP = false;
	bool shouldLogToMonitorTEMP = false;

	//Check if the file is empty
	if (cfgFile.eof ())
	{
		cout << "Error: Empty Config File" << endl;

		cfgFile.close ();
		return false;

	}

	//cfgFile.getline (currentLine, 50);
	currentLine = ScanNextLine (cfgFile);

	//Check for start line
	if (currentLine.find ("Start Simulator Configuration File") == string::npos)
	{
		//First line does not contain start command
		cout << "Error: No start command" << endl;

		cfgFile.close ();
		return false;

	}

	//A standard config file should contain exactly 17 lines, first and last are start and end, 15 values
	for (int i = 0; i < 15; i++)
	{
		if (cfgFile.eof ())
		{
			cout << "Error: config file ended unexpectedly" << endl;

			cfgFile.close ();
			return false;

		} else
		{
			//Note, scanning is done this way because we cannot assume that the file
			//will always be in the same order
			currentLine = ScanNextLine (cfgFile);

			if (currentLine.find ("Version/Phase:") != string::npos)
			{
				sscanf(currentLine.c_str(), "Version/Phase:%f", &versionNumberTEMP);

				//cout << "Found Version " << versionNumberTEMP << endl;

				//break;

			} else if (currentLine.find ("File Path:") != string::npos && currentLine.find ("Log") == string::npos) //Need to make sure that this isn't confused for the "Log File Path:" field
			{
				//It is easiest to pull this out as a substring, assumes path is < 50 chars long
				//This assumes there is a space after "File Path:"
				metaDataFilePathTEMP = currentLine.substr (11, 50);

				//cout << "Found mdf Path: " << metaDataFilePathTEMP << endl;

				//break;

			} else if (currentLine.find ("Monitor display time") != string::npos) //Added "d" because it was conflicting with Log to Monitor
			{

    			sscanf(currentLine.c_str(), "Monitor display time {msec}:%d", &monitorDispTimeTEMP);

				//cout << "Found Monitor Time " << monitorDispTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Processor cycle time") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Processor cycle time {msec}:%d", &processorCycleTimeTEMP);

				//cout << "Found Processor Time " << processorCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Scanner cycle time") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Scanner cycle time {msec}:%d", &scannerCycleTimeTEMP);

				//cout << "Found Scanner Time " << scannerCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Hard drive cycle") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Hard drive cycle time {msec}:%d", &hardDriveCycleTimeTEMP);

				//cout << "Found HDD Time " << hardDriveCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Keyboard cycle time") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Keyboard cycle time {msec}:%d", &keyboardCycleTimeTEMP);

				//cout << "Found Keyboard Time " << keyboardCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Memory cycle time") != string::npos) //Search for "Memory c" to find Memory cycle time
			{

    			sscanf(currentLine.c_str(), "Memory cycle time {msec}:%d", &memoryCycleTimeTEMP);

				//cout << "Found Memory Time " << memoryCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Projector cycle time") != string::npos)
			{

				//cout << "FOUND PROJECTOR LINE: " << currentLine << endl;

    			sscanf(currentLine.c_str(), "Projector cycle time {msec}:%d", &projectorCycleTimeTEMP);

				//cout << "Found Projector Time " << projectorCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Log to") != string::npos)
			{

				//cout << "FOUND LOG LINE:" << currentLine << endl;

    			if (currentLine.find ("Both") != string::npos)
    			{
    				shouldLogToMonitorTEMP = true;
    				shouldLogToFileTEMP = true;

    				//cout << "Logging to both" << endl;

    			} else if (currentLine.find ("Monitor") != string::npos)
    			{
    				shouldLogToMonitorTEMP = true;

    				//cout << "Logging to monitor only" << endl;

    			} else if (currentLine.find ("File") != string::npos) //"file"?
    			{
    				shouldLogToFileTEMP = true;

    				//cout << "Logging to file only" << endl;

    			} else
    			{
    				OutputToLog (string ("Error cannot interpret: ") + currentLine, true);
    				//cout << "Error: Can not interpret: " << currentLine << endl;

    			}

				//break;

			} else if (currentLine.find ("Log File") != string::npos)
			{

    			//It is easiest to pull this out as a substring, assumes path is < 50 chars long
				//This assumes there is a space after "File Path:"
				logFilePathTEMP = currentLine.substr (15, 50);

				//cout << "Found Log Path: " << logFilePathTEMP << endl;

				//break;

			} else if (currentLine.find ("System memory") != string::npos)
			{

				if (currentLine.find ("kbytes") != string::npos)
				{
					sscanf(currentLine.c_str(), "System memory {kbytes}:%d", &systemMemoryTEMP);

				} else if (currentLine.find ("Mbytes") != string::npos)
				{
					int temp;
					sscanf(currentLine.c_str(), "System memory {Mbytes}:%d", &temp);

					systemMemoryTEMP = temp * 1000;

				} else if (currentLine.find ("Gbytes") != string::npos)
				{
					int temp;
					sscanf(currentLine.c_str(), "System memory {Gbytes}:%d", &temp);

					systemMemoryTEMP = temp * 1000000;

				} else
				{
					//Can't find a unit, just assume KB
					sscanf(currentLine.c_str(), "System memory {%*s}:%d", &systemMemoryTEMP);

				}

				//break;

			} else if (currentLine.find ("Projector quantity") != string::npos)
			{
				sscanf(currentLine.c_str(), "Projector quantity:%d", &projectorQuantityTEMP);    			

				cout << "Found pq " << projectorQuantityTEMP << endl;

				//break;

			} else if (currentLine.find ("drive quantity") != string::npos)
			{
				sscanf(currentLine.c_str(), "Hard drive quantity:%d", &harddriveQuantityTEMP);    			

				cout << "Found HDDq " << harddriveQuantityTEMP << endl;

				//break;

			} else if (currentLine.find ("Memory block size") != string::npos)
			{

				if (currentLine.find ("kbytes") != string::npos)
				{
					sscanf(currentLine.c_str(), "Memory block size {kbytes}:%d", &memoryBlockSizeTEMP);

				} else if (currentLine.find ("Mbytes") != string::npos)
				{
					int temp;
					sscanf(currentLine.c_str(), "Memory block size {Mbytes}:%d", &temp);

					memoryBlockSizeTEMP = temp * 1000;

				} else if (currentLine.find ("Gbytes") != string::npos)
				{
					int temp;
					sscanf(currentLine.c_str(), "Memory block size {Gbytes}:%d", &temp);

					memoryBlockSizeTEMP = temp * 1000000;

				} else
				{
					//Can't find a valid unit, just assume KB
					sscanf(currentLine.c_str(), "Memory block size {%*s}:%d", &memoryBlockSizeTEMP);

				}

				//break;

			} else
			{
				cout << "Error: Could not find valid config data, perhaps a field is missing?" << endl;

				cfgFile.close ();
				return false;

			}
		}

	}

	currentLine = ScanNextLine (cfgFile);

	//Check for end line
	if (currentLine.find ("End Simulator Configuration File") == string::npos)
	{
		//First line does not contain start command
		OutputToLog ("Error: No end config file command", true);
		//cout << "Error: No end command" << endl;

		cfgFile.close ();
		return false;

	} else
	{
		//Check for any extra spaces at the end of the mdf file name (for some reason this prevents the file from being opened)
		char testCharM;
		char testCharL;

		do
		{
			testCharL = logFilePathTEMP [logFilePathTEMP.length () - 1];
			testCharM = metaDataFilePathTEMP [metaDataFilePathTEMP.length () - 1];

			if (testCharM == ' ')
			{
				metaDataFilePathTEMP.erase (metaDataFilePathTEMP.length () - 1);

			}

			if (testCharL == ' ')
			{
				logFilePathTEMP.erase (logFilePathTEMP.length () - 1);

			}

		} while (testCharM == ' ' || testCharL == ' ');

		//Config file was read successfully, now store all read values into the sent file
		sentFile.SetVersionNumber (versionNumberTEMP);

		sentFile.SetMDFPath (metaDataFilePathTEMP);
		sentFile.SetLGFPath (logFilePathTEMP);

		sentFile.SetMonitorTime (monitorDispTimeTEMP);
		sentFile.SetProcessorTime (processorCycleTimeTEMP);
		sentFile.SetScannerTime (scannerCycleTimeTEMP);
		sentFile.SetHardDriveTime (hardDriveCycleTimeTEMP);
		sentFile.SetKeyboardTime (keyboardCycleTimeTEMP);
		sentFile.SetMemoryTime (memoryCycleTimeTEMP);
		sentFile.SetProjectorTime (projectorCycleTimeTEMP);
		sentFile.SetSystemMemoryKB (systemMemoryTEMP);
		sentFile.SetProjectorQuantity (projectorQuantityTEMP);
		sentFile.SetHardDriveQuantity (harddriveQuantityTEMP);
		sentFile.SetMemoryBlockSize (memoryBlockSizeTEMP);

		sentFile.SetLogPreferences (shouldLogToFileTEMP, shouldLogToMonitorTEMP);

	}

	cfgFile.close ();
	return true;


}

//Returns true if the command is parse-able, false if there are any errors/issues that it found
//Takes the potential command sent to it and attempts to find its command character, its keyword, and its duration then, if all are found, it "runs" it
//S=OS, A=Application, P=Process, I=Input, O=Output
bool ParseCommand (string sentCommand)
{
	char commandChar = sentCommand [0];

	for (int i = 0; (i < sentCommand.length() && (commandChar == ' ' || commandChar == '\n' || commandChar == '\t')); i++)
	{
		commandChar = sentCommand [i];

	}

	if (commandChar == 'S')
	{
		if (sentCommand.find ("begin") != string::npos) //being command found
		{
			if (currentlyRunningSystem) //The system is already running
			{
				OutputToLog ("Error: A begin command has already been processed", true);
				//cout << "Error: A begin command has already been processed" << endl;

			} else //The system has not been initialized before and a begin command was sent
			{
				currentlyRunningSystem = true;

				return true;

			}

		} else if (sentCommand.find ("finish") != string::npos) //finish command found
		{
			if (!currentlyRunningSystem) //The system is already finished
			{
				OutputToLog ("Error: A finish command has already been processed", true);
				//cout << "Error: A finish command has already been processed" << endl;

			} else //The system is running and a finish command was sent
			{
				currentlyRunningSystem = false;

				return true;

			}

		} else //No valid keyword found
		{
			OutputToLog ("Error: No valid keyword found", true);
			//cout << "Error: No valid keyword found" << endl;

			return false;

		}

	} else if (commandChar == 'A')
	{
		if (sentCommand.find ("begin") != string::npos) //being command found
		{
			if (currentlyRunningApplication) //An application is already running
			{
				OutputToLog ("Error: A begin command has already been processed", true);
				//cout << "Error: A begin command has already been processed" << endl;

			} else //The application has not been initialized before and a begin command was sent
			{
				currentlyRunningApplication = true;

				OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - OS: starting process " + to_string (currentPCB.GetPID ()), true);

				currentPCB.SetState (2);

				return true;

			}

		} else if (sentCommand.find ("finish") != string::npos) //finish command found
		{
			if (!currentlyRunningApplication) //The application is already finished
			{
				OutputToLog ("Error: A finish command has already been processed", true);
				//cout << "Error: A finish command has already been processed" << endl;

			} else //The application is running and a finish command was sent
			{
				currentlyRunningApplication = false;

				return true;

			}

		} else //No valid keyword found
		{
			OutputToLog ("Error: No valid keyword found", true);
			//cout << "Error: No valid keyword found" << endl;

			return false;

		}

	} else if (commandChar == 'P')
	{
		if (sentCommand.find ("run") == string::npos)
		{
			//First line does not contain start command
			OutputToLog ("Error: No \"run\" keyword found", true);
			//cout << "Error: No \"run\" keyword found" << endl;

			return false;

		} else
		{
			int duration = 0;

			if (sscanf(sentCommand.c_str(), " P{run}%d", &duration) <= 0) //Couldn't find a duration
			{
				OutputToLog ("Error: No duration found", true);
				//cout << "Error: no duration found" << endl;

				return false;

			}

			duration *= currentConfFile.GetProcessorTime (); //Convert from units to ms

			OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": start processing action (Run Time: " + to_string(duration) + "ms)", true);
			OutputToLog (string (to_string (RunTimerThread (duration * 1000, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": end processing action", true);

			//OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetProcessorTime ()), true);
			//cout << sentCommand << " - " << (duration * processorCycleTime) << endl;

			return true;

		}


	} else if (commandChar == 'I')
	{	
		//Read in the keyword
		int duration = 0;
		string keyword;

		char tempChar = '|';

		if (sentCommand.find ("{") == string::npos)
		{
			//First line does not contain start command
			OutputToLog ("Error: No keyword bracket found", true);
			//cout << "Error: No keyword bracket found" << endl;

			return false;

		} else
		{
			int index = sentCommand.find ("{") + 1;

			tempChar = sentCommand [index];

			while (tempChar != '}')
			{
				keyword.push_back (tempChar);

				index++;
				tempChar = sentCommand [index];

			}

		}

		//Done this way so sscanf can deal with keywords which have a space such as "hard drive"
		string commandToParse = " I{";
		commandToParse += keyword;
		commandToParse += "}%d";

		if (sscanf(sentCommand.c_str(), commandToParse.c_str (), &duration) <= 0) //Couldn't find a duration AND a keyword
		{
			OutputToLog ("Error: No duration found", true);
			//cout << "Error: no duration found" << endl;
			//cout << "Extra info: Keyword: " << keyword << endl; //This isn't given it's own OutputToLog because it was just for initial debugging

			return false;

		}

		if (keyword.find ("hard drive") != string::npos)
		{
			duration *= currentConfFile.GetHardDriveTime (); //Convert from units to ms

			currentPCB.SetState (3);

			OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": start hard drive input on HDD " + to_string (RunHardDriveThread ()) + "(Run Time: " + to_string(duration) + "ms)", true);
			OutputToLog (string (to_string (RunTimerThread (duration * 1000, 'H'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": end hard drive input", true);

			currentPCB.SetState (1);
			currentPCB.SetState (2);

			//OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetHardDriveTime ()), true);
			//cout << sentCommand << " - " << (duration * hardDriveCycleTime) << endl;

		} else if (keyword.find ("keyboard") != string::npos)
		{
			duration *= currentConfFile.GetKeyboardTime (); //Convert from units to ms

			currentPCB.SetState (3);

			OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": start keyboard input (Run Time: " + to_string(duration) + "ms)", true);
			OutputToLog (string (to_string (RunTimerThread (duration * 1000, 'K'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": end keyboard input", true);

			currentPCB.SetState (1);
			currentPCB.SetState (2);

			//OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetKeyboardTime ()), true);
			//cout << sentCommand << " - " << (duration * keyboardCycleTime) << endl;

		} else if (keyword.find ("scanner") != string::npos)
		{
			duration *= currentConfFile.GetScannerTime (); //Convert from units to ms

			currentPCB.SetState (3);

			OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": start scanner input (Run Time: " + to_string(duration) + "ms)", true);
			OutputToLog (string (to_string (RunTimerThread (duration * 1000, 'S'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": end scanner input", true);

			currentPCB.SetState (1);
			currentPCB.SetState (2);

			//OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetScannerTime ()), true);
			//cout << sentCommand << " - " << (duration * scannerCycleTime) << endl;

		} else
		{
			OutputToLog (string ("Error: Unrecognized keyword \"" + keyword + "\""), true);
			//cout << "Error: Unrecognized keyword \"" << keyword << "\"" << endl;

			return false;

		}

		return true;

	} else if (commandChar == 'O')
	{
		//Read in the keyword
		int duration = 0;
		string keyword;

		char tempChar = '|';

		if (sentCommand.find ("{") == string::npos)
		{
			//First line does not contain start command
			OutputToLog ("Error: No keyword bracket found", true);
			//cout << "Error: No keyword bracket found" << endl;

			return false;

		} else
		{
			int index = sentCommand.find ("{") + 1;

			tempChar = sentCommand [index];

			while (tempChar != '}')
			{
				keyword.push_back (tempChar);

				index++;
				tempChar = sentCommand [index];

			}

		}

		//Done this way so sscanf can deal with keywords which have a space such as "hard drive"
		string commandToParse = string (" O{") + keyword + "}%d";
		//commandToParse += keyword;
		//commandToParse += "}%d";

		if (sscanf(sentCommand.c_str(), commandToParse.c_str (), &duration) <= 0) //Couldn't find a duration AND a keyword
		{
			OutputToLog ("Error: No duration found", true);
			//cout << "Error: no duration found" << endl;
			//cout << "Extra info: Keyword: " << keyword << endl; //Just extra debug info

			return false;

		}

		if (keyword.find ("hard drive") != string::npos)
		{
			duration *= currentConfFile.GetHardDriveTime (); //Convert from units to ms

			currentPCB.SetState (3);

			OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": start hard drive output on HDD " + to_string (RunHardDriveThread ()) + " (Run Time: " + to_string(duration) + "ms)", true);
			OutputToLog (string (to_string (RunTimerThread (duration * 1000, 'H'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": end hard drive output", true);

			currentPCB.SetState (1);
			currentPCB.SetState (2);

			//OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetHardDriveTime ()), true);
			//cout << sentCommand << " - " << (duration * hardDriveCycleTime) << endl;

		} else if (keyword.find ("monitor") != string::npos)
		{
			duration *= currentConfFile.GetMonitorTime (); //Convert from units to ms

			currentPCB.SetState (3);

			OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": start monitor output (Run Time: " + to_string(duration) + "ms)", true);
			OutputToLog (string (to_string (RunTimerThread (duration * 1000, 'M'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": end monitor output", true);

			currentPCB.SetState (1);
			currentPCB.SetState (2);

			//OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetMonitorTime ()), true);
			//cout << sentCommand << " - " << (duration * monitorDispTime) << endl;

		} else if (keyword.find ("projector") != string::npos)
		{
			duration *= currentConfFile.GetProjectorTime (); //Convert from units to ms

			currentPCB.SetState (3);

			OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": start projector output on PROJ " + to_string (RunProjectorThread ()) + " (Run Time: " + to_string(duration) + "ms)", true);
			OutputToLog (string (to_string (RunTimerThread (duration * 1000, 'P'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": end projector output", true);

			currentPCB.SetState (1);
			currentPCB.SetState (2);

			//OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetProjectorTime ()), true);
			//cout << sentCommand << " - " << (duration * projectorCycleTime) << endl;

		} else
		{
			OutputToLog (string ("Error: Unrecognized keyword \"") + keyword + "\"", true);
			//cout << "Error: Unrecognized keyword \"" << keyword << "\"" << endl;

			return false;

		}

		return true;

	} else if (commandChar == 'M')
	{
		int duration = -1;

		//Check for the allocate keyword, if it can't be found check for block keyword
		if (sscanf(sentCommand.c_str(), " M{allocate}%d", &duration) <= 0 && sscanf(sentCommand.c_str(), " M{block}%d", &duration) <= 0) //Couldn't find a duration
		{
			OutputToLog ("Error: No duration found or the keyword may be invalid", true);
			//cout << "Error: no duration found or the keyword may be invalid" << endl;

			return false;

		}

		//Need to detect if it's blocking or allocating
		if (sentCommand.find ("allocate") != string::npos)
		{
			duration *= currentConfFile.GetMemoryTime (); //Convert from units to ms

			OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": allocating memory (Run Time: " + to_string(duration) + "ms)", true);

			int newMemLocation = GetMemoryAddress (currentConfFile.GetMemoryBlockSize ());

			stringstream ss;
			ss << setfill('0') << setw(8) << hex << newMemLocation;
			string s = ss.str();

			OutputToLog (string (to_string (RunTimerThread (duration * 1000, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": memory allocated at 0x" + s, true);

			//OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetHardDriveTime ()), true);
			//cout << sentCommand << " - " << (duration * hardDriveCycleTime) << endl;

		} else if (sentCommand.find ("block") != string::npos)
		{
			duration *= currentConfFile.GetMemoryTime (); //Convert from units to ms

			OutputToLog (string (to_string (RunTimerThread (1, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": start memory blocking (Run Time: " + to_string(duration) + "ms)", true);
			OutputToLog (string (to_string (RunTimerThread (duration * 1000, 'X'))) + " - Process " + to_string (currentPCB.GetPID ()) + ": end memory blocking", true);

			//OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetMonitorTime ()), true);
			//cout << sentCommand << " - " << (duration * monitorDispTime) << endl;

		} else
		{
			OutputToLog (string ("Error: could not find allocate or block in memory command \"") + sentCommand + "\"", true);

			return false;

		}

		//OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetMemoryTime ()), true);
		//cout << sentCommand << " - " << (duration * memoryCycleTime) << endl;

		return true;

	} else
	{
		OutputToLog (string ("Error: Unknown command char \'") + commandChar + "\"", true);
		//cout << "Error: Unknown command char \'" << commandChar << "\'" << endl;

		return false;

	}

}

//Returns the next whole line of the sent input file
//Must be read into a char array which is then converted to a string object which is returned
//Note: Does not work for lines over 50 characters in length
string ScanNextLine (ifstream& sentStream)
{
	char line[50];

	sentStream.getline (line, 50);

	string lineStr = line;

	return lineStr;

}

//Returns the next set of characters up to (but not including) the delim char (the delim char is marked as "read" but it is not returned in the string)
//Must be read into a char array which is then converted to a string object which is returned
//Note: Does not work for lines over 50 characters in length
//Since some sets of characters start with un-needed whitespace the function has a quick loop to shorten the string which prevents any parsing errors
string ScanNextLine (ifstream& sentStream, char delimChar)
{
	char line[50];

	sentStream.getline (line, 50, delimChar);

	string lineStr = line;

	while (lineStr[0] < 65) //Trim whitespace, tabs, newlines, etc. 65 == ASCII 'A'
	{
		lineStr.erase (lineStr.begin ());

	}

	return lineStr;

}

//As of Sim02 this function isn't needed because the information is no longer required output
//Ouputs all the data read in from the config file in the required format
//This function assumes all data has already been correctly scanned and stored into the currentConfFile object
void OutputConfigFileData (bool toFile, bool toMonitor)
{
	//Legacy Output Format
	//cout << "Configuration File Data" << endl;	
	//cout << "Monitor = " << monitorDispTime << "ms/cycle" << endl;	
	//cout << "Processor = " << processorCycleTime << "ms/cycle" << endl;
	//cout << "Scanner = " << scannerCycleTime << "ms/cycle" << endl;
	//cout << "Hard Drive = " << hardDriveCycleTime << "ms/cycle" << endl;
	//cout << "Keyboard = " << keyboardCycleTime << "ms/cycle" << endl;
	//cout << "Memory = " << memoryCycleTime << "ms/cycle" << endl;
	//cout << "Projector = " << projectorCycleTime << "ms/cycle" << endl;
	//cout << "Logged to: ";

	//Note that to use the string operator '+' the first string literal needs to be inside a string constructor
	OutputToLog (string ("Monitor = ") + to_string (currentConfFile.GetMonitorTime ()) + "ms/cycle", true);
	OutputToLog (string ("Processor = ") + to_string (currentConfFile.GetProcessorTime ()) + "ms/cycle", true);
	OutputToLog (string ("Scanner = ") + to_string (currentConfFile.GetScannerTime ()) + "ms/cycle", true);
	OutputToLog (string ("Hard Drive = ") + to_string (currentConfFile.GetHardDriveTime ()) + "ms/cycle", true);
	OutputToLog (string ("Keyboard = ") + to_string (currentConfFile.GetKeyboardTime ()) + "ms/cycle", true);
	OutputToLog (string ("Memory = ") + to_string (currentConfFile.GetMemoryTime ()) + "ms/cycle", true);
	OutputToLog (string ("Projector = ") + to_string (currentConfFile.GetProjectorTime ()) + "ms/cycle", true);
	OutputToLog (string ("System Memory = ") + to_string (currentConfFile.GetSystemMemory ()) + "KB", true);
	OutputToLog (string ("Logged to: "), false);


	if (toFile && toMonitor)
	{
		OutputToLog (string ("Monitor and ") + currentConfFile.GetLGFPath (), true);
		//cout << "Monitor and " << logFilePath << endl;

	} else if (toMonitor)
	{
		OutputToLog ("Monitor", true);
		//cout << "Monitor" << endl;

	} else if (toFile)
	{
		OutputToLog (currentConfFile.GetLGFPath (), true);
		//cout << logFilePath << endl;

	}

	return;
}

//Returns true if the IO operation could be completed, false otherwise (i.e. the log file isn't already opened or the config file did not specify where to output)
//This function is used as a module to handle output so that all other functions do not need to worry about where to output. All other functions send their output string and this function outputs it accordingly
bool OutputToLog (string sentOutput, bool createNewLine)
{
	if (currentConfFile.ShouldLogToFile () && !logFile.is_open ())
	{
		cout << "FATAL ERROR: Cannot output to file, there seems to have been an error in opening the file" << endl;

		return false;

	} else if (currentConfFile.ShouldLogToFile () && currentConfFile.ShouldLogToMonitor ()) //Log to both
	{
		logFile << sentOutput;
		cout << sentOutput;

		if (createNewLine)
		{
			logFile << endl;
			cout << endl;

		}

		return true;

	} else if (currentConfFile.ShouldLogToFile ())
	{
		logFile << sentOutput;

		if (createNewLine)
		{
			logFile << endl;

		}

		return true;

	} else if (currentConfFile.ShouldLogToMonitor ())
	{
		cout << sentOutput;

		if (createNewLine)
		{
			cout << endl;

		}

		return true;

	} else
	{
		cout << "FATAL ERROR: Output is incorrectly configured and cannot be made" << endl;

		return false;

	}

}

/* Replacec with pthread version above
float WaitForMicroSeconds (unsigned int sentTime)
{
	chrono::steady_clock::time_point timer;

	//cout << "Waiting for " << CountToSeconds (sentTime) << " seconds..." << endl;

	auto dur = thisClock.WaitForMicroSeconds (sentTime).time_since_epoch (). count ();

	//cout << "Done!" << endl;

	timer = chrono::steady_clock::now ();

	auto durs = chrono::duration_cast<chrono::microseconds>(timer-systemStart);

	//cout << "dur count = " << CountToSeconds (durs.count ()) << endl;

	return CountToSeconds (durs.count ());

}
*/

void* WaitForMicroSeconds (void* sentStruct)
{
	thread_args* sentArgs = (thread_args*) sentStruct;

	long sentTime = (long) (*sentArgs).count;
	char sentDevice = (char) (*sentArgs).deviceType;

	//pthread_mutex_lock(&lock);

	if (sentDevice == 'H')
	{
		//pthread_mutex_lock(&lock);
		//Handled already by lockHDD

	} else if (sentDevice == 'P')
	{
		//pthread_mutex_lock(&lock);
		//Handled already by lockProj

	} else if (sentDevice == 'M')
	{
		pthread_mutex_lock(&lockMonitor);

	} else if (sentDevice == 'S')
	{
		pthread_mutex_lock(&lockScanner);

	} else if (sentDevice == 'K')
	{
		pthread_mutex_lock(&lockKeyboard);

	} else
	{
		pthread_mutex_lock(&lock);

	}

	long t = (long) sentTime;

	chrono::steady_clock::time_point timer;

	//cout << "Waiting for " << CountToSeconds (t) << " seconds..." << endl;

	auto dur = thisClock.WaitForMicroSeconds (t).time_since_epoch (). count ();

	//cout << "Done!" << endl;

	timer = chrono::steady_clock::now ();

	auto durs = chrono::duration_cast<chrono::microseconds>(timer-systemStart);

	//cout << "dur count = " << CountToSeconds (durs.count ()) << endl;

	stamp = CountToSeconds (durs.count ());

	//pthread_mutex_unlock(&lock);

	if (sentDevice == 'H')
	{
		//pthread_mutex_unlock(&lock);
		//Handled already by lockHDD

	} else if (sentDevice == 'P')
	{
		//pthread_mutex_unlock(&lock);
		//Handled already by lockProj

	} else if (sentDevice == 'M')
	{
		pthread_mutex_unlock(&lockMonitor);

	} else if (sentDevice == 'S')
	{
		pthread_mutex_unlock(&lockScanner);

	} else if (sentDevice == 'K')
	{
		pthread_mutex_unlock(&lockKeyboard);

	} else
	{
		pthread_mutex_unlock(&lock);

	}

	pthread_exit (0);

}

float CountToSeconds (unsigned int sentCount)
{
	return (float) sentCount / (float) 1000000;

}

//Just a function to reduce clutter in code since these lines would have to be anywhere that a timer thread is made
float RunTimerThread (long sentTime, char sentDevice)
{
	pthread_t tid;
	pthread_attr_t attr;

	thread_args argsToSend;
	argsToSend.count = sentTime;
	argsToSend.deviceType = sentDevice;

	pthread_create(&tid, NULL, WaitForMicroSeconds, (void *) &argsToSend);
	pthread_join(tid, NULL);

	return stamp;

}

//sentSize should always be the current config's memory block size
unsigned int GetMemoryAddress (unsigned int sentSize)
{
	unsigned int temp = memoryPosition;
	int sysMemory = currentConfFile.GetSystemMemory ();

	//cout << "Adding " << sentSize << "kb to memory, current MP is " << memoryPosition << ", new is " << memoryPosition + sentSize << endl;

	if ((memoryPosition + sentSize) > sysMemory) //Memory exceeded, need to wrap around and start at 0x000 again
	{
		temp = 0;
		memoryPosition = sentSize;

	} else
	{
		memoryPosition += sentSize; //Move the starting position to it's new location after "allocatiion"


	}

	return temp; //Return the starting position
	
}

void* GetProjectorNumber (void* sentArg)
{
	pthread_mutex_lock(&lockProj);

	long temp = onProjector;

	onProjector = (onProjector + 1) % currentConfFile.GetProjectorQuantity ();

	pthread_mutex_unlock(&lockProj);

	return (void*)temp;

}

void* GetHardDriveNumber (void * sentArg) //sentArg is never used, should just always be null
{
	pthread_mutex_lock(&lockHDD);

	long temp = onHardDrive;

	onHardDrive = (onHardDrive + 1) % currentConfFile.GetHardDriveQuantity ();

	pthread_mutex_unlock(&lockHDD);

	return (void*)temp;

}

int RunHardDriveThread ()
{
	pthread_t tid;
	pthread_attr_t attr;
	void* retVal;	//This is the hard drive number 

	pthread_create(&tid, NULL, GetHardDriveNumber, (void *) NULL);
	pthread_join(tid, &retVal);

	int number = (long) retVal;

	return number;

}

int RunProjectorThread ()
{
	pthread_t tid;
	pthread_attr_t attr;
	void* retVal;	//This is the projector number 

	pthread_create(&tid, NULL, GetProjectorNumber, (void *) NULL);
	pthread_join(tid, &retVal);

	int number = (long) retVal;

	return number;

}