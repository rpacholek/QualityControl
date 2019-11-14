#include <Framework/DataProcessorSpec.h>
#include <Framework/DeviceSpec.h>
#include <Framework/DataProcessorSpec.h>

#include "QualityControl/Alarm.h"
#include "QualityControl/AlarmFactory.h"

namespace o2::quality_control::alarm
{

using namespace o2::framework;

o2::framework::DataProcessorSpec
  AlarmFactory::create(std::string alarmName, std::string configurationSource)
{
  Alarm qcAlarm{ alarmName, configurationSource };

  DataProcessorSpec newAlarm{
    qcAlarm.getDeviceName(),
    qcAlarm.getInputs(),
    Outputs{ qcAlarm.getOutputSpec() },
    AlgorithmSpec{},
    Options{},
  };
  // this needs to be moved at the end
  newAlarm.algorithm = adaptFromTask<Alarm>(std::move(qcAlarm));

  return newAlarm;
}

void AlarmFactory::customizeInfrastructure(std::vector<framework::CompletionPolicy>& policies)
{
  auto matcher = [](framework::DeviceSpec const& device) {
    return device.name.find(Alarm::createAlarmIdString()) != std::string::npos;
  };
  auto callback = [](gsl::span<PartRef const> const& inputs) {
    // TODO: Check if need to check nullptr (done in checker::run)
    for (auto& input : inputs) {
      if (!(input.header == nullptr || input.payload == nullptr)) {
        return framework::CompletionPolicy::CompletionOp::Consume;
      }
    }
    return framework::CompletionPolicy::CompletionOp::Wait;
  };

  framework::CompletionPolicy checkerCompletionPolicy{ "alarmCompletionPolicy", matcher, callback };
  policies.push_back(checkerCompletionPolicy);
}

} // namespace o2::quality_control::alarm
