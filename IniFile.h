#pragma once

#include <string>
#include <map>


class IniFile
{
private:
	class IniSection
	{
	protected:
		friend class IniFile;

		std::string							m_name;
		std::map<std::string, std::string>	m_keys;

	public:

		IniSection(const std::string& name = "");
		~IniSection();

		IniSection& operator= (const IniSection& rhs);

		std::string GetKey(const std::string& name);
		void SetKey(const std::string& name, const std::string& value);
	};

	bool				m_updated;
	std::string			m_file_name;
	bool				m_use_exceptions;
	std::map<std::string, IniSection>	m_sections;

public:

	IniFile(const std::string& file_name = "", bool enable_exceptions = false);

	bool Open(const std::string& file_name);
	bool Save(const std::string& file_name);

	template<typename T> T GetKey(const std::string& section_name, const std::string& key_name, const T& default_value);
	template<typename T> void SetKey(const std::string& section_name, const std::string& key_name, const T& value);
	bool GetKeyStruct(const std::string& section_name, const std::string& key_name, void* buffer, const size_t buffer_length);
	void SetKeyStruct(const std::string& section_name, const std::string& key_name, const void* buffer, const size_t buffer_length);
	//bool GetKeyStruct(const std::string& section_name, const std::string& key_name, std::unique_ptr<char>& struct_ptr);
	//void SetKeyStruct(const std::string& section_name, const std::string& key_name, std::unique_ptr<char> struct_ptr);
};
