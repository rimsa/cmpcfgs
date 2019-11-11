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

#include <fstream>
#include <cassert>

#include <Instruction.h>

std::map<Addr, Instruction*> Instruction::m_instrsMap;

Instruction::Instruction(Addr addr, int size, const std::string& text) :
	m_addr(addr), m_size(size), m_text(text) {
}

Instruction::~Instruction() {
}

Instruction* Instruction::get(Addr addr, int size) {
	Instruction* instr = m_instrsMap[addr];
	if (instr) {
		if (instr->m_size == 0)
			instr->m_size = size;
		else
			assert(instr->m_size == size);
	} else {
		instr = new Instruction(addr, size);
		m_instrsMap[addr] = instr;
	}

	return instr;
}

void Instruction::load(std::string filename) {
	std::ifstream input(filename);

	for (std::string line; getline(input, line); ) {
		std::size_t n = line.find(':');
		if (n == std::string::npos)
			continue;

		Addr addr = std::stoul(line.substr(0, n), 0, 16);
		if (addr == 0)
			continue;

		std::string tmp = line.substr(n+1);
		if (tmp.empty())
			continue;

		n = tmp.find(':');
		if (n == std::string::npos)
			continue;

		int size = std::stoi(tmp);
		if (size <= 0)
			continue;

		std::string text = tmp.substr(n+1);
		if (text.empty())
			continue;

		Instruction* instr = Instruction::get(addr, size);
		instr->m_text = text;
	}

	input.close();
}

void Instruction::clear() {
	for (std::map<Addr, Instruction*>::iterator it = m_instrsMap.begin(),
			ed = m_instrsMap.end(); it != ed; it++) {
		delete it->second;
	}
}
