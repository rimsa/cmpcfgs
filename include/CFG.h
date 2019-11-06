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

#ifndef _CFG_H
#define _CFG_H

#include <map>
#include <string>

#include <CfgNode.h>

class CFG {
public:
	enum Status {
		UNCHECKED,
		INVALID,
		VALID
	};

	CFG(Addr addr);
	virtual ~CFG();

	Addr addr() const { return m_addr; }
	bool isInsideMain() const { return m_insideMain; }
	const std::string& functionName() const { return m_functionName; }
	enum Status status() const { return m_status; }

	CfgNode* entryNode() const { return m_entryNode; }
	CfgNode* exitNode() const { return m_exitNode; }
	CfgNode* haltNode() const { return m_haltNode; }
	CfgNode* nodeByAddr(Addr addr) const;
	std::list<CfgNode*> nodes() const;

	bool containsNode(CfgNode* node);
	void addEdge(CfgNode* from, CfgNode* to);

	void markInsideMain();
	void setFunctionName(const std::string& functionName);
	void addNode(CfgNode* node);

	void compress();
	enum CFG::Status check();

	std::string toDOT() const;
	void dumpDOT(const std::string& fileName);

private:
	Addr m_addr;
	enum Status m_status;
	bool m_insideMain;
	std::string m_functionName;
	CfgNode* m_entryNode;
	CfgNode* m_exitNode;
	CfgNode* m_haltNode;
	std::map<Addr, CfgNode*> m_nodesMap;

};

#endif
