#include <TObject.h>
#include <TArrayD.h>

#include <Framework/DataProcessorSpec.h>
#include <Framework/DataSpecUtils.h>
#include <Framework/Task.h>
#include <Common/Exceptions.h>


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
  private:
    std::string mName;
    int mSize = 100;
    int mFrequency = 10;
    //int mDuration = 100;
    int mQuantity = 100;
    TArrayD* mObject;
    bool mFirstTime = true;
    
    framework::OutputSpec mOutputSpec;

};


class CollectorDevice : public framework::Task {
  public:
    CollectorDevice(std::string configSource);
    void run(framework::ProcessingContext& ctx) override;
    framework::Inputs getInputs() { return mInputs; };
  private:
    framework::Inputs mInputs;
    std::map<std::string, int> mCollected;
    int mGlobalReceived;
};

}
