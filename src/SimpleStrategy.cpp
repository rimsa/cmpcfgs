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

#include <iostream>

#include <CFGsContainer.h>
#include <SimpleStrategy.h>

SimpleStrategy::SimpleStrategy(const StrategyConfig& config) : Strategy(config) {
}

SimpleStrategy::~SimpleStrategy() {
}

void SimpleStrategy::process() {
	SimpleStrategy::Report total;

	if (m_fout.is_open())
		m_fout << "file,cfg,instrs,blocks,phantoms,edges,calls" << std::endl;

	std::set<CFG*> bCFGs = m_b->cfgs();
	for (CFG* cfg : m_a->cfgs()) {
		if (cfg->status() != CFG::VALID)
			continue;

		Addr addr = cfg->addr();
		if (!this->isAddrInRange(addr))
			continue;

		std::set<CFG*>::iterator it = std::find_if(bCFGs.begin(), bCFGs.end(),
		             [addr](const CFG* tmp) -> bool { return tmp->addr() == addr; });
		if (it != bCFGs.end() && (*it)->status() == CFG::VALID) {
			SimpleStrategy::Report r = compareCFGs(cfg, *it);
			total.matched += r.matched;
			total.unmatched.a += r.unmatched.a;
			total.unmatched.b += r.unmatched.b;

			if (m_config.detailed) {
				std::cout << std::hex;
				std::cout << "CFG 0x" << addr << (!m_config.both ? ": both files" : "") << std::endl;

				std::cout << std::dec;
				std::cout << r;
				std::cout << std::endl;
			}

			if (m_fout.is_open()) {
				m_fout << "both,0x" << std::hex << addr << std::dec
						<< "," << r.matched.instrs << "," << r.matched.blocks
						<< "," << r.matched.phantoms << "," << r.matched.edges
						<< "," << r.matched.calls << std::endl;
				m_fout << "A,0x" << std::hex << addr << std::dec
						<< "," << r.unmatched.a.instrs << "," << r.unmatched.a.blocks
						<< "," << r.unmatched.a.phantoms << "," << r.unmatched.a.edges
						<< "," << r.unmatched.a.calls << std::endl;
				m_fout << "B,0x" << std::hex << addr << std::dec
						<< "," << r.unmatched.b.instrs << "," << r.unmatched.b.blocks
						<< "," << r.unmatched.b.phantoms << "," << r.unmatched.b.edges
						<< "," << r.unmatched.b.calls << std::endl;
			}

			bCFGs.erase(it);
		} else {
			if (!m_config.both) {
				SimpleStrategy::Stats s = extractStats(cfg);
				total.unmatched.a += s;

				if (m_config.detailed) {
					std::cout << std::hex;
					std::cout << "CFG 0x" << addr << ": file A" << std::endl;

					std::cout << std::dec;
					std::cout << s << std::endl;
					std::cout << std::endl;
				}

				if (m_fout.is_open()) {
					m_fout << "A,0x" << std::hex << addr << std::dec
							<< "," << s.instrs << "," << s.blocks
							<< "," << s.phantoms << "," << s.edges
							<< "," << s.calls << std::endl;
				}
			}
		}
	}

	if (!m_config.both) {
		for (CFG* cfg : bCFGs) {
			if (cfg->status() != CFG::VALID)
				continue;

			Addr addr = cfg->addr();
			if (!this->isAddrInRange(addr))
				continue;

			SimpleStrategy::Stats s = extractStats(cfg);
			total.unmatched.b += s;

			if (m_config.detailed) {
				std::cout << std::hex;
				std::cout << "CFG 0x" << cfg->addr() << ": file B" << std::endl;

				std::cout << std::dec;
				std::cout << s << std::endl;
				std::cout << std::endl;
			}

			if (m_fout.is_open()) {
				m_fout << "B,0x" << std::hex << cfg->addr() << std::dec
						<< "," << s.instrs << "," << s.blocks
						<< "," << s.phantoms << "," << s.edges
						<< "," << s.calls << std::endl;
			}
		}
	}

	if (m_config.detailed)
		std::cout << "Total:" << std::endl;
	std::cout << total;
}

template<typename T>
std::set<T> SimpleStrategy::matchGeneric(std::set<T>& a, std::set<T>& b) {
	std::set<T> match;

	typename std::set<T>::iterator itA = a.begin();
	while (itA != a.end()) {
		const T& v = *itA;

		typename std::set<T>::iterator itB = b.find(v);
		if (itB != b.end()) {
			b.erase(itB);

			match.insert(v);
			itA = a.erase(itA);
		} else {
			itA++;
		}
	}

	return match;
}

std::set<CfgData::Call> SimpleStrategy::matchCalls(std::set<CfgData::Call>& a, std::set<CfgData::Call>& b) {
	std::set<CfgData::Call> match;

	std::set<CfgData::Call>::iterator itA = a.begin();
	while (itA != a.end()) {
		CfgData::Call callA = *itA;

		std::set<CfgData::Call>::iterator itB = b.find(callA);
		if (itB != b.end()) {
			CfgData::Call callB = *itB;

			// Always remove, it may be added again later.
			itA = a.erase(itA);
			itB = b.erase(itB);

			CfgData::Call newCall(callA.instr);

			std::set<Addr>::iterator it2A = callA.calls.begin();
			while (it2A != callA.calls.end()) {
				std::set<Addr>::iterator it2B = callB.calls.find(*it2A);
				if (it2B != callB.calls.end()) {
					callB.calls.erase(it2B);

					newCall.calls.insert(*it2A);
					it2A = callA.calls.erase(it2A);
				} else {
					it2A++;
				}
			}

			if (newCall.calls.size() > 0)
				match.insert(newCall);

			// If it still has at least one call, add it back.
			if (!callA.calls.empty())
				a.insert(callA);
			if (!callB.calls.empty())
				b.insert(callB);
		} else {
			itA++;
		}
	}

	return match;
}

int SimpleStrategy::countCalls(const std::set<CfgData::Call>& calls) {
	int total = 0;
	for (CfgData::Call c : calls)
		total += c.calls.size();

	return total;
}

SimpleStrategy::Stats SimpleStrategy::extractStats(CFG* cfg) {
	Stats s;

	CfgData data(cfg);
	s.cfgs = 1;
	s.instrs = data.instrs().size();
	s.blocks = data.blocks().size();
	s.phantoms = data.phantoms().size();
	s.edges = data.edges().size();
	s.calls = countCalls(data.calls());

	return s;
}

SimpleStrategy::Report SimpleStrategy::compareCFGs(CFG* a, CFG* b) {
	SimpleStrategy::Report r;

	CfgData dataA(a);
	CfgData dataB(b);

	r.matched.cfgs = 1;
	r.unmatched.a.cfgs = 0;
	r.unmatched.b.cfgs = 0;

	std::set<Addr> instrsA = dataA.instrs();
	std::set<Addr> instrsB = dataB.instrs();
	std::set<Addr> instrsAB = matchGeneric(instrsA, instrsB);
	r.matched.instrs = instrsAB.size();
	r.unmatched.a.instrs = instrsA.size();
	r.unmatched.b.instrs = instrsB.size();

	std::set<CfgData::Node> blocksA = dataA.blocks();
	std::set<CfgData::Node> blocksB = dataB.blocks();
	std::set<CfgData::Node> blocksAB = matchGeneric(blocksA, blocksB);
	r.matched.blocks = blocksAB.size();
	r.unmatched.a.blocks = blocksA.size();
	r.unmatched.b.blocks = blocksB.size();

	std::set<Addr> phantomsA = dataA.phantoms();
	std::set<Addr> phantomsB = dataB.phantoms();
	std::set<Addr> phantomsAB = matchGeneric(phantomsA, phantomsB);
	r.matched.phantoms = phantomsAB.size();
	r.unmatched.a.phantoms = phantomsA.size();
	r.unmatched.b.phantoms = phantomsB.size();

	std::set<CfgData::Edge> edgesA = dataA.edges();
	std::set<CfgData::Edge> edgesB = dataB.edges();
	std::set<CfgData::Edge> edgesAB = matchGeneric(edgesA, edgesB);
	r.matched.edges = edgesAB.size();
	r.unmatched.a.edges = edgesA.size();
	r.unmatched.b.edges = edgesB.size();

	std::set<CfgData::Call> callsA = dataA.calls();
	std::set<CfgData::Call> callsB = dataB.calls();
	std::set<CfgData::Call> callsAB = matchCalls(callsA, callsB);
	r.matched.calls = countCalls(callsAB);
	r.unmatched.a.calls = countCalls(callsA);
	r.unmatched.b.calls = countCalls(callsB);

	return r;
}

std::ostream& operator<<(std::ostream& os, const SimpleStrategy::Stats& stats) {
	os << "cfgs(" << stats.cfgs << "), "
		<< "instrs(" << stats.instrs << "), "
		<< "blocks(" << stats.blocks << "), "
		<< "phantoms(" << stats.phantoms << "), "
		<< "edges(" << stats.edges << "), "
		<< "calls(" << stats.calls << ")";
	return os;
}

std::ostream& operator<<(std::ostream& os, const SimpleStrategy::Report& report) {
	os << "matched:      " << report.matched << std::endl;
	os << "unmatched(A): " << report.unmatched.a << std::endl;
	os << "unmatched(B): " << report.unmatched.b << std::endl;
	return os;
}
