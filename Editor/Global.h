#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>


/*
Proposal: Change "Globals" to "AppSettings" or something similar, make a .cpp for it
Define an instance of the class within there and have the header declare an extern for it
Or keep the Get() function and just pull the global scope variable instead of a static
*/
class Settings
{
public:
	Settings()
	{
		bShowAxis = true;
		bShowGrid = true;
		bShowFaces = true;
		bShowWireframe = false;
		bShowTexture = true;
		bShowBones = true;
		bShowJoints = true;
		bUseLighting = false;
		bOldFormatWarnings = true;
		bEnforceGameLimits = false;
		bSpecularReflection = false;
		bEnvironmentMapping = false;

		sGameDirectory = "";
		sProjectDirectory = "./Projects/";
		sProjectFilePath = "";
		for (int i = 0; i < 5; i++)
			sRecentFile[i] = "";
		sEnvironmentMapFile = "ENVMAP.TGA"; // Search in "./Data/", then check sGameDirectory

		iColorBits = 32;
		iDepthBits = 16;
	}

	bool bShowAxis; // Show the gizmo/axis to help orientate the mesh correctly
	bool bShowGrid; // Show the grid to help guide with sizing
	bool bShowFaces; // Show the solid faces of the mesh (if this is false then bShowTexture has no effect!)
	bool bShowWireframe; // Show the wireframe overlayed on the mesh
	bool bShowTexture; // Show the texture of the mesh
	bool bShowBones; // Show the bones
	bool bShowJoints; // Show the joints between bones
	bool bUseLighting; // Process dynamic lighting on the mesh
	bool bOldFormatWarnings;
	bool bEnforceGameLimits; // Run checks and disallow any actions that would make a project not valid for the original games.
	bool bSpecularReflection; // Render specular reflections of light
	bool bEnvironmentMapping; // Render the environment map

	std::string sGameDirectory;
	std::string sProjectDirectory;
	std::string sProjectFilePath;
	std::string sRecentFile[5];
	std::string sEnvironmentMapFile;
	
	int	iColorBits; // The number of bits allocated to the color channels of the renderer per sample
	int iDepthBits; // The number of bits allocated to the depth channel of the renderer per sample
};


class GlobalContainer
{
private:
	GlobalContainer(){}

public:

	static Settings* GlobalVar()
	{
		static Settings g;
		return &g;
	}

};

#endif