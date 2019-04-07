
#ifndef GLOBALS_H
#define GLOBALS_H

//#include "Model.h"

#include <string>
using namespace std;


class Globals
{
public:

	Globals()
	{
		GameLocation = "";
		WorkingDirectory = "";
		EnvironmentMap = "";
		SpecularMap = "";
	}
	~Globals(){}

	//Global Variables
	string		GameLocation;
	string		WorkingDirectory;
	string		EnvironmentMap;
	string		SpecularMap;

	int			WinX, WinY, WinW, WinH;
	bool		Maximized;

	int			BufferColorBits, BufferDepthBits;
	bool		UseMipMaps;
};

class GlobalVariables
{
private:

	GlobalVariables(){}

public:

	static Globals* SharedGlobalVariable()
	{
		static Globals g;
		return &g;
	}
};


#endif