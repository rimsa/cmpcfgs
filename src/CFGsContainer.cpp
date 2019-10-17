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
#include <cctype>
#include <cassert>
#include <algorithm>

#include <CFGsContainer.h>

CFGsContainer::CFGsContainer(const std::string& filename, const std::string& name)
	: m_input(filename, std::fstream::in), m_name(name) {
	m_input >> std::noskipws;

	m_currentToken = nextToken();
	processCFGs();

	m_input.close();
}

CFGsContainer::~CFGsContainer() {
	for (std::map<Addr, CFG*>::iterator it = m_cfgsMap.begin(),
			ed = m_cfgsMap.end(); it != ed; it++) {
		delete it->second;
	}
}

CFG* CFGsContainer::cfg(Addr addr) const {
	std::map<Addr, CFG*>::const_iterator it = m_cfgsMap.find(addr);
	return it != m_cfgsMap.end() ? it->second : 0;
}

std::set<CFG*> CFGsContainer::cfgs() const {
	std::set<CFG*> cfgs;

	std::transform(m_cfgsMap.begin(), m_cfgsMap.end(),
		std::inserter(cfgs, cfgs.begin()),
		[](const std::map<Addr, CFG*>::value_type &pair) {
			return pair.second;
		}
	);

	return cfgs;
}

void CFGsContainer::compressAll() {
	for (std::map<Addr, CFG*>::iterator it = m_cfgsMap.begin(),
			ed = m_cfgsMap.end(); it != ed; it++) {
		CFG* cfg = it->second;
		cfg->compress();
	}
}

void CFGsContainer::checkAll() {
	for (std::map<Addr, CFG*>::iterator it = m_cfgsMap.begin(),
			ed = m_cfgsMap.end(); it != ed; it++) {
		CFG* cfg = it->second;
		cfg->check();
	}
}

void CFGsContainer::dumpAll(const char* directory) {
	for (std::map<Addr, CFG*>::iterator it = m_cfgsMap.begin(),
			ed = m_cfgsMap.end(); it != ed; it++) {
		CFG* cfg = it->second;
		if (cfg->status() != CFG::VALID)
			continue;

		std::stringstream ss;
		ss << directory << "/cfg" << m_name << "-0x" << std::hex << cfg->addr() << ".dot";
		cfg->dumpDOT(ss.str());
	}
}

inline
int nextChar(std::fstream& input) {
	if (input.eof())
		return -1;

	char ch;
	input >> ch;
	return ch;
}

CFGsContainer::Lexeme CFGsContainer::nextToken() {
	Lexeme lex;

    int state = 1;
    while (state != 8) {
        int c = nextChar(m_input);
        switch (state) {
            case 1:
            		if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
					state = 1;
				} else if (c == '0') {
					lex.token += (char) c;
					lex.type = Lexeme::TKN_NUMBER;
					lex.data.number = 0;
					state = 2;
				} else if (c >= '1' && c <= '9') {
					lex.token += (char) c;
					lex.type = Lexeme::TKN_NUMBER;
					state = 4;
				} else if (std::isalpha(c)) {
					lex.token += (char) std::tolower(c);
					state = 5;
				} else if (c == '[') {
					lex.token += (char) c;
					lex.type = Lexeme::TKN_BRACKET_OPEN;
					state = 8;
				} else if (c == ']') {
					lex.token += (char) c;
					lex.type = Lexeme::TKN_BRACKET_CLOSE;
					state = 8;
				} else if (c == '\"') {
					lex.type = Lexeme::TKN_TEXT;
					state = 6;
				} else if (c == '\'') {
					lex.token += (char) c;
					lex.type = Lexeme::TKN_PRIME;
					state = 8;
				} else if (c == '#') {
					state = 7;
				} else if (c == -1) {
					lex.type = Lexeme::TKN_EOF;
					state = 8;
				} else {
					lex.type = Lexeme::TKN_INVALID_TOKEN;
					state = 8;
				}

				break;
            case 2:
				if (std::tolower(c) == 'x') {
					lex.token += (char) std::tolower(c);
					lex.type = Lexeme::TKN_ADDR;
					state = 3;
				} else {
					if (c != -1)
						m_input.putback(c);

					state = 8;
				}

            		break;
            case 3:
				if (std::isdigit(c) ||
					(std::tolower(c) >= 'a' && std::tolower(c) <= 'f')) {
					lex.token += (char) std::tolower(c);
					state = 3;
				} else {
					lex.data.addr = std::stoul(lex.token.substr(2), 0, 16);

					if (c != -1)
						m_input.putback(c);

					state = 8;
				}

            		break;
            case 4:
				if (std::isdigit(c)) {
					lex.token += (char) c;
					state = 4;
				} else {
					lex.data.number = std::stoi(lex.token);

					if (c != -1)
						m_input.putback(c);

					state = 8;
				}

            		break;
            case 5:
				if (std::isalpha(c)) {
					lex.token += (char) std::tolower(c);
					state = 5;
				} else {
					if (lex.token == "cfg") {
						lex.type = Lexeme::TKN_CFG;
					} else if (lex.token == "node") {
						lex.type = Lexeme::TKN_NODE;
					} else if (lex.token == "exit") {
						lex.type = Lexeme::TKN_EXIT;
					} else if (lex.token == "halt") {
						lex.type = Lexeme::TKN_HALT;
					} else if (lex.token == "true") {
						lex.type = Lexeme::TKN_BOOL;
						lex.data.boolean = true;
					} else if (lex.token == "false") {
						lex.type = Lexeme::TKN_BOOL;
						lex.data.boolean = false;
					} else {
						lex.type = Lexeme::TKN_INVALID_TOKEN;
					}

					if (c != -1)
						m_input.putback(c);

					state = 8;
				}

				break;
			case 6:
				if (c == -1) {
					lex.type = Lexeme::TKN_UNEXPECTED_EOF;
					state = 8;
				} else {
					if (c == '\"')
						state = 8;
					else {
						lex.token += (char) c;
						state = 6;
					}
				}

				break;
			case 7:
				if (c == -1) {
					state = 8;
				} else {
					if (c == '\n')
						state = 1;
					else
						state = 7;
				}

				break;
			default:
				lex.type = Lexeme::TKN_INVALID_TOKEN;
				state = 8;
				break;
		}
	}

	return lex;
}

void CFGsContainer::matchToken(enum Lexeme::Type type) {
	if (m_currentToken.type == type) {
		m_currentToken = nextToken();
	} else {
		assert(false);
	}
}

void CFGsContainer::processCFGs() {
	while (m_currentToken.type == Lexeme::TKN_BRACKET_OPEN) {
		matchToken(Lexeme::TKN_BRACKET_OPEN);

		while (m_currentToken.type == Lexeme::TKN_CFG ||
			   m_currentToken.type == Lexeme::TKN_NODE) {
			switch (m_currentToken.type) {
				case Lexeme::TKN_CFG:
				{
					matchToken(Lexeme::TKN_CFG);

					Addr addr = m_currentToken.data.addr;
					matchToken(Lexeme::TKN_ADDR);

					CFG* cfg = this->cfg(addr);
					if (!cfg) {
						cfg = new CFG(addr);
						m_cfgsMap[addr] = cfg;
					}

					bool insideMain = m_currentToken.data.boolean;
					matchToken(Lexeme::TKN_BOOL);
					if (insideMain)
						cfg->markInsideMain();

					std::string name = m_currentToken.token;
					matchToken(Lexeme::TKN_TEXT);
					cfg->setFunctionName(name);

					bool complete = m_currentToken.data.boolean;
					matchToken(Lexeme::TKN_BOOL);
					(void) complete;

					break;
				}
				case Lexeme::TKN_NODE:
				{
					matchToken(Lexeme::TKN_NODE);

					Addr addr = m_currentToken.data.addr;
					matchToken(Lexeme::TKN_ADDR);

					CFG* cfg = this->cfg(addr);
					assert(cfg != 0);

					addr = m_currentToken.data.addr;
					matchToken(Lexeme::TKN_ADDR);

					CfgNode::BlockData* blockData = new CfgNode::BlockData(addr);
					CfgNode* block = cfg->nodeByAddr(addr);
					if (block) {
						assert(block->type() == CfgNode::CFG_PHANTOM);
						block->setData(blockData);
					} else {
						block = new CfgNode(CfgNode::CFG_BLOCK);
						block->setData(blockData);
						cfg->addNode(block);
					}

					matchToken(Lexeme::TKN_BRACKET_OPEN);
					while (m_currentToken.type == Lexeme::TKN_NUMBER) {
						int size = m_currentToken.data.number;
						matchToken(Lexeme::TKN_NUMBER);

						blockData->addInstruction(Instruction::get(addr, size));
						addr += size;
					}
					matchToken(Lexeme::TKN_BRACKET_CLOSE);

					matchToken(Lexeme::TKN_BRACKET_OPEN);
					while (m_currentToken.type == Lexeme::TKN_ADDR) {
						addr = m_currentToken.data.addr;
						matchToken(Lexeme::TKN_ADDR);

						CFG* call = this->cfg(addr);
						if (!call) {
							call = new CFG(addr);
							m_cfgsMap[addr] = call;
						}

						blockData->addCall(call);
					}
					matchToken(Lexeme::TKN_BRACKET_CLOSE);

					bool indirection = m_currentToken.data.boolean;
					matchToken(Lexeme::TKN_BOOL);

					blockData->setIndirect(indirection);

					matchToken(Lexeme::TKN_BRACKET_OPEN);
					while (m_currentToken.type == Lexeme::TKN_ADDR ||
						   m_currentToken.type == Lexeme::TKN_EXIT ||
						   m_currentToken.type == Lexeme::TKN_HALT) {
						CfgNode* succ;
						bool virtua = false;

						switch (m_currentToken.type) {
							case Lexeme::TKN_ADDR:
								addr = m_currentToken.data.addr;
								matchToken(Lexeme::TKN_ADDR);

								if (m_currentToken.type == Lexeme::TKN_PRIME) {
									matchToken(Lexeme::TKN_PRIME);
									virtua = true;
								}

								if (!(succ = cfg->nodeByAddr(addr))) {
									CfgNode::PhantomData* phantomData =
											new CfgNode::PhantomData(addr);

									succ = new CfgNode(CfgNode::CFG_PHANTOM);
									succ->setData(phantomData);

									cfg->addNode(succ);
								}

								break;
							case Lexeme::TKN_EXIT:
								matchToken(Lexeme::TKN_EXIT);

								succ = cfg->exitNode();
								if (!succ) {
									succ = new CfgNode(CfgNode::CFG_EXIT);
									cfg->addNode(succ);
								}

								break;
							case Lexeme::TKN_HALT:
								matchToken(Lexeme::TKN_HALT);

								succ = cfg->haltNode();
								if (!succ) {
									succ = new CfgNode(CfgNode::CFG_HALT);
									cfg->addNode(succ);
								}

								break;
							default:
								assert(false);
						}

						cfg->addEdge(block, succ, virtua);
					}
					matchToken(Lexeme::TKN_BRACKET_CLOSE);

					break;
				}
				default:
					assert(false);
			}
		}

		matchToken(Lexeme::TKN_BRACKET_CLOSE);
	}
}
