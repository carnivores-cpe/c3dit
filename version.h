#ifndef VERSION_H
#define VERSION_H


#define VERSION_MAJOR	1
#define VERSION_MINOR	5
#define VERSION_RELEASE	1

#define MACROSTRING(x) #x

#define RESOURCE_VERSION VERSION_MAJOR,VERSION_MINOR,VERSION_RELEASE,0
#define RESOURCE_VERSION_STR(maj,min) #maj

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "26";
	static const char MONTH[] = "10";
	static const char YEAR[] = "2011";
	static const double UBUNTU_VERSION_STYLE = 11.10;
	
	//Software Status
	static const char STATUS[] = "Beta";
	static const char STATUS_SHORT[] = "b";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 5;
	static const long BUILD = 376;
	static const long REVISION = 2048;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1018;
	#define RC_FILEVERSION 1,4,376,2048
	#define RC_FILEVERSION_STRING "1, 4, 376, 2048\0"
	static const char FULLVERSION_STRING[] = "1.4.376.2048";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 31;
	

}
#endif //VERSION_h
