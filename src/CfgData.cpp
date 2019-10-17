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

#include <cassert>

#include <CfgData.h>
#include <CFG.h>

CfgData::CfgData(CFG* cfg) {
	for (CfgNode* node : cfg->nodes()) {
		Addr from = CfgNode::node2addr(node);

		if (node->type() == CfgNode::CFG_PHANTOM) {
			m_phantoms.insert(from);
			continue;
		}

		for (CfgNode::Edge edgeSucc : node->successors()) {
			Addr to = CfgNode::node2addr(edgeSucc.node);
			bool virtua = edgeSucc.virtua;

			m_edges.insert(CfgData::Edge(from, to, virtua));
		}

		if (node->type() != CfgNode::CFG_BLOCK)
			continue;

		CfgNode::BlockData* block = static_cast<CfgNode::BlockData*>(node->data());
		m_blocks.insert(CfgData::Node(block->addr(), block->size()));

		Instruction* last = 0;
		for (Instruction* instr : block->instructions()) {
			m_instrs.insert(instr->addr());
			last = instr;
		}
		assert(last != 0);

		CfgData::Call call(last->addr());
		for (CFG* calledCfg : block->calls()) {
			call.calls.insert(calledCfg->addr());
		}

		if (!call.calls.empty())
			m_calls.insert(call);

		if (block->hasIndirection())
			m_indirects.insert(last->addr());
	}
}

CfgData::~CfgData() {
}
