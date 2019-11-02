// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file   BenchmarkTask.cxx
///

#include <string>

#include <TCanvas.h>
#include <TH1.h>
#include <TArrayD.h>

#include "QualityControl/QcInfoLogger.h"
#include "Benchmark/BenchmarkTask.h"
#include "Benchmark/SpinSleep.h"

namespace o2::quality_control_modules::benchmark
{

BenchmarkTask::~BenchmarkTask()
{
  if (mHistogram) {
    delete mHistogram;
  }
}

void BenchmarkTask::initialize(o2::framework::InitContext& /*ctx*/)
{
  QcInfoLogger::GetInstance() << "initialize BenchmarkTask" << AliceO2::InfoLogger::InfoLogger::endm;

  // this is how to get access to custom parameters defined in the config file at qc.tasks.<task_name>.taskParameters
  if (auto param = mCustomParameters.find("myOwnKey"); param != mCustomParameters.end()) {
    QcInfoLogger::GetInstance() << "Custom parameter - myOwnKey : " << param->second << AliceO2::InfoLogger::InfoLogger::endm;
  }

  mHistogram = new TH1F("example", "example", 20, 0, 30000);
  getObjectsManager()->startPublishing(mHistogram);
  getObjectsManager()->addMetadata(mHistogram->GetName(), "custom", "34");
}

void BenchmarkTask::startOfActivity(Activity& /*activity*/)
{
  if (mCustomParameters.count("time")) {
    mDuration = std::stol(mCustomParameters["time"]);
    LOG(INFO) << " ------> Time: " << mCustomParameters["time"];
  } else {
    LOG(INFO) << " ------> Time: NOTIME";
  }
  QcInfoLogger::GetInstance() << "startOfActivity" << AliceO2::InfoLogger::InfoLogger::endm;
  mHistogram->Reset();
}

void BenchmarkTask::startOfCycle()
{
  QcInfoLogger::GetInstance() << "startOfCycle" << AliceO2::InfoLogger::InfoLogger::endm;
}

void BenchmarkTask::monitorData(o2::framework::ProcessingContext& ctx)
{
  // In this function you can access data inputs specified in the JSON config file, for example:
  //   "query": "random:ITS/RAWDATA/0"
  // which is correspondingly <binding>:<dataOrigin>/<dataDescription>/<subSpecification
  // One can also access conditions from CCDB, via separate API (see point 3)

  // Use Framework/DataRefUtils.h or Framework/InputRecord.h to access and unpack inputs (both are documented)
  // One can find additional examples at:
  // https://github.com/AliceO2Group/AliceO2/blob/dev/Framework/Core/README.md#using-inputs---the-inputrecord-api

  // Some examples:

  // 1. In a loop
  /*
  for (auto&& input : ctx.inputs()) {
    // get message header
    if (input.header != nullptr && input.payload != nullptr) {
      const auto* header = header::get<header::DataHeader*>(input.header);
      // get payload of a specific input, which is a char array.
      // const char* payload = input.payload;

      // for the sake of an example, let's fill the histogram with payload sizes
      mHistogram->Fill(header->payloadSize);
    }
  }
  */

  // 2. Using get("<binding>")

  // get the payload of a specific input, which is a char array. "random" is the binding specified in the config file.
   auto payload = ctx.inputs().get<TArrayD*>("random");
   for (int i = 0; i<payload->GetSize(); ++i) {
    mHistogram->Fill((*payload)[i]);
   }

  // get payload of a specific input, which is a structure array:
  //  const auto* header = header::get<header::DataHeader*>(ctx.inputs().get("random").header);
  //  struct s {int a; double b;};
  //  auto array = ctx.inputs().get<s*>("random");
  //  for (int j = 0; j < header->payloadSize / sizeof(s); ++j) {
  //    int i = array.get()[j].a;
  //  }

  // get payload of a specific input, which is a root object
  //   auto h = ctx.inputs().get<TH1F*>("histos");
  //   Double_t stats[4];
  //   h->GetStats(stats);
  //   auto s = ctx.inputs().get<TObjString*>("string");
  //   LOG(INFO) << "String is " << s->GetString().Data();

  // 3. Access CCDB. If it is enough to retrieve it once, do it in initialize().
  // Remember to delete the object when the pointer goes out of scope or it is no longer needed.
  //   TObject* condition = TaskInterface::retrieveCondition("QcTask/example"); // put a valid condition path here
  //   if (condition) {
  //     LOG(INFO) << "Retrieved " << condition->ClassName();
  //     delete condition;
  //   }
  
  spinsleep(mDuration);
}

void BenchmarkTask::endOfCycle()
{
  QcInfoLogger::GetInstance() << "endOfCycle" << AliceO2::InfoLogger::InfoLogger::endm;
}

void BenchmarkTask::endOfActivity(Activity& /*activity*/)
{
  QcInfoLogger::GetInstance() << "endOfActivity" << AliceO2::InfoLogger::InfoLogger::endm;
}

void BenchmarkTask::reset()
{
  // clean all the monitor objects here

  QcInfoLogger::GetInstance() << "Resetting the histogram" << AliceO2::InfoLogger::InfoLogger::endm;
  mHistogram->Reset();
}

} // namespace o2::quality_control_modules::benchmark