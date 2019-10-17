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

#include <Strategy.h>
#include <Instruction.h>
#include <CFGsContainer.h>

Strategy::Strategy(const StrategyConfig& config) : m_config(config), m_a(0), m_b(0) {
	if (config.instrs)
		Instruction::load(std::string(config.instrs));

	m_a = new CFGsContainer((std::string(config.input1)), "A");
	m_b = new CFGsContainer((std::string(config.input2)), "B");

	if (config.compress) {
		m_a->compressAll();
		m_b->compressAll();
	}

	m_a->checkAll();
	m_b->checkAll();

	if (config.dump) {
		m_a->dumpAll(config.dump);
		m_b->dumpAll(config.dump);
	}

	if (config.output) {
		m_fout.open(config.output);
		if (!m_fout.is_open())
			throw std::string("Unable to open output file: ") + config.output;
	}
}

Strategy::~Strategy() {
	if (m_fout.is_open())
		m_fout.close();

	if (m_a)
		delete m_a;

	if (m_b)
		delete m_b;

	if (m_config.instrs)
		Instruction::clear();
}

bool Strategy::isAddrInRange(Addr addr) const {
	// if the range list is empty, consider the address in range.
	if (m_config.ranges.size() == 0)
		return true;

	for (std::list<std::pair<Addr, Addr> >::const_iterator it =  m_config.ranges.cbegin(),
			ed = m_config.ranges.cend(); it != ed; it++) {
		if (addr >= it->first && addr <= it->second)
			return true;
	}

	return false;
}
