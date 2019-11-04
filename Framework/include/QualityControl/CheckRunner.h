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
/// \file   CheckRunner.h
/// \author Barthelemy von Haller
/// \author Piotr Konopka
///

#ifndef QC_CHECKER_CHECKER_H
#define QC_CHECKER_CHECKER_H

// std & boost
#include <chrono>
#include <memory>
#include <string>
#include <map>
#include <vector>
// O2
#include <Common/Timer.h>
#include <Framework/Task.h>
#include <Headers/DataHeader.h>
#include <Monitoring/MonitoringFactory.h>
#include <Framework/DataProcessorSpec.h>
#include <Configuration/ConfigurationFactory.h>
// QC
#include "QualityControl/CheckInterface.h"
#include "QualityControl/DatabaseInterface.h"
#include "QualityControl/MonitorObject.h"
#include "QualityControl/QcInfoLogger.h"
#include "QualityControl/Check.h"

namespace o2::framework
{
struct InputSpec;
struct OutputSpec;
class DataAllocator;
} // namespace o2::framework

namespace o2::monitoring
{
class Monitoring;
}

class TClass;

namespace o2::quality_control::checker
{

/// \brief The class in charge of running the checks on a MonitorObject.
///
/// A CheckRunner is in charge of loading/instantiating the proper checks for a given MonitorObject, to configure them
/// and to run them on the MonitorObject in order to generate a quality. At the moment, a checker also stores quality in the repository.
///
/// TODO Evaluate whether we should have a dedicated device to store in the database.
///
/// \author Barthélémy von Haller
class CheckRunner : public framework::Task
{
 public:
  /// Constructor
  /**
   * \brief CheckRunner constructor
   *
   * Create CheckRunner device that will perform check operation with defineds checks.
   * Depending on the constructor, it can be a single check device or a group check device.
   * Group check assumes that the input of the checks is the same!
   *
   * @param checkName Check name from the configuration
   * @param checkNames List of check names, that operate on the same inputs.
   * @param configurationSource Path to configuration
   */
  CheckRunner(Check check, std::string configurationSource);
  CheckRunner(std::vector<Check> checks, std::string configurationSource);

  /// Destructor
  ~CheckRunner() override;

  /// \brief CheckRunner init callback
  void init(framework::InitContext& ctx) override;

  /// \brief CheckRunner process callback
  void run(framework::ProcessingContext& ctx) override;

  framework::Inputs getInputs() { return mInputs; };
  framework::Outputs getOutputs() { return mOutputs; };

  /// \brief Unified DataDescription naming scheme for all checkers
  static o2::header::DataDescription createCheckRunnerDataDescription(const std::string taskName);
  static o2::framework::Inputs createInputSpec(const std::string checkName, const std::string configSource);

  std::string getDeviceName() { return mDeviceName; };
  static std::string createCheckRunnerIdString() { return "QC-CHECK-RUNNER"; };
  static std::string createCheckRunnerName(std::vector<Check> checks);

 private:
  /**
   * \brief Evaluate the quality of a MonitorObject.
   *
   * The Check's associated with this MonitorObject are run and a global quality is built by
   * taking the worse quality encountered. The MonitorObject is modified by setting its quality
   * and by calling the "beautifying" methods of the Check's.
   *
   * @param mo The MonitorObject to evaluate and whose quality will be set according
   *        to the worse quality encountered while running the Check's.
   */
  std::vector<Check*> check(std::map<std::string, std::shared_ptr<MonitorObject>> moMap);

  /**
   * \brief Store the MonitorObject in the database.
   *
   * @param mo The MonitorObject to be stored in the database.
   */
  void store(std::vector<Check*>& checks);

  /**
   * \brief Send the MonitorObject on FairMQ to whoever is listening.
   */
  void send(std::vector<Check*>& checks, framework::DataAllocator& allocator);

  /**
   * \brief Load a library.
   * Load a library if it is not already in the cache.
   * \param libraryName The name of the library to load.
   */
  //void loadLibrary(const std::string libraryName);

  /**
   * \brief Update cached monitor object with new one.
   *
   * \param mo The MonitorObject to be updated
   */
  void update(std::shared_ptr<MonitorObject> mo);

  /**
   * \brief Collect input specs from Checks
   *
   * \param checks List of all checks
   */
  static o2::framework::Outputs collectOutputs(const std::vector<Check>& checks);

  inline void initDatabase();
  inline void initMonitoring();

  void updateRevision();

  /**
   * Get the check specified by its name and class.
   * If it has never been asked for before it is instantiated and cached. There can be several copies
   * of the same check but with different names in order to have them configured differently.
   * @todo Pass the name of the task that will use it. It will help with getting the correct configuration.
   * @param checkName
   * @param className
   * @return the check object
   */
  //CheckInterface* getCheck(std::string checkName, std::string className);

  // General state
  std::string mDeviceName;
  std::vector<Check> mChecks;
  std::string mConfigurationSource;
  o2::quality_control::core::QcInfoLogger& mLogger;
  std::shared_ptr<o2::quality_control::repository::DatabaseInterface> mDatabase;
  std::shared_ptr<o2::configuration::ConfigurationInterface> mConfigFile; // used in init only
  std::map<std::string, unsigned int> mMonitorObjectRevision;
  unsigned int mGlobalRevision;

  // DPL
  o2::framework::Inputs mInputs;
  o2::framework::Outputs mOutputs;

  // Checks cache
  std::vector<std::string> mCheckList;
  std::vector<std::string> mLibrariesLoaded;
  std::map<std::string, CheckInterface*> mChecksLoaded;
  std::map<std::string, TClass*> mClassesLoaded;
  std::map<std::string, std::shared_ptr<MonitorObject>> mMonitorObjects;
  std::map<std::string, std::shared_ptr<QualityObject>> mQualityObjects;
  std::map<std::string, CheckInterface*> mCheckInreface;

  // monitoring
  std::shared_ptr<o2::monitoring::Monitoring> mCollector;
  std::chrono::system_clock::time_point startFirstObject;
  std::chrono::system_clock::time_point endLastObject;
  uint64_t mTotalObjectsReceived = 0, mTotalObjectsPublished = 0, mTotalCalls = 0;
  std::chrono::duration<double> mRunDuration, mStoreDuration, mCheckDuration;
  AliceO2::Common::Timer timer;
};

} // namespace o2::quality_control::checker

#endif // QC_CHECKER_CHECKER_H
