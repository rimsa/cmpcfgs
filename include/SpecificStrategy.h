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

#ifndef _SPECIFICSTRATEGY_H
#define _SPECIFICSTRATEGY_H

#include <set>
#include <Strategy.h>

class SpecificStrategy : public Strategy {
public:
	struct Stats {
		int instrs;
		struct {
			int perfect;
			int conflict;
		} blocks;
		int phantoms;
		struct {
			struct {
				int perfect;
				int conflict;
			} internal, external;
		} edges;
		int calls;
		int indirects;

		Stats() : instrs(0), blocks({0, 0}), phantoms(0),
				edges({{0, 0}, {0, 0}}), calls(0), indirects(0) {}
		Stats(const Stats& stats) : instrs(stats.instrs),
				blocks({stats.blocks.perfect, stats.blocks.conflict}), phantoms(stats.phantoms),
				edges({{stats.edges.internal.perfect, stats.edges.internal.conflict},
					   {stats.edges.external.perfect, stats.edges.external.conflict}}),
				calls(stats.calls), indirects(stats.indirects) {}
		virtual ~Stats() {}

		Stats& operator+=(const Stats& stats) {
			instrs += stats.instrs;
			blocks.perfect += stats.blocks.perfect;
			blocks.conflict += stats.blocks.conflict;
			phantoms += stats.phantoms;
			edges.internal.perfect += stats.edges.internal.perfect;
			edges.internal.conflict += stats.edges.internal.conflict;
			edges.external.perfect += stats.edges.external.perfect;
			edges.external.conflict += stats.edges.external.conflict;
			calls += stats.calls;
			indirects += stats.indirects;

			return *this;
		}
	};

	SpecificStrategy(const StrategyConfig& config);
	virtual ~SpecificStrategy();

	void process();

private:
	struct Info {
		std::set<Addr> instrs;
		struct Block {
			std::set<CfgData::Node> perfect, conflict;
		} blocks;
		std::set<Addr> phantoms;
		struct Edge {
			struct {
				std::set<CfgData::Edge> perfect, conflict;
			} internal, external;
		} edges;
		std::set<CfgData::Call> calls;
		std::set<Addr> indirects;
	};

	struct Report {
		SpecificStrategy::Info present, missing;
	};

	void matchAddresses(std::set<Addr>& aInstrs, std::set<Addr>& bInstrs);
	void matchBlocks(SpecificStrategy::Info::Block& aBlocks,
			SpecificStrategy::Info::Block& bBlocks);
	void matchEdges(SpecificStrategy::Info::Edge& aEdges,
			SpecificStrategy::Info::Edge& bEdges);
	void matchCalls(std::set<CfgData::Call>& aCalls,
			std::set<CfgData::Call>& bCalls);
	SpecificStrategy::Report compareCFGs(CFG* a, CFG* b);

	SpecificStrategy::Info extractInfo(CFG* cfg);
	SpecificStrategy::Stats extractStats(const SpecificStrategy::Info& info);

};

std::ostream& operator<<(std::ostream& os, const SpecificStrategy::Stats& stats);

#endif
