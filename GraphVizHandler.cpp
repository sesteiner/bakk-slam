#include <iostream>
#include <sstream>
#include "GraphVizHandler.h"
#include "line.h"
#include <math.h>

Agnode_t* GraphvizHandler::addNode(std::string name)
	{
		std::string shape = std::string("shape");
		std::string shape_value = std::string("box");

		Agnode_t* n = agnode(graph_, &name[0u], 1);
		agsafeset(n, &shape[0u], &shape_value[0u], &shape_value[0u]);
		return n;
	}
	
Agnode_t* GraphvizHandler::formatNode(Agnode_t* node, std::string name)
{
	std::string label = std::string("label");
	std::string label_value = std::string(name);
	agset(node, &label[0u], &label_value[0u]);
	return node;
}

Agedge_t* GraphvizHandler::addEdge(Agnode_t* n1, Agnode_t* n2, std::string name, bool draw_reverse)
	{
		if(draw_reverse)
		{
			return agedge(graph_, n2, n1, &name[0u], 1);
		}
		else
		{
			return agedge(graph_, n1, n2, &name[0u], 1);
		}
	}
	
Agedge_t* GraphvizHandler::formatEdge(Agedge_t* edge, bool set_constraint, bool set_color, bool highlight, bool draw_reverse, PORT_DIRECTION port)
{

	
	if(set_constraint) //these are the "grid" edges because the default is false
	{
		std::string constraint = std::string("constraint");
		std::string constraint_value = std::string("true");
		agset(edge, &constraint[0u], &constraint_value[0u]);
		
		std::string weight = std::string("weight");
		std::string weight_value = std::string("40");
		agset(edge, &weight[0u], &weight_value[0u]);
	}
	
	if(!set_color) //these are the "grid" edges because the default is true
	{	
		std::string color = std::string("color");
		std::string color_value = std::string("red");
		agset(edge, &color[0u], &color_value[0u]);
		
		std::string style = std::string("style");
		std::string style_value = std::string("invis");
		agset(edge, &style[0u], &style_value[0u]);
	}
	else
	{
	
		if(highlight)
		{
			std::string color = std::string("color");
			std::string color_value = std::string("blue");
			agset(edge, &color[0u], &color_value[0u]);
		
			std::string style = std::string("style");
			std::string style_value = std::string("solid");
			agset(edge, &style[0u], &style_value[0u]);
		}
		else
		{
			std::string style = std::string("style");
			std::string style_value = std::string("solid");
			agset(edge, &style[0u], &style_value[0u]);
		}
	}

	if(port!=PORT_DIRECTION::C)
	{
		
		std::string port_v = std::string("c");
		
		switch(port)
		{
			case PORT_DIRECTION::N: port_v = "n";break;
			case PORT_DIRECTION::NE: port_v = "ne";break;
			case PORT_DIRECTION::E: port_v = "e";break;
			case PORT_DIRECTION::SE: port_v = "se";break;
			case PORT_DIRECTION::S: port_v = "s";break;
			case PORT_DIRECTION::SW: port_v = "sw";break;
			case PORT_DIRECTION::W: port_v = "w";break;
			case PORT_DIRECTION::NW: port_v = "nw";break;
			case PORT_DIRECTION::C: port_v = "c";break;
		}
		std::string headport = std::string("headport");
		std::string tailport = std::string("tailport");
		agset(edge, &headport[0u], &port_v[0u]);
		agset(edge, &tailport[0u], &port_v[0u]);
	}
	
	if(draw_reverse) 
	{
		std::string dir = std::string("dir");
		std::string dir_value = std::string("back");
		agset(edge, &dir[0u], &dir_value[0u]);
		/*
		std::string color = std::string("color");
		std::string color_value = std::string("blue");
		agset(edge, &color[0u], &color_value[0u]);
		*/
	}
	
	return edge;
}


//small function for recursive action
std::vector<Agnode_t*> GraphvizHandler::createLineGrid(std::vector<Agnode_t*> prev_nodes, Line* line, std::vector<predicate_setting>* setting, int position)
{

	if(position == this->s_->predicates->size()) //deepest lvl
		{
		//std::string nodename = getNodeLabelFromSetting(*setting);
		  //std::cout << nodename << std::endl;
			Agnode_t* node = formatNode(addNode(getNodeIdFromSetting(line, *setting)), getNodeLabelFromSetting(*setting));
			prev_nodes.push_back(node);			
		}
	else
	{
		setting->push_back(predicate_setting(this->s_->c->bool_const(this->s_->predicates->at(position).c_str()), true));
		prev_nodes = createLineGrid(prev_nodes, line, setting, position+1);
		setting->pop_back();
		
		setting->push_back(predicate_setting(this->s_->c->bool_const(this->s_->predicates->at(position).c_str()), false));
		prev_nodes = createLineGrid(prev_nodes,line, setting, position+1);
		setting->pop_back();
	}
	return prev_nodes;
}

std::vector<Agnode_t*> GraphvizHandler::addStructurNodes()
{
	std::vector<Agnode_t*> nodes = std::vector<Agnode_t*>();
	
	Agnode_t* n_prev = addNode("programm");
	nodes.push_back(n_prev);
	int max_nodes = (s_->predicates->size());
	for(int i = 0;i < std::pow(2,max_nodes); i++)
	{
		Agnode_t* n_1 = addNode(std::to_string(i));
		formatEdge(addEdge(n_prev, n_1, "aa"),true, true);
		n_prev = n_1;
		nodes.push_back(n_1);
	}
	
	return nodes;


}

std::vector<Agnode_t*> GraphvizHandler::addLineStuff(Line* line, std::vector<Agnode_t*> previous_nodes)
{
	std::vector<Agnode_t*> current_nodes; 
	int index = 0;
	bool first_line=false;
	
	if(previous_nodes.size()==0)
		first_line=true;
	

	//add the abstraction line also on the left side for overview purposes	
	//std::cout << line->getAbstraction()->getVerificationConjecture() << std::endl;

	
	std::string abs;
	abs = line->getAbstraction()->getReadableRepresenation();
	
	Agnode_t* n_abs = addNode("L" + std::to_string(line->getLine()) + " " +abs);

	std::string attr_fixedsize = std::string("fixedsize");
	std::string attr_fixedsize_value = std::string("true");
	agset(n_abs, &attr_fixedsize[0u], &attr_fixedsize_value[0u]);	
	
	std::string attr_width = std::string("width");
	std::string attr_width_value = std::string("4");
	agset(n_abs, &attr_width[0u], &attr_width_value[0u]);
	
	
	if(!first_line)
		formatEdge(addEdge(previous_nodes.at(0),n_abs,"aa"), true, false);
		
	current_nodes.push_back(n_abs);
					
	//Do this for all values of pred and setting
	std::vector<predicate_setting>* setting = new std::vector<predicate_setting>();
	current_nodes =	createLineGrid(current_nodes, line, setting, 0);
	delete setting;
	
	//TODO: somehow sort his mess of current_nodes 
	
		
	
	if(first_line)//horizontal grid only in first line
	{ 
		//insert ghost node for better spacing
		//std::cout<< "first line" << std::endl;
		Agnode_t* n_prev = n_abs;
		
		/*
		std::vector<Agnode_t*> structure_nodes = addStructurNodes();
		std::cout << "current_nodes.size() = " << current_nodes.size() << "structure_nodes.size() = " << structure_nodes.size() << std::endl;
		assert(current_nodes.size()==structure_nodes.size());
		for(int i = 0; i < structure_nodes.size(); i++)
		{
			Agedge_t* e = formatEdge(addEdge(structure_nodes.at(i),current_nodes.at(i),"aa"), true, false);
			std::string minlen = std::string("minlen");
			std::string minlen_value = std::string(std::to_string(structure_nodes.size()-i+1));
			agset(e, &minlen[0u], &minlen_value[0u]);	
		}
		*/
		
		for(Agnode_t* n : current_nodes) 
		{
			Agedge_t* e;
			if(n != n_abs)
			{	
				e = formatEdge(addEdge(n_prev,n,"aa"), false, false);				
			}
			//std::string attr_len = std::string("weight");
			//std::string attr_len_value = std::string("2");
			//agset(e, &attr_len[0u], &attr_len_value[0u]);		
			n_prev = n;
		}
	}
	else //not first line -> previous_nodes is not empty
	{
	assert(current_nodes.size()==previous_nodes.size());
		for(int i = 0; i < current_nodes.size(); i++)
		{
			formatEdge(addEdge(previous_nodes.at(i),current_nodes.at(i),"aa"), true, false);
		}	
	}
	
	return current_nodes;
}

std::string GraphvizHandler::getNodeIdFromSetting(Line* line, std::vector<predicate_setting> setting)
{
	//Predicates always in Alphabetical order -> since we get a copy we dont acutally change the order
	std::sort (setting.begin(),setting.end(),sortPredicateSettingFunction);
	
	std::string nodename = "L" + std::to_string(line->getLine());
	
	for(predicate_setting set : setting)
	{
		nodename = nodename + set.predicate->to_string() + std::to_string(set.bool_setting);
	}
	return nodename;
}

std::string GraphvizHandler::getNodeLabelFromSetting(std::vector<predicate_setting> setting)
{
	//Predicates always in Alphabetical order -> since we get a copy we dont acutally change the order
	std::sort (setting.begin(),setting.end(),sortPredicateSettingFunction);
	
	//std::string line_nr = "L" + std::to_string(line->getLine());
	std::string predicates = "";
	std::string p_setting = "";
	
	for(predicate_setting set : setting)
	{
		predicates = predicates + set.predicate->to_string();
		p_setting = p_setting + std::to_string(set.bool_setting);
	}
	return predicates + "\\n" + p_setting + "\\n";
}

Agedge_t* GraphvizHandler::addEdgeStuff(Line* this_line, Line* next_line, std::vector<predicate_setting> setting, std::vector<predicate_setting> setting_target, PORT_DIRECTION port)
{	
	
	bool draw_reverse = false;
	if(this_line->getLine()>next_line->getLine())
	{
		draw_reverse = true;
		port=PORT_DIRECTION::SW; //change this if the lines should be spaced out a bit more
	}

	bool port_means_constraint = false;
	if(port!=PORT_DIRECTION::C)
	{
		//std::cout << "  port_means_constraint!!!!!!!!!!!!!" << std::endl;
		port_means_constraint = true;
	}
		
		
	std::string node_start = getNodeIdFromSetting(this_line, setting);
	std::string node_end = getNodeIdFromSetting(next_line, setting_target);

		#ifdef DEBUGBOOLMC
		std::cout << "  Edge [" << node_start << " -> " << node_end << "]" << std::endl;
		#endif // ifdef DEBUGBOOLMC


	Agnode_t* n1 = addNode(node_start); //returns pointer to existing node
	Agnode_t* n2 = addNode(node_end); //returns pointer to existing node
	Agedge_t* e = formatEdge(addEdge(n1,n2,"aa", draw_reverse), port_means_constraint, true, false, draw_reverse, port); //might return pointer to existing node;

	std::string color = std::string("color");
	std::string color_value = std::string("black");
	agset(e, &color[0u], &color_value[0u]);
	
	//addNode("L" + std::to_string(line->getLine()) + pred + "F"); //returns pointer to existing node
	//std::vector<Agnode_t*> current_nodes;
	return e;
}



Agraph_t* GraphvizHandler::createGraph(std::vector<Line*>* lines)
	{
		//Creating a Node (if one with this name exits it points to that one)=> Agnode_t *agnode(Agraph_t*, char*, int);
		//Creating a Edge beween two nodes => Agedge_t *agedge(Agraph_t*, Agnode_t*, Agnode_t*, char*, int);
	
		//do pre-formatting GRAPH
		std::string splines = std::string("outputorder");
		std::string splines_value = std::string("breadthfirst");
		agattr(this->graph_, AGRAPH, &splines[0u], &splines_value[0u]);
		
		//do pre-formatting EDGES
		std::string constraint = std::string("constraint");
		std::string constraint_value = std::string("false");
		agattr(this->graph_, AGEDGE, &constraint[0u], &constraint_value[0u]);
		
		std::string style = std::string("style");
		std::string style_value = std::string("solid");
		agattr(this->graph_, AGEDGE, &style[0u], &style_value[0u]);
		
		std::string weight = std::string("weight");
		std::string weight_value = std::string("1");
		agattr(this->graph_, AGEDGE, &weight[0u], &weight_value[0u]);
		
		std::string color = std::string("color");
		std::string color_value = std::string("black");
		agattr(this->graph_, AGEDGE, &color[0u], &color_value[0u]);
		
		std::string minlen = std::string("minlen");
		std::string minlen_value = std::string("1");
		agattr(this->graph_, AGEDGE, &minlen[0u], &minlen_value[0u]);	
		
		std::string headport = std::string("headport");
		std::string tailport = std::string("tailport");
		std::string portPos_value = std::string("c");
		agattr(this->graph_, AGEDGE, &headport[0u], &portPos_value[0u]);
		agattr(this->graph_, AGEDGE, &tailport[0u], &portPos_value[0u]);
	
		std::string dir = std::string("dir");
		std::string dir_value = std::string("forward");
		agattr(this->graph_, AGEDGE, &dir[0u], &dir_value[0u]);
		
		//do pre-formatting NODES
		std::string attr_fixedsize = std::string("fixedsize");
		std::string attr_fixedsize_default = std::string("false");
		agattr(this->graph_, AGNODE, &attr_fixedsize[0u], &attr_fixedsize_default[0u]);
			
		std::string attr_width = std::string("width");
		std::string attr_width_default = std::string("0.75");			
		agattr(this->graph_, AGNODE, &attr_width[0u], &attr_width_default[0u]);
		
		style_value = std::string("");
		agattr(this->graph_, AGNODE, &style[0u], &style_value[0u]);	
		
		//left alignment of label in nodes
		std::string label = std::string("label");
		std::string label_default = std::string("\\N\\l");
		agattr(this->graph_, AGNODE, &label[0u], &label_default[0u]);
		
	
		int current_line=0;	
		
		//Create Subgrid so the Graph has a grid shape
		//the other edges need to be set constraint = false to not disturb this beautiful order!!
		std::vector<Agnode_t*> dummy;
		for(Line* line : (*lines))
		{
			dummy = line->printLine(this, dummy);
		}
		
		//add Edges according to the abstraction

		std::vector<predicate_setting>* setting = new std::vector<predicate_setting>();
		if(lines->size()>0)
		{
		  lines->front()->try_every_start_setting(this, setting, 0);
	  }
	  else
	  {
		  std::cout << "Your Program has less than 2 Lines? What? -> Try again!" << std::endl;
	  }
	  
	  delete setting;

		return graph_;
	}
	
bool GraphvizHandler::createImage(std::vector<Line*>* lines)
{
	Agnode_t *n, *m;
	Agedge_t *e;

	GVC_t* gvc;
	gvc = gvContext(); /* library function */
	createGraph(lines); //graph is created in Constructor
	gvLayout(gvc, graph_, "dot"); /* library function */	
	drawGraph(gvc);

	//This stuff frees stuff that is already free
	
	//gvLayoutJobs(gvc, g);
	// Write the graph according to -T and -o options */
	//gvRenderJobs(gvc, g);


	//gvFreeLayout(gvc, graph_); /* library function */
	//agclose(graph_); /* library function */
	//gvFreeContext(gvc);

	return false;
}
	

