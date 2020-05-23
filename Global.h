
#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>

class Globals
{
public:
	Globals()
	{
		this->bUseLighting = false;
		this->bShowAxis = true;
		this->bShowGrid = true;
		this->bShowJoints = true;
		this->bShowBones = true;
		this->bShowSolid = true;
		this->bShowTexture = true;
		this->bShowWireframe = false;
	}

	bool			bShowAxis,
					bShowGrid,
					bShowFaces,
					bShowWireframe,
					bShowSolid,
					bShowTexture,
					bShowBones,
					bShowJoints,
					bUseLighting;

	std::string		sSpecularMap,
					sEnvironmentMap,
					sGameDirectory,
					sRecentFile[5];
	
	unsigned int	iColorBits,
					iDepthBits;
};

class GlobalContainer
{
private:
	GlobalContainer(){}

public:

	static Globals* GlobalVar()
	{
		static Globals g;
		return &g;
	}

};

#endif