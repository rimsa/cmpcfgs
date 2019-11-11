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
#include <sstream>
#include <cassert>
#include <algorithm>

#include <CFG.h>

CFG::CFG(Addr addr) : m_addr(addr), m_status(CFG::UNCHECKED),
		m_functionName("unknown"),
		m_entryNode(new CfgNode(CfgNode::CFG_ENTRY)),
		m_exitNode(0), m_haltNode(0) {
}

CFG::~CFG() {
	delete m_entryNode;

	if (m_exitNode)
		delete m_exitNode;

	if (m_haltNode)
		delete m_haltNode;

	for (std::map<Addr, CfgNode*>::iterator it = m_nodesMap.begin(),
			ed = m_nodesMap.end(); it != ed; it++) {
		delete it->second;
	}
}

CfgNode* CFG::nodeByAddr(Addr addr) const {
	std::map<Addr, CfgNode*>::const_iterator it = m_nodesMap.find(addr);
	return it != m_nodesMap.end() ? it->second : 0;
}

std::list<CfgNode*> CFG::nodes() const {
	std::list<CfgNode*> nodes;

	nodes.push_back(m_entryNode);

	if (m_exitNode)
		nodes.push_back(m_exitNode);

	if (m_haltNode)
		nodes.push_back(m_haltNode);

	std::transform(m_nodesMap.begin(), m_nodesMap.end(),
	    std::back_inserter(nodes),
	    [](const std::map<Addr, CfgNode*>::value_type &pair) {
			return pair.second;
		}
	);

	return nodes;
}

void CFG::setFunctionName(const std::string& functionName) {
	m_functionName = functionName;
}

void CFG::addNode(CfgNode* node) {
	Addr addr;

	switch (node->type()) {
		case CfgNode::CFG_ENTRY:
			assert(false);
			break;
		case CfgNode::CFG_BLOCK:
		case CfgNode::CFG_PHANTOM:
			addr = CfgNode::node2addr(node);
			assert(addr != 0);

			assert(m_nodesMap[addr] == 0);
			m_nodesMap[addr] = node;

			if (addr == m_addr)
				this->addEdge(m_entryNode, node);

			break;
		case CfgNode::CFG_EXIT:
			assert(m_exitNode == 0);
			m_exitNode = node;
			break;
		case CfgNode::CFG_HALT:
			assert(m_haltNode == 0);
			m_haltNode = node;
			break;
		default:
			assert(false);
	}

	m_status = CFG::UNCHECKED;
}

bool CFG::containsNode(CfgNode* node) {
	if (!node)
		return false;

	switch (node->type()) {
		case CfgNode::CFG_ENTRY:
			return node == m_entryNode;
		case CfgNode::CFG_EXIT:
			return node == m_exitNode;
		case CfgNode::CFG_HALT:
			return node == m_haltNode;
		default:
			CfgNode* tmp = CFG::nodeByAddr(CfgNode::node2addr(node));
			return tmp == node;
	}
}

void CFG::addEdge(CfgNode* from, CfgNode* to) {
	assert(from != 0 && this->containsNode(from));
	assert(to != 0 && this->containsNode(to));

	from->addSuccessor(to);
	to->addPredecessor(from);

	m_status = CFG::UNCHECKED;
}

std::string dotFilter(const std::string& name) {
	std::stringstream ss;

	const char* str = name.c_str();
	while (*str) {
		if (*str == '<' || *str == '>')
			ss << "\\";

		ss << *str;
		str++;
	}

	return ss.str();
}

void CFG::compress() {
	std::map<Addr, CfgNode*>::iterator it = m_nodesMap.begin(),
			ed = m_nodesMap.end();
	while (it != ed) {
		CfgNode* node = it->second;
		if (node->type() != CfgNode::CFG_BLOCK) {
			it++;
			continue;
		}

		// Check if we only have one predecessor.
		const std::set<CfgNode::Edge>& nodePreds = node->predecessors();
		if (nodePreds.size() != 1) {
			it++;
			continue;
		}

		// Check if the predecessor is a block.
		CfgNode* pred = nodePreds.begin()->node;
		if (pred->type() != CfgNode::CFG_BLOCK) {
			it++;
			continue;
		}

		// Check if the predecessor has only one successor,
		// which must be our node.
		const std::set<CfgNode::Edge>& predSuccs = pred->successors();
		if (predSuccs.size() != 1) {
			it++;
			continue;
		}
		assert(predSuccs.begin()->node == node);

		// Check if the predecessor has no indirect jumps or calls.
		CfgNode::BlockData* predData = static_cast<CfgNode::BlockData*>(pred->data());
		assert(predData != 0);
		if (predData->hasIndirection() || predData->calls().size() > 0) {
			it++;
			continue;
		}

		// Check if the node's first instruction is immediately after the predecessor instruction.
		CfgNode::BlockData* nodeData = static_cast<CfgNode::BlockData*>(node->data());
		if ((predData->addr() + predData->size()) != nodeData->addr()) {
			it++;
			continue;
		}

		// We can proceed to remove the node now.

		// Step 1: Transfer the node instructions to the predecessor.
		predData->addInstructions(nodeData->instructions());

		// Step 2: Fix the predecessor with all the node successors and
		// fix all the node sucessors with the new predecessor
		pred->clearSuccessors();
		for (CfgNode::Edge edgeSucc : node->successors()) {
			pred->addSuccessor(edgeSucc.node);

			bool removed = edgeSucc.node->removePredecessor(node);
			assert(removed);
			edgeSucc.node->addPredecessor(pred);
		}

		// Step 3: Remove the node from the CFG.
		it = m_nodesMap.erase(it);
		delete node;
	}

	m_status = CFG::UNCHECKED;
}

enum CFG::Status CFG::check() {
	m_status = CFG::INVALID;

	if (!m_exitNode && !m_haltNode)
		goto out;

	if (m_entryNode->hasPredecessor() || !m_entryNode->hasSuccessors() ||
		(m_exitNode && (m_exitNode->hasSuccessors() || !m_exitNode->hasPredecessor())) ||
		(m_haltNode && (m_haltNode->hasSuccessors() || !m_haltNode->hasPredecessor())))
		goto out;

	for (std::map<Addr, CfgNode*>::iterator it = m_nodesMap.begin(),
			ed = m_nodesMap.end(); it != ed; it++) {
		CfgNode* node = it->second;
		assert(node != 0);

		if (!node->hasPredecessor() ||
			(node->type() != CfgNode::CFG_PHANTOM && !node->hasSuccessors()))
			goto out;
	}

	m_status = CFG::VALID;

out:
	return m_status;
}

std::string CFG::toDOT() const {
	std::stringstream ss;
	int unknown = 1;

	ss << std::hex;
	ss << "digraph \"0x" << m_addr << "\" {" << std::endl;
	ss << "  label = \"0x" << m_addr << " (" << m_functionName << ")\"" << std::endl;
    ss << "  labelloc = \"t\"" << std::endl;
    ss << "  node[shape=record]" << std::endl;
    ss << std::endl;

	std::list<CfgNode*> nodes = this->nodes();
	for (std::list<CfgNode*>::const_iterator it = nodes.cbegin(),
			ed = nodes.cend(); it != ed; it++) {
		CfgNode* node = *it;
		switch (node->type()) {
			case CfgNode::CFG_ENTRY:
			    ss << "  Entry [label=\"\",width=0.3,height=0.3,shape=circle,fillcolor=black,style=filled]" << std::endl;
			    break;
			case CfgNode::CFG_EXIT:
				ss << "  Exit [label=\"\",width=0.3,height=0.3,shape=circle,fillcolor=black,style=filled,peripheries=2]" << std::endl;
			    break;
			case CfgNode::CFG_HALT:
				ss << "  Halt [label=\"\",width=0.3,height=0.3,shape=square,fillcolor=black,style=filled,peripheries=2]" << std::endl;
			    break;
			case CfgNode::CFG_BLOCK: {
				assert(node->data() != 0);
				CfgNode::BlockData* blockData = static_cast<CfgNode::BlockData*>(node->data());

				Addr addr =  blockData->addr();
				ss << "  \"0x" << addr << "\" [label=\"{" << std::endl;
				ss << "    0x" << addr << " [" << std::dec << blockData->size() << "]\\l" << std::endl;
				ss << "    | [instrs]\\l" << std::endl;

				std::list<Instruction*> instrs = blockData->instructions();
				for (std::list<Instruction*>::const_iterator it = instrs.cbegin(), ed = instrs.cend();
						it != ed; it++) {
					Instruction* instr = *it;
					ss << "    &nbsp;&nbsp;0x" << std::hex << instr->addr() << " \\<+"
							<< std::dec << instr->size() << "\\>: " << dotFilter(instr->text())
							<< "\\l" << std::endl;
				}

				std::set<CFG*> calls = blockData->calls();
				if (!calls.empty()) {
					ss << "     | [calls]\\l" << std::endl;
					ss << std::hex;
					for (std::set<CFG*>::const_iterator it = calls.cbegin(), ed = calls.cend();
							it != ed; it++) {
						CFG* called = *it;
						ss << "     &nbsp;&nbsp;0x" << called->addr() << " ("
							<< dotFilter(called->functionName()) << ")\\l" << std::endl;
					}
				}

				ss << "  }\"]" << std::endl;

                if (blockData->hasIndirection()) {
                		ss << "  \"Unknown" << std::dec << unknown << "\" [label=\"?\", shape=none]" << std::endl;
                		ss << "  \"0x" << std::hex << blockData->addr() << "\" -> \"Unknown" << std::dec << unknown << "\" [style=dashed]" << std::endl;
                    unknown++;
                }

				ss << std::hex;
				break;
			}
			case CfgNode::CFG_PHANTOM: {
				assert(node->data() != 0);
				CfgNode::PhantomData* phantomData = static_cast<CfgNode::PhantomData*>(node->data());

				ss << "  \"0x" << phantomData->addr() << "\" [label=\"{" << std::endl;
				ss << "     0x" << phantomData->addr() << "\\l" << std::endl;
				ss << "  }\", style=dashed]" << std::endl;

				break;
			}
			default:
				assert(false);
		}

		std::set<CfgNode::Edge> succs = node->successors();
		for (std::set<CfgNode::Edge>::iterator it = succs.begin(), ed = succs.end();
				it != ed; it++) {
			switch (node->type()) {
				case CfgNode::CFG_ENTRY:
					ss << "  Entry -> ";
					break;
				case CfgNode::CFG_BLOCK:
				case CfgNode::CFG_PHANTOM:
					ss << "  \"0x" << CfgNode::node2addr(node) << "\" -> ";
					break;
				default:
					assert(false);
			}

			CfgNode* succ = it->node;
			switch (succ->type()) {
				case CfgNode::CFG_EXIT:
					ss << "Exit";
					break;
				case CfgNode::CFG_HALT:
					ss << "Halt";
					break;
				case CfgNode::CFG_BLOCK:
				case CfgNode::CFG_PHANTOM:
					ss << "\"0x" << CfgNode::node2addr(succ) << "\"";
					break;
				case CfgNode::CFG_ENTRY:
				default:
					assert(false);
			}

			ss << std::endl;
		}
	}
	ss << "}" << std::endl;

	return ss.str();
}

void CFG::dumpDOT(const std::string& fileName) {
	std::ofstream fout(fileName);
	if (!fout.is_open())
		throw std::string("Unable to write file: ") + fileName;

	fout << this->toDOT();
	fout.close();
}
