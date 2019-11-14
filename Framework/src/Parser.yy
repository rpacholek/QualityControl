%skeleton "lalr1.cc"
%require "3.0"
%debug
%defines
%define api.namespace {o2::quality_control::alarm}
%define parser_class_name {Parser}

%parse-param { class Scanner & scanner }
%parse-param { class Alarm & driver }

%define parse.error verbose

%{
#include "QualityControl/Expression.h"
#include "QualityControl/Scanner.h"
#include "QualityControl/Alarm.h"

#undef yylex
#define yylex scanner.yylex
%}

%union {
  std::string*   token_value;
  AlarmExpression* expression_value;
}

%token                END   0   "end of file"
%token  <token_value> VALUE
%token  <token_value> EXPOP
%token  <token_value> EXPUNAROP
%token                PARENTHESIS_OPEN
%token                PARENTHESIS_CLOSE
%token  <token_value> VALOP
%type   <expression_value> expression

%locations

%%

list_options : END | start END;

start : expression                  { driver.set_expression(std::unique_ptr<AlarmExpression>($1)); }

expression
    : expression EXPOP expression   { $$ = new AlarmExpression($2, std::unique_ptr<AlarmExpression>($1), std::unique_ptr<AlarmExpression>($3)); }
    | EXPUNAROP expression          { $$ = new AlarmExpression($1, std::unique_ptr<AlarmExpression>($2)); }
    | VALUE VALOP VALUE             { 
                                      auto val1 = AlarmExpression::parseValue($1);
                                      auto val2 = AlarmExpression::parseValue($3);
                                      driver.register_value(val1);
                                      driver.register_value(val2);
                                      $$ = new AlarmExpression($2, val1, val2); 
                                    }
    | PARENTHESIS_OPEN expression PARENTHESIS_CLOSE { $$ = $2; }
    ;

%%

namespace o2::quality_control::alarm {
  void Parser::error( const location_type &loc, const std::string &err_message)
  {
    std::cerr << "Error: " << err_message << " (" << loc << ")\n";
  }
}
