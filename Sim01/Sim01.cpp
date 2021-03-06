/*
	Bryce Monaco
	CS 446
	Project 1

	Compile with: g++ Sim01.cpp -o Sim01 -std=c++11 or makefile ("make" with the cd being the Sim01 directory)

	Notes: 	In the case of a fatal error the simulation will print directly to the console since
			the error might be output-related. 

			(Note to grader: MetaDataObject.cpp is not included in the submission, see line 28)

			Most functions contain cout calls which are commented out, these were used for debug purposes 
			and are left as comments incase they are needed again as well as to show a bit of the debug process.

	TODO:	Store mdf commands in an ADT

*/

#include <iostream>
#include <ostream>
#include <fstream>
#include <string>
#include <sstream> 	//Not actually used in latest version
#include <vector>	//Not actually used in latest version

#include "ConfigFile.cpp"
//#include "MetaDataObject.cpp" //This doesn't get used, might be useful for future projects

#include <stdio.h>	//Used primarily in ParseCommand for sscanf functionality

using namespace std;

ofstream logFile; //Gets opened and closed whenever a new config file is scanned successfully
string currentLGFPath; //Stores the path to the current log file so that it doesn't need to be passed as an argument to multiple functions

ConfigFile currentConfFile; //Assuming ScanConfigFile () runs successfully, this contains all the information read in from the current config file
vector<ConfigFile> allConfigFiles; //Not used, currently once a config file is done it just gets overwritten in the currentConfFile

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

int main (int argc, char* argv[])
{

	 if (argc < 2) 
	 {
        // Tell the user how to run the program
        cout << "Usage: " << argv[0] << " NAME_OF_CONFIG_FILE_WITH_EXTENSION (Multiple config files allowed with spaces between names)" << endl;

        return 0;
    } else
    {
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

		    if (!OutputToLog ("Configuration File Data", true)) //OutputToLog returns false if output is incorrectly configured
		    {
		    	cout << "FATAL ERROR: There was an output error. Closing the simulation." << endl;

		    	return 0;

		    }

		    OutputConfigFileData (currentConfFile.ShouldLogToFile (), currentConfFile.ShouldLogToMonitor ());

		    RunMetaDataFile ();

		    OutputToLog(string ("(End of config file: ") + argv[i] + ")", true); //Not required output but this helps make the log easier to read with multiple config files

		    if ((i + 1) < argc)
		    {
		    	OutputToLog ("\n", false); //Create an extra line between multiple config files, again not required output but it makes the log easier to read

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

	OutputToLog ("\nMeta-Data Metrics", true);
	//cout << endl << "Meta-Data Metrics" << endl;

	//Check if the file is empty
	if (mdfFile.eof ())
	{
		OutputToLog ("Error: Empty Meta Data File", true);
		//cout << "Error: Empty Meta Data File" << endl;

		mdfFile.close ();
		return false;

	}

	currentLine = ScanNextLine (mdfFile);

	//Check for start line
	if (currentLine.find ("Start Program Meta-Data Code:") == string::npos) //True if not found
	{
		//First line does not contain start line
		OutputToLog ("Error: No start program command", true);
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

			} else if (currentLine.find ("Monitor d") != string::npos) //Added "d" because it was conflicting with Log to Monitor
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

			OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetProcessorTime ()), true);
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
			OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetHardDriveTime ()), true);
			//cout << sentCommand << " - " << (duration * hardDriveCycleTime) << endl;

		} else if (keyword.find ("keyboard") != string::npos)
		{
			OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetKeyboardTime ()), true);
			//cout << sentCommand << " - " << (duration * keyboardCycleTime) << endl;

		} else if (keyword.find ("scanner") != string::npos)
		{
			OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetScannerTime ()), true);
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
			OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetHardDriveTime ()), true);
			//cout << sentCommand << " - " << (duration * hardDriveCycleTime) << endl;

		} else if (keyword.find ("monitor") != string::npos)
		{
			OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetMonitorTime ()), true);
			//cout << sentCommand << " - " << (duration * monitorDispTime) << endl;

		} else if (keyword.find ("projector") != string::npos)
		{
			OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetProjectorTime ()), true);
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

		OutputToLog (string (sentCommand) + " - " + to_string (duration * currentConfFile.GetMemoryTime ()), true);
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