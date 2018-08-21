#ifndef STRUCTS_H
#define STRUCTS_H 1


#include <iomanip>
#include <map>
#include <vector>
#include <string>


//install with sudo apt-get install z3 something something
#include <z3++.h>

class Line;

/**
 * Struct with data, which can be obtained from withing the parser.
 **/
struct smt_params
{
  z3::context*                     	c;
  z3::solver*                      	s;
  int                              	line_no;
  std::map<std::string, z3::expr>* 	exprs;
  std::map<std::string, z3::expr>* 	abs_exprs;
  std::vector<std::string>*				 	predicates;	  
  std::vector<z3::expr>* 				 	 	predicates_expr;
  std::map<std::string, z3::expr>* 	arbitraries; //each time we create a var for a * we store it here so we can check validity by setting all of those arbitrary values first true and then false -> arbitrary means in both cases the model must be satisfiable!
int*				   											arbitraries_counter;
  bool*				   										all_abstractions_valid;
  std::vector<Line*>*		   					program_flow;
	bool  														bool_mc_has_endless_loop;

  smt_params(z3::context* cont, z3::solver* solv) {
		c = cont;
		s = solv;
		line_no = -1;
		exprs = new std::map<std::string, z3::expr>();
		abs_exprs = new std::map<std::string, z3::expr>();
		predicates = new std::vector<std::string>();
		predicates_expr = new std::vector<z3::expr>();
		arbitraries = new std::map<std::string, z3::expr>();
		arbitraries_counter = new int(0);
		all_abstractions_valid = new bool(true);
		program_flow = new std::vector<Line*>();
		bool_mc_has_endless_loop = true;
  };

  ~smt_params() {
  };
};

struct predicate_setting 
{
	z3::expr* predicate;
	bool bool_setting;
	
	predicate_setting(z3::expr pred, bool set) {
		predicate = new z3::expr(pred);
		bool_setting = set;
	};
	
  ~predicate_setting() {
  };
  
  friend std::ostream& operator<<(std::ostream &stream, const predicate_setting &obj);
};

struct line_eval
{
	Line* line;
	bool eval;
	
	line_eval(Line* l, bool e) {
	line = l;
	eval = e;
	};
	
	~line_eval()
	{
	}
};

enum class PORT_DIRECTION
{
	N,
	NE,
	E,
	SE,
	S,
	SW,
	W,
	NW,
	C
};

#endif // ifndef STRUCTS_H


