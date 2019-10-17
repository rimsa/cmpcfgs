/*

   Compare two CFGs in CFGgrind format.

   Copyright (C) 2019, Andrei Rimsa (andrei@cefetmg.br)

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#ifndef _CFGSCONTAINER_H
#define _CFGSCONTAINER_H

#include <map>
#include <set>
#include <string>
#include <fstream>
#include <sstream>

#include <CFG.h>

class CFGsContainer {
public:
	CFGsContainer(const std::string& filename, const std::string& name = "");
	virtual ~CFGsContainer();

	CFG* cfg(Addr addr) const;
	std::set<CFG*> cfgs() const;

	void compressAll();
	void checkAll();
	void dumpAll(const char* directory);

private:
	struct Lexeme {
		enum Type {
			TKN_INVALID_TOKEN = -2,
			TKN_UNEXPECTED_EOF,
			TKN_EOF,
			TKN_BRACKET_OPEN,
			TKN_BRACKET_CLOSE,
			TKN_CFG,
			TKN_NODE,
			TKN_EXIT,
			TKN_HALT,
			TKN_ADDR,
			TKN_NUMBER,
			TKN_BOOL,
			TKN_TEXT,
			TKN_PRIME
		};

		enum Type type;
		std::string token;

		union {
			Addr addr;
			int number;
			bool boolean;
		} data;

		Lexeme() : type(TKN_EOF), token("") {}
	};

	std::fstream m_input;
	std::string m_name;
	Lexeme m_currentToken;
	std::map<Addr, CFG*> m_cfgsMap;

	Lexeme nextToken();
	void matchToken(enum Lexeme::Type type);
	void processCFGs();

};

#endif
