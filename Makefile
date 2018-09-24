VPATH = include

all: lemon flex
	g++ -g -std=c++11 -o slam flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt

wall: lemon flex
	g++ -g -Wall -std=c++11 -o  slam flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt

debug: lemon flex
	g++ -g -std=c++11 -DDEBUG -DPRINT_MODEL -o slam flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt

debugcex: lemon flex
	g++ -g -std=c++11 -DDEBUG -DDEBUGCEXANALYSIS -DPRINT_MODEL -o slam flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt
	
debugverify: lemon flex
	g++ -g -std=c++11 -DDEBUG -DDEBUGLEMON -DDEBUGVERIFY -DPRINT_MODEL -o slam flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt

debugmc: lemon flex
	g++ -g -std=c++11 -DDEBUG -DDEBUGBOOLMC -DPRINT_MODEL -o slam flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt
	
debuglemon: lemon flex
	g++ -g -std=c++11 -DDEBUGLEMON -DPRINT_MODEL -o slam flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt

debugflex: lemon flex
	g++ -g -std=c++11 -DDEBUGFLEX -DPRINT_MODEL -o slam flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt

z3debug: lemon flex
	g++ -g -std=c++11 -DPRINT_MODEL -DTYPES -o hoare flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt

syntax: lemon flex
	g++ -g -std=c++11 -DNO_PROOF -o slam flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt

lines: lemon flex
	g++ -g -std=c++11 -DLINES -o slam flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt
	
types: lemon flex
	g++ -g -std=c++11 -DTYPES -o slam flex.cpp lemon.c slam.cpp line.cpp GraphVizHandler.cpp -lz3 -lgvc -lcgraph -lcdt


lemon:
	lemon lemon.y

flex:
	flex -+ -o flex.cpp --header-file=flex.h flex.l 

submission:
	tar -cf submission.tar slam.cpp line.cpp GraphVizHandler.cpp tests/bubble.hoare tests/magic.hoare tests/max.hoare tests/search.hoare

ours:
	tar -cf solution.tar flex.l lemon.y slam.h slam.cpp line.cpp GraphVizHandler.cpp 


clean:
	rm out.dot
	rm out.png
	rm flex.cpp
	rm flex.h
	rm lemon.h
	rm lemon.c
	rm lemon.out
	rm slam

