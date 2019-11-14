#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <memory>
#include <any>

#include <Framework/DataProcessorSpec.h>
#include <Framework/DataRef.h>

#include "QualityControl/Quality.h"
#include "QualityControl/QualityObject.h"
#include "QualityControl/Check.h"

namespace o2::quality_control::alarm
{

class Value
{
 public:
  Value(){};

  virtual std::string getName() { return ""; };

  virtual bool isValid() { return false; };
  virtual bool isReady() { return false; };

  // Compare inside expression
  virtual std::size_t typeHash() { return typeid(void).hash_code(); };
  virtual int compare(std::shared_ptr<Value>&) { return 0; };
  virtual std::any getCompareObject() { return 0; };

  // Update value from DPL
  virtual framework::Inputs getInputs() { return {}; };
  virtual void update(framework::DataRef&){};
};

class QualityValue : public Value
{
 public:
  typedef core::Quality value_type;
  inline static const std::string prefix = "Quality";
  std::size_t typeHash() override { return typeid(QualityValue::value_type).hash_code(); };

  QualityValue(std::string value);

  std::string getName() { return mName; };

  bool isValid() override { return true; };
  bool isReady() override { return true; };

  int compare(std::shared_ptr<Value>& value) override;
  std::any getCompareObject() override { return quality; };

 private:
  std::string mName;
  QualityValue::value_type quality;
};

class CheckValue : public Value
{
 public:
  typedef core::Quality value_type;
  inline static const std::string prefix = "Check";
  std::size_t typeHash() override { return typeid(CheckValue::value_type).hash_code(); };

  CheckValue(std::string name);

  std::string getName() { return mName; };

  bool isValid() override;
  bool isReady() override;
  int compare(std::shared_ptr<Value>& value) override;
  std::any getCompareObject() override { return mLastQuality; };
  framework::Inputs getInputs() override;
  void update(framework::DataRef& data) override;

 private:
  std::string mName;
  int lastUpdate = 0;
  CheckValue::value_type mLastQuality;
};

enum class AlarmReturn {
  True,
  False,
  Outdated, // Value inside older then set
  Undefined // The value could not arive
};

class AlarmExpression
{
 public:
  AlarmExpression();
  AlarmExpression(std::string* op, std::unique_ptr<AlarmExpression> left, std::unique_ptr<AlarmExpression> right);
  AlarmExpression(std::string* op, std::unique_ptr<AlarmExpression> left);
  AlarmExpression(std::string* op, std::shared_ptr<Value> lest, std::shared_ptr<Value> right);

  //AlarmExpression(AlarmExpression&) = delete;
  //AlarmExpression(const AlarmExpression&) = delete;
  //AlarmExpression& operator=(const AlarmExpression&) = delete;
  ~AlarmExpression() = default;

  /**
   * Run eval recursivly throught whole expression.
   * Returns
   * EVALUATION IS LAZY!
   */
  AlarmReturn eval();

  void setValueLifetime(long lifetime) { this->lifetime = lifetime; };

  /**
   * Value comes in form "Prefix::Name", where the Prefix indicates 
   * which implementation will be use and Name will be passed to the constructor.
   */
  static std::shared_ptr<Value> parseValue(std::string* value);

 private:
  // Unlimited lifetime
  long lifetime = 0;
  int op = 0;

  std::unique_ptr<AlarmExpression> expr_left;
  std::unique_ptr<AlarmExpression> expr_right;

  std::shared_ptr<Value> value_left;
  std::shared_ptr<Value> value_right;

  enum ExpressionOp : int {
    op_or,  // |
    op_and, // &
    op_xor, // ^
    op_not  // !
  };

  enum ValueOp : int {
    op_eq,   // ==
    op_neq,  // !=
    op_lt,   // <
    op_lteq, // <=
    op_gt,   // >
    op_gteq  // >=
  };

  static ValueOp parseValueOp(std::string* op);
  static ExpressionOp parseExpresionOp(std::string* op);

  AlarmReturn evalExpression();
  AlarmReturn evalValues();
};

} // namespace o2::quality_control::alarm

#endif
