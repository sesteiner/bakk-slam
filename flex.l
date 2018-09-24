%{
  #include "lemon.h"
  #include <iostream>
  #include <fstream>

  #include "slam.h"

  #include <map>

	//install using branch in folder dependancies
	#include "cxxopts.hpp"

%}


%option noyywrap
%option   yylineno

%%

"forall"                 { return FORALL;}
"in"                     { return IN;}
":"                      { return COLON;}
"["                      { return LSQBR;}
"]"                      { return RSQBR;}
"{"                      { return LBR; }
"}"                      { return RBR; }
"("                      { return LPAREN; }
"void"                   { return VOID;}
"int"                    { return INT;}
","                      { return COMMA;}
"/*@"                    { return ABSTRACTION_START;}
"*/"                     { return ABSTRACTION_END;}
"abstraction"            { return ABSTRACTION_TOKEN; }
"predicate"		 { return PREDICATE_TOKEN; }
"assert"		 { return ASSERT; }
"skip"		 	 { return SKIP; }
")"                      { return RPAREN; }
"if"                     { return IF; }
"while"                  { return WHILE; }
"else"                   { return ELSE; }
"&&"                     { return AND; }
"||"                     { return OR; }
"!"                      { return NOT; }
"?"                      { return CONDITIONAL; }
">="                     { return GREATER_EQUAL; }
">"                      { return GREATER; }
"=="                     { return EQUALS; }
"="                      { return ASSIGN; }
"<"                      { return SMALLER; }
"<="                     { return SMALLER_EQUAL; }
";"                      { return SEMI; }
"+"                      { return PLUS; }
"*"                      { return MUL; }
"/"                      { return DIV; }
"-"                      { return MINUS; }
"%"                      { return MOD; }

"true"                   { return TRUE; }
"false"                  { return FALSE; }
[0-9]+                   { return CONST; }
[a-zA-Z]+[a-zA-Z0-9]*    { return VAR; }
"/*"[^@]*"*/"            { }

<<EOF>>                  {  yyterminate(); }

[ \t\r]                  { }
[\n]                     { }


%%

#include <z3++.h>

void* ParseAlloc(void* ( *allocProc )(size_t));
void* Parse(void*, int, std::string*, struct smt_params);
void* ParseFree(void*, void ( * freeProc )(void*));



int parse(std::istream& in)
{
  void* pParser = ParseAlloc(malloc);

  yyFlexLexer* flex = new yyFlexLexer(&in);

  int yv = flex->yylex();


  z3::context c;
  z3::solver  s(c);

  struct smt_params params = smt_params(&c, &s);
  



  while (yv != 0)
  {
    std::string* text = new std::string(flex->YYText());

    params.line_no = flex->lineno();

#ifdef DEBUGFLEX
    std::cout << flex->lineno() << " TOKEN: " << yv << " :" << *text <<
    std::endl;
#endif // ifdef DEBUGFLEX


    Parse(pParser, yv, text, params);

    yv = flex->yylex();
  }

  Parse(pParser, 0, 0x0, params);
  ParseFree(pParser, free);

  int ret = flex->yylex();

	
	delete params.exprs;
	delete params.abs_exprs;
	delete params.predicates;
	delete params.predicates_expr;
	delete params.arbitraries;
	delete params.arbitraries_counter;
	delete params.all_abstractions_valid;
	delete params.program_flow;
	
	
  delete flex;
  return ret;
}



int main(int argc, char** argv)
{
  try
  {
  /*
    std::cout << "test" << std::endl;
  	cxxopts::Options options("SLAM-Analyser", "Analyses student files");
  	std::cout << "test" << std::endl;
  	options.add_options()
			("d,debug", "Enable debugging")
			("f,file", "File name", cxxopts::value<std::string>())
			("f2,file2", "File name of second Step", cxxopts::value<std::string>())
			;
  	std::cout << "test" << std::endl;
  	auto result = options.parse(argc, argv);	
  	std::cout << "test" << std::endl;
  	std::fstream inputFile;
    inputFile.open(result["f"].as<std::string>(), std::fstream::in);
    if(!inputFile){
        std::cerr << "Error opening input file";
        exit(1);
    }
    
    std::cout << "test" << std::endl;
    std::istream&    s   = inputFile;
    std::cout << "test" << std::endl;
    return parse(s);
  std::cout << "test" << std::endl;  
    */
  return parse(std::cin);
  }
  catch (std::exception e)
  {
    std::cout << e.what() << std::endl;
  }
  catch (z3::exception ze)
  {
    std::cout << ze << std::endl;
  }

  return -1;
}
