// art framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "fhiclcpp/ParameterSet.h"

// data container includes
#include "nusimdata/SimulationBase/MCTruth.h"

// root includes
#include "TTree.h"

// c++ includes
#include <vector>
#include <iterator>
#include <typeinfo>
#include <memory>
#include <cmath>
#include <algorithm>
#include <string>
#include <iostream>

namespace nnbar {

/// LArSoft module to study deep learning topologies
class DLTopology : public art::EDAnalyzer {
    
public:

  explicit DLTopology(fhicl::ParameterSet const& pset);
  void beginJob();
  //void endJob();
  void analyze(art::Event const& evt);

private:

  void CreateTree();
  void InitializeBranches();
  void Clear();

  // Input tree
  TTree* fTree;

  // Config parameters
  bool fIsSignal;

  // Event ID
  int fRun;
  int fSubrun;
  int fEvent;

  // Signal variables
  int fMultiplicity;
  double fInvariantMass;
  double fMomentum;

  // Background variables
  bool fNC;
  int fInteractionType;
  double fNuEnergy;
  double fQSqr;
  int fNuPdg;
  double fLepEnergy;

}; // class DLTopology

DLTopology::DLTopology(fhicl::ParameterSet const& pset) :
    EDAnalyzer(pset),
    fTree(nullptr),
    fIsSignal(pset.get<bool>("IsSignal"))
{} // function DLTopology::DLTopology

void DLTopology::CreateTree() {

  if (!fTree) {
    art::ServiceHandle<art::TFileService> tfs;
    fTree = tfs->make<TTree>("dl_tree","dl_tree");
  }
} // function DLTopology::CreateTree

void DLTopology::InitializeBranches() {

  fTree->Branch("Run",&fRun,"Run/I");
  fTree->Branch("Subrun",&fSubrun,"Subrun/I");
  fTree->Branch("Event",&fEvent,"Event/I");

  if (fIsSignal) {
    fTree->Branch("Multiplicity",&fMultiplicity,"Multiplicity/I");
    fTree->Branch("InvariantMass",&fInvariantMass,"InvariantMass/D");
    fTree->Branch("Momentum",&fMomentum,"Momentum/D");
  }
  else {
    fTree->Branch("NC",&fNC,"NC/B");
    fTree->Branch("InteractionType",&fInteractionType,"InteractionType/I");
    fTree->Branch("NuEnergy",&fNuEnergy,"NuEnergy/D");
    fTree->Branch("QSqr",&fQSqr,"QSqr/D");
    fTree->Branch("NuPdg",&fNuPdg,"NuPdg/I");
    fTree->Branch("LepEnergy",&fLepEnergy,"LepEnergy/D");
  }

} // function DLTopology::InitializeBranches

void DLTopology::Clear() {

  // Clear tree variables at start of event
  fRun = 0;
  fSubrun = 0;
  fEvent = 0;

  if (fIsSignal) {
    fMultiplicity = 0;
    fInvariantMass = 0;
    fMomentum = 0;
  }
  else {
    fNC = 0;
    fInteractionType = 0;
    fNuEnergy = 0;
    fQSqr = 0;
    fNuPdg = 0;
    fLepEnergy = 0;
  }

} // function DLTopology::Clear

void DLTopology::beginJob() {

  CreateTree();
  InitializeBranches();

} // function DLTopology::beginJob

void DLTopology::analyze(art::Event const& evt) {

  Clear();

  fRun = (int) evt.id().run();
  fSubrun = (int) evt.id().subRun();
  fEvent = (int) evt.id().event();

  // MC truth information
  art::Handle<std::vector<simb::MCTruth>> TruthListHandle;
  std::vector<art::Ptr<simb::MCTruth>> TruthList;
  if (evt.getByLabel("generator",TruthListHandle))
    art::fill_ptr_vector(TruthList,TruthListHandle);
  art::Ptr<simb::MCTruth> mct = TruthList[0];

  if (fIsSignal) {
    fMultiplicity = 0;
    double px = 0;
    double py = 0;
    double pz = 0;
    double e = 0;
    for (int it = 0; it < mct->NParticles(); ++it) {
      simb::MCParticle part = mct->GetParticle(it);
      if (part.StatusCode() == 1 && (abs(part.PdgCode()) == 211 || part.PdgCode() == 111)) {
        ++fMultiplicity;
        px += part.Px();
        py += part.Py();
        pz += part.Pz();
        e += part.E();
      }
    }
    fMomentum = sqrt(pow(px,2)+pow(py,2)+pow(pz,2));
    fInvariantMass = sqrt(pow(e,2)-pow(fMomentum,2));
  }
  else {
    fNC = (bool) mct->GetNeutrino().CCNC();
    fInteractionType = mct->GetNeutrino().InteractionType();
    fNuEnergy = mct->GetNeutrino().Nu().E();
    fQSqr = mct->GetNeutrino().QSqr();
    fNuPdg = mct->GetNeutrino().Nu().PdgCode();
    fLepEnergy = mct->GetNeutrino().Lepton().E();
  }

  // Fill event tree
  fTree->Fill();

} // function DLTopology::analyze

DEFINE_ART_MODULE(DLTopology)

} // namespace nnbar

