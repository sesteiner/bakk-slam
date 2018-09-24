#ifndef LINE_H
#define LINE_H 1


#include <assert.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include "GraphVizHandler.h"
#include "structs.h"
#include "slam.h"

//install with sudo apt-get install z3 something something
#include <z3++.h>

//install with sudo apt-get install libgraphviz-dev
#include <graphviz/gvc.h>


class GraphvizHandler;
class Expression;

// Not Line specific
bool sortLinesFunction(Line* i, Line* j);
bool sortPredicateSettingFunction(predicate_setting i, predicate_setting j);
bool sortStringFunction(std::string i, std::string j);

//
// Each code line is stored as Line which contains the expression and the abstraction
// This Class is used to e.g. verify if the abstraction is valid given the expression in the c-program
//
class Line 
{
  private:
    z3::check_result try_arbitrary_setting(z3::solver* solver, z3::solver* solver2, std::vector<z3::expr*>* arb_vec, int position);
    z3::check_result try_arbitrary_setting(z3::solver* solver, std::vector<z3::expr*>* arb_vec, int position, z3::check_result desired_result );


  protected:
    smt_params* s_;
    int line_;
    Expression* expression_;
    Expression* abstraction_;    
    bool abstraction_valid_;    
    Line* previous_line_;
		Line* next_line_;
		
    // Setter for Expression
    void setExpression(Expression* expr) { expression_ = expr; }

    // Setter forAbstraction
    virtual void setAbstraction(Expression* abs) // virtual because assignment does it differently
    {
      abstraction_ = abs;
    }

  public:

    // Constructor
    Line(smt_params* s, int line, Expression* expr, Expression* abs) : s_(s), line_(line), expression_(expr), abstraction_(abs), 
    																																	 abstraction_valid_(false), previous_line_(nullptr), next_line_(nullptr)
    {
    }

    int getLine() { return line_; }
    Expression* getExpression() { return expression_; }
    virtual Expression* getAbstraction() { return abstraction_; } 
    Line* getPreviousLine() { return previous_line_; }
    Line* getNextLine() { return next_line_; }
    
    void setPreviousLine(Line* previous_line) { previous_line_ = previous_line; }
    virtual void setNextLine(Line* next_line) = 0;     
		virtual z3::expr getVerificationConjectureExpression();
		virtual z3::expr getVerificationConjectureAbstraction();
		
		//
		//	Function adds all predicates to the solver and returns a vector with the variables necessary to 
		//	iterate over to get the full truth-table (with single-state-assignment we need e.g. p, p1)
		// 	DON'T forget to delete the vector aferwards!!
		//
		std::vector<z3::expr*>* add_predicates(z3::solver* solver, bool single_state_assingment);

		bool verifyAbstraction();
		void AnalyseCounterExample(std::vector<line_eval*>* path_lines);
		bool findNewPredicates(z3::solver* solver, std::vector<std::vector<z3::expr>*>* abstractions, std::vector<line_eval*>* path_lines, int depth = 0);
		std::vector<z3::expr> removeDuplicates(std::vector<z3::expr> vec_expr);
			
    // Destructor
    virtual ~Line()
    {
			/*
			if(expression_!=NULL)
					delete expression_;
					
			if(abstraction_!=NULL)
					delete abstraction_;
					*/
    }
    
    void try_every_start_setting(GraphvizHandler* gh, std::vector<predicate_setting>* setting, int position);
    //this implementation should only be made through the virtual Classes ConditionalLine and NormalLine
    virtual void try_every_target_setting(GraphvizHandler* gh, std::vector<predicate_setting> setting, 
    																			std::vector<line_eval*>* path_lines) = 0; 
    
    // Pure virtual method that creates the Model
    // expression
    virtual std::vector<Agnode_t*>  printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes) = 0;
    virtual void printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<predicate_setting> setting_target, 
    												std::vector<line_eval*>* path_lines, bool cond_is_sat = true)  = 0;

};

//
// Classes purely to overload try_every_target_setting for conditional Lines (like if,if-else, while) vs. "normal" lines (assignment, assert)
//
class ConditionalLine : public virtual Line
{
	public:
  // Constructor
  ConditionalLine(smt_params* s, int line, Expression* expr, Expression* abs) : Line(s, line, expr, abs)
  {
  }
	
	void try_every_target_setting(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<line_eval*>* path_lines);
};

class NormalLine : public virtual Line
{
	public:
  // Constructor
  NormalLine(smt_params* s, int line, Expression* expr, Expression* abs) : Line(s, line, expr, abs)
  {
  }
  
	void try_every_target_setting(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<line_eval*>* path_lines);
};

class SkipLine : public virtual Line
{
	public:
  // Constructor
  SkipLine(smt_params* s, int line, Expression* expr, Expression* abs) : Line(s, line, expr, abs)
  {
  }
  
	void try_every_target_setting(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<line_eval*>* path_lines);
};

class EndLine : public virtual Line
{
	public:
  // Constructor
  EndLine(smt_params* s, int line, Expression* expr, Expression* abs) : Line(s, line, expr, abs)
  {
  }
  
	void try_every_target_setting(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<line_eval*>* path_lines);
};


//
// Class for Lines that are Assingments
//
class LineAssignment : public virtual Line, public virtual NormalLine
{
	std::vector<Expression*>* abstraction_vec_;
	protected:
		
    // Setter forAbstraction
    virtual void setAbstraction(Expression* abs) // virtual because assignment does it differently
    {
    std::cout << " This should not be used! -> abstraction setting is only done in constructor" << std::endl;
      abstraction_ = abs;
    }
    
  public:
    LineAssignment(smt_params* s, int line, Expression* expr, std::vector<Expression*>* abs) : Line(s, line, expr, nullptr), 
    																																NormalLine(s, line, expr, nullptr), abstraction_vec_(abs)
    {
		  #ifdef DEBUGBOOLMC
  		std::cout << "creating Line asssignment with ["<< abstraction_vec_->size()  <<"] abstractions" << std::endl;
			#endif // ifdef DEBUGBOOLMC

    }
    
    Expression* getAbstraction() 
    { 
		  return abstraction_vec_->back(); 
    } 
    
    virtual ~LineAssignment()
    {
	    delete abstraction_vec_;
    }
    
    void substituteStuffInLastLine(std::vector<z3::expr>* vec_expr, LineAssignment* ass);
    
   void setNextLine(Line* next_line)   
    { 
   	  #ifdef DEBUGBOOLMC
   		std::cout << "NextLine from Line " << this->getLine() << " to " << next_line->getLine() << std::endl;
   		#endif // ifdef DEBUGBOOLMC
   	 	next_line_ = next_line; 
    }
    
    std::vector<Agnode_t*> printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes);  
    void printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat = true); 
    
    z3::expr getVerificationConjectureExpression();
		z3::expr getVerificationConjectureAbstraction(); 
};

//
//	Class for Lines that are Ifs
//
class LineIf : public virtual Line, public virtual ConditionalLine
{
  protected:
    std::vector<Line*>* if_block_;
  public:
    void setIfBlock(std::vector<Line*>* if_block);
		std::vector<Line*>* getIfBlock() { return if_block_; }
    
    LineIf(smt_params* s, int line, Expression* expr, Expression* abs, std::vector<Line*>* if_block) : Line(s, line, expr, abs), ConditionalLine(s, line, expr, abs), if_block_(if_block)
    {
    	std::sort (if_block->begin(),if_block->end(),sortLinesFunction);
    }
    
    void setNextLine(Line* next_line) 
  	{
	    #ifdef DEBUGBOOLMC
   		std::cout << "NextLine from IF Line " << this->getLine() << " to " << next_line->getLine() << std::endl;
   		#endif // ifdef DEBUGBOOLMC
    	if_block_->back()->setNextLine(next_line);
    	next_line_ = next_line; 
  	}
    
    virtual ~LineIf()
    {
		  if(if_block_!=NULL)
		  {
		  	for(Line* line : *if_block_)
		  	{
		  		delete line; 
		  	}    
		  	delete if_block_;		
		  }
    }
    
    std::vector<Agnode_t*> printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes);
    void printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat = true);
};

//
//	Class for Lines that are If-Elses
//
class LineIfElse : public virtual Line, public virtual ConditionalLine
{
  protected:
    std::vector<Line*>* if_block_;
		std::vector<Line*>* else_block_;
  public:
    void setIfBlock(std::vector<Line*>* if_block);
    void setElseBlock(std::vector<Line*>* else_block);
    std::vector<Line*>* getIfBlock() { return if_block_; }
    std::vector<Line*>* getElseBlock() { return else_block_; }
    
    LineIfElse(smt_params* s, int line, Expression* expr, Expression* abs,  std::vector<Line*>* if_block,  std::vector<Line*>* else_block) 
    																															: Line(s, line, expr, abs), ConditionalLine(s, line, expr, abs), if_block_(if_block),  else_block_(else_block)
    {
		  std::sort (if_block->begin(),if_block->end(),sortLinesFunction);
		  std::sort (else_block_->begin(),else_block_->end(),sortLinesFunction);
    }
    
    void setNextLine(Line* next_line) 
  	{
    	#ifdef DEBUGBOOLMC
   		std::cout << "NextLine from IF-Else Line " << this->getLine() << " to " << next_line->getLine() << std::endl;
   		std::cout << "  if-block " << if_block_->back()->getLine() << " to " << next_line->getLine() << std::endl;
   		std::cout << "  else-block " << else_block_->back()->getLine() << " to " << next_line->getLine() << std::endl;
   		#endif // ifdef DEBUGBOOLMC
   		
    	if_block_->back()->setNextLine(next_line);
    	else_block_->back()->setNextLine(next_line);
    	next_line_ = next_line; 
  	}
  	
  	
    virtual ~LineIfElse()
    {
		  if(if_block_!=NULL)
		  {
		  	for(auto line : *if_block_)
		  	{
		  		delete line; 
		  	}    
		  	delete if_block_;		
		  }
		  		  
		 	if(else_block_!=NULL)
		  {
		  	for(auto line : *else_block_)
		  	{
		  		delete line; 
		  	}    
		  	delete else_block_;		
		  }
    }
    
    std::vector<Agnode_t*> printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes);
    void printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat = true);
};

//
//	Class for Lines that are Whiles
//
class LineWhile : public virtual Line, public virtual ConditionalLine
{
  protected:
		std::vector<Line*>* while_block_;
  public:
    void setWhileBlock(std::vector<Line*>* while_block);
    std::vector<Line*>* getWhileBlock()
    {
			return while_block_;
    }
    
    LineWhile(smt_params* s, int line, Expression* expr, Expression* abs,  std::vector<Line*>* while_block) : Line(s, line, expr, abs), ConditionalLine(s, line, expr, abs), while_block_(while_block)
    {
  		std::sort (while_block_->begin(),while_block_->end(),sortLinesFunction);
    }
    
    void setNextLine(Line* next_line) 
  	{
    	#ifdef DEBUGBOOLMC
   		std::cout << "NextLine from While Line " << this->getLine() << " to " << next_line->getLine() << std::endl;
   		std::cout << "  while-block " << while_block_->back()->getLine() << " to " << this->getLine() << std::endl;
   		#endif // ifdef DEBUGBOOLMC
   		
    	while_block_->back()->setNextLine(this);
    	next_line_ = next_line; 
  	}
  	
    virtual ~LineWhile()
    {
		  if(while_block_!=NULL)
		  {
		  	for(auto line : *while_block_)
		  	{
		  		delete line; 
		  	}    
		  	delete while_block_;		
		  }
    }
    
    std::vector<Agnode_t*> printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes);
    void printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat = true);
};

//
//	Class for Lines that are Assert
//
class LineAssert : public virtual Line, public virtual EndLine
{
  public:
    LineAssert(smt_params* s, int line, Expression* expr, Expression* abs) : Line(s, line, expr, abs), EndLine(s, line, expr, abs)
    {
    }
    
    virtual ~LineAssert()
    {
    }
        
	  void setNextLine(Line* next_line)   
    { 
    	#ifdef DEBUGBOOLMC
   		std::cout << "NextLine from Line " << this->getLine() << " to " << next_line->getLine() << std::endl;
   		#endif // ifdef DEBUGBOOLMC
   		next_line_ = next_line; 
    }
            
    std::vector<Agnode_t*> printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes);
    void printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat = true);
};	

//
//	Class for Lines that are Skips
//
class LineSkip : public virtual Line, public virtual NormalLine
{
  public:
    LineSkip(smt_params* s, int line, Expression* expr, Expression* abs) : Line(s, line, expr, abs), NormalLine(s, line, expr, abs)
    {
    }
    
    std::vector<Agnode_t*> printLine(GraphvizHandler* gh, std::vector<Agnode_t*> previous_nodes);
    void printEdges(GraphvizHandler* gh, std::vector<predicate_setting> setting, std::vector<predicate_setting> setting_target, std::vector<line_eval*>* path_lines, bool cond_is_sat = true);
};


#endif // ifndef LINE_H


