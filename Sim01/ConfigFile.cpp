#include <string>
#include <iostream>

using namespace std;

class ConfigFile
{
	public:
		ConfigFile();
		~ConfigFile();

		//Assignment Functions
		void SetVersionNumber (float sentValue);
		void SetMDFPath (string sentPath);
		void SetLGFPath (string sentPath);
		void SetMonitorTime (int sentValue);
		void SetProcessorTime (int sentValue);
		void SetScannerTime (int sentValue);
		void SetHardDriveTime (int sentValue);
		void SetKeyboardTime (int sentValue);
		void SetMemoryTime (int sentValue);
		void SetProjectorTime (int sentValue);

		//Access Functions
		float GetVersionNumber () const;
		string GetMDFPath () const;
		string GetLGFPath () const;
		int GetMonitorTime () const;
		int GetProcessorTime () const;
		int GetScannerTime () const;
		int GetHardDriveTime () const;
		int GetKeyboardTime () const;
		int GetMemoryTime () const;
		int GetProjectorTime () const;

		bool ShouldLogToFile () const;
		bool ShouldLogToMonitor () const;

		//Operator Overloads
		ConfigFile& operator= (const ConfigFile& sentConfig);

	private:
		float versionNumber;		//Float to allow 1.XX

		string metaDataFilePath;	//Path to the current mdf, note only ScanConfigFile () can modify this
		string logFilePath;			//Path to the global log file (if one is used)

		//ifstream metaDataFile	//Made local variable in meta data run function
		//ofstream logFile			//This should be a global var in the main Sim

		int monitorDispTime;		//msec
		int processorCycleTime;		//msec
		int scannerCycleTime;		//msec
		int hardDriveCycleTime;		//msec
		int keyboardCycleTime;		//msec
		int memoryCycleTime;		//msec
		int projectorCycleTime;		//msec

		bool shouldLogToFile;
		bool shouldLogToMonitor;
	
};

ConfigFile::ConfigFile ()
{
	shouldLogToFile = false;
	shouldLogToMonitor = false;

}

ConfigFile::~ConfigFile ()
{
	//Nothing to destruct

}

/////////////////////////////////////////////////////
/// Assignment Functions                          ///
/////////////////////////////////////////////////////

void ConfigFile::SetVersionNumber (float sentValue)
{
	versionNumber = sentValue;

	return;

}

void ConfigFile::SetMDFPath (string sentPath)
{
	metaDataFilePath = sentPath;

	return;

}
 
void ConfigFile::SetLGFPath (string sentPath)
{
	logFilePath = sentPath;

	return;

}

void ConfigFile::SetMonitorTime (int sentValue)
{
	monitorDispTime = sentValue;

	return;

}

void ConfigFile::SetProcessorTime (int sentValue)
{
	processorCycleTime = sentValue;

	return;

}

void ConfigFile::SetScannerTime (int sentValue)
{
	scannerCycleTime = sentValue;

	return;

}

void ConfigFile::SetHardDriveTime (int sentValue)
{
	hardDriveCycleTime = sentValue;

	return;

}

void ConfigFile::SetKeyboardTime (int sentValue)
{
	keyboardCycleTime = sentValue;

	return;

}

void ConfigFile::SetMemoryTime (int sentValue)
{
	memoryCycleTime = sentValue;

	return;

}

void ConfigFile::SetProjectorTime (int sentValue)
{
	projectorCycleTime = sentValue;

	return;

}

/////////////////////////////////////////////////////
/// Access Functions                              ///
/////////////////////////////////////////////////////

float ConfigFile::GetVersionNumber () const
{
	return versionNumber;

}

string ConfigFile::GetMDFPath () const
{
	return metaDataFilePath;

}

string ConfigFile::GetLGFPath () const
{
	return logFilePath;

}

int ConfigFile::GetMonitorTime () const
{
	return monitorDispTime;

}

int ConfigFile::GetProcessorTime () const
{
	return processorCycleTime;

}

int ConfigFile::GetScannerTime () const
{
	return scannerCycleTime;

}

int ConfigFile::GetHardDriveTime () const
{
	return hardDriveCycleTime;

}

int ConfigFile::GetKeyboardTime () const
{
	return keyboardCycleTime;

}

int ConfigFile::GetMemoryTime () const
{
	return memoryCycleTime;

}

int ConfigFile::GetProjectorTime () const
{
	return projectorCycleTime;

}

bool ConfigFile::ShouldLogToFile () const
{
	return shouldLogToFile;

}

bool ConfigFile::ShouldLogToMonitor () const
{
	return shouldLogToMonitor;

}

ConfigFile& ConfigFile::operator= (const ConfigFile& sentConfig)
{
	versionNumber = sentConfig.GetVersionNumber ();

	cout << "OP= recieved VN: " << sentConfig.GetVersionNumber () << " VN set to " << versionNumber << endl;

	metaDataFilePath = sentConfig.GetMDFPath ();
	logFilePath = sentConfig.GetLGFPath ();

	monitorDispTime = sentConfig.GetMonitorTime ();			//msec
	processorCycleTime = sentConfig.GetProcessorTime ();	//msec
	scannerCycleTime = sentConfig.GetScannerTime ();		//msec
	hardDriveCycleTime = sentConfig.GetHardDriveTime ();	//msec
	keyboardCycleTime = sentConfig.GetKeyboardTime ();		//msec
	memoryCycleTime = sentConfig.GetMemoryTime ();			//msec
	projectorCycleTime = sentConfig.GetProjectorTime ();	//msec

	shouldLogToFile = sentConfig.ShouldLogToFile ();
	shouldLogToMonitor = sentConfig.ShouldLogToMonitor ();

	return *this;

}