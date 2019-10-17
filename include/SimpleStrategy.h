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

#ifndef _SIMPLESTRATEGY_H
#define _SIMPLESTRATEGY_H

#include <set>
#include <Strategy.h>

class SimpleStrategy : public Strategy {
public:
	struct Stats {
		int cfgs;
		int instrs;
		int blocks;
		int phantoms;
		int edges;
		int calls;
		int indirects;

		Stats() : cfgs(0), instrs(0), blocks(0), phantoms(0), edges(0), calls(0), indirects(0) {}
		Stats(const Stats& stats) : cfgs(stats.cfgs), instrs(stats.instrs), blocks(stats.blocks),
				phantoms(stats.phantoms), edges(stats.edges), calls(stats.calls),
				indirects(0) {}
		virtual ~Stats() {}

		Stats& operator+=(const Stats& stats) {
			cfgs += stats.cfgs;
			instrs += stats.instrs;
			blocks += stats.blocks;
			phantoms += stats.phantoms;
			edges += stats.edges;
			calls += stats.calls;
			indirects += stats.indirects;

			return *this;
		}
	};

	struct Report {
		Stats matched;
		struct {
			Stats a, b;
		} unmatched;

		Report() {}
		virtual ~Report() {}
	};

	SimpleStrategy(const StrategyConfig& config);
	virtual ~SimpleStrategy();

	void process();

private:
	template<typename T> std::set<T> matchGeneric(std::set<T>& a, std::set<T>& b);

	std::set<CfgData::Call> matchCalls(std::set<CfgData::Call>& a, std::set<CfgData::Call>& b);
	int countCalls(const std::set<CfgData::Call>& calls);
	Stats extractStats(CFG* cfg);
	Report compareCFGs(CFG* a, CFG* b);

};

std::ostream& operator<<(std::ostream& os, const SimpleStrategy::Stats& stats);
std::ostream& operator<<(std::ostream& os, const SimpleStrategy::Report& report);

#endif
