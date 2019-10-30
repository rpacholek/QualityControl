#include <string>
#include <TH1I.h>
#include <TArrayD.h>
#include <TRandom1.h>

#include <Framework/DataRefUtils.h>
#include <Framework/ControlService.h>
#include <Configuration/ConfigurationFactory.h>

#include "QualityControl/BenchmarkDevices.h"

using namespace o2::quality_control::benchmark;
using namespace AliceO2::Common;
using namespace o2::configuration;
using namespace o2::framework;
using namespace o2::monitoring;

o2::header::DataDescription createDescription(std::string name, std::string postfix = "") {
  o2::header::DataDescription description;
  description.runtimeInit(std::string(name.substr(0, o2::header::DataDescription::size - 4) + postfix).c_str());
  return description;
}

std::unique_ptr<Monitoring> initMonitoring(std::unique_ptr<ConfigurationInterface>& config) {
  try {
    std::string monitoringPath = config->get<std::string>(std::string("qc.config.monitoring.url"), std::string("infologger:///debug?qc"));
    if (monitoringPath.empty()) {
      monitoringPath = "infologger:///debug?qc";
    }
    std::unique_ptr<Monitoring> monitor = MonitoringFactory::Get(monitoringPath);
    return monitor;
  } catch (...) {
    std::string diagnostic = boost::current_exception_diagnostic_information();
    LOG(ERROR) << "Unexpected exception, diagnostic information follows:\n"
               << diagnostic;
    throw;
  }
  return nullptr;
}

GeneratorDevice::GeneratorDevice(std::string name, std::string sourceConf)
  :mName(name),
   mConfigSource(sourceConf),
   mOutputSpec("BENC", createDescription(name))
{
  try {
    mConfigFile = ConfigurationFactory::getConfiguration(sourceConf);

    const auto& conf = mConfigFile->getRecursive("qc.benchmark." + name);
    mSize = std::stoi(conf.get<std::string>("size"));
    mFrequency = std::stoi(conf.get<std::string>("frequency"));
    mDuration = std::stoi(conf.get<std::string>("duration"));
    mDelay = std::stoi(conf.get<std::string>("delay"));
    mAdaptivePeriod = std::stoi(conf.get<std::string>("adaptive"));
    //mQuantity = std::stoi(conf.get<std::string>("quantity"));


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

void GeneratorDevice::init(InitContext&) {
  LOG(INFO) << "Initiate generator";
  mMonitoring = initMonitoring(mConfigFile);
}

void GeneratorDevice::run(framework::ProcessingContext& ctx) {
  if (mFirstTime) {
    mMonitoring->send({ 0, "QC/generator/start_end" });
    mFirstTime = false;
    sleep(mDelay); // Wait to setup whole workflow

    mExperimentTimer.reset(mDuration*second);
    mStatTimer.reset(mStatPeriod);
    mAdaptiveTimer.reset(mAdaptivePeriod*second);

    LOG(INFO) << "============Start sending============";
    mMonitoring->send({ 1, "QC/generator/start_end" });
    return;
  }

  if (mAdaptivePeriod && mAdaptiveTimer.isTimeout()) {
    mAdaptiveTimer.reset(mAdaptivePeriod*second);
    mFrequency = mFrequency / 2;
  }

  if (!mExperimentTimer.isTimeout()) {
    usleep(mFrequency);
    auto output = framework::DataSpecUtils::asConcreteDataMatcher(mOutputSpec);
    ctx.outputs().snapshot(
        framework::Output{ output.origin, output.description }, 
        *mObject
    );
    ++mQuantity;
  } else {
    mMonitoring->send({ 2, "QC/generator/start_end" });
    LOG(INFO) << "============Finished - now sleep=========";
    sleep(mDelay); // Wait to complete - around 2 cycles

    ctx.services().get<ControlService>().readyToQuit(true);
  }

  if(mStatTimer.isTimeout()){
    int sub = mQuantity - mLastQuantity;
    mLastQuantity = mQuantity;
    mMonitoring->send({ mQuantity, "QC/generator/total/objects_published" });
    mMonitoring->send({ sub, "QC/generator/rate/objects_published_per_10_sec" });
    mMonitoring->send({ mFrequency, "QC/generator/last/frequency" });
    mStatTimer.reset(mStatPeriod); //Every 10s
  }
}

CollectorDevice::CollectorDevice(std::string sourceConf) {
  try {
    mConfigFile = ConfigurationFactory::getConfiguration(sourceConf);

    const auto& conf = mConfigFile->getRecursive("qc.checks");

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

  mStatTimer.reset(mStatPeriod);
}

void CollectorDevice::init(InitContext&) {
  mMonitoring = initMonitoring(mConfigFile);
}


void CollectorDevice::run(framework::ProcessingContext& ctx) {
  LOG(INFO) << "Running collection";
  for (const auto& input : mInputs) {
    auto dataRef = ctx.inputs().get(input.binding.c_str());
    if (dataRef.header != nullptr && dataRef.payload != nullptr) {
      //auto moArray = ctx.inputs().get<TObjArray*>(input.binding.c_str());
      mGlobalReceived++;
      mCollected[input.binding]++;
    }
  }
 
  if(mStatTimer.isTimeout()){
    mMonitoring->send({ mGlobalReceived, "QC/collector/total/objects_received" });
    mStatTimer.reset(mStatPeriod); //Every 10s
  } 
}
