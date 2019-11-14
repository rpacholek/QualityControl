#include <boost/algorithm/string.hpp>
#include <boost/throw_exception.hpp>

#include <Common/Exceptions.h>
#include <Framework/DataRefUtils.h>

#include "QualityControl/Expression.h"
#include "QualityControl/QualityObject.h"

using namespace AliceO2::Common;
using namespace o2::quality_control::alarm;
using namespace o2::quality_control::checker;
using namespace o2::quality_control::core;
using namespace o2::framework;

AlarmExpression::AlarmExpression(std::string* op, std::unique_ptr<AlarmExpression> left, std::unique_ptr<AlarmExpression> right)
  : op(AlarmExpression::parseExpresionOp(op)),
    expr_left(std::move(left)),
    expr_right(std::move(right)),
    value_left(nullptr),
    value_right(nullptr)
{
}

AlarmExpression::AlarmExpression(std::string* op, std::unique_ptr<AlarmExpression> left)
  : op(AlarmExpression::parseExpresionOp(op)),
    expr_left(std::move(left)),
    expr_right(nullptr),
    value_left(nullptr),
    value_right(nullptr)
{
}

AlarmExpression::AlarmExpression(std::string* op, std::shared_ptr<Value> left, std::shared_ptr<Value> right)
  : op(AlarmExpression::parseValueOp(op)),
    expr_left(nullptr),
    expr_right(nullptr),
    value_left(left),
    value_right(right)
{
  if (value_left->typeHash() != value_right->typeHash()) {
    BOOST_THROW_EXCEPTION(FatalException() << errinfo_details("Compare objects are different"));
  }
}

AlarmReturn AlarmExpression::eval()
{
  if (value_left != nullptr && value_right != nullptr) {
    return evalValues();
  } else if (expr_left != nullptr) {
    return evalExpression();
  }
  BOOST_THROW_EXCEPTION(FatalException() << errinfo_details("Neither value nor expression set - empty expression"));
}

AlarmExpression::ValueOp AlarmExpression::parseValueOp(std::string* op)
{
  if (*op == "==") {
    return ValueOp::op_eq;
  } else if (*op == "!=") {
    return ValueOp::op_neq;
  } else if (*op == "<") {
    return ValueOp::op_neq;
  } else if (*op == "<=") {
    return ValueOp::op_neq;
  } else if (*op == ">") {
    return ValueOp::op_neq;
  } else if (*op == ">=") {
    return ValueOp::op_neq;
  }
  BOOST_THROW_EXCEPTION(FatalException() << errinfo_details("Unknown value operator '" + *op + "'"));
}

AlarmExpression::ExpressionOp AlarmExpression::parseExpresionOp(std::string* op)
{
  if (*op == "|") {
    return ExpressionOp::op_or;
  } else if (*op == "&") {
    return ExpressionOp::op_and;
  } else if (*op == "^") {
    return ExpressionOp::op_xor;
  } else if (*op == "!") {
    return ExpressionOp::op_not;
  }
  BOOST_THROW_EXCEPTION(FatalException() << errinfo_details("Unknown expression operator '" + *op + "'"));
}

AlarmReturn AlarmExpression::evalExpression()
{
  if ((expr_left != nullptr) & (expr_right != nullptr)) {
    switch (op) {
      case ExpressionOp::op_or:
        if (expr_left->eval() == AlarmReturn::True) {
          return AlarmReturn::True;
        } else {
          return expr_right->eval();
        }
        break;
      case ExpressionOp::op_and: {
        auto val = expr_left->eval();
        if (val != AlarmReturn::True) {
          return val;
        } else {
          return expr_right->eval();
        }
      } break;
      case ExpressionOp::op_xor: {
        auto val_l = expr_left->eval();
        auto val_r = expr_right->eval();
        if ((val_l == AlarmReturn::True && val_r == AlarmReturn::False) ||
            (val_l == AlarmReturn::False && val_r == AlarmReturn::True)) {
          return AlarmReturn::True;
        } else if (val_l == val_r) {
          return val_l;
        } else if (val_l == AlarmReturn::Undefined || val_l == AlarmReturn::Outdated) {
          return val_l;
        } else {
          return val_r;
        }
      } break;
    }
  } else {
    if (op == ExpressionOp::op_not) {
      auto nval = expr_left->eval();
      if (nval == AlarmReturn::True) {
        return AlarmReturn::False;
      } else if (nval == AlarmReturn::False) {
        return AlarmReturn::True;
      } else {
        return nval;
      }
    }
  }
  BOOST_THROW_EXCEPTION(FatalException() << errinfo_details("Expression evaluation error"));
}

AlarmReturn AlarmExpression::evalValues()
{
  if (!value_left->isReady() || !value_right->isReady()) {
    return AlarmReturn::Undefined;
  }
  if (!value_left->isValid() || !value_right->isValid()) {
    return AlarmReturn::Outdated;
  }

  int comp = value_left->compare(value_right);
  if (comp < 0) {
    if (op == ValueOp::op_neq || op == ValueOp::op_lt || op == ValueOp::op_lteq) {
      return AlarmReturn::True;
    } else {
      return AlarmReturn::False;
    }
  } else if (comp == 0) {
    if (op == ValueOp::op_eq || op == ValueOp::op_lteq || op == ValueOp::op_gteq) {
      return AlarmReturn::True;
    } else {
      return AlarmReturn::False;
    }

  } else {
    if (op == ValueOp::op_neq || op == ValueOp::op_gt || op == ValueOp::op_gteq) {
      return AlarmReturn::True;
    } else {
      return AlarmReturn::False;
    }
  }
}

std::shared_ptr<Value> AlarmExpression::parseValue(std::string* value)
{
  std::vector<std::string> results;
  boost::algorithm::split(results, *value, boost::is_any_of(":"));

  if (results.size() == 3) {
    if (results[0] == QualityValue::prefix) {
      return std::make_shared<QualityValue>(results[2]);
    } else if (results[0] == CheckValue::prefix) {
      return std::make_shared<CheckValue>(results[2]);
    }
  }
  BOOST_THROW_EXCEPTION(FatalException() << errinfo_details("Value parse error: probably not in form \"Prefix::Name\""));
}

QualityValue::QualityValue(std::string value) : mName(value)
{
  if ((value == "Good") || (value == "good")) {
    quality = Quality::Good;
  } else if ((value == "Medium") || (value == "medium")) {
    quality = Quality::Medium;
  } else if ((value == "Bad") || (value == "bad")) {
    quality = Quality::Bad;
  } else if ((value == "Null") || (value == "null")) {
    quality = Quality::Null;
  } else {
    BOOST_THROW_EXCEPTION(FatalException() << errinfo_details("Quality value parse error: unknown name '" + value + "'"));
  }
}

int QualityValue::compare(std::shared_ptr<Value>& value)
{
  QualityValue::value_type val = std::any_cast<QualityValue::value_type>(value->getCompareObject());
  return this->quality.getLevel() - val.getLevel();
}

CheckValue::CheckValue(std::string value) : mName(value), mLastQuality(Quality::Null) {}

int CheckValue::compare(std::shared_ptr<Value>& value)
{
  CheckValue::value_type val = std::any_cast<CheckValue::value_type>(value->getCompareObject());
  return mLastQuality.getLevel() - val.getLevel();
}

bool CheckValue::isValid()
{
  return true;
}

bool CheckValue::isReady()
{
  return mLastQuality.getLevel() != Quality::Null.getLevel();
}

Inputs CheckValue::getInputs()
{
  return { { "check-" + mName, "QC", checker::Check::createCheckerDataDescription(mName), 0 } };
}

void CheckValue::update(DataRef& data)
{
  auto qo = DataRefUtils::as<QualityObject>(data);
  mLastQuality = qo->getQuality();
}
