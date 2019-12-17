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

#include <CFG.h>
#include <CfgNode.h>

CfgNode::CfgNode(enum CfgNode::Type type) : m_type(type), m_data(0) {
}

CfgNode::~CfgNode() {
	if (m_data)
		delete m_data;
}

void CfgNode::setData(CfgNode::Data* data) {
	assert(data != 0);

	PhantomData* phantomData = dynamic_cast<PhantomData*>(data);
	if (phantomData) {
		assert(m_data == 0);
		assert(m_type == CfgNode::CFG_PHANTOM);
		m_data = phantomData;
		return;
	}

	BlockData* blockData = dynamic_cast<BlockData*>(data);
	if (blockData) {
		switch (m_type) {
			case CfgNode::CFG_PHANTOM:
				assert(m_data != 0);
				delete m_data;

				m_type = CfgNode::CFG_BLOCK;
				m_data = blockData;
				return;
			case CfgNode::CFG_BLOCK:
				assert(m_data == 0);
				m_data = blockData;
				return;
			default:
				assert(false);
		}
	}

	assert(false);
}

bool CfgNode::addSuccessor(CfgNode* succ) {
	assert(this->type() != CfgNode::CFG_EXIT && this->type() != CfgNode::CFG_HALT);
	assert(succ->type() != CfgNode::CFG_ENTRY);

	return m_succs.insert(CfgNode::Edge(succ)).second;
}

void CfgNode::addSuccessors(const std::set<CfgNode::Edge>& succs) {
	m_succs.insert(succs.cbegin(), succs.cend());
}

bool CfgNode::removeSuccessor(CfgNode* succ) {
	return m_succs.erase(CfgNode::Edge(succ)) > 0;
}

void CfgNode::clearSuccessors() {
	m_succs.clear();
}

bool CfgNode::addPredecessor(CfgNode* pred) {
	assert(this->type() != CfgNode::CFG_ENTRY);
	assert(pred->type() != CfgNode::CFG_PHANTOM);
	assert(pred->type() != CfgNode::CFG_EXIT && pred->type() != CfgNode::CFG_HALT);

	return m_preds.insert(CfgNode::Edge(pred)).second;
}

void CfgNode::addPredecessors(const std::set<CfgNode::Edge>& preds) {
	m_preds.insert(preds.cbegin(), preds.cend());
}

bool CfgNode::removePredecessor(CfgNode* pred) {
	return m_preds.erase(CfgNode::Edge(pred)) > 0;
}

void CfgNode::clearPredecessors() {
	m_preds.clear();
}

Addr CfgNode::node2addr(CfgNode* node) {
	switch (node->type()) {
		case CfgNode::CFG_BLOCK:
		case CfgNode::CFG_PHANTOM:
			assert(node->data() != 0);
			return node->data()->addr();
		default:
			return 0;
	}
}

void CfgNode::BlockData::setIndirect(bool indirect) {
	m_indirection = indirect;
}

void CfgNode::BlockData::addInstruction(Instruction* instr) {
	assert(instr != 0 && instr->size() > 0);

	m_instrs.push_back(instr);
	m_size += instr->size();
}

void CfgNode::BlockData::addInstructions(const std::list<Instruction*>& instrs) {
	for (Instruction* instr : instrs)
		this->addInstruction(instr);
}

Instruction* CfgNode::BlockData::firstInstruction() const {
	return m_instrs.empty() ? 0: m_instrs.front();
}

Instruction* CfgNode::BlockData::lastInstruction() const {
	return m_instrs.empty() ? 0: m_instrs.back();
}

void CfgNode::BlockData::clearInstructions() {
	m_instrs.clear();
	m_size = 0;
}

void CfgNode::BlockData::addCall(CFG* cfg) {
	m_calls.insert(cfg);
}

void CfgNode::BlockData::clearCalls() {
	m_calls.clear();
}

void CfgNode::BlockData::addSignalHandler(int sigid, CFG* cfg) {
	std::map<int, CFG*>::const_iterator it = m_signalHandlers.find(sigid);
	if (it != m_signalHandlers.end()) {
		assert(it->second->addr() == cfg->addr());
	} else {
		m_signalHandlers[sigid] = cfg;
	}
}

void CfgNode::BlockData::clearSignalHandlers() {
	m_signalHandlers.clear();
}