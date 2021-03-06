AliAnalysisTask *AddTaskJPSIFilter_pp(TString cfg="ConfigJpsi_nano_pp.C",
				      Bool_t gridconf=kFALSE,
				      ULong64_t triggers=AliVEvent::kCentral | AliVEvent::kSemiCentral | AliVEvent::kMB,
				      TString period="",
				      Bool_t storeLS = kFALSE,
				      Bool_t hasMC_aod = kFALSE){
  //get the current analysis manager
  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if (!mgr) {
    Error("AddTaskJPSIFilter", "No analysis manager found.");
    return 0;
  }
  
  //check for output aod handler
  if (!mgr->GetOutputEventHandler()||mgr->GetOutputEventHandler()->IsA()!=AliAODHandler::Class()) {
    Warning("AddTaskJPSIFilter","No AOD output handler available. Not adding the task!");
    return 0;
  }

  //Do we have an MC handler?
  Bool_t hasMC=(AliAnalysisManager::GetAnalysisManager()->GetMCtruthEventHandler()!=0x0)||hasMC_aod;
  
  //Do we run on AOD?
  Bool_t isAOD=mgr->GetInputEventHandler()->IsA()==AliAODInputHandler::Class();

  //Allow merging of the filtered aods on grid trains
  if(mgr->GetGridHandler()) {
    printf(" SET MERGE FILTERED AODs \n");
    //mgr->GetGridHandler()->SetMergeAOD(kTRUE);
  }

  //set config file name
  TString configFile("");
  printf("%s \n",gSystem->pwd());
  TString trainRoot=gSystem->Getenv("TRAIN_ROOT");
  if(cfg.IsNull()) cfg="ConfigJpsi_jb_PbPb.C";

  // the different paths
  TString alienPath("alien:///alice/cern.ch/user/j/jbook/PWGDQ/dielectron/macrosJPSI");
  TString alirootPath("$ALICE_PHYSICS/PWGDQ/dielectron/macrosJPSI/");

  ////////// >>>>>>>>>> alien config
  if(gridconf && !gSystem->Exec(Form("alien_cp %s/%s .",alienPath.Data(),cfg.Data()))) {
    gSystem->Exec(Form("ls -l %s",gSystem->pwd()));
    configFile=gSystem->pwd();
  }
  else {
    printf("ERROR: couldn't copy file %s/%s from grid \n", alienPath.Data(),cfg.Data() );
    return;
  }
  ///////// >>>>>>>>> aliroot config
  if(!gridconf) configFile=alirootPath.Data();
  ///////// add config to path
  configFile+="/";
  configFile+=cfg.Data();


  //gROOT->LoadMacro("$ALICE_PHYSICS/PWGDQ/dielectron/macros/ConfigBJpsi_ff_PbPbFilter.C");
  //  gROOT->LoadMacro("$ALICE_PHYSICS/PWGDQ/dielectron/macrosJPSI/ConfigBJpsi_ff_PbPbFilter.C");
  //  gROOT->LoadMacro("$ALICE_PHYSICS/PWGDQ/dielectron/macrosJPSI/ConfigJpsi_nano_PbPb.C");

  //load dielectron configuration file (only once)
  //  TString checkconfig="ConfigJpsi_jb_PbPb";
  if (!gROOT->GetListOfGlobalFunctions()->FindObject(cfg.Data()))
    gROOT->LoadMacro(configFile.Data());

  AliDielectron *jpsi=ConfigJpsi_nano_pp(0,hasMC,period);
  
  if(isAOD) {
    //add options to AliAODHandler to duplicate input event
    AliAODHandler *aodHandler = (AliAODHandler*)mgr->GetOutputEventHandler();
    aodHandler->SetCreateNonStandardAOD();
    aodHandler->SetNeedsHeaderReplication();
    if(!period.Contains("LHC10h")) aodHandler->SetNeedsTOFHeaderReplication();
    aodHandler->SetNeedsVZEROReplication();
    /*aodHandler->SetNeedsTracksBranchReplication();
    aodHandler->SetNeedsCaloClustersBranchReplication();
    aodHandler->SetNeedsVerticesBranchReplication();
    aodHandler->SetNeedsCascadesBranchReplication();
    aodHandler->SetNeedsTrackletsBranchReplication();
    aodHandler->SetNeedsPMDClustersBranchReplication();
    aodHandler->SetNeedsJetsBranchReplication();
    aodHandler->SetNeedsFMDClustersBranchReplication();
    //aodHandler->SetNeedsMCParticlesBranchReplication();
    aodHandler->SetNeedsDimuonsBranchReplication();*/
    //    if(hasMC) aodHandler->SetNeedsV0sBranchReplication();
    if(hasMC) aodHandler->SetNeedsMCParticlesBranchReplication();
    jpsi->SetHasMC(hasMC);
  }
  
  //Create task and add it to the analysis manager
  AliAnalysisTaskDielectronFilter *task=new AliAnalysisTaskDielectronFilter("jpsi_DielectronFilter");
  task->SetTriggerMask(triggers);
  //  task->SetTriggerMask(AliVEvent::kMB+AliVEvent::kCentral+AliVEvent::kSemiCentral+AliVEvent::kEMCEGA+AliVEvent::kEMCEJE);
  //  task->SetTriggerMask(AliVEvent::kMB+AliVEvent::kCentral+AliVEvent::kSemiCentral);
  if (!hasMC) task->UsePhysicsSelection();

  //   //Add event filter
  //   AliDielectronEventCuts *eventCuts=new AliDielectronEventCuts("eventCuts","Vertex Track && |vtxZ|<10 && ncontrib>0");
  //   if(!hasMC) eventCuts->SetRequireVertex();
  //   if (isAOD) eventCuts->SetVertexType(AliDielectronEventCuts::kVtxAny);
  //   eventCuts->SetMinVtxContributors(1);
  //   eventCuts->SetVertexZ(-10.,10.);
  //   eventCuts->SetCentralityRange(0.0,90.0);
  //   task->SetEventFilter(eventCuts);

  task->SetDielectron(jpsi);
  if(storeLS) task->SetStoreLikeSignCandidates(storeLS);
  task->SetCreateNanoAODs(kTRUE);
  task->SetStoreEventsWithSingleTracks(kTRUE);
  //task->SetStoreHeader(kTRUE);
  mgr->AddTask(task);

  //----------------------
  //create data containers
  //----------------------
  
  
  TString containerName = mgr->GetCommonFileName();
  containerName += ":PWGDQ_dielectronFilter";
 
  //create output container
  
  AliAnalysisDataContainer *cOutputHist1 =
    mgr->CreateContainer("jpsi_FilterQA",
                         THashList::Class(),
                         AliAnalysisManager::kOutputContainer,
                         containerName.Data());
  
  AliAnalysisDataContainer *cOutputHist2 =
    mgr->CreateContainer("jpsi_FilterEventStat",
                         TH1D::Class(),
                         AliAnalysisManager::kOutputContainer,
                         containerName.Data());
  
  
  mgr->ConnectInput(task,  0, mgr->GetCommonInputContainer());
  mgr->ConnectOutput(task, 0, mgr->GetCommonOutputContainer());
  mgr->ConnectOutput(task, 1, cOutputHist1);
  mgr->ConnectOutput(task, 2, cOutputHist2);
  
  return task;
}
