#include "QualityControl/Alarm.h"
#include "QualityControl/Scanner.h"
#include "QualityControl/Parser.h"

#include <sstream>
#include <algorithm>
#include <boost/throw_exception.hpp>

#include <Common/Exceptions.h>

using namespace AliceO2::Common;
using namespace AliceO2::InfoLogger;
using namespace o2::quality_control::alarm;
using namespace o2::configuration;
using namespace o2::framework;

o2::header::DataDescription Alarm::createAlarmDataDescription(const std::string& alarmName)
{
  if (alarmName.empty()) {
    BOOST_THROW_EXCEPTION(FatalException() << errinfo_details("Empty alarm name for data description"));
  }
  o2::header::DataDescription description;
  description.runtimeInit(std::string(alarmName.substr(0, o2::header::DataDescription::size - 4) + "-chk").c_str());
  return description;
}

o2::header::DataOrigin Alarm::createAlarmDataOrigin()
{
  return header::DataOrigin{ "QC" };
}

std::string Alarm::createAlarmIdString()
{
  return "QC-ALARM-";
}

Alarm::Alarm(const std::string& alarmName, const std::string& configurationSource)
  : mName(alarmName),
    mDeviceName(createAlarmIdString() + alarmName),
    mOutputSpec({ "al" }, createAlarmDataOrigin(), createAlarmDataDescription(alarmName)),
    mLogger(QcInfoLogger::GetInstance())
{
  auto config = ConfigurationFactory::getConfiguration(configurationSource);
  std::string expressionString = config->get<std::string>("qc.alarms." + alarmName + ".condition");
  parse_string(expressionString);
  gather_inputs();
}

void Alarm::gather_inputs()
{
  for (auto& value : mValues) {
    mLogger << value->getName() << ": " << value->getInputs().size() << AliceO2::InfoLogger::InfoLogger::endm;
    for (auto& input : value->getInputs()) {
      mUpdateMap[input.binding].push_back(value);
      if (std::find(mInputs.begin(), mInputs.end(), input) == mInputs.end()) {
        mInputs.push_back(input);
      }
    }
  }
}

void Alarm::parse_string(const std::string& input)
{
  try {
    std::istringstream stream(input);

    Scanner scanner(&stream);
    Parser parser(scanner, *this);

    if (parser.parse() != 0) {
      BOOST_THROW_EXCEPTION(FatalException() << errinfo_details("Parser terminated incorectly"));
    }

    if (mExpression == nullptr) {
      BOOST_THROW_EXCEPTION(FatalException() << errinfo_details("Expression empty after parsing"));
    }
  } catch (...) {
    std::string diagnostic = boost::current_exception_diagnostic_information();
    LOG(ERROR) << "Unexpected exception, diagnostic information follows:\n"
               << diagnostic;
    throw;
  }
}

void Alarm::set_expression(std::unique_ptr<AlarmExpression> exp)
{
  mExpression = std::move(exp);
}

void Alarm::register_value(std::shared_ptr<Value> value)
{
  mValues.push_back(value);
}

void Alarm::run(framework::ProcessingContext& pCtx)
{
  bool update = false;
  for (const auto& input : mInputs) {
    DataRef dataRef = pCtx.inputs().get(input.binding.c_str());

    // Check if there is some content
    if (dataRef.header != nullptr && dataRef.payload != nullptr) {
      update = true;
      // Update all values that register the need for this data
      for (auto& value : mUpdateMap[input.binding]) {
        value->update(dataRef);
      }
    }
  }

  if (update) {
    auto result = mExpression->eval();
    switch (result) {
      case AlarmReturn::True:
        mLogger << "ALARM: True" << AliceO2::InfoLogger::InfoLogger::endm;
        break;
      case AlarmReturn::False:
        mLogger << "ALARM: False" << AliceO2::InfoLogger::InfoLogger::endm;
        break;
      case AlarmReturn::Outdated:
        mLogger << "ALARM: Outdated" << AliceO2::InfoLogger::InfoLogger::endm;
        break;
      case AlarmReturn::Undefined:
        mLogger << "ALARM: Undefined" << AliceO2::InfoLogger::InfoLogger::endm;
        break;
    }
  }
}
