% include {
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <z3++.h>
#include <graphviz/gvc.h>
#include "slam.h"
#include "line.h"
#include "structs.h"
#include "GraphVizHandler.h"

bool proof_valid = true;
}



//this defines the precidence / hirachy of the grammar from least to most important
% left CONDITIONAL.
% left OR.
% left AND.
% left NOT.
% left GREATER.
% left GREATER_EQUAL.
% left EQUALS.
% left SMALLER_EQUAL.
% left SMALLER.
% left MINUS.
% left PLUS.
% left MUL.
% left DIV.
% left MOD.


//adds a 4th argument to the parse function - which all action routines have access to
//smt_params is defined in slam.h
% extra_argument {  struct smt_params s }

//what tokens look like - since we have numbers and words we use string
% token_type { std::string*}

//here we define the token-types we are going to need
//each type that is not in the list below ist generated with the default_type
% default_type{  z3::expr* }

% type condition{  z3::expr* }
% type abs_condition{  z3::expr* }

% type line_no { unsigned int}

% type assert{ Expression* }

% type predicate_declaration {   z3::expr* } //these need to be stored in the globally available smt_params

% type abstraction { Expression* }

% type abstraction_assignment{ std::vector<Expression*>* } //necessary for multiple abstractions in assignments

% type assignment{ Expression* }

% type abs_assignment{ std::vector<Expression*>* }

% type while{ Expression* }

% type if{ Expression*}

% type expr{  Line* }

% type block{  std::vector<Line*>* }

//% type stmt{  std::pair<struct hoare_assert*, struct hoare_assert*>* }


%default_destructor
{
  delete $$;
}

%token_destructor {
  delete $$;
}

% destructor predicate_declaration {
	delete $$;
}

//we need the block NOT to delete the vector* with lines!!
% destructor block {
}

//we need the expr NOT to delete the Line*
% destructor expr {
}



% syntax_error{  std::cout << "Cannot parse line " << std::setw(3) << s.line_no << " -> " << "PLEASE CHECK THE SYNTAX" <<
  std::endl << std::endl;
  proof_valid = false;
  return; }

/*
*******************THE PROGRAM BLOCK*********************
*/
program::= next_step.
{
}

next_step::= function next_step.
{

}

next_step::= .
{
}

/*
*******************THE FUNCTION BLOCK*********************
*/
function::= VOID VAR LPAREN parameters RPAREN predicate_declaration LBR block(b) RBR.
{
std::cout << "====================== NEXT STEP =============================" << std::endl;
	std::sort (b->begin(),b->end(),sortLinesFunction);
	std::sort (s.predicates->begin(),s.predicates->end(),sortStringFunction);


	#ifdef DEBUGLEMON
	std::cout << "Parsing Function(" << std::setw(3) << s.line_no << ")" << std::endl;
	#endif // ifdef DEBUGLEMON

	#ifdef NO_PROOF
	std::cout << "Finished with Parsing" << std::endl;
	#else
	//std::cout << "Proof is " << ( proof_valid ? "valid" : "invalid" ) << std::endl;
	#endif // ifndef NO_PROOF

	if(*s.all_abstractions_valid)
		std::cout << "ALL ABSTRACTIONS ARE VALID!" << std::endl;
	else
		std::cout << "NOT ALL ABSTRACTIONS ARE VALID!" << std::endl;

	
	
	
	// GRAPHVIZ
	/*
	std::vector<std::string>* predicate_order = new std::vector<std::string>();
	predicate_order->push_back("r");
	predicate_order->push_back("q");
	predicate_order->push_back("p");
	GraphvizHandler gh = GraphvizHandler(&s, predicate_order);
	*/
	GraphvizHandler gh = GraphvizHandler(&s);
	gh.createImage(b); // also starts the Boolean MC
	
	if(s.bool_mc_has_endless_loop)
		std::cout << "This Abstraction leads to an endless loop and never terminates!" << std::endl;
		
		
	
	
	for(Line* line : *b)
	{		
		delete line;
	}
		
	delete b;
}

/*	END FUNCTION BLOCK	*/

parameters::= INT VAR(v) LSQBR RSQBR.
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << s.line_no << " Parsing Parameter: " << *v << std::endl;
	#endif // ifdef DEBUGLEMON

	z3::sort so  = s.c->array_sort(s.c->int_sort(), s.c->int_sort());
	z3::expr exp = s.c->constant(( *v ).c_str(), so);
	s.exprs->insert(std::make_pair(*v, exp));
	delete v;
}

parameters::= INT VAR(v) LSQBR RSQBR COMMA parameters.
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << s.line_no << " Parsing Parameter: " << *v << std::endl;
	#endif // ifdef DEBUGLEMON

	z3::sort so  = s.c->array_sort(s.c->int_sort(), s.c->int_sort());
	z3::expr exp = s.c->constant(( *v ).c_str(), so);
	s.exprs->insert(std::make_pair(*v, exp));
	delete v;
}


parameters::= INT VAR(v) COMMA parameters.
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << s.line_no << " Parsing Parameter: " << *v << std::endl;
	#endif // ifdef DEBUGLEMON

  z3::expr exp = s.c->int_const(( *v ).c_str());
  s.exprs->insert(std::make_pair(*v, exp));
  delete v;
}

parameters::= INT VAR(v).
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << s.line_no << " Parsing Parameter: " << *v << std::endl;
	#endif // ifdef DEBUGLEMON

  z3::expr exp = s.c->int_const(( *v ).c_str());
  s.exprs->insert(std::make_pair(*v, exp));
  delete v;
}

parameters::= .

predicate_declaration::= line_no(no) ABSTRACTION_START PREDICATE_TOKEN VAR(v) COLON condition(cond) ABSTRACTION_END predicate_declaration. // e.g. /*@ predicate p : (a<2) */
{ 
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no << " Parsing Predicate: " << *v << " = " << *cond << std::endl;
	#endif // ifdef DEBUGLEMON

	z3::expr predicate = s.c->bool_const(( *v ).c_str()); //get BOOLEAN sort for predicate symbol
	s.abs_exprs->insert(std::make_pair(*v, predicate)); //add it to the symbol map

	s.predicates->push_back(*v);
	s.predicates_expr->push_back(*cond);
	delete cond;
	delete v;

}

predicate_declaration::= .


//block is undefined unit with contain basically anything
block(b) ::= expr(e) block(b2).
{
	if(b2==NULL)
	{ b2 = new std::vector<Line*>(); }
	else
	{ 
		e->setNextLine(b2->back()); 
		b2->back()->setPreviousLine(e);
	}
	
	b2->push_back(e);
	


	if(e->verifyAbstraction()==false)
		(*s.all_abstractions_valid) = false;

	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << s.line_no << " Parsing Block finished" << std::endl;
	#endif // ifdef DEBUGLEMON

	b = b2;
}

block(b) ::= .
{
	b = NULL;
	//&	std::cout << "Parsing empty Block" << std::endl;
}

line_no(no) ::= .
{
  no = s.line_no;
}

////// HERE we have the expressions that may have a abstraction
//assert assignment expr


/* 
####### Section Abstraction ##########
*/

/*@ abstraction assignment */
abstraction_assignment(abs) ::= line_no(no) ABSTRACTION_START ABSTRACTION_TOKEN abs_assignment(a) ABSTRACTION_END.
{
	//check if abs_assingment is not empty!!
	if(a->size()==0)
		{
		std::cout << "Assingment in line "<< no << "has no abstraction!" << std::endl;
		throw;
		}
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no<< " Parsing Assignment-Abstraction " << std::endl;
	#endif // ifdef DEBUGLEMON
  abs = a;
}

/*@ abstraction if */
abstraction(abs) ::= line_no(no) ABSTRACTION_START ABSTRACTION_TOKEN IF abs_condition(cond) ABSTRACTION_END.
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no << " Parsing IF-Abstraction " << std::endl;
	#endif // ifdef DEBUGLEMON
  abs = new IfExpression(s.c, no, *cond);
  delete cond;
}

/*@ abstraction while */
abstraction(abs) ::= line_no(no) ABSTRACTION_START ABSTRACTION_TOKEN WHILE abs_condition(cond) ABSTRACTION_END.
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no<< " Parsing WHILE-Abstraction " << std::endl;
	#endif // ifdef DEBUGLEMON
  abs = new WhileExpression(s.c, no, *cond);
  delete cond;
}

/*@ abstraction assert(p) */
abstraction(abs) ::= line_no(no) ABSTRACTION_START ABSTRACTION_TOKEN ASSERT abs_condition(cond) ABSTRACTION_END.
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no<< " Parsing ASSERT-Abstraction " << std::endl;
	#endif // ifdef DEBUGLEMON
  abs = new Assert(s.c, no, *cond);
  delete cond;
}

/*@ abstraction skip */
abstraction(abs) ::= line_no(no) ABSTRACTION_START ABSTRACTION_TOKEN SKIP ABSTRACTION_END.
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no<< " Parsing SKIP-Abstraction " << std::endl;
	#endif // ifdef DEBUGLEMON
  abs = new SkipExpression(s.c, no);
}

/* This is not allowed -> each line MUST have an abstraction, if its empty then 
   use skip */
// use this to check if the students left out a abstraction!!
abstraction ::= line_no(no).
{
	std::cout << "You forgot to create an abstraction in Line (" << std::setw(3) << no<< ")" << " - If it's supposed to be empty use SKIP!" << std::endl;
	throw;
}

/* 
####### Section Expression ##########
*/

/*@ expression assingment */
expr(e) ::= line_no(no) assignment(a) abstraction_assignment(abs).
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no<< " Parsing Expression ASSIGNMENT" << std::endl;
	#endif // ifdef DEBUGLEMON
	e = new LineAssignment(&s, no, a, abs);
}

/*@ expression assingment with skip abstraction */
expr(e) ::= line_no(no) assignment(a) abstraction(abs).
{
	std::vector<Expression*>* vec_abs = new std::vector<Expression*>();
	vec_abs->push_back(abs);
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no<< " Parsing Expression ASSIGNMENT" << std::endl;
	#endif // ifdef DEBUGLEMON
	e = new LineAssignment(&s, no, a, vec_abs);
}


/*@ expression assert */
expr(e) ::= line_no(no) ASSERT condition(cond) SEMI abstraction(abs).
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no<< " Parsing Expression ASSERT" << std::endl;
	#endif // ifdef DEBUGLEMON
	Expression* ass = new Assert(s.c, no, *cond);
	e = new LineAssert(&s, no, ass, abs);
	delete cond;
}

/*@ expression if */
expr(e) ::= IF line_no(no) condition(cond) abstraction(abs) LBR block(b) RBR.
{
	std::cout << std::setw(3) << no<< " Parsing Expression IF" << std::endl;
	if (b == nullptr)	{
		std::cout << "Cannot parse line " << std::setw(3) << no<< " because if-block is empty " << std::endl;
		return;
	}
	Expression* expr = new IfExpression(s.c, no, *cond);

	e = new LineIf(&s, no, expr, abs, b);
	
	//e->setIfBlock(b);
	delete cond;
}

/*@ expression while */
expr(e) ::= WHILE line_no(no) condition(cond) abstraction(abs) LBR block(b) RBR.
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no<< " Parsing Expression WHILE" << std::endl;
	#endif // ifdef DEBUGLEMON

  if (b == nullptr)  {
    std::cout << "Cannot parse line " << std::setw(3) << no<< " because while-block is empty " << std::endl;
    return;
  }

	WhileExpression* expr = new WhileExpression(s.c, no, *cond);
	e = new LineWhile(&s, no, expr, abs,b);
	//e->setWhileBlock(b);
	delete cond;
}


expr(e) ::= IF line_no(no) condition(cond) abstraction(abs) LBR block(b) RBR ELSE LBR block(b2) RBR.
{
#ifdef DEBUGLEMON
std::cout << std::setw(3) << no<< " Parsing Expression IF-ELSE" << std::endl;
#endif // ifdef DEBUGLEMON

	if (b == nullptr)
	{
	std::cout << "Cannot parse line " << std::setw(3) << no<< " because if-block is empty " << std::endl;
	return;
	}
	

IfExpression* expr =  new IfExpression(s.c, no, *cond);
e = new LineIfElse(&s, no, expr, abs, b, b2);

//e->setIfBlock(b);
//e->setElseBlock(b2);
delete cond;
}


/* 
####### Section CONDITION ##########
*/
condition(i) ::=
  LPAREN FORALL integer(v) IN LPAREN term(r1) COMMA term(r2) RPAREN COLON
  condition(con) RPAREN.
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << s.line_no << " Parsing CONDITION" << std::endl;
	#endif // ifdef DEBUGLEMON

  z3::expr exp   = *v;
  z3::expr range = ( exp >= *r1 && exp < *r2);
  z3::expr cond  = z3::forall(exp, implies(range, *con)) ;
  i = new z3::expr(cond);
  delete v;
  delete r1;
  delete r2;
  delete con;
}



condition(i) ::= condition(c) CONDITIONAL condition(c1) COLON condition(c2).
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << s.line_no << " Parsing CONDITION" << std::endl;
	#endif // ifdef DEBUGLEMON

	//TODO: Work out how this should look!!!
	i = c1;
	delete c2;
	delete c;
}

condition(c) ::= LPAREN condition(i) RPAREN.
{
  *i = ( *i );
  c  = i;
}



condition(i) ::= condition(c1) AND condition(c2).
{
  *c1 = *c1 && *c2;
  delete c2;
  i = c1;
}
condition(i) ::= condition(c1) OR condition(c2).
{
  *c1 = *c1 || *c2;
  delete c2;
  i = c1;
}

condition(i) ::= NOT condition(c).
{
  *c = !*c;
  i  = c;
}


condition(i) ::= term(t1) GREATER term(t2).
{
  try
  {
    if(!t1->is_bool()&&!t2->is_bool())
    {
      *t1 = *t1 > *t2;
      i   = t1;
      delete t2;
    }
    else
    {
      
      throw std::invalid_argument("Comparing booleans with >,>=,<,<= is not allowed! (line " + std::to_string(s.line_no) + ")" );
    }
  }
  catch (const std::invalid_argument& e)
  {
    std::cout << e.what() << std::endl; 
    throw;
  }
}

condition(i) ::= term(t1) GREATER_EQUAL term(t2).
{
  try
  {
    if(!t1->is_bool()&&!t2->is_bool())
    {
      *t1 = *t1 >= *t2;
      i   = t1;
      delete t2;
    }
    else
      throw std::invalid_argument("Comparing booleans with >,>=,<,<= is not allowed! (line " + std::to_string(s.line_no) + ")" );
  }
  catch (const std::invalid_argument& e)
  {
    std::cout << e.what() << std::endl; 
    throw;
  }
}

condition(i) ::= term(t1) EQUALS term(t2).
{
 std::cout << "equality" << std::endl; 
  try
  {
    if(!t1->is_bool()&&!t2->is_bool())
    {
      *t1 = *t1 == *t2;
      i   = t1;
      delete t2;
    }
    else
      throw std::invalid_argument("Comparing booleans and intergers is not allowed! (line " + std::to_string(s.line_no) + ")" );
  }
  catch (const std::invalid_argument& e)
  {
    std::cout << e.what() << std::endl; 
    throw;
  }
}

condition(i) ::= condition(t1) EQUALS condition(t2).
{
      *t1 = *t1 == *t2;
      i   = t1;
      delete t2;
}

condition(i) ::= term(t1) EQUALS condition(t2).
{ 
  try
  {	
    if(!(t1->is_bool()))
    {
      throw std::invalid_argument("Comparing booleans and intergers is not allowed! (line " + std::to_string(s.line_no) + ")" );
    }

    z3::solver s_temp(*s.c);
    z3::expr conjecture1 = implies(*t1, *t2);
    z3::expr conjecture2 = implies(*t2, *t1);

    s_temp.add(conjecture1);
    s_temp.add(conjecture2);

    switch (s_temp.check()) {
	    case z3::unsat:   
					#ifdef DEBUGLEMON
					std::cout << "\"" << *t1 << " equals " << *t2 << "\"" << " is not satisfiable" << std::endl;
					#endif // ifdef DEBUGLEMON
					i = new z3::expr(s.c->bool_val(false)); 
					break;
	    case z3::sat:   
					#ifdef DEBUGLEMON
					std::cout << "\"" << *t1 << " equals " << *t2 << "\"" << " is satisfiable" << std::endl; 
					#endif // ifdef DEBUGLEMON  
					i = new z3::expr(s.c->bool_val(true)); 
					break;
	    case z3::unknown: 
					#ifdef DEBUGLEMON
					std::cout << "\"" << *t1 << " equals " << *t2 << "\"" << " is unknown" << std::endl; 
					#endif // ifdef DEBUGLEMON  
					std::string name = "arbitrary"+std::to_string(*s.arbitraries_counter);
					*s.arbitraries_counter = *s.arbitraries_counter+1;
					z3::expr arbitrary = s.c->bool_const(name.c_str()); //get BOOLEAN sort for predicate symbol
					s.abs_exprs->insert(std::make_pair(name, arbitrary)); //add it to the symbol map
					i = &arbitrary;
					s.arbitraries->insert(std::make_pair(name.c_str(), arbitrary));
					break;
    }
  }
  catch (const std::invalid_argument& e)
  {
		std::cout << "invalid argument problem" << std::endl;
		std::cout << e.what() << std::endl; 
		throw;
  }
  delete t1;
  delete t2;
}

condition(i) ::= term(t1) SMALLER term(t2).
{
  try
  {
    if(!t1->is_bool()&&!t2->is_bool())
    {
      *t1 = *t1 < *t2;
      i   = t1;
      delete t2;
    }
    else
      throw std::invalid_argument("Comparing booleans with >,>=,<,<= is not allowed! (line " + std::to_string(s.line_no) + ")" );
  }
  catch (const std::invalid_argument& e)
  {
    std::cout << e.what() << std::endl; 
    throw;
  }
}

condition(i) ::= term(t1) SMALLER_EQUAL term(t2).
{
  try
  {
    if(!t1->is_bool()&&!t2->is_bool())
    {
      *t1 = *t1 <= *t2;
      i = t1;
      delete t2;
    }
    else
      throw std::invalid_argument("Comparing booleans with >,>=,<,<= is not allowed! (line " + std::to_string(s.line_no) + ")" );
  }
  catch (const std::invalid_argument& e)
  {
    std::cout << e.what() << std::endl; 
    throw;
  }
}


condition(i) ::= TRUE.
{
  i = new z3::expr(s.c->bool_val(true));
}
condition(i) ::= FALSE.
{
  i = new z3::expr(s.c->bool_val(false));
}
/*
//try to make the usage of only p eqvialent to p==true
condition(i) ::= term(t1).
{ 
  try
  {	
    if(!(t1->is_bool()))
    {
      throw std::invalid_argument("usage of variable by it self (meaning to imply variable==true) but variable is not a bool!!! (line " + std::to_string(s.line_no) + ")" );
    }

		*t1 = *t1 == z3::expr(s.c->bool_val(true));
		i = t1;
  }
  catch (const std::invalid_argument& e)
  {
		std::cout << "invalid argument problem" << std::endl;
		std::cout << e.what() << std::endl; 
		throw;
  }
}
*/
/* 
####### Section CONDITION Abstraction ##########
*/

abs_condition(i) ::= abs_condition(cond) CONDITIONAL abs_condition(c1) COLON abs_condition(c2).
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << s.line_no << " Parsing CONDITION - Ternary" << std::endl;
	#endif // ifdef DEBUGLEMON	
	z3::expr* ternary = new z3::expr(ite(*cond,*c1,*c2));
	i = ternary;
	delete cond;
	delete c1;
	delete c2;
}

abs_condition(c) ::= LPAREN abs_condition(c1) RPAREN.
{
  c  = c1;
}

abs_condition(c) ::= abs_condition(c1) AND abs_condition(c2).
{
  c  = new z3::expr(*c1 && *c2);
}
abs_condition(c) ::= abs_condition(c1) OR abs_condition(c2).
{
  c  = new z3::expr(*c1 || *c2);
}

abs_condition(i) ::= NOT abs_condition(c).
{
  *c = !*c;
  i  = c;
}

abs_condition(i) ::= abs_condition(t1) EQUALS abs_condition(t2).
{
  try
  {	
    if(!(t1->is_bool())) {
      throw std::invalid_argument("Comparing booleans and intergers is not allowed! (line " + std::to_string(s.line_no) + ")" );
    }
		z3::expr* equ = new z3::expr(*t1 == *t2);
		i = equ;

  }
  catch (const std::invalid_argument& e)
  {
    std::cout << "invalid argument problem" << std::endl;
    std::cout << e.what() << std::endl; 
    throw;
  }
  delete t1;
  delete t2;
}


abs_condition(i) ::= TRUE.
{
  i = new z3::expr(s.c->bool_val(true));
}
abs_condition(i) ::= FALSE.
{
  i = new z3::expr(s.c->bool_val(false));
}

abs_condition(i) ::= MUL.
{
	*s.arbitraries_counter = *s.arbitraries_counter+1;
	std::string name = "arbitrary"+std::to_string(*s.arbitraries_counter);
	z3::expr arbitrary = s.c->bool_const(name.c_str()); //get BOOLEAN sort for predicate symbol
	s.abs_exprs->insert(std::make_pair(name, arbitrary)); //add it to the symbol map
	i = new z3::expr(arbitrary);
	s.arbitraries->insert(std::make_pair(name.c_str(), arbitrary));
}

abs_condition(i) ::= VAR(var).
{
try
  {
    i = new z3::expr(s.abs_exprs->at(*var));
    if(!(i->is_bool()))
	{
		std::cout << "HORRIBLE WRONGNESS: This Predicate does not have sort bool!! " << *var << std::endl;
	}
	
  }
  catch (std::out_of_range o)
  {
    std::cout << "Cannot find Variable definition for Predicate " << *var <<
    std::endl;
    throw;
  }
  delete var;
}

/* 
####### Section ASSIGNMENTS ##########
*/
assignment(a) ::= line_no(no) INT VAR(v) ASSIGN term(t) SEMI.
{	
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no<< " Parsing ASSIGNMENT: " << *v << " = " << *t << std::endl;
	#endif // ifdef DEBUGLEMON
  z3::expr exp = s.c->int_const(( *v).c_str());
  s.exprs->insert(std::make_pair((*v), exp));
  a = new Assignment(s.c,no, exp, *t);
  delete v;
  delete t;
}


assignment(a) ::= line_no(no) VAR(v) ASSIGN term(t) SEMI.
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no<< " Parsing ASSIGNMENT: " << *v + "1" << " = " << *t << std::endl;
	#endif // ifdef DEBUGLEMON

  try
  {
    z3::expr exp = s.c->int_const(( *v).c_str());
    a = new Assignment(s.c,no, exp, *t);
  }
  catch (std::out_of_range o)
  {
    std::cout << "Cannot find Variable definition for Varibale " << *v <<
    std::endl;
    delete v;
    delete t;
    throw;
  }
  delete v;
  delete t;
}

/* 
####### Section ASSIGNMENTS Abstraction ##########
*/

abs_assignment(a) ::= line_no(no) VAR(v) ASSIGN abs_condition(t) SEMI abs_assignment(a2).
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << no<< " Parsing ASSIGNMENT Abstraction: " << *v + "1" << " = " << *t << std::endl;
	#endif // ifdef DEBUGLEMON

  try
  {
    //create a new Predicate e.g. p1 that is the "new value" in contrast to p which represents the state of p before this line
    z3::expr predicate = s.c->bool_const(( *v).c_str());
    s.abs_exprs->insert(std::make_pair(( *v), predicate)); //add it to the symbol map
    a2->push_back(new Assignment(s.c,no, predicate, *t));
    a = a2;
  }
  catch (std::out_of_range o)
  {
    std::cout << "Cannot find Variable definition for Predicate " << *v + "1"<< std::endl;
    delete v;
    delete t;
    throw;
  }
  delete v;
  delete t;
}

abs_assignment(a) ::= .
{
	a = new std::vector<Expression*>();
}

/* 
####### Section TERM and VALUE ##########
*/
integer(i) ::= VAR(v).
{
	#ifdef DEBUGLEMON
	std::cout << std::setw(3) << s.line_no << " Parsing Integer " << std::endl;
	#endif // ifdef DEBUGLEMON
  i = new z3::expr(( s.c )->int_const(( *v ).c_str()));
  delete v;
}


term(t) ::= LPAREN term(i) RPAREN.
{
  *i = ( *i );
  t  = i;
}
term(t) ::= value(v).
{
  t = v;
}


term(t) ::= term(v)  PLUS term(t2).
{
  *v = ( *v ) + *t2;
  t  = v;
  delete t2;
}

term(t) ::= term(v)  MINUS term(t2).
{
  *v = ( *v ) - *t2;
  t  = v;
  delete t2;
}

term(t) ::= term(v)  MUL term(t2).
{
  *v = ( *v ) * *t2;
  t  = v;
  delete t2;
}

term(t) ::= term(v)  DIV term(t2).
{
  *v = ( *v ) / *t2;
  t  = v;
  delete t2;
}



value(v) ::= MINUS VAR(var).
{
  try
  {
    v = new z3::expr(s.exprs->at(*var));
  }
  catch (std::out_of_range o)
  {
    std::cout << "Cannot find Variable definition for Varibale " << *var <<
    std::endl;
    throw;
  }

  delete var;
}

value(v) ::= VAR(var).
{
  try
  {
    v = new z3::expr(s.exprs->at(*var));
  }
  catch (std::out_of_range o)
  {
    std::cout << "Cannot find Variable definition for Varibale " << *var <<
    std::endl;
    throw;
  }
  delete var;
}


value(v) ::= VAR(var) LSQBR term(con) RSQBR.
{
  try
  {
    v = new z3::expr(z3::select(s.exprs->at(*var), *con));
  }
  catch (std::out_of_range o)
  {
    std::cout << "Cannot find Variable definition for Varibale " << *var <<
    std::endl;
    throw;
  }
  delete con;
  delete var;
}

value(v) ::= CONST(con).
{
  v = new z3::expr(s.c->int_val(( *con ).c_str()));

  delete con;
}

value(v) ::= MINUS CONST(con).
{
  v = new z3::expr(-s.c->int_val(( *con ).c_str()));

  delete con;
}

