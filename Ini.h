
#ifndef INI_H
#define INI_H

#include <vector>
#include <string>

using namespace std;


class _Key
{
public:

	_Key();
	_Key( string, string );
	~_Key();

	string mName;
	string mValue;
};

class _Group
{
private:
public:
	_Group();
	_Group(string);
	~_Group();

	string mName;
	vector<_Key> mKeys;
};


class INIFILE
{
private:
	int pGetSectionIndex( string );
	int pGetKeyIndex( int, string );

	vector<_Group> Groups;
	string Name;

public:

	INIFILE();
	INIFILE( string );
	~INIFILE();

	int		GetValueInt( string, string, int );
	float	GetValueFloat( string, string, float );
	string	GetValueString( string, string, string );

	void	SetValueInt( string, string, int );
	void	SetValueFloat( string, string, float );
	void	SetValueString( string, string, string );
};

#endif // INI_H
