%{
/* C++ declarations */
#include "QualityControl/Alarm.h"
#include "QualityControl/Scanner.h"

#define yyterminate() return( token::END )
#undef YY_DECL
#define YY_DECL int Scanner::yylex(Parser::semantic_type* const lval, Parser::location_type* loc)

using namespace o2::quality_control::alarm;
typedef Parser::token token;
typedef Parser::token_type token_type;
%}

%option c++
%option batch
%option debug
%option yyclass="Scanner"
%option noyywrap
%option nodefault

%%
%{
  yylval = lval;

%}

[A-Za-z]+"::"[A-Za-z0-9]+ {
  lval->token_value = new std::string(yytext);
  return token::VALUE;
}

"&"|"|"|"^" {
  lval->token_value = new std::string(yytext);
  return token::EXPOP;
}

"=="|"!="|"<"|"<="|">="|">" {
  lval->token_value = new std::string(yytext);
  return token::VALOP;
}

"!" {
  lval->token_value = new std::string(yytext);
  return token::EXPUNAROP;
}

"(" {
  return token::PARENTHESIS_OPEN;
}

")" {
  return token::PARENTHESIS_CLOSE;
}

[ \t\r]+ {
  loc->step();
}

%%
