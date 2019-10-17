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

#include <map>
#include <cassert>
#include <iostream>

#include <CFG.h>
#include <CFGsContainer.h>
#include <SpecificStrategy.h>

SpecificStrategy::SpecificStrategy(const StrategyConfig& config) : Strategy(config) {
}

SpecificStrategy::~SpecificStrategy() {
}

void SpecificStrategy::process() {
	struct {
		SpecificStrategy::Stats present, missing;
	} total;

	if (m_fout.is_open())
		m_fout << "cfg,type,instrs,blocks_perfect,blocks_conflict,"
		       << "phantoms,"
		       << "edges_internal_perfect,edges_internal_conflict,"
			   << "edges_external_perfect,edges_external_conflict,"
			   << "calls,indirect" << std::endl;

	int cfgs = 0;
	for (CFG* aCFG : m_a->cfgs()) {
		if (aCFG->status() != CFG::VALID)
			continue;

		Addr addr = aCFG->addr();
		if (!this->isAddrInRange(addr))
			continue;

		CFG* bCFG = m_b->cfg(addr);
		if (bCFG == 0 || bCFG->status() != CFG::VALID)
			continue;

		SpecificStrategy::Report report = compareCFGs(aCFG, bCFG);

		SpecificStrategy::Stats aStats = this->extractStats(report.present);
		SpecificStrategy::Stats bStats = this->extractStats(report.missing);

		if (m_config.detailed) {
			std::cout << std::hex;
			std::cout << "CFG 0x" << aCFG->addr() << std::endl;
			std::cout << std::dec;
			std::cout << "present: " << aStats << std::endl;
			std::cout << "missing: " << bStats << std::endl;
			std::cout << std::endl;
		}

		if (m_fout.is_open()) {
			m_fout << "0x" << std::hex << aCFG->addr() << std::dec
					<< ",present," << aStats.instrs << ","
					<< aStats.blocks.perfect << "," << aStats.blocks.conflict << ","
					<< aStats.phantoms << ","
					<< aStats.edges.internal.perfect << "," << aStats.edges.internal.conflict << ","
					<< aStats.edges.external.perfect << "," << aStats.edges.external.conflict << ","
					<< aStats.calls << "," << aStats.indirects << std::endl;

			m_fout << "0x" << std::hex << bCFG->addr() << std::dec
					<< ",missing," << bStats.instrs << ","
					<< bStats.blocks.perfect << "," << bStats.blocks.conflict << ","
					<< bStats.phantoms << ","
					<< bStats.edges.internal.perfect << "," << bStats.edges.internal.conflict << ","
					<< bStats.edges.external.perfect << "," << bStats.edges.external.conflict << ","
					<< bStats.calls << "," << bStats.indirects << std::endl;
		}

		total.present += aStats;
		total.missing += bStats;

		cfgs++;
	}

	if (m_config.detailed)
		std::cout << "Total: " << std::endl;
	std::cout << "present: cfgs(" << cfgs << "), " << total.present << std::endl;
	std::cout << "missing: cfgs(0), " << total.missing << std::endl;
}

void SpecificStrategy::matchAddresses(std::set<Addr>& aInstrs, std::set<Addr>& bInstrs) {
	for (std::set<Addr>::iterator bIT = bInstrs.begin(),
			bED = bInstrs.end(); bIT != bED; ) {
		std::set<Addr>::iterator aIT = aInstrs.find(*bIT),
				aED = aInstrs.end();
		if (aIT != aED) {
			bIT = bInstrs.erase(bIT);
		} else {
			bIT++;
		}
	}
}

void SpecificStrategy::matchBlocks(SpecificStrategy::Info::Block& aBlocks,
		SpecificStrategy::Info::Block& bBlocks) {
	for (std::set<CfgData::Node>::iterator bIT = bBlocks.perfect.begin(),
			bED = bBlocks.perfect.end(); bIT != bED; ) {
		bool next = true;
		for (std::set<CfgData::Node>::iterator aIT = aBlocks.perfect.begin(),
				aED = aBlocks.perfect.end(); aIT != aED; aIT++) {
			const CfgData::Node& aNode = *aIT;
			const CfgData::Node& bNode = *bIT;
			if (aNode == bNode) {
				bIT = bBlocks.perfect.erase(bIT);
				next = false;
				break;
			} else if (aNode.start < (bNode.start + bNode.size) &&
					   bNode.start < (aNode.start + aNode.size)) {
				aBlocks.conflict.insert(aNode);
				aBlocks.perfect.erase(aIT);

				bBlocks.conflict.insert(bNode);
				bIT = bBlocks.perfect.erase(bIT);

				next = false;
				break;
			}
		}

		if (next)
			bIT++;
	}
}

void SpecificStrategy::matchEdges(SpecificStrategy::Info::Edge& aEdges,
		SpecificStrategy::Info::Edge& bEdges) {
	for (std::set<CfgData::Edge>::iterator bIT = bEdges.internal.perfect.begin(),
			bED = bEdges.internal.perfect.end(); bIT != bED; ) {
		std::set<CfgData::Edge>::iterator aIT = aEdges.internal.perfect.find(*bIT),
				aED = aEdges.internal.perfect.end();
		if (aIT != aED) {
			bIT = bEdges.internal.perfect.erase(bIT);
			continue;
		}

		aIT = aEdges.external.perfect.find(*bIT),
				aED = aEdges.external.perfect.end();
		if (aIT != aED) {
			aEdges.external.conflict.insert(*aIT);
			aEdges.external.perfect.erase(aIT);

			bEdges.internal.conflict.insert(*bIT);
			bIT = bEdges.internal.perfect.erase(bIT);
			continue;
		}

		bIT++;
	}

	for (std::set<CfgData::Edge>::iterator bIT = bEdges.external.perfect.begin(),
			bED = bEdges.external.perfect.end(); bIT != bED; ) {
		std::set<CfgData::Edge>::iterator aIT = aEdges.external.perfect.find(*bIT),
				aED = aEdges.external.perfect.end();
		if (aIT != aED) {
			bIT = bEdges.external.perfect.erase(bIT);
			continue;
		}

		aIT = aEdges.internal.perfect.find(*bIT),
				aED = aEdges.internal.perfect.end();
		if (aIT != aED) {
			aEdges.internal.conflict.insert(*aIT);
			aEdges.internal.perfect.erase(aIT);

			bEdges.external.conflict.insert(*bIT);
			bIT = bEdges.external.perfect.erase(bIT);
			continue;
		}

		bIT++;
	}
}

void SpecificStrategy::matchCalls(std::set<CfgData::Call>& aCalls,
		std::set<CfgData::Call>& bCalls) {
	std::list<CfgData::Call> newCalls;

	for (std::set<CfgData::Call>::iterator bIT = bCalls.begin(),
			bED = bCalls.end(); bIT != bED; ) {
		std::set<CfgData::Call>::iterator aIT = aCalls.find(*bIT),
				aED = aCalls.end();
		if (aIT != aED) {
			CfgData::Call nCall = *bIT;
			bIT = bCalls.erase(bIT);

			for (std::set<Addr>::iterator bAddrIT = nCall.calls.begin(),
					bAddrED = nCall.calls.end(); bAddrIT != bAddrED; ) {
				std::set<Addr>::iterator aAddrIT = (*aIT).calls.find(*bAddrIT),
									aAddrED = (*aIT).calls.end();
				if (aAddrIT != aAddrED) {
					bAddrIT = nCall.calls.erase(bAddrIT);
				} else {
					bAddrIT++;
				}
			}

			if (nCall.calls.size() > 0)
				newCalls.push_back(nCall);
		} else {
			bIT++;
		}
	}

	bCalls.insert(newCalls.begin(), newCalls.end());
}

SpecificStrategy::Report SpecificStrategy::compareCFGs(CFG* a, CFG* b) {
	SpecificStrategy::Report r = { .present = this->extractInfo(a),
			.missing = this->extractInfo(b) };

	this->matchAddresses(r.present.instrs, r.missing.instrs);
	this->matchBlocks(r.present.blocks, r.missing.blocks);
	this->matchAddresses(r.present.phantoms, r.missing.phantoms);
	this->matchEdges(r.present.edges, r.missing.edges);
	this->matchCalls(r.present.calls, r.missing.calls);
	this->matchAddresses(r.present.indirects, r.missing.indirects);

	return r;
}

SpecificStrategy::Info SpecificStrategy::extractInfo(CFG* cfg) {
	SpecificStrategy::Info info;

	CfgData data(cfg);
	info.instrs = data.instrs();
	info.blocks.perfect = data.blocks();
	info.phantoms = data.phantoms();
	info.edges.external.perfect = data.edges();
	info.calls = data.calls();
	info.indirects = data.indirects();

	for (CfgNode* node : cfg->nodes()) {
		if (node->type() != CfgNode::CFG_BLOCK)
			continue;

		CfgNode::BlockData* block = static_cast<CfgNode::BlockData*>(node->data());
		Instruction* previous = 0;
		for (Instruction* instr : block->instructions()) {
			if (previous != 0)
				info.edges.internal.perfect.insert(CfgData::Edge(previous->addr(), instr->addr()));

			previous = instr;
		}
	}

	return info;
}

SpecificStrategy::Stats SpecificStrategy::extractStats(const SpecificStrategy::Info& info) {
	Stats s;

	s.instrs = info.instrs.size();

	s.blocks.perfect = info.blocks.perfect.size();
	s.blocks.conflict = info.blocks.conflict.size();

	s.phantoms += info.phantoms.size();

	s.edges.internal.perfect = info.edges.internal.perfect.size();
	s.edges.internal.conflict = info.edges.internal.conflict.size();
	s.edges.external.perfect = info.edges.external.perfect.size();
	s.edges.external.conflict = info.edges.external.conflict.size();

	s.calls = 0;
	for (CfgData::Call call : info.calls)
		s.calls += call.calls.size();

	s.indirects += info.indirects.size();

	return s;
}

std::ostream& operator<<(std::ostream& os, const SpecificStrategy::Stats& stats) {
	os << "instrs(" << stats.instrs << "), "
		<< "blocks(perfect: " << stats.blocks.perfect
			<< ", conflicts: " << stats.blocks.conflict << "), "
		<< "phantoms(" << stats.phantoms << "), "
		<< "edges(internal(perfect: " << stats.edges.internal.perfect
			<< ", conflicts: " << stats.edges.internal.conflict
			<< "), external(perfect: " << stats.edges.external.perfect
			<< ", conflicts: " << stats.edges.external.conflict << ")), "
		<< "calls(" << stats.calls << "), "
		<< "indirects(" << stats.indirects << ")";
	return os;
}
