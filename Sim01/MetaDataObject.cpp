#include <string>

using namespace std;

class MetaDataObject
{
	public:
		MetaDataObject();
		~MetaDataObject();

		bool SetCommand (char sentCommand); 	//Checks if the command is valid (SAPIOM), false if it isn't (Note: X is default, unassigned)
		bool SetKeyword (string sentWord);		//Checks if the keyword is valid, given it's command, command must be assigned first
		void SetDuration (int sentDuration);

		char GetCommand ();
		string GetKeyword ();
		int GetDuration ();

		bool IsValidCommand (char sentCommand);

	private:
		char command;
		string keyword;
		int duration;
	
};

MetaDataObject::MetaDataObject ()
{
	command = 'X'; //X is unassigned
	keyword = "NONE";
	duration = -1;

}

MetaDataObject::~MetaDataObject ()
{
	//Nothing to destruct

}

bool MetaDataObject::SetCommand (char sentCommand)
{
	if (IsValidCommand (sentCommand))
	{
		command = sentCommand;
		keyword = "NONE"; //Reset the keyword

		return true;

	} else
	{
		return false; //Invalid command

	}
}

//Stores the sent keyword but first checks to make sure it is valid for its command
bool MetaDataObject::SetKeyword (string sentWord)
{
	if (command == 'X') //Command has not been assigned yet
	{
		return false;

	} else
	{
		if (command == 'S')
		{
			if (sentWord == "begin" || sentWord == "finish")
			{
				keyword = sentWord;

				return true;

			} else
			{
				return false;

			}

		} else if (command == 'A')
		{
			if (sentWord == "begin" || sentWord == "finish")
			{
				keyword = sentWord;

				return true;

			} else
			{
				return false;

			}

		} else if (command == 'P')
		{
			if (sentWord == "run")
			{
				keyword = sentWord;

				return true;

			} else
			{
				return false;

			}

		} else if (command == 'I')
		{
			if (sentWord == "hard drive" || sentWord == "keyboard" || sentWord == "scanner")
			{
				keyword = sentWord;

				return true;

			} else
			{
				return false;

			}

		} else if (command == 'O')
		{
			if (sentWord == "hard drive" || sentWord == "monitor" || sentWord == "projector")
			{
				keyword = sentWord;

				return true;

			} else
			{
				return false;

			}

		} else if (command == 'M')
		{
			if (sentWord == "block" || sentWord == "allocate")
			{
				keyword = sentWord;

				return true;

			} else
			{
				return false;

			}

		} else //The stored command is not recognized
		{
			return false;

		}

	}
}

void MetaDataObject::SetDuration (int sentDuration)
{
	if (sentDuration >= 0)
	{
		duration = sentDuration;

	}

	return;

}

char MetaDataObject::GetCommand ()
{
	return command;

}
 
string MetaDataObject::GetKeyword ()
{
	return keyword;

}

int MetaDataObject::GetDuration ()
{
	return duration;

}


//Should mostly be used internally by this class but it's public just in case outside classes need it
bool MetaDataObject::IsValidCommand (char sentCommand)
{
	return (sentCommand == 'S' || sentCommand == 'A' || sentCommand == 'P' || sentCommand == 'I' || sentCommand == 'O' || sentCommand == 'M');

}