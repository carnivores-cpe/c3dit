
#include "header.h"
#include "Ini.h"
#include <cstdio>
#include <cstdlib>


//===========================================================//
// class._Key
//===========================================================//
_Key::_Key()
{
	mName = "";
	mValue = "";
}

_Key::_Key( string lName, string lValue )
{
	mName = lName;
	mValue = lValue;
}

_Key::~_Key()
{
}

//===========================================================//
// class._Group
//===========================================================//
_Group::_Group()
{
}

_Group::_Group( string lName )
{
	this->mName = lName;
}

_Group::~_Group()
{
	if ( mKeys.size() > 0 ) mKeys.clear();
}

//===========================================================//
// class.INIFILE
//===========================================================//
INIFILE::INIFILE()
{
}

INIFILE::INIFILE( string FileName )
{
	FILE *fp;
	
	Name = FileName;

	// Attempt to open the file
	fp = fopen( FileName.c_str(), "r" );

	if ( fp == NULL )
	{
		// It didn't exist so we make a new one
		fp = fopen( FileName.c_str(), "w" );

		fputs( "; C3Dit Settings\n", fp );

		fclose( fp );
	}
	else
	{

		string lCurSection = "";
		string lCurKey = "";
		string lCurValue = "";

		while( !feof( fp ) )
		{
			char lBuf[512];
			fgets( lBuf, 511, fp );
			string lLine = lBuf;

			int nonspaceindex = lLine.find_first_not_of(" \t");

			// ignore comments ";" and newlines
			if ( lLine[nonspaceindex] == ';' ) continue;
			if ( lLine[nonspaceindex] == '\n' ) continue;
			if ( lLine[nonspaceindex] == '\0' ) continue;

			// Section
			if ( lLine[nonspaceindex] == '[' )
			{
				lCurSection = "";
				for ( int i = 0; lLine[ nonspaceindex + 1 + i ] != ']'; ++i )
					lCurSection += lLine[ nonspaceindex + 1 + i ];

				printf("IniSection: %s\n", lCurSection.c_str() );

				this->Groups.push_back( _Group( lCurSection ) );

				lCurSection = "";
			}

			// Key + Value
			if ( (lLine[nonspaceindex] >= 'A' && lLine[nonspaceindex] <= 'Z') || (lLine[nonspaceindex] >= 'a' && lLine[nonspaceindex] <= 'z') )
			{
				lCurKey = "";
				for ( int i = 0; true; ++i )
				{
					if ( lLine[ nonspaceindex + i ] == ' ' ) break;
					if ( lLine[ nonspaceindex + i ] == '\t' ) break;
					if ( lLine[ nonspaceindex + i ] == '=' ) break;
					lCurKey += lLine[ nonspaceindex + i ];
				}

				// Value
				lCurValue = "";
				int iValBegin = lLine.find_first_of("=") + 1;
				for ( int i = 0; true; ++i )
				{
					if ( lLine[ iValBegin + i ] == '\t' ) continue;
					if ( lLine[ iValBegin + i ] == '\n' ) break;
					if ( lLine[ iValBegin + i ] == '\0' ) break;
					if ( lLine[ iValBegin + i ] == ';' ) break;
					lCurValue += lLine[ iValBegin + i ];
				}

				printf( "\tIniKey: %s = %s\n", lCurKey.c_str(), lCurValue.c_str() );
				int iCurSection = this->Groups.size() - 1;
				this->Groups[ iCurSection ].mKeys.push_back( _Key( lCurKey, lCurValue ) );

				lCurKey = "";
				lCurValue = "";
			}
		}

		fclose( fp );
	}
}

INIFILE::~INIFILE()
{
	FILE *fp = NULL;

	fp = fopen( Name.c_str(), "w" );

	for (auto s = 0u; s<Groups.size(); ++s ) {
		fprintf( fp, "[%s]\n", Groups[s].mName.c_str()  );

		for (auto k=0u; k<Groups[s].mKeys.size(); ++k ) {
			fprintf( fp, "%s=%s\n", Groups[s].mKeys[k].mName.c_str(), Groups[s].mKeys[k].mValue.c_str() );
		}

		fputs( "\n", fp );
	}

	fclose( fp );

	this->Groups.clear();
}


int	INIFILE::GetValueInt( string Group, string Key, int Default )
{
	int _Result = Default;

	for (int s = 0; s < this->Groups.size(); ++s )
	{
		if ( this->Groups[s].mName == Group )
		for ( int k = 0; k < this->Groups[s].mKeys.size(); ++k )
		{
			if ( this->Groups[s].mKeys[k].mName == Key )
			{
				_Result = atoi( this->Groups[s].mKeys[k].mValue.c_str() );
			}
		}
	}

	return _Result;
}

float INIFILE::GetValueFloat( string Group, string Key, float Default )
{
	float _Result = Default;

	for (int s = 0; s < this->Groups.size(); ++s )
	{
		if ( this->Groups[s].mName == Group )
		for ( int k = 0; k < this->Groups[s].mKeys.size(); ++k )
		{
			if ( this->Groups[s].mKeys[k].mName == Key )
			{
				_Result = atof( this->Groups[s].mKeys[k].mValue.c_str() );
			}
		}
	}

	return _Result;
}

string INIFILE::GetValueString( string Group, string Key, string Default )
{
	string _Result = Default;

	for (int s = 0; s < this->Groups.size(); ++s )
	{
		if ( this->Groups[s].mName == Group )
		for ( int k = 0; k < this->Groups[s].mKeys.size(); ++k )
		{
			if ( this->Groups[s].mKeys[k].mName == Key )
			{
				_Result = this->Groups[s].mKeys[k].mValue;
			}
		}
	}

	return _Result;
}

void INIFILE::SetValueInt( string Section, string Key, int Value )
{
	char cBuf[MAX_PATH];
	sprintf( cBuf, "%d", Value );
	this->SetValueString( Section, Key, cBuf );
}

void INIFILE::SetValueFloat( string Section, string Key, float Value )
{
	char cBuf[260];
	sprintf( cBuf, "%f", Value );
	this->SetValueString( Section, Key, cBuf );
}

void INIFILE::SetValueString( string Section, string Key, string Value )
{
	int iSectionIndex = this->pGetSectionIndex( Section );
	
	// Create the section if it does not exist.
	if ( iSectionIndex == -1 )
	{
		this->Groups.push_back( _Group( Section ) );
		iSectionIndex = this->pGetSectionIndex( Section );
	}

	int iKeyIndex = this->pGetKeyIndex( iSectionIndex, Key );

	// Create the key in the section if it does not exist.
	if ( iKeyIndex == -1 )
	{
		this->Groups[ iSectionIndex ].mKeys.push_back( _Key( Key, "(null)" ) );
		iKeyIndex = this->pGetKeyIndex( iSectionIndex, Key );
	}

	// Set the key's value
	this->Groups[ iSectionIndex ].mKeys[ iKeyIndex ].mValue = Value;
}

int INIFILE::pGetSectionIndex( string Section )
{
	for (int s = 0; s < this->Groups.size(); ++s )
	{
		if ( this->Groups[s].mName == Section ) return s;
	}

	return -1;
}

int INIFILE::pGetKeyIndex( int Section, string Key )
{
	for ( int k = 0; k < this->Groups[Section].mKeys.size(); ++k )
	{
		if ( this->Groups[Section].mKeys[k].mName == Key ) return k;
	}

	return -1;
}
