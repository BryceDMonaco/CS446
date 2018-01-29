/*
	Bryce Monaco
	CS 446
	Project 1

	Compile with: g++ Sim01.cpp -o Sim01 -std=c++11

*/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <stdio.h>

using namespace std;

//Global configuration variables
float versionNumber;		//Float to allow 1.XX

string metaDataFilePath;	//Path to the current mdf, note only ScanConfigFile () can modify this
string logFilePath;			//Path to the global log file (if one is used)

//ifstream metaDataFile;	//Made local variable in meta data run function
ofstream logFile;

int monitorDispTime;		//msec
int processorCycleTime;		//msec
int scannerCycleTime;		//msec
int hardDriveCycleTime;		//msec
int keyboardCycleTime;		//msec
int memoryCycleTime;		//msec
int projectorCycleTime;		//msec

bool shouldLogToFile = false;
bool shouldLogToMonitor = false;

bool RunMetaDataFile ();
bool ScanConfigFile (string cfgFileName);
bool ParseCommand (string sentCommand);
string ScanNextLine (ifstream& sentStream);
string ScanNextLine (ifstream& sentStream, char delimChar);
void OutputConfigFileData ();

int main (int argc, char* argv[])
{

	 if (argc < 2) 
	 {
        // Tell the user how to run the program
        cout << "Usage: " << argv[0] << " NAME_OF_CONFIG_FILE_WITH_EXTENSION" << endl;

        return 0;
    } else
    {
    	//cout << "Opening config file \"" << argv [1] << "\"..." << endl;

    	if (!ScanConfigFile (argv [1])) //Function returns false if there was an error
    	{
    		cout << "There was an error with the config file." << endl;

    	}

    }

    //From this point on, the config file has been read successfully

    OutputConfigFileData ();

    RunMetaDataFile ();

	return 0;

}

//Returns true if the entire file is parsed correctly
bool RunMetaDataFile ()
{
	ifstream mdfFile;
	mdfFile.open (metaDataFilePath);
	string currentLine;

	cout << "Opened" << endl;

	//Check if the file is empty
	if (mdfFile.eof ())
	{
		cout << "Error: Empty Meta Data File" << endl;

		mdfFile.close ();
		return false;

	}

	currentLine = ScanNextLine (mdfFile);

	//Check for start line
	if (currentLine.find ("Start Program Meta-Data Code:") == string::npos) //True if not found
	{
		//First line does not contain start line
		cout << "Error: No start program command" << endl;

		mdfFile.close ();
		return false;

	}

	currentLine = ScanNextLine (mdfFile, ';');

	//Check for start command
	if (currentLine.find ("S{begin}0") == string::npos) //True if not found
	{
		//First line does not contain start command
		cout << "Error: No start command found" << endl;

		mdfFile.close ();
		return false;

	}

	bool reachedEndOfFile = false;

	do
	{
		if (mdfFile.eof ())
		{
			cout << "Error: mdf file ended unexpectedly" << endl;

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

				//cout << "Found EOF" << endl;

			} else
			{	
				//cout << "Parsing Command: " << currentLine << endl;
 				//S=OS, A=Application, P=Process, I=Input, O=Output 
				if (!ParseCommand (currentLine))
				{
					cout << "There was an error with the command \"" << currentLine << "\"" << endl;

					mdfFile.close ();
					return false;

				}

			}

		}

		



	} while (!reachedEndOfFile);

	return true;

}

//Returns true if successful, false otherwise
bool ScanConfigFile (string cfgFileName)
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

	//A standard config file should contain exactly 13 lines, first and last are start and end, 11 values
	for (int i = 0; i < 11; i++)
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

			} else if (currentLine.find ("Monitor") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Monitor display time {msec}:%d", &monitorDispTimeTEMP);

				//cout << "Found Monitor Time " << monitorDispTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Processor") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Processor cycle time {msec}:%d", &processorCycleTimeTEMP);

				//cout << "Found Processor Time " << processorCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Scanner") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Scanner cycle time {msec}:%d", &scannerCycleTimeTEMP);

				//cout << "Found Scanner Time " << scannerCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Hard") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Hard drive cycle time {msec}:%d", &hardDriveCycleTimeTEMP);

				//cout << "Found HDD Time " << hardDriveCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Keyboard") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Keyboard cycle time {msec}:%d", &keyboardCycleTimeTEMP);

				//cout << "Found Keyboard Time " << keyboardCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Memory") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Memory cycle time {msec}:%d", &memoryCycleTimeTEMP);

				//cout << "Found Memory Time " << memoryCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Projector") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Projector cycle time {msec}:%d", &projectorCycleTimeTEMP);

				//cout << "Found Projector Time " << projectorCycleTimeTEMP << endl;

				//break;

			} else if (currentLine.find ("Log:") != string::npos)
			{

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

    			}

				//break;

			} else if (currentLine.find ("Log File") != string::npos)
			{

    			//It is easiest to pull this out as a substring, assumes path is < 50 chars long
				//This assumes there is a space after "File Path:"
				logFilePathTEMP = currentLine.substr (15, 50);

				//cout << "Found Log Path: " << logFilePathTEMP << endl;

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
		cout << "Error: No end command" << endl;

		cfgFile.close ();
		return false;

	} else
	{
		//Config file was read successfully, now store all read values into their global vars
		versionNumber = versionNumberTEMP;

		metaDataFilePath = metaDataFilePathTEMP;
		logFilePath = logFilePathTEMP;	

		monitorDispTime = monitorDispTimeTEMP;
		processorCycleTime = processorCycleTimeTEMP;
		scannerCycleTime = scannerCycleTimeTEMP;
		hardDriveCycleTime = hardDriveCycleTimeTEMP;
		keyboardCycleTime = keyboardCycleTimeTEMP;
		memoryCycleTime = memoryCycleTimeTEMP;
		projectorCycleTime = projectorCycleTimeTEMP;

		shouldLogToFile = shouldLogToFileTEMP;
		shouldLogToMonitor = shouldLogToMonitorTEMP;

	}

	cfgFile.close ();
	return true;


}

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
		if (sentCommand.find ("begin") == string::npos)
		{
			//First line does not contain start command
			cout << "Error: No \"begin\" command found" << endl;

			return false;

		} else
		{
			return true;

		}

	} else if (commandChar == 'A')
	{
		if (sentCommand.find ("begin") == string::npos)
		{
			//First line does not contain start command
			cout << "Error: No \"begin\" command found" << endl;

			return false;

		} else
		{
			return true;

		}

	} else if (commandChar == 'P')
	{
		if (sentCommand.find ("run") == string::npos)
		{
			//First line does not contain start command
			cout << "Error: No \"run\" command found" << endl;

			return false;

		} else
		{
			int duration = 0;

			if (sscanf(sentCommand.c_str(), " P{run}%d", &duration) <= 0) //Couldn't find a duration
			{
				cout << "Error: no duration found" << endl;

			}

			cout << sentCommand << " - " << (duration * processorCycleTime) << endl;

			return true;

		}


	} else if (commandChar == 'I')
	{
		cout << "Input!" << endl;

		return true;

	} else if (commandChar == 'O')
	{
		cout << "Output!" << endl;

		return true;

	} else if (commandChar == 'M')
	{
		cout << "Memory!" << endl;

		return true;

	} else
	{
		cout << "Error: Unknown command char \'" << commandChar << "\'" << endl;

		return false;

	}

}

string ScanNextLine (ifstream& sentStream)
{
	char line[50];

	sentStream.getline (line, 50);

	string lineStr = line;

	return lineStr;

}

//Overload with delim char added
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

void OutputConfigFileData ()
{
	cout << "Configuration File Data" << endl;
	cout << "Monitor = " << monitorDispTime << "ms/cycle" << endl;
	cout << "Processor = " << processorCycleTime << "ms/cycle" << endl;
	cout << "Scanner = " << scannerCycleTime << "ms/cycle" << endl;
	cout << "Hard Drive = " << hardDriveCycleTime << "ms/cycle" << endl;
	cout << "Keyboard = " << keyboardCycleTime << "ms/cycle" << endl;
	cout << "Memory = " << memoryCycleTime << "ms/cycle" << endl;
	cout << "Projector = " << projectorCycleTime << "ms/cycle" << endl;
	cout << "Logged to: ";

	if (shouldLogToMonitor && shouldLogToFile)
	{
		cout << "Monitor and " << logFilePath << endl;

	} else if (shouldLogToMonitor)
	{
		cout << "Monitor" << endl;

	} else if (shouldLogToFile)
	{
		cout << logFilePath << endl;

	}

	return;
}