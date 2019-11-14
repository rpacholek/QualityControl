#ifndef QC_ALARM_ALARMFACTORY_H
#define QC_ALARM_ALARMFACTORY_H

#include <string>
#include <vector>

#include <Framework/DataProcessorSpec.h>

namespace o2::framework
{
struct CompletionPolicy;
}

namespace o2::quality_control::alarm
{

/// \brief Factory in charge of creating DataProcessorSpec of QC task
class AlarmFactory
{
 public:
  AlarmFactory() = default;
  virtual ~AlarmFactory() = default;

  o2::framework::DataProcessorSpec create(std::string taskName, std::string configurationSource);

  static void customizeInfrastructure(std::vector<framework::CompletionPolicy>& policies);
};

} // namespace o2::quality_control::alarm

#endif // QC_CORE_TASKFACTORY_H
