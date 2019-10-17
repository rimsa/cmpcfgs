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

#ifndef _INSTRUCTION_H
#define _INSTRUCTION_H

#include <map>
#include <string>

typedef unsigned long Addr;

class Instruction {
public:
	virtual ~Instruction();

	Addr addr() const { return m_addr; }
	int size() const { return m_size; }
	const std::string& text() const { return m_text; }

	static Instruction* get(Addr addr, int size = 0);
	static void load(std::string filename);
	static void clear();

private:
	Addr m_addr;
	int m_size;
	std::string m_text;

	static std::map<Addr, Instruction*> m_instrsMap;

	Instruction(Addr addr, int size, const std::string& text = "???");

};

#endif
