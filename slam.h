#ifndef SLAM_H
#define SLAM_H 1


#include <assert.h>
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

#include "structs.h"
#include "GraphVizHandler.h"
#include "line.h"


//install with sudo apt-get install z3 something something
#include <z3++.h>

//install with sudo apt-get install libgraphviz-dev
#include <graphviz/gvc.h>

class GraphvizHandler;

/*
 * Types of check -> used for printing the check-type information for each line.
 */
enum class CHECK_TYPE
{
  LOOP_PRECONDITION,  // The outer predcondition of a loop construct
  LOOP_POSTCONDITION, // The outer postcondition of a loop construct

  LOOP_PRE_BODY,  // The inner loop body precondition
  LOOP_POST_BODY, // The inner loop body postcondition

  IF_PRE_THEN,  // The inner if body precondition
  IF_POST_THEN, // The inner if body postcondition

  IF_PRE_ELSE,  // The inner else body precondition
  IF_POST_ELSE, // The inner else body postcondition
  STRENGTH,     // A strengethening step check
  ASSIGNMENT,    // A assignment check
  IF_PRE_POST
};



/**
 * Abstract base class for any of the supported C statements including empty lines for strengthening.
 * We consider Assignments and (IF, IF-Else, While)-Conditions Expressions
 **/
class Expression
{
  
  protected:
    int line_;
    // The Expression
    z3::expr expression_;
    

    // The z3::context (used for creating z3::expr and solving)
    z3::context*        c_;

    // Setter for Expression
    void setExpression(z3::expr expr)
    {
      expression_ = expr;
    }

  public:
  bool needsSingleStateAssingment_;
    // Constructor
    Expression(z3::context* c, int line, bool singlestate = false) : c_(c), line_(line), expression_(c->bool_val(true)), needsSingleStateAssingment_(singlestate)
    {
	//	std::cout << "new Expression created" << std::endl;
    }


    z3::expr getExpression()
    {
			generateAndSetExpression();
			return expression_;
    }

    // Destructor
    virtual ~Expression()
    {
    }

    // Pure virtual method for creating the z3::expr from its parts
    // e.g. for assignment -> has variable and terms -> make an expression out of it
    // and set the struct slam_assert expression_
    virtual void generateAndSetExpression() = 0;


    // Pure virtual method that generates a z3::expr that represents the 
    // expression
    virtual z3::expr getVerificationConjecture() = 0;
    
    // Returns a readable string of the Expression: eg. "if(p == true)"
    virtual std::string getReadableRepresenation() = 0;

    // Prints the line information of every Hoare step
    // @param valid - If the step is valid
    // @param line_no1 - line number of the first line used in the check
    // @param line_no2 - line number of the second line used in the check
    // @param CHECK_TYPE -  type of the check done in this step
    void print(bool valid, int line_no1, int line_no2, CHECK_TYPE type);

    // Checks, if two conditions (a and b) are equivalent
    // Uses solver s for this
    // Returns true, if both are equivalent
    bool isEqual(z3::solver* s, z3::expr a, z3::expr b);

};


/*
Class for Assignemnts
*/
class Assignment : public Expression
{
  private:
    // The varibale, to which term_to_assign_ is assigned.
    z3::expr var_;	
    // The term to assign to var_
    z3::expr term_to_assign_;

  public:
    Assignment(z3::context* c, int line, z3::expr variable, z3::expr term) : Expression(c, line, true), var_(variable), term_to_assign_(
        term)
    {
    }
		
		z3::expr getVar() {return var_;};
		z3::expr getTermToAssign() { return term_to_assign_;};
    z3::expr getVerificationConjecture();
    std::string getReadableRepresenation();
    void generateAndSetExpression();
};

/*
Class for Asserts
*/
class Assert : public Expression
{
  private:
    // assert statement
    z3::expr assert_;

  public:
    Assert(z3::context* c, int line, z3::expr assert) : Expression(c, line), assert_(assert)
    {
    }

    z3::expr getVerificationConjecture();
    std::string getReadableRepresenation();
    void generateAndSetExpression();
};


/**
Class for a While-Expression
*/
class WhileExpression : public Expression
{
  protected:
    // The condition of the while
    z3::expr condition_;
  public:
    WhileExpression(z3::context* c, int line, z3::expr condition) : Expression(c, line), condition_(condition)
    {
    }

    z3::expr getVerificationConjecture();
    std::string getReadableRepresenation();
    void generateAndSetExpression();
};

/**
Class for a IF-Expression
*/
class IfExpression : public Expression
{
  protected:
    // The condition of the if
    z3::expr condition_;
  public:
    IfExpression(z3::context* c, int line, z3::expr condition) : Expression(c, line), condition_(condition)
    {
    }

    z3::expr getVerificationConjecture();
    std::string getReadableRepresenation();
    void generateAndSetExpression();
};

/**
Class for If-Else-Expressions. Inherits from If-Expression
*/
class IfElseExpression : public IfExpression
{
  public:
    IfElseExpression(z3::context* c, int line, z3::expr condition) : IfExpression(c, line, condition)
    {
    }

    z3::expr getVerificationConjecture();
    std::string getReadableRepresenation();
    void generateAndSetExpression();
};

/**
Class for Skip-Expressions.
*/
class SkipExpression : public Expression
{
  public:
    SkipExpression(z3::context* c, int line) : Expression(c, line)
    {
    }

    z3::expr getVerificationConjecture();
    std::string getReadableRepresenation();
    void generateAndSetExpression();
};

void printLine(Line* line);
bool sortLinesFunction(Line* i, Line* j);
#endif // ifndef SLAM_H


