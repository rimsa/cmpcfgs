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

#ifndef _CFGNODE_H
#define _CFGNODE_H

#include <set>
#include <list>
#include <Instruction.h>

class CFG;

class CfgNode {
public:
	enum Type {
		CFG_ENTRY,
		CFG_BLOCK,
		CFG_PHANTOM,
		CFG_EXIT,
		CFG_HALT
	};

	class Data {
	public:
		virtual ~Data() {}

		Addr addr() const { return m_addr; }

	protected:
		Addr m_addr;

		Data(Addr addr) : m_addr(addr) {}
	};

	class PhantomData : public Data {
	public:
		PhantomData(Addr addr) : Data(addr) {}
		virtual ~PhantomData() {};
	};

	class BlockData : public Data {
	public:
		BlockData(Addr addr) : Data(addr), m_size(0), m_indirection(false) {}
		virtual ~BlockData() {};

		int size() const { return m_size; }
		bool hasIndirection() const { return m_indirection; }
		const std::list<Instruction*>& instructions() const { return m_instrs; }
		const std::set<CFG*>& calls() const { return m_calls; }

		void setIndirect(bool indirect = true);

		void addInstruction(Instruction* instr);
		void addInstructions(const std::list<Instruction*>& instrs);
		Instruction* firstInstruction() const;
		Instruction* lastInstruction() const;
		void clearInstructions();

		void addCall(CFG* cfg);
		void addCalls(const std::set<CFG*>& calls);
		void clearCalls();

	private:
		int m_size;
		bool m_indirection;
		std::list<Instruction*> m_instrs;
		std::set<CFG*> m_calls;
	};

	struct Edge {
		CfgNode* node;
		bool virtua;

		Edge(CfgNode* node) : node(node), virtua(false) {}
		Edge(CfgNode* node, bool virtua) : node(node), virtua(virtua) {}
		Edge(const Edge& e) : node(e.node), virtua(e.virtua) {}
		virtual ~Edge() {}

		Edge& operator=(const Edge& e) {
			node = e.node;
			virtua = e.virtua;
			return *this;
		}

		bool operator<(const Edge& e) const {
			return node < e.node;
		}

		bool operator==(const Edge& e) const {
			return node == e.node;
		}
	};

	CfgNode(enum CfgNode::Type type);
	virtual ~CfgNode();

	enum CfgNode::Type type() const { return m_type; }
	Data* data() const { return m_data; }
	const std::set<CfgNode::Edge>& successors() const { return m_succs; }
	const std::set<CfgNode::Edge>& predecessors() const { return m_preds; }

	void setData(Data* data);

	int countSuccessors() const { return m_succs.size(); }
	bool hasSuccessors() const { return m_succs.size() > 0; }
	bool addSuccessor(CfgNode* succ, bool virtua = false);
	void addSuccessors(const std::set<CfgNode::Edge>& succs);
	bool removeSuccessor(CfgNode* succ);
	void clearSuccessors();

	int countPredecessor() const { return m_preds.size(); }
	bool hasPredecessor() const { return m_preds.size() > 0; }
	bool addPredecessor(CfgNode* pred, bool virtua = false);
	void addPredecessors(const std::set<CfgNode::Edge>& preds);
	bool removePredecessor(CfgNode* pred);
	void clearPredecessors();

	static Addr node2addr(CfgNode* node);

private:
	enum CfgNode::Type m_type;
	Data* m_data;
	std::set<CfgNode::Edge> m_succs;
	std::set<CfgNode::Edge> m_preds;

};

#endif
