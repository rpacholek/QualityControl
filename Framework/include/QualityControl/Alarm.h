
#ifndef QC_ALARM_H
#define QC_ALARM_H

#include <string>
// O2
#include <Framework/Task.h>
#include <Framework/DataProcessorSpec.h>
#include <Framework/DataRef.h>
#include <Configuration/ConfigurationFactory.h>
#include <Headers/DataHeader.h>
// Qc
#include "QualityControl/Expression.h"
#include "QualityControl/QcInfoLogger.h"

namespace o2::quality_control::alarm
{

class Alarm : public framework::Task
{
 public:
  Alarm(const std::string& alarmName, const std::string& configurationSource);
  ~Alarm() override = default;
  Alarm(Alarm&&) = default;

  void run(framework::ProcessingContext& pCtx) override;

  const framework::Inputs& getInputs() { return mInputs; };
  const framework::OutputSpec getOutputSpec() { return mOutputSpec; };

  static std::string createAlarmIdString();
  /// \brief Unified DataOrigin for Quality Control tasks
  static header::DataOrigin createAlarmDataOrigin();
  /// \brief Unified DataDescription naming scheme for all tasks
  static header::DataDescription createAlarmDataDescription(const std::string& alarmName);

  std::string getName() const { return mName; };
  std::string getDeviceName() const { return mDeviceName; };

  void set_expression(std::unique_ptr<AlarmExpression> exp);
  void register_value(std::shared_ptr<Value> value);

 private:
  void parse_string(const std::string& input);
  void gather_inputs();

  std::string mName;
  std::string mDeviceName;
  std::unique_ptr<AlarmExpression> mExpression = nullptr;
  std::vector<std::shared_ptr<Value>> mValues;
  std::map<std::string /* binding */, std::vector<std::shared_ptr<Value>>> mUpdateMap;

  framework::Inputs mInputs;
  framework::OutputSpec mOutputSpec;

  o2::quality_control::core::QcInfoLogger& mLogger;
};

} // namespace o2::quality_control::alarm

#endif
