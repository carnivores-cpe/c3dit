
#include "stdafx.h"

#include "IniFile.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>


IniFile::IniSection::IniSection(const std::string& name) :
	m_name(name)
{
}


IniFile::IniSection::~IniSection()
{
	this->m_keys.clear();
}


std::string IniFile::IniSection::GetKey(const std::string& name)
{
	const auto key = this->m_keys.find(name);

	if (key != this->m_keys.end())
	{
		return key->second;
	}

	return "";
}


void IniFile::IniSection::SetKey(const std::string& name, const std::string& value)
{
	this->m_keys[name] = value;
}


IniFile::IniSection& IniFile::IniSection::operator= (const IniFile::IniSection& rhs)
{
	this->m_name = rhs.m_name;
	std::copy(rhs.m_keys.begin(), rhs.m_keys.end(), this->m_keys.begin());
	return *this;
}


IniFile::IniFile(const std::string& file_name, bool enable_exceptions) :
	m_updated(false),
	m_file_name(file_name),
	m_use_exceptions(enable_exceptions)
{
	this->Open(file_name);
}


bool IniFile::Open(const std::string& file_name)
{
	if (file_name.empty()) {
		return false;
	}

	std::ifstream file;

	this->m_updated = false;
	this->m_file_name = file_name;

	file.open(file_name, std::ios::binary);

	if (!file.is_open())
	{
		return false;
	}

	std::string current_section = "";

	while (!file.eof())
	{
		std::string line;
		getline(file, line, '\n');
		line += "\n";

		bool section_open = false;
		//bool key_def = false;
		bool key_value_def = false;
		bool is_string = false;
		std::string section_name = "";
		std::string key_name = "";
		std::string key_value = "";

		/// TODO: C++11 this, REGEX?
		for (size_t i = 0; i < line.size(); i++)
		{
			char c = line[i];

			// Ignore whitespace
			if (!is_string && (c == '\t' || c == ' '))
			{
				continue;
			}

			// -- Check for comment, newline and Windows newline
			if (c == ';' || c == '\n' || (c == '\r' && line[i + 1] == '\n'))
			{
				// -- Run a check for incomplete lines, throw exception/errors
				if (section_open && m_use_exceptions)
				{
					/// TODO: Throw exception if exceptions are enabled
				}
				break;
			}

			if (!is_string && c == '[')
			{
				current_section = "";
				section_open = true;
				continue;
			}
			else if (!is_string && c == ']')
			{
				section_open = false;
				this->m_sections[section_name] = IniSection(section_name);
				current_section = section_name;
				break;
			}

			if (section_open)
			{
				if (c == ' ' && m_use_exceptions)
				{
					/// TODO: Throw exception if exceptions are enabled
				}
				else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.')
				{
					section_name.append(1, c);
				}
			}

			if (current_section != "" && !section_open)
			{
				if (c == '=')
				{
					key_value_def = true;
				}
				else if (c == '\"' && key_value_def)
				{
					if (is_string) is_string = false;
					else is_string = true;
				}
				else if (is_string && (c == ' ' || c == '\\' || c == '~' || c == '-' || c == ':' || c == '/' || c == '[' || c == ']' || c == '.' || c == '(' || c == ')'))
				{
					if (key_value_def)
					{
						key_value.append(1, c);
					}
				}
				else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '.')
				{
					if (key_value_def)
					{
						key_value.append(1, c);
					}
					else
					{
						key_name.append(1, c);
					}
				}
				continue;
			}
		}

		if (key_value_def)
		{
			this->m_sections[current_section].m_keys[key_name] = key_value;
			//key_def = false;
			key_value_def = false;
		}
	}

	file.close();

	/*
	#ifdef _DEBUG
	for (auto& s : this->m_sections)
	{
		IniSection &ig = s->second;
		std::cout << "Section: " << s.m_name << endl;
		std::cout << "Size: " << s.m_keys.size() << endl;
		for (auto& k : s.m_keys)
		{
			std::cout << "\tKey : \"" << k->first << "\" = \"" << k->second << "\"" << endl;
		}
	}
	#endif //_DEBUG
	*/

	return true;
}


bool IniFile::Save(const std::string& file_name)
{
	/// TODO: Actually write this ;-;
	std::ofstream f(file_name);

	if (f.is_open())
	{
		// Some error here
		return false;
	}

	f << "; Auto-generated INI by the IniFile class library." << std::endl;
	f << std::endl;

	for (auto section : m_sections)
	{
		f << '[' << section.first << ']' << std::endl;
	
		for (auto key : section.second.m_keys)
		{
			f << key.first << " = " << key.second << std::endl;
		}

		f << std::endl;
	}

	return false;
}


template<typename T>
T IniFile::GetKey(const std::string& section_name, const std::string& key_name, const T& default_value)
{
	const auto section_i = this->m_sections.find(section_name);

	if (section_i != this->m_sections.end())
	{
		T return_value;
		std::stringstream(section_i->second.getKey(key_name)) >> return_value;
		return return_value;
	}

	return default_value;
}


template <> bool IniFile::GetKey(const std::string& section_name, const std::string& key_name, const bool& default_value)
{
	const auto section_i = this->m_sections.find(section_name);

	if (section_i != this->m_sections.end())
	{
		std::stringstream ss;
		ss << std::nouppercase << section_i->second.GetKey(key_name);
		std::string return_value = ss.str();

		if (!return_value.compare("true") || !return_value.compare("1"))
			return true;
		else if (!return_value.compare("false") || !return_value.compare("0"))
			return false;
	}

	return default_value;
}


template<typename T> void IniFile::SetKey(const std::string& section_name, const std::string& key_name, const T& value)
{
	this->m_updated = false;
	auto section_i = this->m_sections.find(section_name);

	if (section_i != this->m_sections.end())
	{
		IniSection& section = section_i->second;
		std::ostringstream oss;
		oss << value;
		section.SetKey(key_name, oss.str());
		this->m_updated = true;
	}
}

template <> void IniFile::SetKey(const std::string& section_name, const std::string& key_name, const bool& value)
{
	this->m_updated = false;
	auto section_i = this->m_sections.find(section_name);

	if (section_i != this->m_sections.end())
	{
		if (value)
			section_i->second.SetKey(key_name, "true");
		else
			section_i->second.SetKey(key_name, "false");
		this->m_updated = true;
	}
}


static unsigned GetHexByte(const char* c)
{
	unsigned r = 0;

	if (*c >= '0' && *c <= '9') r = *c - '0';
	else if (*c >= 'A' && *c <= 'Z') r = *c - 'A' + 10;
	else if (*c >= 'a' && *c <= 'z') r = *c - 'a' + 10;
	else return -1;

	r <<= 4;
	c++;

	if (*c >= '0' && *c <= '9') r = *c - '0';
	else if (*c >= 'A' && *c <= 'Z') r = *c - 'A' + 10;
	else if (*c >= 'a' && *c <= 'z') r = *c - 'a' + 10;
	else return -1;

	return r;
}


bool IniFile::GetKeyStruct(const std::string& section_name, const std::string& key_name, void* buffer, const size_t buffer_length)
{
	/// NOTE: Allocate a temporary buffer to manipulate, free it at the end. Do not operate directly on the supplied buffer until we are finished.
	const auto section_i = this->m_sections.find(section_name);
	uint8_t* data = new uint8_t[buffer_length + 2];

	if (section_i != this->m_sections.end())
	{
		IniSection& section = section_i->second;
		std::string key_value = section.GetKey(key_name);

		if (key_value.length() != 2 * buffer_length + 2)
		{
			goto KEYSTRUCTFAIL;
		}

		int value = 0;
		unsigned char checksum = 0;
		const char* c = nullptr;
		size_t l = 0u;

		for (c = key_value.data(), l = buffer_length; l != 0; c += 2, l--)
		{
			if ((value = GetHexByte(c)) == -1) goto KEYSTRUCTFAIL;
			*data++ = value;
			checksum += value;
		}

		if ((value = GetHexByte(c)) == -1) goto KEYSTRUCTFAIL;

		bool checksum_ok = ((unsigned char)value == checksum);
		if (checksum_ok) {
			std::memcpy(buffer, data, buffer_length);
		}
		return checksum_ok;
	}

KEYSTRUCTFAIL:
	delete[] data;
	return false;
}

void IniFile::SetKeyStruct(const std::string& section_name, const std::string& key_name, const void* buffer, const size_t buffer_length)
{
	char hex_char[] = "0123456789ABCDEF";
	const unsigned char* bin_buffer = nullptr;
	auto section_i = this->m_sections.find(section_name);
	uint32_t checksum = 0;

	this->m_updated = false;

	if (section_i != this->m_sections.end())
	{
		IniSection& section = section_i->second;
		std::string struct_str = "";
		const unsigned char* cucp_buffer = reinterpret_cast<const unsigned char*>(buffer);

		for (bin_buffer = cucp_buffer; bin_buffer < cucp_buffer + buffer_length; bin_buffer++)
		{
			struct_str += hex_char[*bin_buffer >> 4];
			struct_str += hex_char[*bin_buffer & 0xF];
			checksum += *bin_buffer;
		}

		// Checksum is the total sum of the struct & 0xFF
		struct_str += hex_char[(checksum & 0xF) >> 4];
		struct_str += hex_char[checksum & 0x0F];

		section.SetKey(key_name, struct_str);
		this->m_updated = true;
	}
}


// -- Explicity define the templates (specialisations)
template <> signed IniFile::GetKey<signed>(const std::string& section_name, const std::string& key_name, const signed& default_value);
template <> unsigned IniFile::GetKey<unsigned>(const std::string& section_name, const std::string& key_name, const unsigned& default_value);
template <> float IniFile::GetKey<float>(const std::string& section_name, const std::string& key_name, const float& default_value);
template <> double IniFile::GetKey<double>(const std::string& section_name, const std::string& key_name, const double& default_value);
template <> std::string IniFile::GetKey<std::string>(const std::string& section_name, const std::string& key_name, const std::string& default_value);

template <> void IniFile::SetKey<bool>(const std::string& section_name, const std::string& key_name, const bool& value);
template <> void IniFile::SetKey<signed>(const std::string& section_name, const std::string& key_name, const signed& value);
template <> void IniFile::SetKey<unsigned>(const std::string& section_name, const std::string& key_name, const unsigned& value);
template <> void IniFile::SetKey<float>(const std::string& section_name, const std::string& key_name, const float& value);
template <> void IniFile::SetKey<double>(const std::string& section_name, const std::string& key_name, const double& value);
template <> void IniFile::SetKey<std::string>(const std::string& section_name, const std::string& key_name, const std::string& value);
