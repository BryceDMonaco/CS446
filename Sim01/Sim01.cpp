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

ifstream metaDataFile;	
ofstream logFile;

int monitorDispTime;		//msec
int processorCycleTime;		//msec
int scannerCycleTime;		//msec
int hardDriveCycleTime;		//msec
int keyboardCycleTime;		//msec
int projectorCycleTime;		//msec

bool shouldLogToFile;
bool shouldLogToConsole;

bool ScanConfigFile (string cfgFileName);
string ScanNextLine (ifstream& sentStream);

int main (int argc, char* argv[])
{

	 if (argc < 2) 
	 {
        // Tell the user how to run the program
        cout << "Usage: " << argv[0] << " NAME_OF_CONFIG_FILE" << endl;

        return 0;
    } else
    {
    	cout << "Opening config file \"" << argv [1] << "\"..." << endl;

    	if (ScanConfigFile (argv [1]))
    	{
    		cout << "Config file read successfully." << endl;

    	} else
    	{
    		cout << "There was an error with the config file." << endl;

    	}

    }

	return 0;

}

//Returns true if successful, false otherwise
bool ScanConfigFile (string cfgFileName)
{
	//Open the config fire
	ifstream cfgFile;
	cfgFile.open (cfgFileName);
	string currentLine;

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
			cout << "Error: file ended unexpectedly" << endl;

			cfgFile.close ();
			return false;

		} else
		{
			//Note, scanning is done this way because we cannot assume that the file
			//will always be in the same order
			currentLine = ScanNextLine (cfgFile);

			if (currentLine.find ("Version/Phase:") != string::npos)
			{
				stringstream ss(currentLine);
				string temp = "Version/Phase: ";

				ss >> temp >> versionNumber;

				cout << "Found Version " << versionNumber << endl;

				//break;

			} else if (currentLine.find ("File Path:") != string::npos && currentLine.find ("Log") == string::npos) //Need to make sure that this isn't confused for the "Log File Path:" field
			{
				//It is easiest to pull this out as a substring, assumes path is < 50 chars long
				//This assumes there is a space after "File Path:"
				metaDataFilePath = currentLine.substr (11, 50);

				cout << "Found mdf Path: " << metaDataFilePath << endl;

				//break;

			} else if (currentLine.find ("Monitor") != string::npos)
			{

    			sscanf(currentLine.c_str(), "Monitor display time {msec}:%d", &monitorDispTime);

				cout << "Found Monitor Time " << monitorDispTime << endl;

				break;

			}


		}

	}

	cfgFile.close ();
	return true;


}

string ScanNextLine (ifstream& sentStream)
{
	char line[50];

	sentStream.getline (line, 50);

	string lineStr = line;

	return lineStr;

}