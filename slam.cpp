#include "slam.h"
#include <iomanip>
#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>

std::ostream& operator<<(std::ostream &stream, const predicate_setting &p) 
{ 
	return stream << *(p.predicate) << "=" << p.bool_setting; 
}

std::ostream & operator<<(std::ostream& os, CHECK_TYPE t)
{
  switch (t)
  {
    case CHECK_TYPE::LOOP_PRECONDITION:
      os << "(loop precondition)  ";
      break;
    case CHECK_TYPE::LOOP_POSTCONDITION:
      os << "(loop postcondition) ";
      break;
    case CHECK_TYPE::LOOP_PRE_BODY:
      os << "(loop pre body)      ";
      break;
    case CHECK_TYPE::LOOP_POST_BODY:
      os << "(loop post body)     ";
      break;

    case CHECK_TYPE::IF_PRE_THEN:
      os << "(if pre then)        ";
      break;
    case CHECK_TYPE::IF_POST_THEN:
      os << "(if post then)       ";
      break;

    case CHECK_TYPE::IF_PRE_ELSE:
      os << "(if pre else)        ";
      break;
    case CHECK_TYPE::IF_POST_ELSE:
      os << "(if post else)       ";
      break;
    case CHECK_TYPE::STRENGTH:
      os << "(strengthening)      ";
      break;
    case CHECK_TYPE::ASSIGNMENT:
      os << "(assignment)         ";
      break;

    case CHECK_TYPE::IF_PRE_POST:
      os << "(if pre - post)      ";
      break;
  }
  return os;
}



void Expression::print(bool valid, int line_no1, int line_no2, CHECK_TYPE type)
{
#ifdef LINES
  std::cout << "line " << std::setw(4) << line_no1 << "  -" <<
  std::setw(4) <<
    line_no2  <<
    ": " <<
    std::setw(8) << ( valid ? "valid" : "invalid" ) << std::endl;
#else
  #ifdef TYPES
  std::cout << "line " << std::setw(4) << line_no1  << "  -" <<
  std::setw(4) <<
    line_no2 <<
    " " << type << ": " <<
    std::setw(8) << ( valid ? "valid" : "invalid" ) << std::endl;
  #endif // ifdef TYPES

#endif // ifdef LINES
}


bool Expression::isEqual(z3::solver* s, z3::expr a, z3::expr b)
{
  if (!a.is_bool())
  {
    std::cout << "expr a has invalid type!" << std::endl;
    return false;
  }
  if (!b.is_bool())
  {
    std::cout << "expr b has invalid type!" << std::endl;
    return false;
  }

  s->add(!( implies(a, b) && implies(b, a)));

  auto status =  s->check();
  assert(status==z3::sat || status==z3::unsat);

  bool valid = (status == z3::unsat);

#ifdef PRINT_MODEL
  std::cout << *s << std::endl;
  if (( s->check() == z3::unknown ))
  {
    std::cout << "unknown" << std::endl;
  }
  else if (!valid)
  {
    std::cout << "the original query was " << std::endl;
    std::cout << s->to_smt2() << std::endl;
    std::cout << "the SMT model is" << std::endl;
    std::cout << s->get_model() << std::endl;
  }
#endif

  s->reset();
  return valid;
}



std::string Assignment::getReadableRepresenation() 
{
	std::string abs;
	std::stringstream  os;
	os << getVerificationConjecture();
	abs = os.str();
	abs = "Assign " + abs;
	return abs;
}
    
std::string IfExpression::getReadableRepresenation() 
{
	std::string abs;
	std::stringstream  os;
	os << getVerificationConjecture();
	abs = os.str();
	abs = "IF ( " + abs + " )";
	return abs;
}

std::string IfElseExpression::getReadableRepresenation() 
{
	std::string abs;
	std::stringstream  os;
	os << getVerificationConjecture();
	abs = os.str();
	abs = "IF ( " + abs + " )";
	return abs;
}

std::string WhileExpression::getReadableRepresenation() 
{
	std::string abs;
	std::stringstream  os;
	os << getVerificationConjecture();
	abs = os.str();
	abs = "WHILE ( " + abs + " )";
	return abs;
}

std::string Assert::getReadableRepresenation() 
{
	std::string abs;
	std::stringstream  os;
	os << getVerificationConjecture();
	abs = os.str();
	abs = "ASSERT ( " + abs + " )";
	return abs;
}  

std::string SkipExpression::getReadableRepresenation() 
{
	std::string abs;
	abs = "Skip";
	return abs;
}    
    
//implementation of pure virtual function verify
//this function checks if the abstraction is a valid
//abstraction of the expression given the predicates

z3::expr WhileExpression::getVerificationConjecture()
{
return condition_;
}


z3::expr IfElseExpression::getVerificationConjecture()
{
return condition_;
}



z3::expr IfExpression::getVerificationConjecture()
{
return condition_;
}



z3::expr Assignment::getVerificationConjecture()
{
	//std::cout << "VAR: " << var_ << " TERM: " << term_to_assign_ << std::endl;
	if(var_.is_bool())
	{
		return c_->bool_const(( var_.to_string() + "1").c_str()) == term_to_assign_;
	}
	else
	{
		return c_->int_const(( var_.to_string() + "1").c_str()) == term_to_assign_;
	}
}

z3::expr Assert::getVerificationConjecture()
{
return assert_;
}

z3::expr SkipExpression::getVerificationConjecture()
{
return z3::expr(c_->bool_val(true));
}



//implementation of pure virtual function
//this generates a z3::expression from the saved data
//of each Expression-Class -> e.g. variable and assignment
//and then calls setExpression(expr) with the generated expr

void WhileExpression::generateAndSetExpression()
{
  
}

void IfElseExpression::generateAndSetExpression()
{
  
}

void IfExpression::generateAndSetExpression()
{
  
}

void SkipExpression::generateAndSetExpression()
{
  
}


void Assignment::generateAndSetExpression()
{
 
}

void Assert::generateAndSetExpression()
{
  
}


