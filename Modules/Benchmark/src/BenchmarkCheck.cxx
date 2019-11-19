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
/// \file   BenchmarkCheck.cxx
/// \author Piotr Konopka
///

#include "Benchmark/BenchmarkCheck.h"
#include "Benchmark/SpinSleep.h"
#include "QualityControl/MonitorObject.h"
#include "QualityControl/Quality.h"

#include <fairlogger/Logger.h>
// ROOT
#include <TH1.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

namespace o2::quality_control_modules::benchmark
{

void BenchmarkCheck::configure(std::string) {
  if (mCustomParameters.count("time")) {
    mDuration = std::stol(mCustomParameters["time"]);
    LOG(INFO) << " ------> Time: " << mCustomParameters["time"];
  } else {
    LOG(INFO) << " ------> Time: NOTIME";
  }
}

Quality BenchmarkCheck::check(std::map<std::string, std::shared_ptr<MonitorObject>>* moMap)
{
  Quality result = Quality::Null;
  spinsleep(mDuration);
  if (moMap->count("QcTask/counter")){
	auto* h = dynamic_cast<TH1F*>((*moMap)["QcTask/counter"]->getObject());
	LOG(INFO) << "Check counter: " << h->GetEntries() << " Time: " << duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
  }

  return result;
}

std::string BenchmarkCheck::getAcceptedType() { return "TH1"; }

void BenchmarkCheck::beautify(std::shared_ptr<MonitorObject> mo, Quality checkResult)
{
  if (mo->getName() == "example") {
    spinsleep(mDuration);
  }
}

} // namespace o2::quality_control_modules::benchmark
