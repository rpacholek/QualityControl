#include <string>
#include <TH1I.h>
#include <TArrayD.h>
#include <TRandom1.h>

#include <Framework/DataRefUtils.h>
#include <Configuration/ConfigurationFactory.h>
#include <Framework/ControlService.h>

#include "QualityControl/BenchmarkDevices.h"

using namespace o2::quality_control::benchmark;
using namespace AliceO2::Common;
using namespace o2::configuration;
using namespace o2::framework;

o2::header::DataDescription createDescription(std::string name, std::string postfix = "") {
  o2::header::DataDescription description;
  description.runtimeInit(std::string(name.substr(0, o2::header::DataDescription::size - 4) + postfix).c_str());
  return description;
}

GeneratorDevice::GeneratorDevice(std::string name, std::string sourceConf):mName(name),mOutputSpec("BENC", createDescription(name)){
  try {
    std::unique_ptr<ConfigurationInterface> config = ConfigurationFactory::getConfiguration(sourceConf);

    const auto& conf = config->getRecursive("qc.benchmark." + name);
    mSize = std::stoi(conf.get<std::string>("size"));
    mFrequency = std::stoi(conf.get<std::string>("frequency"));
    //mDuration = std::stoi(conf.get<std::string>("duration"));
    mQuantity = std::stoi(conf.get<std::string>("quantity"));
    LOG(INFO) << "Params: " << mSize << " f" << mFrequency;

  } catch (...) {
    std::string diagnostic = boost::current_exception_diagnostic_information();
    LOG(ERROR) << "Unexpected exception, diagnostic information follows:\n"
               << diagnostic;
    throw;
  }
  createObject();
}

void GeneratorDevice::createObject() {
    TRandom1 rand;
    mObject = new TArrayD(mSize);
    for (int i = 0; i<mSize; ++i){
      mObject->AddAt(rand.Rndm(), i);
    }
  }

std::vector<GeneratorDevice> GeneratorDevice::createGeneratorDevices(std::string configSource)
{
  std::vector<GeneratorDevice> devices;
  try {
    std::unique_ptr<ConfigurationInterface> config = ConfigurationFactory::getConfiguration(configSource);
    for (auto& conf: config->getRecursive("qc.benchmark")){
      devices.push_back(GeneratorDevice(conf.first, configSource));
    }
  } catch (...) {
    std::string diagnostic = boost::current_exception_diagnostic_information();
    LOG(ERROR) << "Unexpected exception, diagnostic information follows:\n"
               << diagnostic;
    throw;
  }
  return devices;
}

void GeneratorDevice::init(InitContext& context) {
  LOG(INFO) << "Initiate generator";
  sleep(10);
}

void GeneratorDevice::run(framework::ProcessingContext& ctx) {
  if (mFirstTime) {
    mFirstTime = false;
    sleep(30); // Wait 2min to setup whole workflow

    // TODO: Influxdb started marker

    LOG(INFO) << "Start sending";
    return;
  }
  if (mQuantity > 0) {
    usleep(mFrequency);
    auto output = framework::DataSpecUtils::asConcreteDataMatcher(mOutputSpec);
    ctx.outputs().snapshot(
        framework::Output{ output.origin, output.description }, 
        *mObject
    );
    --mQuantity;
  } else {
    LOG(INFO) << "Finished - now sleep";
    sleep(20); // Wait 20 seconds to complete - around 2 cycles

    // TODO: Influxdb stop marker
    
    ctx.services().get<ControlService>().readyToQuit(true);
  }
}

CollectorDevice::CollectorDevice(std::string sourceConf) {
  try {
    std::unique_ptr<ConfigurationInterface> config = ConfigurationFactory::getConfiguration(sourceConf);

    const auto& conf = config->getRecursive("qc.checks");

    for (auto& [checkName, checkConf]: conf) {
      (void)checkConf;
      InputSpec input{checkName, {"QC", createDescription(checkName, "-chk")}};
      mInputs.push_back(input);
    }

  } catch (...) {
    std::string diagnostic = boost::current_exception_diagnostic_information();
    LOG(ERROR) << "Unexpected exception, diagnostic information follows:\n"
               << diagnostic;
    throw;
  }

}

void CollectorDevice::run(framework::ProcessingContext& ctx) {
  LOG(INFO) << "Running collection";
  mGlobalReceived++;
  for (const auto& input : mInputs) {
    auto dataRef = ctx.inputs().get(input.binding.c_str());
    if (dataRef.header != nullptr && dataRef.payload != nullptr) {
      //auto moArray = ctx.inputs().get<TObjArray*>(input.binding.c_str());
      mCollected[input.binding]++;
    }
  }
  
  // TODO: Publish stats
}
