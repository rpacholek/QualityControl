#ifndef QC_SCANNER_H
#define QC_SCANNER_H

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "QualityControl/Parser.h"

namespace o2::quality_control::alarm
{
class Scanner : public yyFlexLexer
{
 public:
  Scanner(std::istream* input) : yyFlexLexer(input){};
  virtual ~Scanner(){};
  virtual int yylex(Parser::semantic_type* const lval, Parser::location_type* loc);

 private:
  Parser::semantic_type* yylval = nullptr;
};
} // namespace o2::quality_control::alarm

#endif
