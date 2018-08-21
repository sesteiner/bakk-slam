#ifndef GRAPHVIZHANDLER_H
#define GRAPHVIZHANDLER_H 1


#include <assert.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include "slam.h"

//install with sudo apt-get install z3 something something
#include <z3++.h>

//install with sudo apt-get install libgraphviz-dev
#include <graphviz/gvc.h>


//GRAPHVIZ STUFF
class Line;
struct smt_params;
struct predicate_setting;

class GraphvizHandler 
{
	protected:
		  // The z3::context (used for creating z3::expr and solving)
	smt_params* s_;
	Agraph_t* graph_;
	std::vector<std::string>* visited_nodes_;

	public:
	GraphvizHandler(smt_params* s) : s_(s), graph_(NULL)
	{
		std::string name = std::string("Boolean MC");
		std::string attr = std::string("shape");
		std::string attr_value = std::string("square");

		graph_ = agopen(&name[0u], Agdirected, 0);
		visited_nodes_ = new std::vector<std::string>();
		agsafeset(graph_, &attr[0u], &attr_value[0u], &attr_value[0u]);
	}
	~GraphvizHandler()
	{
	}
	
	std::vector<std::string>* getVisitedNodes() { return visited_nodes_; };	
	void addVisitedNode(std::string nodename) { visited_nodes_->push_back(nodename); };
	
	std::vector<Agnode_t*> createLineGrid(std::vector<Agnode_t*> prev_nodes, Line* line, std::vector<predicate_setting>* setting, int position);

	std::string getNodeLabelFromSetting(std::vector<predicate_setting> setting);
	std::string getNodeIdFromSetting(Line* line, std::vector<predicate_setting> setting);
	//std::string getNodeNameAndSortFromPredicateSetting(Line* line, std::vector<predicate_setting>* setting);

	Agnode_t* addNode(std::string name);
	
	Agnode_t* formatNode(Agnode_t* node, std::string name);

	Agedge_t* addEdge(Agnode_t* n1, Agnode_t* n2, std::string name, bool draw_reverse = false);
	
	Agedge_t* formatEdge(Agedge_t* edge, bool set_constraint_to_false = false, bool set_color = true, bool draw_reverse = false, PORT_DIRECTION port = PORT_DIRECTION::C );
	
	Agraph_t* createGraph(std::vector<Line*>* lines);

	void drawGraph(GVC_t* gvc)
	{ 
	gvRenderFilename (gvc, graph_, "dot", "out.dot");
	gvRenderFilename (gvc, graph_, "png", "out.png"); }

	bool createImage(std::vector<Line*>* lines);
		
	std::vector<Agnode_t*> addLineStuff(Line* line, std::vector<Agnode_t*> previous_nodes);
	
	void addEdgeStuff(Line* this_line, Line* next_line, std::vector<predicate_setting> setting, std::vector<predicate_setting> setting_target, PORT_DIRECTION port = PORT_DIRECTION::C);
	
	std::vector<Agnode_t*> addStructurNodes();
};

//GRAPHVIZ STUFF
#endif // ifndef GRAPHVIZHANDLER_H
