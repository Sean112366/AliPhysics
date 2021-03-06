/**************************************************************************
 * Copyright(c) 2013-2014, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/
#ifdef __ECLIPSE_IDE
//  few includes and external declarations just for the IDE
#include "Riostream.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TChain.h"
#include "AliAnalysisTaskSE.h"
#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"
#include "AliAODInputHandler.h"
#include "AliAnalysisTaskPIDResponse.h"
#include "AliAnalysisTaskPIDqa.h"
#include "AliPhysicsSelectionTask.h"
#include "AliCentralitySelectionTask.h"
#include "AliTaskCDBconnect.h"
#include "AliAnalysisTaskPIDCombined.h"
#include "AliESDInputHandler.h"
#include "AliMCEventHandler.h"
#include "AliMultSelectionTask.h"
extern AliAnalysisGrid* CreateAlienHandler(const char *, Bool_t);
extern AliCentralitySelectionTask *AddTaskCentrality(Bool_t fillHistos=kTRUE, Bool_t aod=kFALSE);
extern AliAnalysisTask *AddTaskPIDResponse(Bool_t, Bool_t,Bool_t,TString);
//extern AliAnalysisTask *AddTaskPIDqa(const char *);
extern TChain* CreateESDChain(
  const char* aDataDir = "ESDfiles.txt",
  Int_t aRuns          = 20,
  Int_t offset         = 0,
  Bool_t addFileName   = kFALSE,
  Bool_t addFriend     = kFALSE,
  const char* check    = 0);
extern TChain* CreateAODChain(
    const char* aDataDir = "AODfiles.txt",
    Int_t aRuns          = 20,
    Int_t offset         = 0,
    Bool_t addFileName   = kFALSE,
    const char* friends  = "",
    const char* check    = 0);
AliPhysicsSelectionTask* AddTaskPhysicsSelection(
    Bool_t mCAnalysisFlag = kFALSE,
    Bool_t deprecatedFlag = kTRUE,
    UInt_t computeBG = 0,
    Bool_t useSpecialOutput=kFALSE);
AliMultSelectionTask *AddTaskMultSelection(
    Bool_t lCalibration = kFALSE,
    TString lExtraOptions = "",
    Int_t lNDebugEstimators = 1,
    const TString lMasterJobSessionFlag = "");
extern AliAnalysisTaskSE *AddCorrelationsStudiesTask(const char *, const char *, const char *, const char *);

#include "runCorrelationsStudiesConfigMacro.H"
#endif // ifdef __ECLIPSE_IDE declaration and includes for the ECLIPSE IDE

void runCorrelationsStudiesTask(const char *sRunMode = "full", Bool_t gridMerge = kTRUE) {

  gSystem->AddIncludePath("-I$ALICE_PHYSICS/include");
  gSystem->AddIncludePath("-I$ALICE_ROOT/include");

  gROOT->LoadMacro("$ALICE_PHYSICS/PWGCF/Correlations/macros/Unfoldedhistos/runCorrelationsStudiesConfigMacro.H");
  gROOT->LoadMacro("$ALICE_PHYSICS/PWGCF/Correlations/macros/Unfoldedhistos/runCorrelationsStudiesConfigMacro.C");
  runCorrelationsStudiesConfigMacro();

  AliAnalysisGrid       *alienHandler   =   NULL;
  AliAnalysisManager    *mgr;

  if (!bTrainScope) {
    mgr = new AliAnalysisManager("Correlation studies");

    // Enable debug printouts
    mgr->SetDebugLevel(AliLog::kInfo);
    AliLog::SetGlobalLogLevel(AliLog::kInfo);
  }
  else {
    mgr = AliAnalysisManager::GetAnalysisManager();
  }


  if (!bTrainScope) {
    /* we only do this outside trains scope */
    if (bUseESD) {
      AliESDInputHandler* esdH = new AliESDInputHandler();
      /* taken from fast MC trains */
      if (bMConlyTruth) {
        esdH->SetReadFriends(kFALSE);
        esdH->SetNeedField();
      }
      mgr->SetInputEventHandler(esdH);
    }
    else if (bUseAOD) {
      AliAODInputHandler* aodH = new AliAODInputHandler();
      mgr->SetInputEventHandler(aodH);
    }
    else {
      cout << "Neither AOD nor ESD data specified. ABORTING!!!" << endl;
      return;
    }

    if (bMC && !bUseAOD){
        AliMCEventHandler* mcH = new AliMCEventHandler();
        /* taken from fast MC trains */
        if (bMConlyTruth) {
          mcH->SetReadTR(kFALSE);
          mcH->SetPreReadMode(AliMCEventHandler::kLmPreRead);
        }
        mgr->SetMCtruthEventHandler(mcH);
    }

    if (bGRIDPlugin) {
      gROOT->LoadMacro("$ALICE_PHYSICS/PWGCF/Correlations/macros/Unfoldedhistos/CreateAlienHandler.C");
      alienHandler = CreateAlienHandler(sRunMode,gridMerge);
      if (!alienHandler) return;

      mgr->SetGridHandler(alienHandler);
    }

    if (bUsePIDResponse ) {
      gROOT->LoadMacro("$ALICE_ROOT/ANALYSIS/macros/AddTaskPIDResponse.C");
      AddTaskPIDResponse(bMC,kTRUE,kTRUE,szpass);
    }

    if (bUseESD) {
      if (bUsePhysicsSelection) {
        gROOT->LoadMacro("$ALICE_PHYSICS/OADB/macros/AddTaskPhysicsSelection.C");
        AddTaskPhysicsSelection(bMC);
      }

      if (bUseMultiplicityTask) {
        gROOT->LoadMacro("$ALICE_PHYSICS/OADB/COMMON/MULTIPLICITY/macros/AddTaskMultSelection.C");
        AddTaskMultSelection(kFALSE); // user mode:
      }

      if (bUseCentralityTask) {
        gROOT->LoadMacro("$ALICE_PHYSICS/OADB/macros/AddTaskCentrality.C");
        AliCentralitySelectionTask *taskCentrality = AddTaskCentrality();
        if (bMC) {
          taskCentrality->SetMCInput();
        }
      }
    }
    /* this ends what we do outside the trains scope */
  }

  gROOT->LoadMacro("$ALICE_PHYSICS/PWGCF/Correlations/macros/Unfoldedhistos/AddCorrelationsStudiesTask.C");

  /* load the configurations and start the corresponding tasks */
  ifstream configfile;
  configfile.open("configuration.txt");
  while (!configfile.eof()) {
    /* we need four configuration lines per task */
    TString line;
    TString firstline;
    TString secondline;
    TString thirdline;
    TString fourthline;
    TString fifthline;

    line.ReadLine(configfile);
    while ((line.BeginsWith("#") || line.IsWhitespace()) && !configfile.eof()) line.ReadLine(configfile);
    firstline = line;
    if (configfile.eof()) continue;

    line.ReadLine(configfile);
    while ((line.BeginsWith("#") || line.IsWhitespace()) && !configfile.eof()) line.ReadLine(configfile);
    secondline = line;

    line.ReadLine(configfile);
    while ((line.BeginsWith("#") || line.IsWhitespace()) && !configfile.eof()) line.ReadLine(configfile);
    thirdline = line;

    line.ReadLine(configfile);
    while ((line.BeginsWith("#") || line.IsWhitespace()) && !configfile.eof()) line.ReadLine(configfile);
    fourthline = line;

    line.ReadLine(configfile);
    while ((line.BeginsWith("#") || line.IsWhitespace()) && !configfile.eof()) line.ReadLine(configfile);
    fifthline = line;

    if (firstline.Length() && secondline.Length() && thirdline.Length() && fourthline.Length() && fifthline.Length())
      AddCorrelationsStudiesTask(firstline.Data(),secondline.Data(),thirdline.Data(),fourthline.Data(),fifthline.Data());
    else {
      cout << "Wrong configuration file. ABORTING!!!" << endl;
      return;
    }
  }

  TChain* chain = 0;

  if (!bTrainScope) {
    /* we only do this outside trains scope */
    if (!bGRIDPlugin) {
      Int_t numFiles = 0;
      ifstream dataInStream;
      dataInStream.open(szLocalFileList.Data());
      if ( !dataInStream ) {
        cout<<"Data list file does not exist: " << szLocalFileList.Data() << endl;
        return;
      }

      string datafileline;

      while ( !dataInStream.eof() ) {
        getline(dataInStream, datafileline);
        if(datafileline.compare("") != 0) {//checks if there is an empty line in the data list
          numFiles++;
        }
      }
      // No need to create a chain - this is handled by the plugin
      if (bUseESD) {
        gROOT->LoadMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/CreateESDChain.C");
        chain = CreateESDChain(szLocalFileList.Data(),numFiles);
      }
      else if (bUseAOD) {
        gROOT->LoadMacro("$ALICE_PHYSICS/PWG/EMCAL/macros/CreateAODChain.C");
        chain = CreateAODChain(szLocalFileList.Data(),numFiles);
      }
    }

    if (!mgr->InitAnalysis())
      return;

    mgr->PrintStatus();
    if (bGRIDPlugin){
      mgr->StartAnalysis("grid");
    }
    else{
      gSystem->SetFPEMask(0x0);
      mgr->StartAnalysis("local",chain);
    }
    /* this ends what we do outside the trains scope */
  }
}
