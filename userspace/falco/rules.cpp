#include "rules.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


falco_rules::falco_rules(sinsp* inspector, lua_State *ls, string lua_main_filename)
{
	m_ls = ls;

	m_lua_parser = new lua_parser(inspector, m_ls);

	load_compiler(lua_main_filename);
}


void falco_rules::load_compiler(string lua_main_filename)
{
	ifstream is;
	is.open(lua_main_filename);
	if(!is.is_open())
	{
		throw sinsp_exception("can't open file " + lua_main_filename);
	}

	string scriptstr((istreambuf_iterator<char>(is)),
			 istreambuf_iterator<char>());

	//
	// Load the compiler script
	//
	if(luaL_loadstring(m_ls, scriptstr.c_str()) || lua_pcall(m_ls, 0, 0, 0))
	{
		throw sinsp_exception("Failed to load script " +
			lua_main_filename + ": " + lua_tostring(m_ls, -1));
	}
}

void falco_rules::load_rules(string rules_filename)
{
	lua_getglobal(m_ls, m_lua_load_rules.c_str());
	if(lua_isfunction(m_ls, -1))
	{
		lua_pushstring(m_ls, rules_filename.c_str());
		if(lua_pcall(m_ls, 1, 0, 0) != 0)
		{
			const char* lerr = lua_tostring(m_ls, -1);
			string err = "Error loading rules:" + string(lerr);
			throw sinsp_exception(err);
		}
	} else {
		throw sinsp_exception("No function " + m_lua_load_rules + " found in lua compiler module");
	}
}

sinsp_filter* falco_rules::get_filter()
{
	return m_lua_parser->get_filter();
}

falco_rules::~falco_rules()
{
	delete m_lua_parser;
}
