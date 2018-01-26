/*
	Bryce Monaco
	CS 446
	Project 1

	Compile with: g++ Sim01.cpp -o Sim01 -std=c++11

*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

void ScanConfigFile (string cfgFileName);

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

    	ScanConfigFile (argv [1]);

    }

	return 0;

}

void ScanConfigFile (string cfgFileName)
{
	//Open the config fire
	ifstream cfgFile;
	cfgFile.open (cfgFileName);


}