#pragma once

#include <string>
#include <map>

class CommandSystem
{
public:
	bool Enabled;

	CommandSystem();

	void Add(char const * command);
	void AddTick(char const * command);

	bool Remove(int index);

	void Clear(void);

	bool Save(wchar_t const * fileName);
	bool Load(wchar_t const * fileName);

	void Console_List(void);

	void Do_Commands(void);

	void OnLevelInitPreEntityAllTools(void);

private:
	std::map<int, std::string> m_TickMap;
	std::map<double,std::string> m_Map;

	double m_LastTime;
	int m_LastTick;

	bool IsSupportedByTime(void);
	bool IsSupportedByTick(void);
};

extern CommandSystem g_CommandSystem;
