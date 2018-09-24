#include "line.h"
#include <iomanip>
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include "slam.h"

// Not Line specific
bool sortLinesFunction(Line* i, Line* j) 
{ 
	return i->getLine()<j->getLine();
}

bool sortPredicateSettingFunction(predicate_setting i, predicate_setting j) 
{ 
	return (i.predicate->to_string()) < (j.predicate->to_string());
}

bool sortStringFunction(std::string i, std::string j) 
{ 
	return i < j;
}

// Class Line

z3::expr Line::getVerificationConjectureExpression() { return this->getExpression()->getVerificationConjecture();}

z3::expr Line::getVerificationConjectureAbstraction() { return this->getAbstraction()->getVerificationConjecture();}

z3::expr LineAssignment::getVerificationConjectureExpression() 
{ 
	return this->getExpression()->getVerificationConjecture();
}
z3::expr LineAssignment::getVerificationConjectureAbstraction()
{
	z3::expr e = s_->c->bool_val(true);
	bool first = true;
	for(Expression* e_loop : *abstraction_vec_)
	{
		if(first)
		{
			e = e_loop->getVerificationConjecture();		
			first = false;
		}
		else
		{
			e = e && e_loop->getVerificationConjecture();
		}
	}
	
	return e;
}
		
		
std::vector<z3::expr*>* Line::add_predicates(z3::solver* solver, bool single_state_assingment = true)
{
	//create vector with all arbitraries or predicates
	std::vector<z3::expr*>* vec_arb = new std::vector<z3::expr*>();

	//preperation for single-state assignment
	z3::expr_vector vars = z3::expr_vector(const_cast<z3::context&>(*s_->c));
	z3::expr_vector vars_replacement = z3::expr_vector(const_cast<z3::context&>(*s_->c));
	for (std::map<std::string, z3::expr>::iterator it_expr = s_->exprs->begin(); it_expr != s_->exprs->end(); ++it_expr)
	{
		vars.push_back(it_expr->second);
		vars_replacement.push_back(s_->c->int_const((it_expr->first+"1").c_str()));
	}

	for(int index = 0; index<s_->predicates->size();index++)
	{
		solver->add(s_->c->bool_const(s_->predicates->at(index).c_str()) == (s_->predicates_expr->at(index)));
		z3::expr* arb = new z3::expr(s_->c->bool_const(s_->predicates->at(index).c_str()));
		vec_arb->push_back(arb);

		if(single_state_assingment)
		{
			solver->add(s_->c->bool_const((s_->predicates->at(index)+"1").c_str()) == s_->predicates_expr->at(index).substitute(vars, vars_replacement));
			z3::expr* arb2 = new z3::expr(s_->c->bool_const((s_->predicates->at(index)+"1").c_str()));
			vec_arb->push_back(arb2);
		}
	}

	return vec_arb;
}

bool Line::verifyAbstraction() 
{
	bool abstraction_is_valid = true;

	#ifdef DEBUGVERIFY
	std::cout << "================================" << std::endl;
	std::cout << line_ << " Verifying Line" << std::endl;
	#endif // ifdef DEBUGVERIFY
	z3::check_result result = z3::unsat;
	//z3::check_result result2 = z3::unsat;
	z3::solver solver_code(*(s_->c));
	z3::solver solver_abstraction(*(s_->c));
	z3::expr code = this->getVerificationConjectureExpression();
	z3::expr conjecture_abstraction = this->getVerificationConjectureAbstraction();

	#ifdef DEBUGVERIFY
	std::cout << "Conjecture: " << code << std::endl;
	std::cout << "Conjecture: " << conjecture_abstraction << std::endl;
	#endif // ifdef DEBUGVERIFY

	//create vector with all arbitraries or predicates
	std::vector<z3::expr*>* vec_arb = add_predicates(&solver_code, expression_->needsSingleStateAssingment_);
	std::vector<z3::expr*>* vec_arb2 = add_predicates(&solver_abstraction, expression_->needsSingleStateAssingment_); //take needsSSA from expression, because otherwise for skip this will result in stupid stuff
	solver_code.add(code);
	solver_abstraction.add(code);
	solver_abstraction.add(conjecture_abstraction);
	result = try_arbitrary_setting(&solver_code, &solver_abstraction, vec_arb, 0);
	
	
	if(result==z3::unsat)
	{
		std::cout << "Abstraction of Line " << getLine() << " is not correct" << std::endl;
	}
	
	#ifdef DEBUGVERIFY
	std::cout << "Result of recursiv check: "  << result << std::endl;
	if(result==z3::unsat)
	{
		std::cout << "   ================================" << std::endl;
		std::cout << "Solver Code:" <<std::endl;
		std::cout << solver_code	<< std::endl;
		std::cout << "Solver Abstraction:" <<std::endl;
		std::cout << solver_abstraction	<< std::endl;
		std::cout << "   ================================" << std::endl;
	}
	std::cout << "================================" << std::endl;
	#endif // ifdef DEBUGVERIFY

	(result==z3::sat)? abstraction_is_valid = true : abstraction_is_valid = false;

	for(z3::expr* vec_arb_expr : *vec_arb)
	{
		delete vec_arb_expr;
	}
	delete vec_arb;
	
	for(z3::expr* vec_arb2_expr : *vec_arb2)
	{
		delete vec_arb2_expr;
	}
	delete vec_arb2;						


	return abstraction_is_valid;
}		

//
// 	Checks if on any "setting" (true/false) of the vector both solvers have the same result
// 	returns z3::unsat if not
//	returns z3::sat if all settings have the same result (meaning the truthtables match)
//
z3::check_result Line::try_arbitrary_setting(z3::solver* solver, z3::solver* solver2, std::vector<z3::expr*>* arb_vec, int position)
{
	z3::check_result result = z3::unknown;

	//check if last node
	if(position >= (arb_vec->size()-1)) { //last node 
		 solver->push();
		 solver2->push();
		 
		    
		 solver->add(*(arb_vec->at(position)) == s_->c->bool_val(true));
		 solver2->add(*(arb_vec->at(position)) == s_->c->bool_val(true));

		if(solver->check()!=solver2->check()) {
			solver->pop();
			solver2->pop();
			#ifdef DEBUGVERIFY
			std::cout << std::setw(position*2) << "" << (*(arb_vec->at(position))) << "=true; ";
			std::cout << " => NOT correct" << std::endl;
			#endif // ifdef DEBUGVERIFY

			return z3::unsat;
		} else {
			#ifdef DEBUGVERIFY
			std::cout << std::setw(position*2) << "" << (*(arb_vec->at(position))) << "=true; ";
			std::cout << " => correct" << std::endl;
			#endif // ifdef DEBUGVERIFY

			solver->pop();
			solver2->pop();
		
			solver->push();
			solver2->push();
			#ifdef DEBUGVERIFY
			std::cout << std::setw(position*2) << "" << (*(arb_vec->at(position))) << "=false; "; 
			#endif // ifdef DEBUGVERIFY
		
			solver->add(*(arb_vec->at(position)) == s_->c->bool_val(false));
			solver2->add(*(arb_vec->at(position)) == s_->c->bool_val(false));

			(solver->check()!=solver2->check())? result=z3::unsat : result=z3::sat;
			solver->pop();
			solver2->pop();
			#ifdef DEBUGVERIFY
			if(result==z3::sat)
				std::cout << " => correct" << std::endl;
			else
				std::cout << " => NOT correct" << std::endl;
			#endif // ifdef DEBUGVERIFY

			return result;
		}
	}

	// NOT last variable in vector
	solver->push();
	solver2->push();
	#ifdef DEBUGVERIFY
	std::cout << std::setw(position*2) << "" << (*(arb_vec->at(position))) << "=true; " << std::endl;  
	#endif // ifdef DEBUGVERIFY

	solver->add(*(arb_vec->at(position)) == s_->c->bool_val(true));
	solver2->add(*(arb_vec->at(position)) == s_->c->bool_val(true));

	if(try_arbitrary_setting(solver, solver2, arb_vec, position+1)!=z3::sat)
	{
		solver->pop();
		solver2->pop();
		return z3::unsat;
	}
	else
	{
		solver->pop();
		solver2->pop();	
		#ifdef DEBUGVERIFY
		std::cout << std::setw(position*2) << "" << (*(arb_vec->at(position))) << "=false; " << std::endl;  
		#endif // ifdef DEBUGVERIFY
		solver->push();
		solver2->push();
		solver->add(*(arb_vec->at(position)) == s_->c->bool_val(false));
		solver2->add(*(arb_vec->at(position)) == s_->c->bool_val(false));
	
		if(try_arbitrary_setting(solver, solver2, arb_vec, position+1)!=z3::sat)
		{
		solver->pop();
		solver2->pop();
		return z3::unsat;
		}
		else
		{
		solver->pop();
		solver2->pop();
		return z3::sat;
		}
	}

	//if we get this far something went wrong
	std::cout << "WHAAAAAAAAAAAAAAT" << std::endl;
	result = solver->check();
	solver->pop();
	solver2->pop();
	return result;
}


void LineIf::setIfBlock(std::vector<Line*>* if_block)
{
	if(if_block!=NULL) {
		std::sort(if_block->begin(),if_block->end(),sortLinesFunction);
		if_block_ = if_block;
		}
}

void LineIfElse::setIfBlock(std::vector<Line*>* if_block)
{
	if(if_block!=NULL) {
		std::sort(if_block->begin(),if_block->end(),sortLinesFunction);
		if_block_ = if_block;
		}
}

void LineIfElse::setElseBlock(std::vector<Line*>* else_block)
{
	if(else_block!=NULL) {
		std::sort(else_block->begin(),else_block->end(),sortLinesFunction);
		else_block_ = else_block;
		}
}

void LineWhile::setWhileBlock(std::vector<Line*>* while_block)
{
	if(while_block!=NULL) {
		std::sort(while_block->begin(),while_block->end(),sortLinesFunction);
		while_block_ = while_block;
	}
}
/********************************** Overrides of PRINT LINE ************************************************/
std::vector<Agnode_t*> LineAssignment::printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes) 
{
	std::vector<Agnode_t*> current_nodes = gh->addLineStuff(this, previous_nodes);
	return current_nodes;
}

std::vector<Agnode_t*> LineIf::printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes) 
{
	std::vector<Agnode_t*> current_nodes = gh->addLineStuff(this, previous_nodes);
	
	if(if_block_!=NULL)
	{
	for(Line* subline : *(if_block_))
		{
		current_nodes = subline->printLine(gh, current_nodes);
		}
	}
	return current_nodes;
}

std::vector<Agnode_t*> LineIfElse::printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes) 
{
	std::vector<Agnode_t*> current_nodes = gh->addLineStuff(this, previous_nodes);
	
	if(if_block_!=NULL)
	{
	for(Line* subline : *(if_block_))
		{
		current_nodes = subline->printLine(gh, current_nodes);
		}
	}

	if(else_block_!=NULL)
	{
		for(Line* subline : *(else_block_))
		{
		current_nodes = subline->printLine(gh, current_nodes);
		}
	}
	return current_nodes;
}

std::vector<Agnode_t*> LineWhile::printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes) 
{
	std::vector<Agnode_t*> current_nodes = gh->addLineStuff(this, previous_nodes);
	
	if(this->getWhileBlock()!=NULL)
	{
		for(Line* subline : *(while_block_))
		{
		current_nodes = subline->printLine(gh, current_nodes);
		}
	}
	return current_nodes;
}

std::vector<Agnode_t*> LineAssert::printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes) 
{
	std::vector<Agnode_t*> current_nodes = gh->addLineStuff(this, previous_nodes);
	
	return current_nodes;
}

std::vector<Agnode_t*> LineSkip::printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes) 
{
	std::vector<Agnode_t*> current_nodes = gh->addLineStuff(this, previous_nodes);
	
	return current_nodes;
}

/****************************** Try every Starting Position *********************/

//In the beginning (only start) go for every start setting
void Line::try_every_start_setting(GraphvizHandler* gh, std::vector<predicate_setting>* setting, int position=0)
{

	if(position==this->s_->predicates->size()) //deepest level
	{
		//setting->push_back(predicate_setting(s_->c->bool_const(this->s_->predicates->at(position).c_str()),true));
		//call function with this vector

		#ifdef DEBUGBOOLMC
		std::cout << "Start from: [";
		for(predicate_setting a : *setting)
		{ std::cout << a << ", "; }	
		std::cout << "]" << std::endl;
		#endif // ifdef DEBUGBOOLMC		
		
		
		std::vector<line_eval*>* path_lines = new std::vector<line_eval*>();
		this->try_every_target_setting(gh, *setting, path_lines);
	}
	else
	{
		setting->push_back(predicate_setting(s_->c->bool_const(this->s_->predicates->at(position).c_str()),true));
		try_every_start_setting(gh, setting, position+1);
		setting->pop_back();
		setting->push_back(predicate_setting(s_->c->bool_const(this->s_->predicates->at(position).c_str()),false));
		try_every_start_setting(gh, setting, position+1);
		setting->pop_back();
	}
}

void ConditionalLine::try_every_target_setting(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<line_eval*>* path_lines)
{
	//check if we already had this constellation!! -> then just stop
	std::string nodename = gh->getNodeIdFromSetting(static_cast<Line*>(this), setting);
	if(std::find(gh->getVisitedNodes()->begin(), gh->getVisitedNodes()->end(), nodename) == gh->getVisitedNodes()->end())
	{
		gh->addVisitedNode(nodename);
		#ifdef DEBUGBOOLMC
		std::cout << "  Checking Setting on Conditional Line(" << this->getLine() << "): " << nodename << std::endl;
		#endif // ifdef DEBUGBOOLMC

	}
	else
	{
		#ifdef DEBUGBOOLMC
		std::cout << "  Already checked this setting: " << nodename << std::endl;
		#endif // ifdef DEBUGBOOLMC

		delete path_lines;
		return;
	}
	
  z3::solver solver(*s_->c);
	
	for(predicate_setting pred : setting)
	{
		solver.add(z3::expr(*(pred.predicate) == s_->c->bool_val(pred.bool_setting)));
	}
	
	
	for(int try_twice = 0; try_twice < 2; try_twice++)
	{	
		solver.push();
		if(try_twice == 0)
			solver.add(this->getVerificationConjectureAbstraction());
		else
			solver.add(!(this->getVerificationConjectureAbstraction()));
	
		if(solver.check()==z3::sat)
			{
				#ifdef DEBUGBOOLMC
				if(try_twice == 0)
					std::cout << "   -> cond satisfiable" << std::endl;
				else
					std::cout << "   -> cond not satisfiable" << std::endl;
				#endif // ifdef DEBUGBOOLMC

		
				z3::model m = solver.get_model(); 
		
				std::vector<predicate_setting> setting_target;
				z3::expr new_constraint = s_->c->bool_val(false); //Only works if we use disjunct!!

				for(std::string pr : *(this->s_->predicates))
				{
				    for (int i = 0; i < m.num_consts(); i++) 
				    {
							if((pr)==m[i].name().str())
							{
								//std::cout <<"Found new Edge with " << m[i]() << std::endl;
								z3::expr r = m.get_const_interp(m.get_const_decl(i)); //this should be a bool
								setting_target.push_back(predicate_setting(s_->c->bool_const(pr.c_str()),z3::eq(r,s_->c->bool_val(true)))); //I want to add the normal predicate NOT the p1 on kind
								//new_constraint = new_constraint || (m[i]() != r);
								i = m.num_consts();
							}
				    }
				}
				std::sort (setting.begin(),setting.end(),sortPredicateSettingFunction);
				std::sort (setting_target.begin(),setting_target.end(),sortPredicateSettingFunction);
				
				std::vector<line_eval*>* path_lines_new = new std::vector<line_eval*>(*path_lines);
				
				bool eval = ((try_twice==0)?true:false);
				path_lines_new->push_back(new line_eval(static_cast<Line*>(this),eval, NULL));
				this->printEdges(gh, setting, setting_target, path_lines_new, eval);
			}
		solver.pop();
		}
		delete path_lines;
}

/********************************** Overrides of "Try every Target Setting" ************************************************/
void NormalLine::try_every_target_setting(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<line_eval*>* path_lines)
{
	//check if we already had this constellation!! -> then just stop
	std::string nodename = gh->getNodeIdFromSetting(static_cast<Line*>(this), setting);
	if(std::find(gh->getVisitedNodes()->begin(), gh->getVisitedNodes()->end(), nodename) == gh->getVisitedNodes()->end())
	{
		gh->addVisitedNode(nodename);
		#ifdef DEBUGBOOLMC
		std::cout << "  Checking Setting on Normal Line(" << this->getLine() << "): " << nodename << std::endl;
		#endif // ifdef DEBUGBOOLMC
	}
	else
	{
		#ifdef DEBUGBOOLMC
		std::cout << "  stop - > already checked this setting: " << nodename << std::endl << std::endl;
		#endif // ifdef DEBUGBOOLMC
		return;
	}
	
	//do a for loop where we let z3 find a counterexpample -> print that edge, add it to solver and try again until we find a z3::unsat
  z3::solver solver(*s_->c);
	std::vector<z3::expr*>* arb_vec;
	
	for(predicate_setting pred : setting)
	{
		solver.add(z3::expr(*(pred.predicate) == s_->c->bool_val(pred.bool_setting)));
	}
	

	solver.add(this->getVerificationConjectureAbstraction());

	while(solver.check()==z3::sat)
		{
		z3::model m = solver.get_model(); 
		
		#ifdef DEBUGBOOLMC
		/*
		std::cout << "Solver: " << std::endl;
		std::cout << solver << std::endl;
		std::cout << "Model: " << std::endl;
		std::cout << m << std::endl;
		*/
		#endif // ifdef DEBUGBOOLMC
		
		std::vector<predicate_setting> setting_target;
		z3::expr new_constraint = s_->c->bool_val(false); //Only works if we use disjunct!!
		//somehow extract the setting from the model and save it in the vector	

		for(std::string pr : *(this->s_->predicates))
		{
			bool found_match = false;
			//TODO: get rid of numbers from single state assignment for the setting_target!!
			//usually only ONE of the predicates has a number in it (singel state assignment)
			for (int i = 0; i < m.num_consts(); i++) 
			{			
				if((pr+"1")==m[i].name().str()) //if I finde the ssa one in the model -> take that one
				{
					#ifdef DEBUGBOOLMC
					std::cout <<"Found new Edge with " << m[i]() << " is " << m.get_const_interp(m.get_const_decl(i))  << std::endl;
					#endif // ifdef DEBUGBOOLMC

					z3::expr r = m.get_const_interp(m.get_const_decl(i)); //this should be a bool
					setting_target.push_back(predicate_setting(s_->c->bool_const(pr.c_str()),z3::eq(r,s_->c->bool_val(true)))); //I want to add the normal predicate NOT the p1 on kind
					new_constraint = new_constraint || (m[i]() != r);
					found_match = true;
					continue;
				}
			}
			if(!found_match) //try the "normal" variables
			{		      
				for (int i = 0; i < m.num_consts(); i++) 
				{			
					if((pr)==m[i].name().str()) //if I finde the ssa one in the model -> take that one
					{
						//std::cout <<"Found new Edge with " << m[i]() << std::endl;
						z3::expr r = m.get_const_interp(m.get_const_decl(i)); //this should be a bool
						setting_target.push_back(predicate_setting(s_->c->bool_const(pr.c_str()),z3::eq(r,s_->c->bool_val(true)))); //I want to add the normal predicate NOT the p1 on kind
						new_constraint = new_constraint || (m[i]() != r);
						found_match = true;
						continue;
					}
				}
			}
		}

		solver.add(new_constraint);
		//call printEdges
		std::sort (setting.begin(),setting.end(),sortPredicateSettingFunction);
		std::sort (setting_target.begin(),setting_target.end(),sortPredicateSettingFunction);
		#ifdef DEBUGBOOLMC
		std::cout << gh->getNodeIdFromSetting(static_cast<Line*>(this), setting_target) << std::endl;
		#endif // ifdef DEBUGBOOLMC
		
		std::vector<line_eval*>* path_lines_new = new std::vector<line_eval*>(*path_lines);
		path_lines_new->push_back(new line_eval(static_cast<Line*>(this),true, NULL));
		this->printEdges(gh, setting, setting_target, path_lines_new);
		//we added found model to constraints and now we try again
		}
	delete path_lines;
}

void SkipLine::try_every_target_setting(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<line_eval*>* path_lines)
{
	path_lines->push_back(new line_eval(static_cast<Line*>(this),true, NULL));
	this->printEdges(gh, setting, setting, path_lines);
}

void EndLine::try_every_target_setting(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<line_eval*>* path_lines)
{
	//check if we already had this constellation!! -> then just stop
	std::string nodename = gh->getNodeIdFromSetting(static_cast<Line*>(this), setting);
	if(std::find(gh->getVisitedNodes()->begin(), gh->getVisitedNodes()->end(), nodename) == gh->getVisitedNodes()->end())
	{
		gh->addVisitedNode(nodename);
		#ifdef DEBUGBOOLMC
		std::cout << "  Checking Setting on Assert Line(" << this->getLine() << "): " << nodename << std::endl;
		#endif // ifdef DEBUGBOOLMC

	}
	else
	{
		#ifdef DEBUGBOOLMC
		std::cout << "  Already checked this setting: " << nodename << std::endl;
		#endif // ifdef DEBUGBOOLMC

		delete path_lines;
		return;
	}
	
  z3::solver solver(*s_->c);
	
	for(predicate_setting pred : setting)
	{
		solver.add(z3::expr(*(pred.predicate) == s_->c->bool_val(pred.bool_setting)));
	}
	
	solver.add(this->getVerificationConjectureAbstraction());
	

	

	path_lines->push_back(new line_eval(static_cast<Line*>(this),true, NULL)); //do this inside the cex analysis by adding predicates as true
	s_->bool_mc_has_endless_loop = false;
		
		#ifdef DEBUGCEXANALYSIS
		std::cout << "START PATH: ";
		#endif // ifdef DEBUGCEXANALYSIS
		
		
	for(line_eval* le : *path_lines)
	{
		#ifdef DEBUGCEXANALYSIS
		std::cout << "L" << le->line->getLine() << "(" << std::to_string(le->eval) << ") -> ";
		#endif // ifdef DEBUGCEXANALYSIS
		
	}
		#ifdef DEBUGCEXANALYSIS
		std::cout << "END" <<	std::endl;
		#endif // ifdef DEBUGCEXANALYSIS
		
	if(solver.check()==z3::sat) // 
	{
		#ifdef DEBUGCEXANALYSIS
		std::cout << "This Path does not contradict the assert." << std::endl;
		#endif // ifdef DEBUGCEXANALYSIS
		return;
	}
	else
	{
		//hihglight the CEX Path in Graphviz Graph
			for(line_eval* line_to_highlight : *path_lines)
				{
					if(line_to_highlight->edge!=NULL)
					{
						gh->formatEdge(line_to_highlight->edge, false, true, true);		
					}
				}
	
		#ifdef DEBUGCEXANALYSIS
		std::cout << "This Path does contradict the assert." << std::endl;
		#endif // ifdef DEBUGCEXANALYSIS
		AnalyseCounterExample(path_lines);
	}

	//std::cout << "THISISTHEEND (" << this->getLine() << ")" << std::endl;
}


void Line::AnalyseCounterExample(std::vector<line_eval*>* path_lines)
{
	z3::solver solver(*s_->c);
	//std::vector<z3::expr*>* abs_vec = add_predicates(&solver, false);
	//delete abs_vec;
	bool cex_is_spurious = false;
	std::vector<std::vector<z3::expr>*>* abstractions = new std::vector<std::vector<z3::expr>*>();
	
	
	
	//add predicate conditions
	for(z3::expr pred : *s_->predicates_expr)
	{
	//std::cout << pred << std::endl;
		solver.add(pred);
	}
	solver.push();

	for(int index = path_lines->size()-1;index > 0;index--)
	{
		//highlight the CEX Path in the Graphviz Graph
		
	
		//first line must be a condition e.g. the assert!!!
		if((index == path_lines->size()-1)&&(dynamic_cast<LineAssignment*>(path_lines->at(index)->line)!=NULL))
		{
			std::cout << "Last Line of Program is an Assignment (must be an assert!) -> Syntax Error! " << std::endl;
			return;
		}
		
		LineAssignment* assignment_line = dynamic_cast<LineAssignment*>(path_lines->at(index)->line);
		std::vector<z3::expr>* current_abstractions;
		

		if(abstractions->size()!=0) 
		{ //copy last abstraction line
			current_abstractions = new std::vector<z3::expr>(*abstractions->back());			
		}
		else
		{
			current_abstractions = new std::vector<z3::expr>();
		}
	
		abstractions->push_back(current_abstractions);


		
	//conditions are just added with = true or = false
	//assignments substitute all the previous asserts with the assignment
	
	
		//z3::expr ex = le->line->getVerificationConjectureExpression();
		//substitute if assignment
		
		if(assignment_line!=NULL)
		{
			//is assignment -> do substitutions on previous stuff
			assignment_line->substituteStuffInLastLine(current_abstractions, assignment_line);
		}
		else //its not an assignment line but e.g. an whipath_lines->at(index) condition
		{
			current_abstractions->push_back(path_lines->at(index)->line->getVerificationConjectureExpression()==s_->c->bool_val(path_lines->at(index)->eval));
			//solver.add(abstractions->back()); //adds the code line
		}
		
		//std::cout << solver << std::endl;
		solver.pop();
		solver.push();
		for(z3::expr ex : *current_abstractions	)
		{
		solver.add(ex);
		}
		
		//std::cout << solver << std::endl;
		
		if(solver.check()==z3::unsat)
		{
			#ifdef DEBUGCEXANALYSIS
			std::cout << "Found Conflict (Line " << path_lines->at(index)->line->getLine() << ") - find new predicates" << std::endl;
			#endif // ifdef DEBUGCEXANALYSIS
			
			solver.pop();
			solver.push();
			std::cout << solver << std::endl;
			cex_is_spurious = cex_is_spurious || findNewPredicates(&solver, abstractions, path_lines,0); //the moment we find predicates the CEX is spurious
			break;
		}
		else
		{
			//std::cout << "Do nothing (" << path_lines->at(index)->line->getLine() << ")" << std::endl;
		}
		//std::cout << "Line: " << path_lines->at(index)->line->getLine() <<  solver << std::endl;

		
	}
	
	if(cex_is_spurious)
	{
		std::cout << "CEX is spurious!" << std::endl;
	}
	else
	{
		std::cout << "CEX is NOT spurious! -> Assert CAN be violated" << std::endl;
	}
	

}

void LineAssignment::substituteStuffInLastLine(std::vector<z3::expr>* vec_expr, LineAssignment* ass)
{
	z3::expr_vector vars = z3::expr_vector(const_cast<z3::context&>(*s_->c));
	z3::expr_vector vars_replacement = z3::expr_vector(const_cast<z3::context&>(*s_->c));

	Assignment* ass_expr = dynamic_cast<Assignment*>(ass->getExpression());

	if(ass_expr == NULL)
	{
		std::cout << "Dynamic Cast to Assignment* while trying to substitute has failed!" << std::endl;
		return;
	}

	vars.push_back(ass_expr->getVar());
	vars_replacement.push_back(ass_expr->getTermToAssign());

	//std::cout << "subsituting: " << vars.back() << " for " << vars_replacement.back() << " in: " << std::endl;
	
	for(int index = 0; index < vec_expr->size();index++)//z3::expr ex : *vec_expr)
	{
		//std::cout << ex << " to ";
		vec_expr->at(index) = vec_expr->at(index).substitute(vars, vars_replacement);
		//std::cout << ex << std::endl;
	}
}

bool Line::findNewPredicates(z3::solver* solver, std::vector<std::vector<z3::expr>*>* abstractions, std::vector<line_eval*>* path_lines, int depth)
{	
		int line_version = abstractions->size()-1;
		int index = 0;
		std::vector<z3::expr>* current_abstractions = abstractions->at(line_version);
		std::vector<z3::expr> new_predicates;
		solver->pop();
		solver->push();
		for(index = 0; index <= current_abstractions->size(); index++)//z3::expr ex : *current_abstractions	)
		{
			solver->add(current_abstractions->at(index));
			if(solver->check()==z3::unsat)
			{
				//std::cout << "Found the culprid!: " << current_abstractions->at(index) << std::endl;
				
				for(line_version = abstractions->size()-1; line_version >= 0; line_version--)
				{
					current_abstractions = abstractions->at(line_version);
					if(current_abstractions->size()>index)
					{
						if(current_abstractions->at(index).simplify().to_string()!="false") //don't add false as new predicate (usually the last line-version simplifys to false)
						{
							new_predicates.push_back(current_abstractions->at(index).simplify());
						}
						//std::cout << "in the line before: " << current_abstractions->at(index).simplify() << std::endl;
						//std::cout << " (index=" << index << ", line_version=" << line_version << ", size=" << current_abstractions->size() << ")"<< std::endl;
					}
				}
				
				break;
			}
		}
		
		new_predicates = removeDuplicates(new_predicates);
		for(z3::expr ex : new_predicates)
		{
			std::cout << "new predicate: " << ex << std::endl;
		}
		
		return (new_predicates.size()!=0); //if we found new predicates -> return true;
}

std::vector<z3::expr> Line::removeDuplicates(std::vector<z3::expr> vec_expr)
{
	std::vector<z3::expr> keeping_track;
	bool is_duplicate = false;
	for(z3::expr ex : vec_expr)
	{
		is_duplicate = false;
		//look if we already have it
		for(z3::expr ex2 : keeping_track)
		{
			if(ex.to_string()==ex2.to_string())
			{
				is_duplicate = true;
				break;
			}
		}
		if(!is_duplicate)
		{
			keeping_track.push_back(ex);
		}
	}

	return keeping_track;
}
/********************************** Overrides of Print Edges ************************************************/
//note that the next_line is from the program flow! eg. the next_line of a while-line will point to the line AFTER the block!
void LineAssignment::printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, 
																std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat) 
{
	//whatif last line?
	if(this->getNextLine() == nullptr)
	{		
		std::cout << "Last Line? (nr: " << this->getLine() << ")" << std::endl;
	}
	else
	{
		path_lines->back()->edge = gh->addEdgeStuff(this, this->getNextLine(), setting, setting_target);
		//TODO: add line to path
		this->getNextLine()->try_every_target_setting(gh, setting_target, path_lines); //setting_target is the new initial point
	}
}

void LineIf::printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, 
												std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat)  
{
	if(if_block_==NULL)
	{
		std::cout << "if_block ist null!! -> not possible" << std::endl;
		throw;
	}
	
	//whatif last line?
	if(this->getNextLine() == nullptr)
	{		
		std::cout << "Last Line? (nr: " << this->getLine() << ")" << std::endl;
	}
	else
	{
		if(cond_is_sat)
		{
			path_lines->back()->edge = gh->addEdgeStuff(this, if_block_->front(), setting, setting_target);
			if_block_->front()->try_every_target_setting(gh, setting_target, path_lines); //setting_target is the new initial point
		}
		else
		{
			path_lines->back()->edge = gh->addEdgeStuff(this, this->getNextLine(), setting, setting_target);
			this->getNextLine()->try_every_target_setting(gh, setting_target, path_lines); //setting_target is the new initial point
		}
	}
}

void LineIfElse::printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, 
														std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat)  
{
	if(if_block_==NULL||else_block_==NULL)
	{
		std::cout << "if_block or else_block ist null!! -> not possible" << std::endl;
		throw ;
	}
	
	//whatif last line?
	if(this->getNextLine() == nullptr)
	{		
		std::cout << "Last Line? (nr: " << this->getLine() << ")" << std::endl;
	}
	else
	{
		if(cond_is_sat)
		{
		
			path_lines->back()->edge = gh->addEdgeStuff(this, if_block_->front(), setting, setting_target);
			if_block_->front()->try_every_target_setting(gh, setting_target, path_lines); //setting_target is the new initial point
		}
		else
		{
			path_lines->back()->edge = gh->addEdgeStuff(this, else_block_->front(), setting, setting_target);
			else_block_->front()->try_every_target_setting(gh, setting_target, path_lines); //setting_target is the new initial point
		}
	}
}

void LineWhile::printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, 
														std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat) 
{
	if(while_block_==NULL)
	{
		std::cout << "while block ist null!! -> not possible (should be caught by parser)" << std::endl;
		throw;
	}
	
	//whatif last line?
	if(this->getNextLine() == nullptr)
	{		
		std::cout << "Last Line? (nr: " << this->getLine() << ")" << std::endl;
	}
	else
	{
		if(cond_is_sat)
		{
			path_lines->back()->edge = gh->addEdgeStuff(this, while_block_->front(), setting, setting_target);
			while_block_->front()->try_every_target_setting(gh, setting_target, path_lines); //setting_target is the new initial point
		}
		else
		{
			path_lines->back()->edge = gh->addEdgeStuff(this, this->getNextLine(), setting, setting_target);
			this->getNextLine()->try_every_target_setting(gh, setting_target, path_lines); //setting_target is the new initial point
		}
	}
}

void LineAssert::printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting,
														std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat) 
{
	//whatif last line?
	if(this->getNextLine() == nullptr)
	{		
		std::cout << "Last Line? (nr: " << this->getLine() << ")" << std::endl;
	}
	else
	{
		path_lines->back()->edge = gh->addEdgeStuff(this, this->getNextLine(), setting, setting_target);
		this->getNextLine()->try_every_target_setting(gh, setting_target, path_lines); //setting_target is the new initial point
	}
}

void LineSkip::printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, 
													std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat) 
{
	//whatif last line?
	if(this->getNextLine() == nullptr)
	{		
		std::cout << "Last Line? (nr: " << this->getLine() << ")" << std::endl;
	}
	else
	{
		path_lines->back()->edge = gh->addEdgeStuff(this, this->getNextLine(), setting, setting_target);
		this->getNextLine()->try_every_target_setting(gh, setting_target, path_lines); //setting_target is the new initial point
	}
}


