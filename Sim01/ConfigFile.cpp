#include <string>

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
		float GetVersionNumber ();
		string GetMDFPath ();
		string GetLGFPath ();
		int GetMonitorTime ();
		int GetProcessorTime ();
		int GetScannerTime ();
		int GetHardDriveTime ();
		int GetKeyboardTime ();
		int GetMemoryTime ();
		int GetProjectorTime ();

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

float ConfigFile::GetVersionNumber ()
{
	return versionNumber;

}

string ConfigFile::GetMDFPath ()
{
	return metaDataFilePath;

}

string ConfigFile::GetLGFPath ()
{
	return logFilePath;

}

int ConfigFile::GetMonitorTime ()
{
	return monitorDispTime;

}

int ConfigFile::GetProcessorTime ()
{
	return processorCycleTime;

}

int ConfigFile::GetScannerTime ()
{
	return scannerCycleTime;

}

int ConfigFile::GetHardDriveTime ()
{
	return hardDriveCycleTime;

}

int ConfigFile::GetKeyboardTime ()
{
	return keyboardCycleTime;

}

int ConfigFile::GetMemoryTime ()
{
	return memoryCycleTime;

}

int ConfigFile::GetProjectorTime ()
{
	return projectorCycleTime;

}
