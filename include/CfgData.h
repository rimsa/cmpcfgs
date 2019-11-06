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

#ifndef _CFGDATA_H
#define _CFGDATA_H

#include <set>
#include <Instruction.h>

class CFG;

class CfgData {
public:
	struct Node {
		Addr start;
		int size;

		Node(Addr start, int size) : start(start), size(size) {}
		virtual ~Node() {}

	    bool operator<(const Node& n) const {
	    		if (start == n.start)
	    			return size < n.size;
	    		else
	    			return start < n.start;
	    }

	    bool operator==(const Node& n) const {
	        return start == n.start && size == n.size;
	    }
	};

	// if from is zero the edge is from the entry node,
	// if the to is zero the edge is to the exit node.
	struct Edge {
		Addr from, to;
		Edge(Addr from, Addr to)
			: from(from), to(to) {}
		virtual ~Edge() {}

	    bool operator<(const Edge& e) const {
	    		if (from == e.from)
	    			return to < e.to;
	    		else
	    			return from < e.from;
	    }

	    bool operator==(const Edge& e) const {
	        return from == e.from && to == e.to;
	    }
	};

	struct Call {
		Addr instr;
		std::set<Addr> calls;

		Call(Addr instr) : instr(instr) {}
		virtual ~Call() {}

	    bool operator<(const Call& c) const {
	    		return instr < c.instr;
	    }

	    bool operator==(const Call& c) const {
	        return instr == c.instr;
	    }
	};

	CfgData(CFG* cfg);
	virtual ~CfgData();

	const std::set<Addr>& instrs() const { return m_instrs; }
	const std::set<CfgData::Node>& blocks() const { return m_blocks; }
	const std::set<Addr>& phantoms() const { return m_phantoms; }
	const std::set<CfgData::Edge> edges() const { return m_edges; }
	const std::set<CfgData::Call> calls() const { return m_calls; }
	const std::set<Addr>& indirects() const { return m_indirects; }

private:
	std::set<Addr> m_instrs;
	std::set<CfgData::Node> m_blocks;
	std::set<Addr> m_phantoms;
	std::set<CfgData::Edge> m_edges;
	std::set<CfgData::Call> m_calls;
	std::set<Addr> m_indirects;

};

#endif

