#ifndef ALIANALYSISTASKTRACKINGEFFPID
#define ALIANALYSISTASKTRACKINGEFFPID

/* Copyright(c) 1998-2020, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//*************************************************************************
// Class AliAnalysisTaskTrackingEffPID
// AliAnalysisTaskSE to compute tracking and PID efficiencies for 
//  different particle species
//
// Authors:
//          M. Puccio
//          F. Prino
//          
//*************************************************************************


#include "AliESDtrackCuts.h"
#include "AliAnalysisTaskSE.h"
#include "AliPID.h"
#include "AliEventCuts.h"
#include <THnSparse.h>

class TList;

class AliAnalysisTaskTrackingEffPID : public AliAnalysisTaskSE {
public:
  AliAnalysisTaskTrackingEffPID();
  virtual ~AliAnalysisTaskTrackingEffPID();

  static bool  HasTOF(AliVTrack *t);
  bool ConvertAndSelectAODTrack(AliAODTrack* aTrack, const AliESDVertex vESD, Double_t magField);

  virtual void   UserCreateOutputObjects();
  virtual void   UserExec(Option_t *);
  virtual void   Terminate(Option_t *);

  void SetTrackCuts(AliESDtrackCuts* cuts){
    if(fTrackCuts) delete fTrackCuts;
    fTrackCuts = new AliESDtrackCuts(*cuts);
  }
  AliESDtrackCuts* GetTrackCuts() const{
    return fTrackCuts;
  }
  void SetCollisionSystem(TString collsy){
    collsy.ToLower();
    collsy.ReplaceAll("-","");
    if(collsy=="pbpb" || collsy=="xexe") fIsAA=kTRUE;
    else fIsAA=kFALSE;
  }
  void SetFilterBitCutForAODTracks(int fb){
    fFilterBit=fb;
  }
  void UseOnlyFilterBitCutForAODTracks(){
    fUseTrackCutsForAOD=kFALSE;    
  }
  void UseTrackCutObjectForAODTracks(){
    fUseTrackCutsForAOD=kTRUE;    
  }

  void SetUseGeneratedKine(bool flag) {fUseGeneratedKine=flag;}

  void SetTriggerMask(ULong64_t mask){
    fEventCut.OverrideAutomaticTriggerSelection(mask);
  }
  void SetUseSPDPileup(bool multDep, int nContrCut=5, double dzCut=0.8){
    fEventCut.OverridePileUpCuts(nContrCut,dzCut,3.,2.,5.,kTRUE);
    fEventCut.fUseMultiplicityDependentPileUpCuts=multDep;
  }
  void SetUseMVPileup(bool flag) {fEventCut.fPileUpCutMV=flag;}

  AliEventCuts  fEventCut;


private:
  AliAnalysisTaskTrackingEffPID (const AliAnalysisTaskTrackingEffPID &source);
  AliAnalysisTaskTrackingEffPID &operator=(const AliAnalysisTaskTrackingEffPID &source);

  bool fUseTrackCutsForAOD;       /// flag to switch off/on fTrackCuts for AOD
  bool fUseGeneratedKine;         /// flag to use the generated pt, eta phi
  bool fIsAA;                     /// flag to control collision system
  int  fFilterBit;                /// filter-bit selection for AOD tracks
  AliESDtrackCuts* fTrackCuts;                            /// cut object

  TList* fOutputList;                                     //!<! Output list
  TH1F*  fHistNEvents;                                    //!<!  histo with N of events  
  THnSparseF* fGenerated[AliPID::kSPECIESC][2];           //!<! Generated particles (pt, eta, phi, mult, zvert)
  THnSparseF* fGeneratedEvSel[AliPID::kSPECIESC][2];      //!<! Generated particles after event selection
  THnSparseF* fReconstructed[AliPID::kSPECIESC][2];       //!<! Reconstructed particles (pt, eta, phi, mult, zvert)
  THnSparseF* fReconstructedTOF[AliPID::kSPECIESC][2];    //!<! Reconstructed particles after PID (pt, eta, phi, mult, zvert)
  THnSparseF* fReconstructedPID[AliPID::kSPECIESC][2];    //!<! Reconstructed particles after PID (pt, eta, phi, mult, zvert)


  /// \cond CLASSDEF
  ClassDef(AliAnalysisTaskTrackingEffPID, 1);
  /// \endcond
};


#endif 
