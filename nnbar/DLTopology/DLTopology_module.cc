// art framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "fhiclcpp/ParameterSet.h"

// data container includes
#include "nusimdata/SimulationBase/MCTruth.h"
#include "lardataobj/MCBase/MCTrack.h"
#include "lardataobj/MCBase/MCShower.h"
#include "lardataobj/RecoBase/Wire.h"
#include "larcore/Geometry/Geometry.h"

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
  bool FindObject(int primary_trackid, art::Handle<std::vector<sim::MCTrack>> TrackHandle, art::Handle<std::vector<sim::MCShower>> ShowerHandle);

  // Input tree
  TTree* fTree;

  // Config parameters
  bool fIsSignal;

  // Event ID
  int fRun;
  int fSubrun;
  int fEvent;

  // Event vertex
  double fVertexX;
  double fVertexY;
  double fVertexZ;

  // Containment
  bool fContained;

  // Kinematic variables
  int fPionMultiplicity;
  int fLeptonMultiplicity;
  int fNucleonMultiplicity;
  double fInvariantMass;
  double fMomentum;
  double fMomentumX;
  double fMomentumY;
  double fMomentumZ;

  // Background variables
  std::vector<int> fCosmicPrimaryPdg;
  std::vector<double> fCosmicPrimaryE;

  std::vector<int> fCosmicPrimaryInFVPdg;
  std::vector<double> fCosmicPrimaryInFVE;

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
    fTree->Branch("VertexX",&fVertexX,"VertexX/D");
    fTree->Branch("VertexY",&fVertexY,"VertexY/D");
    fTree->Branch("VertexZ",&fVertexZ,"VertexZ/D");

    fTree->Branch("Contained",&fContained,"Contained/B");

    fTree->Branch("PionMultiplicity",&fPionMultiplicity,"PionMultiplicity/I");
    fTree->Branch("LeptonMultiplicity",&fLeptonMultiplicity,"LeptonMultiplicity/I");
    fTree->Branch("NucleonMultiplicity",&fNucleonMultiplicity,"NucleonMultiplicity/I");
    fTree->Branch("InvariantMass",&fInvariantMass,"InvariantMass/D");
    fTree->Branch("Momentum",&fMomentum,"Momentum/D");
    fTree->Branch("MomentumX",&fMomentumX,"MomentumX/D");
    fTree->Branch("MomentumY",&fMomentumY,"MomentumY/D");
    fTree->Branch("MomentumZ",&fMomentumZ,"MomentumZ/D");
  }

  fTree->Branch("CosmicPrimaryPdg","std::vector<int>",&fCosmicPrimaryPdg);
  fTree->Branch("CosmicPrimaryE","std::vector<double>",&fCosmicPrimaryE);

  fTree->Branch("fCosmicPrimaryInFVPdg","std::vector<int>",&fCosmicPrimaryInFVPdg);
  fTree->Branch("fCosmicPrimaryInFVE","std::vector<double>",&fCosmicPrimaryInFVE);

} // function DLTopology::InitializeBranches

void DLTopology::Clear() {

  // Clear tree variables at start of event
  fRun = 0;
  fSubrun = 0;
  fEvent = 0;

  if (fIsSignal) {
    fVertexX = 0;
    fVertexY = 0;
    fVertexZ = 0;

    fPionMultiplicity = 0;
    fLeptonMultiplicity = 0;
    fNucleonMultiplicity = 0;
    fInvariantMass = 0;
    fMomentum = 0;
    fMomentumX = 0;
    fMomentumY = 0;
    fMomentumZ = 0;
  }

  fCosmicPrimaryPdg.clear();
  fCosmicPrimaryE.clear();

  fCosmicPrimaryInFVPdg.clear();
  fCosmicPrimaryInFVE.clear();

} // function DLTopology::Clear

bool DLTopology::FindObject(int primary_trackid, art::Handle<std::vector<sim::MCTrack>> TrackHandle, art::Handle<std::vector<sim::MCShower>> ShowerHandle) {

  // Check if the primary has a track
  for (std::vector<sim::MCTrack>::const_iterator it_track = TrackHandle->begin(); it_track != TrackHandle->end(); ++it_track) {
    const sim::MCTrack & track = *it_track;
    int trackid = track.TrackID();
    if (trackid == primary_trackid) return true;
  }

  // Check if the primary has a shower
  for (std::vector<sim::MCShower>::const_iterator it_shower = ShowerHandle->begin(); it_shower != ShowerHandle->end(); ++it_shower) {
    const sim::MCShower & shower = *it_shower;
    int showerid = shower.TrackID();
    if (showerid == primary_trackid) return true;
  }

  // Return false if no match found
  return false;

} // function DLTopology::FindObject

void DLTopology::beginJob() {

  CreateTree();
  InitializeBranches();

} // function DLTopology::beginJob

void DLTopology::analyze(art::Event const& evt) {

  Clear();

  fRun = (int) evt.id().run();
  fSubrun = (int) evt.id().subRun();
  fEvent = (int) evt.id().event();

  art::Ptr<simb::MCTruth> mct_nnbar;
  art::Ptr<simb::MCTruth> mct_cosmic;

  // MC truth information
  if (fIsSignal) {
    // nnbar mc truth
    art::Handle<std::vector<simb::MCTruth>> nnbarTruthListHandle;
    std::vector<art::Ptr<simb::MCTruth>> nnbarTruthList;
    if (evt.getByLabel("generator",nnbarTruthListHandle))
      art::fill_ptr_vector(nnbarTruthList,nnbarTruthListHandle);
    mct_nnbar = nnbarTruthList[0];
    // cosmic mc truth
    art::Handle<std::vector<simb::MCTruth>> cosmicTruthListHandle;
    std::vector<art::Ptr<simb::MCTruth>> cosmicTruthList;
    if (evt.getByLabel("cosmic",cosmicTruthListHandle))
      art::fill_ptr_vector(cosmicTruthList,cosmicTruthListHandle);
    mct_cosmic = cosmicTruthList[0];
  }
  else {
    // cosmic mc truth
    art::Handle<std::vector<simb::MCTruth>> cosmicTruthListHandle;
    std::vector<art::Ptr<simb::MCTruth>> cosmicTruthList;
    if (evt.getByLabel("generator",cosmicTruthListHandle))
      art::fill_ptr_vector(cosmicTruthList,cosmicTruthListHandle);
    mct_cosmic = cosmicTruthList[0];
  }

  // MC track information
  art::Handle<std::vector<sim::MCTrack>> TrackHandle;
  evt.getByLabel("mcreco",TrackHandle);

  // MC shower information
  art::Handle<std::vector<sim::MCShower>> ShowerHandle;
  evt.getByLabel("mcreco",ShowerHandle);

  // Wire information
  art::Handle<std::vector<recob::Wire>> WireHandle;
  evt.getByLabel("caldata",WireHandle);

  // Signal topology
  if (fIsSignal) {
    double px = 0;
    double py = 0;
    double pz = 0;
    double e = 0;
    for (int it = 0; it < mct_nnbar->NParticles(); ++it) {
      simb::MCParticle part = mct_nnbar->GetParticle(it);
      if (part.StatusCode() == 1 && (abs(part.PdgCode()) == 211 || part.PdgCode() == 111)) {
        if (fVertexX == 0 && fVertexY == 0 && fVertexZ == 0) {
          fVertexX = part.Position(0).X();
          fVertexY = part.Position(0).Y();
          fVertexZ = part.Position(0).Z();
        }
        ++fPionMultiplicity;
        px += part.Px();
        py += part.Py();
        pz += part.Pz();
        e += part.E();
      }
      else if (part.StatusCode() == 1 && (part.PdgCode() == 2212 || part.PdgCode() == 2112)) {
        ++fNucleonMultiplicity;
        px += part.Px();
        py += part.Py();
        pz += part.Pz();
        e += part.E();
      }
    }
    fMomentum = sqrt(pow(px,2)+pow(py,2)+pow(pz,2));
    fMomentumX = px;
    fMomentumY = py;
    fMomentumZ = pz;
    fInvariantMass = sqrt(pow(e,2)-pow(fMomentum,2));
  }

  for (int it = 0; it < mct_cosmic->NParticles(); ++it) {
    simb::MCParticle part = mct_cosmic->GetParticle(it);
    if (part.StatusCode() == 1) {
      fCosmicPrimaryPdg.push_back(part.PdgCode());
      fCosmicPrimaryE.push_back(part.E());
      if FindObject(part.TrackID(),TrackHandle,ShowerHandle) {
        fCosmicPrimaryInFVPdg.push_back(part.PdgCode());
        fCosmicPrimaryInFVE.push_back(part.E());
      }
    }
  }

  // Fill event tree
  fTree->Fill();

} // function DLTopology::analyze

DEFINE_ART_MODULE(DLTopology)

} // namespace nnbar

