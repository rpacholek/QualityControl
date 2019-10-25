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

using namespace std;

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

  for (auto& [moName, mo] : *moMap) {

    (void)moName;
    if (mo->getName() == "example") {
      auto* h = dynamic_cast<TH1F*>(mo->getObject());

      result = Quality::Good;

      for (int i = 0; i < h->GetNbinsX(); i++) {
        if (i > 0 && i < 8 && h->GetBinContent(i) == 0) {
          result = Quality::Bad;
          break;
        } else if ((i == 0 || i > 7) && h->GetBinContent(i) > 0) {
          result = Quality::Medium;
        }
      }
    }
  }
  
  spinsleep(mDuration);

  return result;
}

std::string BenchmarkCheck::getAcceptedType() { return "TH1"; }

void BenchmarkCheck::beautify(std::shared_ptr<MonitorObject> mo, Quality checkResult)
{
  if (mo->getName() == "example") {
    auto* h = dynamic_cast<TH1F*>(mo->getObject());

    if (checkResult == Quality::Good) {
      h->SetFillColor(kGreen);
    } else if (checkResult == Quality::Bad) {
      LOG(INFO) << "Quality::Bad, setting to red";
      h->SetFillColor(kRed);
    } else if (checkResult == Quality::Medium) {
      LOG(INFO) << "Quality::medium, setting to orange";
      h->SetFillColor(kOrange);
    }
    h->SetLineColor(kBlack);
  }
}

} // namespace o2::quality_control_modules::benchmark
