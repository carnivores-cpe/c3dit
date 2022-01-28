
#ifndef GLOBALS_H
#define GLOBALS_H

//#include "Model.h"

#include <string>


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
	std::string		GameLocation;
	std::string		WorkingDirectory;
	std::string		EnvironmentMap;
	std::string		SpecularMap;

	int			WinX, WinY, WinW, WinH;
	bool		Maximized;

	int			BufferColorBits, BufferDepthBits;
	bool		UseMipMaps;
};

class GlobalVariables
{
private:

	GlobalVariables(){}
	GlobalVariables(const GlobalVariables&) = delete;
	GlobalVariables(GlobalVariables&&) = delete;

public:

	static Globals* SharedGlobalVariable()
	{
		static Globals g;
		return &g;
	}
};


#endif