// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include <Framework/DataSampling.h>
#include "QualityControl/InfrastructureGenerator.h"

using namespace o2;
using namespace o2::framework;

// The customize() functions are used to declare the executable arguments and to specify custom completion and channel
// configuration policies. They have to be above `#include "Framework/runDataProcessing.h"` - that header checks if
// these functions are defined by user and if so, it invokes them. It uses a trick with SFINAE expressions to do that.

void customize(std::vector<CompletionPolicy>& policies)
{
  DataSampling::CustomizeInfrastructure(policies);
  quality_control::customizeInfrastructure(policies);
  
  auto matcher = [](framework::DeviceSpec const& device) {
    return device.name.find("collector") != std::string::npos;
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

  framework::CompletionPolicy checkerCompletionPolicy{ "collectorCompletionPolicy", matcher, callback };
  policies.push_back(checkerCompletionPolicy);
}

void customize(std::vector<ChannelConfigurationPolicy>& policies)
{
  DataSampling::CustomizeInfrastructure(policies);
}

void customize(std::vector<ConfigParamSpec>& workflowOptions)
{
  workflowOptions.push_back(
    ConfigParamSpec{ "config-path", VariantType::String, "", { "Path to the config file. Overwrite the default paths. Do not use with no-data-sampling." } });
  workflowOptions.push_back(
    ConfigParamSpec{ "no-data-sampling", VariantType::Bool, false, { "Skips data sampling, connects directly the task to the producer." } });
}

#include <random>
#include <string>

#include <Framework/runDataProcessing.h>

#include "QualityControl/CheckRunner.h"
#include "QualityControl/InfrastructureGenerator.h"
#include "QualityControl/runnerUtils.h"
#include "QualityControl/ExamplePrinterSpec.h"
#include "QualityControl/BenchmarkDevices.h"

std::string getConfigPath(const ConfigContext& config);

using namespace o2;
using namespace o2::framework;
using namespace o2::quality_control::checker;
using namespace o2::quality_control::benchmark;
using namespace std::chrono;

// clang-format off
WorkflowSpec defineDataProcessing(const ConfigContext& config)
{
  WorkflowSpec specs;

  // Path to the config file
  std::string qcConfigurationSource = getConfigPath(config);
  LOG(INFO) << "Using config file '" << qcConfigurationSource << "'";

  // The producer to generate some data in the workflow
  auto generators = GeneratorDevice::createGeneratorDevices(qcConfigurationSource);
  for (auto generator: generators){
    specs.push_back(
        DataProcessorSpec{
            generator.getName(),
            Inputs{},
            generator.getOutputs(),
            adaptFromTask<GeneratorDevice>(std::move(generator))
        }
    ); 
  }

  // Generation of Data Sampling infrastructure
  DataSampling::GenerateInfrastructure(specs, qcConfigurationSource);

  // Generation of the QC topology (one task, one checker in this case)
  quality_control::generateRemoteInfrastructure(specs, qcConfigurationSource);

  // Finally the printer
  CollectorDevice collector{qcConfigurationSource};
  DataProcessorSpec printer{
    "collector",
    collector.getInputs(),
    Outputs{},
    adaptFromTask<CollectorDevice>(std::move(collector))
  };
  specs.push_back(printer);

  return specs;
}
// clang-format on

// TODO merge this with the one from runReadout.cxx
std::string getConfigPath(const ConfigContext& config)
{
  std::string filename = "benchmark.json";
  std::string defaultConfigPath = getenv("QUALITYCONTROL_ROOT") != nullptr ? std::string(getenv("QUALITYCONTROL_ROOT")) + "/etc/" + filename : "$QUALITYCONTROL_ROOT undefined";
  // The the optional one by the user
  auto userConfigPath = config.options().get<std::string>("config-path");
  // Finally build the config path based on the default or the user-base one
  std::string path = std::string("json:/") + (userConfigPath.empty() ? defaultConfigPath : userConfigPath);
  return path;
}
