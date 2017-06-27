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

  // Wire map
  std::map<int,std::vector<float> > fWireMap;

  // Event ID
  int fRun;
  int fSubrun;
  int fEvent;

  // Event vertex
  double fVertexX;
  double fVertexY;
  double fVertexZ;

  // Containment
  int fVertexAPA;          // APA which contains event vertex
  int fImageAPA;           // APA which is used to create network image
  bool fChargeContained;   // whether all charge deposition occurs in single APA
  bool fTracksContained;   // whether all tracks are contained in single APA
  bool fShowersContained;  // whether all showers are contained in single APA

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

  fTree->Branch("VertexAPA",&fVertexAPA,"VertexAPA/I");
  fTree->Branch("ImageAPA",&fImageAPA,"ImageAPA/I");
  fTree->Branch("ChargeContained",&fChargeContained,"ChargeContained/B");
  fTree->Branch("TracksContained",&fTracksContained,"TracksContained/B");
  fTree->Branch("ShowersContained",&fShowersContained,"ShowersContained/B");

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

  // Clear wire map
  fWireMap.clear();

  // Clear tree variables at start of event
  fRun = 0;
  fSubrun = 0;
  fEvent = 0;

  fVertexX = 0;
  fVertexY = 0;
  fVertexZ = 0;

  fVertexAPA = -1;
  fImageAPA = -1;
  fChargeContained = false;
  fTracksContained = false;
  fShowersContained = false;

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

  TVector3 vertex_position;

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
          vertex_position = part.Position(0).Vect();
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
    vertex_position = mct->GetNeutrino().Nu().Position(0).Vect();
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

  // Find APA containing vertex
  art::ServiceHandle<geo::Geometry> geo;
  for (size_t it_tpc = 0; it_tpc < geo->NTPC(); ++it_tpc) {
    const geo::TPCGeo & tpc = geo->TPC(it_tpc);
    fVertexAPA = -1;
    if (tpc.ContainsPosition(vertex_position)) {
      fVertexAPA = std::floor((float)it_tpc/2);
      break;
    }
  }

  // Identify APA containing network image, and establish whether event crosses multiple APAs
  std::vector<int> apas;
  for (std::vector<recob::Wire>::const_iterator it = WireHandle->begin(); it != WireHandle->end(); ++it) {
    const recob::Wire & wire = *it;
    int apa = std::floor((float)wire.Channel()/2560.);
    if (std::find(apas.begin(),apas.end(),apa) == apas.end()) apas.push_back(apa);
    fWireMap.insert(std::pair<int,std::vector<float> >(wire.Channel(),std::vector<float>(wire.Signal())));
  }
  float best_apa = -1;
  float best_adc = 0;
  int number_channels[3] = { 800, 800, 960 };
  int first_channel[3] = { 0, 800, 1600 };
  for (int apa : apas) {
    float max_adc = 0;
    for (int it_plane = 0; it_plane < 3; ++it_plane) {
      for (int it_x = 0; it_x < number_channels[it_plane]; ++it_x) {
        for (int it_y = 0; it_y < 4492; ++it_y) {
          int w = (apa * 2560) + first_channel[it_plane] + it_x;
          int t = it_y;
          if (fWireMap.find(w) != fWireMap.end() && t < (int)fWireMap[w].size())
            max_adc += fWireMap[w][t];
        }
      }
    }
    if (max_adc > best_adc) {
      best_apa = apa;
      best_adc = max_adc;
    }
  }
  fImageAPA = best_apa;
  if (apas.size() > 1) fChargeContained = false;
  else fChargeContained = true;

  // Check all tracks are contained in a single APA
  fTracksContained = true;
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
    if (particle_contained == false) fTracksContained = false;
  }

  // Check all showers are contained in a single APA
  fShowersContained = true;
  for (std::vector<sim::MCShower>::const_iterator it = ShowerHandle->begin(); it != ShowerHandle->end(); ++it) {
    const sim::MCShower & shower = *it;
    bool particle_contained = false;
    for (size_t it_tpc = 0; it_tpc < geo->NTPC(); ++it_tpc) {
      const geo::TPCGeo & tpc = geo->TPC(it_tpc);
      if (tpc.ContainsPosition(shower.Start().Position().Vect()) && tpc.ContainsPosition(shower.End().Position().Vect())) {
        particle_contained = true;
        break;
      }
    }
    if (particle_contained == false) fShowersContained = false;
  }

/*
  std::vector<double> minx;
  std::vector<double> maxx;
  std::vector<double> miny;
  std::vector<double> maxy;
  std::vector<double> minz;
  std::vector<double> maxz;
  for (size_t it_tpc = 0; it_tpc < geo->NTPC(); ++it_tpc) {
    const geo::TPCGeo & tpc = geo->TPC(it_tpc);
    minx.push_back(tpc.MinX());
    maxx.push_back(tpc.MaxX());
    miny.push_back(tpc.MinY());
    maxy.push_back(tpc.MaxY());
    minz.push_back(tpc.MinZ());
    maxz.push_back(tpc.MaxZ());
  }
  std::cout << "Min X values: ";
  for (double temp : minx) std::cout << temp << ", ";
  std::cout << std::endl << "Max X values: ";
  for (double temp : maxx) std::cout << temp << ", ";
  std::cout << std::endl << "Min Y values: ";
  for (double temp : miny) std::cout << temp << ", ";
  std::cout << std::endl << "Max Y values: ";
  for (double temp : maxy) std::cout << temp << ", ";
  std::cout << std::endl << "Min Z values: ";
  for (double temp : minz) std::cout << temp << ", ";
  std::cout << std::endl << "Max Z values: ";
  for (double temp : maxz) std::cout << temp << ", ";
  std::cout << std::endl;
*/

  // Fill event tree
  fTree->Fill();

} // function DLTopology::analyze

DEFINE_ART_MODULE(DLTopology)

} // namespace nnbar

