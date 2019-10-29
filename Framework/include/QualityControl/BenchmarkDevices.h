#include <chrono>

#include <TObject.h>
#include <TArrayD.h>

#include <Framework/DataProcessorSpec.h>
#include <Framework/DataSpecUtils.h>
#include <Framework/Task.h>
#include <Common/Exceptions.h>
#include <Common/Timer.h>
#include <Monitoring/MonitoringFactory.h>

namespace o2::quality_control::benchmark {


class GeneratorDevice : public framework::Task {
  public:
    GeneratorDevice(std::string name, std::string configSource);
    void run(framework::ProcessingContext& ctx) override;
    void init(framework::InitContext& context) override;
    void createObject();
    static std::vector<GeneratorDevice> createGeneratorDevices(std::string configSource);
    std::string getName() { return mName; }
    framework::Outputs getOutputs() { return { mOutputSpec }; }
    GeneratorDevice(const GeneratorDevice& device):GeneratorDevice(device.mName, device.mConfigSource){}
  private:
    std::string mName;
    std::string mConfigSource;
    int mSize = 100; // number of elements
    int mFrequency = 10; // usec
    int mDuration = 100; // seconds
    int mQuantity = 0;
    int mDelay = 30; // seconds
    TArrayD* mObject;
    bool mFirstTime = true;

    const int mStatPeriod = 10 * 1000 * 1000;
    
    AliceO2::Common::Timer mExperimentTimer;
    AliceO2::Common::Timer mStatTimer;
    
  std::unique_ptr<o2::configuration::ConfigurationInterface> mConfigFile; // used in init only
  std::unique_ptr<o2::monitoring::Monitoring> mMonitoring;

  framework::OutputSpec mOutputSpec;

};


class CollectorDevice : public framework::Task {
  public:
    CollectorDevice(std::string configSource);
    void run(framework::ProcessingContext& ctx) override;
    void init(framework::InitContext& context) override;
    framework::Inputs getInputs() { return mInputs; };
  private:
    framework::Inputs mInputs;
    std::map<std::string, int> mCollected;
    int mGlobalReceived;
    std::unique_ptr<o2::configuration::ConfigurationInterface> mConfigFile; // used in init only
    std::unique_ptr<o2::monitoring::Monitoring> mMonitoring;
    AliceO2::Common::Timer mStatTimer;


    const int mStatPeriod = 10 * 1000 * 1000;
};

}
