CXX=g++
CXXFLAGS=-g -ggdb -O2 -Wall -std=c++11 -Iinclude

LD=ld
LDFLAGS=

RM=rm
RMFLAGS=-rf

OBJS=src/Instruction.o src/CfgNode.o src/CFG.o src/CfgData.o src/CFGsContainer.o src/Strategy.o src/SimpleStrategy.o src/SpecificStrategy.o src/cmpcfgs.o
OUTPUT=cmpcfgs

all: $(OUTPUT)

$(OUTPUT):	$(OBJS)
		$(CXX) -o $@ $(OBJS) $(LDFLAGS)

src/Instruction.o: include/Instruction.h

src/CfgNode.o: include/Instruction.h include/CfgNode.h

src/CFG.o: include/CfgNode.h include/CFG.h

src/CfgData.o: include/CFG.h include/CfgData.h

src/CFGsContainer.o: include/CFG.h include/CFGsContainer.h

src/Strategy.o: include/Instruction.h include/CFGsContainer.h include/Strategy.h

src/SimpleStrategy.o: include/CFGsContainer.h include/SimpleStrategy.h

src/SpecificStrategy.o: include/CFGsContainer.h include/SpecificStrategy.h

src/cmpcfgs.o: include/SimpleStrategy.h include/SpecificStrategy.h

clean:
	$(RM) $(RMFLAGS) $(OBJS) $(OUTPUT)

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<
