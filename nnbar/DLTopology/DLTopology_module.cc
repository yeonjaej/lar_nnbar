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

  if (!fIsSignal) {
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

  fVertexX = 0;
  fVertexY = 0;
  fVertexZ = 0;

  fContained = false;

  fPionMultiplicity = 0;
  fLeptonMultiplicity = 0;
  fNucleonMultiplicity = 0;
  fInvariantMass = 0;
  fMomentum = 0;
  fMomentumX = 0;
  fMomentumY = 0;
  fMomentumZ = 0;

  if (!fIsSignal) {
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
    for (int it = 0; it < mct->NParticles(); ++it) {
      simb::MCParticle part = mct->GetParticle(it);
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
  // Background topology
  else {
    double px = 0;
    double py = 0;
    double pz = 0;
    double e = 0;
    for (int it = 0; it < mct->NParticles(); ++it) {
      simb::MCParticle part = mct->GetParticle(it);
      if (part.StatusCode() == 1) {
        if (abs(part.PdgCode()) == 211 || part.PdgCode() == 111) {
          ++fPionMultiplicity;
          px += part.Px();
          py += part.Py();
          pz += part.Pz();
          e += part.E();
        }
        else if (abs(part.PdgCode()) == 11 || abs(part.PdgCode()) == 13 || abs(part.PdgCode()) == 15) {
          ++fLeptonMultiplicity;
          px += part.Px();
          py += part.Py();
          pz += part.Pz();
          e += part.E();
        }
        else if (part.PdgCode() == 2212 || part.PdgCode() == 2112) {
          ++fNucleonMultiplicity;
          px += part.Px();
          py += part.Py();
          pz += part.Pz();
          e += part.E();
        }
        else std::cout << "Particle not accounted for, of type " << part.PdgCode() << std::endl;
      }
    }
    fMomentum = sqrt(pow(px,2)+pow(py,2)+pow(pz,2));
    fMomentumX = px;
    fMomentumY = py;
    fMomentumZ = pz;
    fInvariantMass = sqrt(pow(e,2)-pow(fMomentum,2));
    fVertexX = mct->GetNeutrino().Nu().Position(0).X();
    fVertexY = mct->GetNeutrino().Nu().Position(0).Y();
    fVertexZ = mct->GetNeutrino().Nu().Position(0).Z();
    fNC = (bool) mct->GetNeutrino().CCNC();
    fInteractionType = mct->GetNeutrino().InteractionType();
    fNuEnergy = mct->GetNeutrino().Nu().E();
    fQSqr = mct->GetNeutrino().QSqr();
    fNuPdg = mct->GetNeutrino().Nu().PdgCode();
    fLepEnergy = mct->GetNeutrino().Lepton().E();
  }

  // See if event spans multiple apas
  std::vector<int> apas;
  for (std::vector<recob::Wire>::const_iterator it = WireHandle->begin(); it != WireHandle->end(); ++it) {
    const recob::Wire & wire = *it;
    int apa = std::floor((float)wire.Channel()/2560.);
    if (std::find(apas.begin(),apas.end(),apa) == apas.end()) apas.push_back(apa);
  }

  // Check all tracks are contained in a single APA
  bool all_particles_contained = true;
  art::ServiceHandle<geo::Geometry> geo;
  for (std::vector<sim::MCTrack>::const_iterator it = TrackHandle->begin(); it != TrackHandle->end(); ++it) {
    const sim::MCTrack & track = *it;
    bool particle_contained = false;
    for (size_t it_tpc = 0; it_tpc < geo->NTPC(); ++it_tpc) {
      const geo::TPCGeo & tpc = geo->TPC(it_tpc);
      if (tpc.ContainsPosition(track.Start().Position().Vect()) && tpc.ContainsPosition(track.End().Position().Vect())) {
        particle_contained = true;
        break;
      }
    }
    if (particle_contained == false) all_particles_contained = false;
  }

  // Check all showers are contained in a single APA
  for (std::vector<sim::MCShower>::const_iterator it = ShowerHandle->begin(); it != ShowerHandle->end(); ++it) {
    if (all_particles_contained == false) break;
    const sim::MCShower & shower = *it;
    bool particle_contained = false;
    for (size_t it_tpc = 0; it_tpc < geo->NTPC(); ++it_tpc) {
      const geo::TPCGeo & tpc = geo->TPC(it_tpc);
      if (tpc.ContainsPosition(shower.Start().Position().Vect()) && tpc.ContainsPosition(shower.End().Position().Vect())) {
        particle_contained = true;
        break;
      }
    }
    if (particle_contained == false) all_particles_contained = false;
  }

  // Put it all together to make a containment check
  if (apas.size() == 1 && all_particles_contained) fContained = true;
  else fContained = false;

  // Fill event tree
  fTree->Fill();

} // function DLTopology::analyze

DEFINE_ART_MODULE(DLTopology)

} // namespace nnbar

