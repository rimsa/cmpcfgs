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

#ifndef _STRATEGY_H
#define _STRATEGY_H

#include <fstream>
#include <list>
#include <CfgData.h>

class CFGsContainer;

struct StrategyConfig {
	bool compress;
	bool detailed;
	bool specific;
	bool both;
	std::list<std::pair<Addr, Addr> > ranges;
	char* instrs;
	char* output;
	char* dump;
	char* input1;
	char* input2;

	StrategyConfig(bool compress = false, bool detailed = false,
			bool specific = false, bool both = false,
			std::list<std::pair<Addr, Addr> > ranges = std::list<std::pair<Addr, Addr> >(),
			char* instrs = 0, char* output = 0,
			char* dump = 0, char* input1 = 0, char* input2 = 0) :
		compress(compress), detailed(detailed), specific(specific), both(both), ranges(ranges),
		instrs(instrs), output(output), dump(dump), input1(input1), input2(input2) {}
	StrategyConfig(const StrategyConfig& config) :
		compress(config.compress), detailed(config.detailed),
		specific(config.specific), both(config.both), ranges(config.ranges),
		instrs(config.instrs), output(config.output), dump(config.dump),
		input1(config.input1), input2(config.input2) {}
	virtual ~StrategyConfig() {}
};

class Strategy {
public:
	virtual ~Strategy();

	virtual void process() = 0;

	bool isAddrInRange(Addr addr) const;

protected:
	StrategyConfig m_config;
	CFGsContainer* m_a;
	CFGsContainer* m_b;
	std::ofstream m_fout;

	Strategy(const StrategyConfig& config);

};

#endif
