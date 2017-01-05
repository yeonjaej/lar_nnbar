// art framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "fhiclcpp/ParameterSet.h"

// data container includes
#include "nusimdata/SimulationBase/MCTruth.h"
#include "lardataobj/MCBase/MCTrack.h"
#include "lardataobj/MCBase/MCShower.h"
#include "lardataobj/RecoBase/Hit.h"
#include "lardataobj/RecoBase/Track.h"
#include "lardataobj/RecoBase/Shower.h"

// root includes
#include "TTree.h"

// c++ includes
#include <vector>
#include <iterator>
#include <typeinfo>
#include <memory>
#include <cmath>
#include <algorithm>

namespace microboone {

class minipart {

public:

  double startpoint[3];
  double momentum[3];
  std::string object_type;
  int object_position;

  minipart(double sp[3], double mom[3], std::string ot, int op) {
    for (int d = 0; d < 3; d++) {
      startpoint[d] = sp[d];
      momentum[d] = mom[d];
    }
    object_type = ot;
    object_position = op;
  }

}; // class minipart

class vertex {

public:

  std::vector<minipart> objects;
  double position[3];

  vertex(minipart o) {
    objects.push_back(o);
    for (int d = 0; d < 3; d++)
      position[d] = o.startpoint[d];
  }

  void add_object(minipart o) {
    for (minipart p : objects)
      if (p.object_position == o.object_position)
        return
    objects.push_back(o);
    for (int d = 0; d < 3; d++) {
      position[d] = 0;
      for (minipart p : objects)
        position[d] += p.startpoint[d];
      position[d] /= objects.size();
    }
  } // function vertex::add_object

}; // class vertex

/// LArSoft module to reconstruct and analyse nnbar events
class nnbarEventAnalyzer : public art::EDAnalyzer {
    
public:

  explicit nnbarEventAnalyzer(fhicl::ParameterSet const& pset);
  void beginJob();
  //void endJob();
  void analyze(art::Event const& evt);

private:

  void CreateTree();
  void InitializeBranches();
  void ClearData();

  // Input tree
  TTree* fTree;

  // MC truth information
  int fNumberPrimaries;
  int fNumberPrimariesTrackLike;
  int fNumberPrimariesShowerLike;

  // MC track information
  int fNumberMCTracks;
  std::vector<double> fMCTrackLength;
  std::vector<double> fMCTrackMomentum;

  // MC shower information
  int fNumberMCShowers;
  std::vector<double> fMCShowerEnergy;

  // Hit information
  std::string fHitModuleLabel;
  int fNumberHits;
  int fHitWires;
  std::vector<double> fHitStartTime;
  std::vector<double> fHitPeakAmp;
  std::vector<double> fHitRMS;
  std::vector<double> fHitIntegral;

  // Track information
  std::string fTrackModuleLabel;
  int fNumberTracks;
  std::vector<double> fTrackLength;
  std::vector<double> fTrackMomentum;

  // Shower information
  std::string fShowerModuleLabel;
  int fNumberShowers;
  std::vector<double> fShowerEnergy;

  // Analysis variables
  double fTrueEventMomentum;
  double fTrueEventEnergy;
  double fTrueEventInvariantMass;

  double fMCRecoEventMomentum;
  double fMCRecoEventEnergy;
  double fMCRecoEventInvariantMass;

  std::vector<double> fRecoEventMomentum;
  std::vector<double> fRecoEventEnergy;
  std::vector<double> fRecoEventInvariantMass;

  int fTrackMultiplicityDiff;
  int fShowerMultiplicityDiff;

  double fVertexCut;

}; // class nnbarEventAnalyzer

nnbarEventAnalyzer::nnbarEventAnalyzer(fhicl::ParameterSet const& pset) :
    EDAnalyzer(pset),
    fTree(nullptr),
    fHitModuleLabel(pset.get<std::string>("HitModuleLabel")),
    fTrackModuleLabel(pset.get<std::string>("TrackModuleLabel")),
    fShowerModuleLabel(pset.get<std::string>("ShowerModuleLabel")),
    fVertexCut(pset.get<double>("VertexCut"))
{} // function nnbarEventAnalyzer::nnbarEventAnalyzer

void nnbarEventAnalyzer::CreateTree() {

  if (!fTree) {
    art::ServiceHandle<art::TFileService> tfs;
    fTree = tfs->make<TTree>("nnbar","nnbar tree");
  }
} // function nnbarEventAnalyzer::CreateTree

void nnbarEventAnalyzer::InitializeBranches() {

  // MC truth information
  fTree->Branch("NumberPrimaries",&fNumberPrimaries,"NumberPrimaries/I");
  fTree->Branch("NumberPrimariesTrackLike",&fNumberPrimariesTrackLike,"NumberPrimariesTrackLike/I");
  fTree->Branch("NumberPrimariesShowerLike",&fNumberPrimariesShowerLike,"NumberPrimariesShowerLike/I");

  // MC track information
  fTree->Branch("NumberMCTracks",&fNumberMCTracks,"NumberMCTracks/I");
  fTree->Branch("MCTrackLength","std::vector<double>",&fMCTrackLength);
  fTree->Branch("MCTrackMomentum","std::vector<double>",&fMCTrackMomentum);

  // MC shower information
  fTree->Branch("NumberMCShowers",&fNumberMCShowers,"NumberMCShowers/I");
  fTree->Branch("MCShowerEnergy","std::vector<double>",&fMCShowerEnergy);

  // Hit information
  fTree->Branch("NumberHits",&fNumberHits,"NumberHits/I");
  fTree->Branch("HitWires",&fHitWires,"HitWires/I");
  fTree->Branch("HitStartTime","std::vector<double>",&fHitStartTime);
  fTree->Branch("HitPeakAmp","std::vector<double>",&fHitPeakAmp);
  fTree->Branch("HitRMS","std::vector<double>",&fHitRMS);
  fTree->Branch("HitIntegral","std::vector<double>",&fHitIntegral);

  // Track information
  fTree->Branch("NumberTracks",&fNumberTracks,"NumberTracks/I");
  fTree->Branch("TrackLength","std::vector<double>",&fTrackLength);
  fTree->Branch("TrackMomentum","std::vector<double>",&fTrackMomentum);

  // Shower information
  fTree->Branch("NumberShowers",&fNumberShowers,"NumberShowers/I");
  fTree->Branch("ShowerEnergy","std::vector<double>",&fShowerEnergy);

  // Analysis variables
  fTree->Branch("TrackMultiplicityDiff",&fTrackMultiplicityDiff,"TrackMultiplicityDiff/I");
  fTree->Branch("ShowerMultiplicityDiff",&fShowerMultiplicityDiff,"ShowerMultiplicityDiff/I");

  fTree->Branch("TrueEventMomentum",&fTrueEventMomentum,"TrueEventMomentum/D");
  fTree->Branch("TrueEventEnergy",&fTrueEventEnergy,"TrueEventEnergy/D");
  fTree->Branch("TrueEventInvariantMass",&fTrueEventInvariantMass,"TrueEventInvariantMass/D");

  fTree->Branch("MCRecoEventMomentum",&fMCRecoEventMomentum,"MCRecoEventMomentum/D");
  fTree->Branch("MCRecoEventEnergy",&fMCRecoEventEnergy,"MCRecoEventEnergy/D");
  fTree->Branch("MCRecoEventInvariantMass",&fMCRecoEventInvariantMass,"MCRecoEventInvariantMass/D");

  fTree->Branch("RecoEventMomentum","std::vector<double>",&fRecoEventMomentum);
  fTree->Branch("RecoEventEnergy","std::vector<double>",&fRecoEventEnergy);
  fTree->Branch("RecoEventInvariantMass","std::vector<double>",&fRecoEventInvariantMass);

} // function nnbarEventAnalyzer::InitializeBranches

void nnbarEventAnalyzer::ClearData() {
  
  fMCTrackLength.clear();
  fMCTrackMomentum.clear();

  fHitStartTime.clear();
  fHitPeakAmp.clear();
  fHitRMS.clear();
  fHitIntegral.clear();

  fMCShowerEnergy.clear();

  fTrackLength.clear();
  fTrackMomentum.clear();

  fShowerEnergy.clear();

} // function nnbarEventAnalyzer::ClearData

void nnbarEventAnalyzer::beginJob() {

  CreateTree();
  InitializeBranches();

} // function nnbarEventAnalyzer::initialize

void nnbarEventAnalyzer::analyze(art::Event const& evt) {

// MC truth information
  art::Handle<std::vector<simb::MCTruth>> TruthListHandle;
  std::vector<art::Ptr<simb::MCTruth>> TruthList;
  if (evt.getByLabel("generator",TruthListHandle))
    art::fill_ptr_vector(TruthList,TruthListHandle);
  art::Ptr<simb::MCTruth> mct = TruthList[0];

  fNumberPrimaries = 0;
  fNumberPrimariesTrackLike = 0;
  fNumberPrimariesShowerLike = 0;
  fTrueEventEnergy = 0;
  double px = 0;
  double py = 0;
  double pz = 0;

  for (int it = 0; it < mct->NParticles(); it++) {
    simb::MCParticle part = mct->GetParticle(it);
    if (part.StatusCode() == 1) {
      if (abs(part.PdgCode()) == 211 || part.PdgCode() == 111) {
        ++fNumberPrimaries;
        fTrueEventEnergy += part.E();
        px += part.Px();
        py += part.Py();
        pz += part.Pz()
      }
      else if (part.PdgCode() == 2212) {
        ++fNumberPrimaries;
        fTrueEventEnergy += part.E() - part.Mass();
        px += part.Px();
        py += part.Py();
        pz += part.Pz();
      }
      if (abs(part.PdgCode()) == 211 || abs(part.PdgCode()) == 2212)
        ++fNumberPrimariesTrackLike;
      else if (part.PdgCode() == 111)
        fNumberPrimariesShowerLike += 2;
    }
  }
  fTrueEventMomentum = sqrt(pow(px,2)+pow(py,2)+pow(pz,2));

// MC track information
  art::Handle<std::vector<sim::MCTrack>> mctrackh;
  evt.getByLabel("mcreco",mctrackh);

  fNumberMCTracks = mctrackh->size();

  for (std::vector<sim::MCTrack>::const_iterator it = mctrackh->begin();
            it != mctrackh->end(); ++it) {
    const sim::MCTrack & mctrack = *it;

    double dx = mctrack.Start().X() - mctrack.End().X();
    double dy = mctrack.Start().Y() - mctrack.End().Y();
    double dz = mctrack.Start().Z() - mctrack.End().Z();
    fMCTrackLength.push_back(sqrt(pow(dx,2)+pow(dy,2)+pow(dz,2)));
    px = mctrack.Start().Px();
    py = mctrack.Start().Py();
    pz = mctrack.Start().Pz();
    fMCTrackMomentum.push_back(0.001*sqrt(pow(px,2)+pow(py,2)+pow(pz,2)));
  }

// MC shower information
  art::Handle<std::vector<sim::MCShower>> mcshowerh;
  evt.getByLabel("mcreco",mcshowerh);

  fNumberMCShowers = mcshowerh->size();

  for (std::vector<sim::MCShower>::const_iterator it = mcshowerh->begin();
            it != mcshowerh->end(); ++it) {
    const sim::MCShower & mcshower = *it;
    fMCShowerEnergy.push_back(0.001*mcshower.Start().E());
  }

// Hit information
  art::Handle<std::vector<recob::Hit>> hith;
  evt.getByLabel(fHitModuleLabel,hith);

  std::vector<int> WiresHit;
  fHitWires = 0;

  fNumberHits = hith->size();

  for (std::vector<recob::Hit>::const_iterator it = hith->begin();
            it != hith->end(); ++it) {
    const recob::Hit & hit = *it;
    if (std::find(WiresHit.begin(),WiresHit.end(),hit.Channel())!=WiresHit.end()) {
      WiresHit.push_back(hit.Channel());
      ++fHitWires;
    }
    fHitStartTime.push_back(hit.StartTick());
    fHitPeakAmp.push_back(hit.PeakAmplitude());
    fHitRMS.push_back(hit.RMS());
    fHitIntegral.push_back(hit.Integral());
  }

// Track information
  art::Handle<std::vector<recob::Track>> trackh;
  evt.getByLabel(fTrackModuleLabel,trackh);

  fNumberTracks = trackh->size();

  for (std::vector<recob::Track>::const_iterator it = trackh->begin();
            it != trackh->end(); ++it) {
    const recob::Track & track = *it;
    double dx = track.Vertex().X() - track.End().X();
    double dy = track.Vertex().Y() - track.End().Y();
    double dz = track.Vertex().Z() - track.End().Z();
    fTrackLength.push_back(sqrt(pow(dx,2)+pow(dy,2)+pow(dz,2)));
    px = 0.001 * track.VertexDirection()[0] * track.VertexMomentum();
    py = 0.001 * track.VertexDirection()[1] * track.VertexMomentum();
    pz = 0.001 * track.VertexDirection()[2] * track.VertexMomentum();
    fTrackMomentum.push_back(sqrt(pow(px,2)+pow(py,2)+pow(pz,2)));
  }

// Shower information
  art::Handle<std::vector<recob::Shower>> showerh;
  evt.getByLabel(fShowerModuleLabel,showerh);

  fNumberShowers = showerh->size();

  for (std::vector<recob::Shower>::const_iterator it = showerh->begin();
            it != showerh->end(); ++it) {
    const recob::Shower & shower = *it;
    fShowerEnergy.push_back(0.001*shower.Energy()[2]);
  }

// Analysis
  fTrackMultiplicityDiff = fNumberTracks - fNumberMCTracks;
  fShowerMultiplicityDiff = fNumberShowers - fNumberMCShowers;

// Vertexing
  std::vector<minipart> all_objects;
  int id = 0;
  for (std::vector<recob::Track>::const_iterator it = trackh->begin();
            it != trackh->end(); ++it) {
    const recob::Track & track = *it;
    double startpoint[3] = { track.Vertex()[0], track.Vertex()[1], track.Vertex()[2] };
    double momentum[3] = { 0.001*track.VertexDirection()[0]*track.VertexMomentum(), 0.001*track.VertexDirection()[1]*track.VertexMomentum(),
              0.001*track.VertexDirection()[2]*track.VertexMomentum() };
    all_objects.push_back(minipart(startpoint,momentum,"t",id));
    ++id;
  }
  for (std::vector<recob::Shower>::const_iterator it = showerh->begin();
            it != showerh->end(); ++it) {
    const recob::Shower & shower = *it;
    double startpoint[3] = { shower.ShowerStart()[0], shower.ShowerStart()[1], shower.ShowerStart()[2] };
    double energy = shower.Energy()[2];
    double momentum[3] = { 0.001*shower.Direction()[0]*energy, 0.001*shower.Direction()[1]*energy, 0.001*shower.Direction()[2]*energy };
    all_objects.push_back(minipart(startpoint,momentum,"s",id));
    ++id;
  }

  std::vector<vertex> vertex_candidates;
  for (int it1 = 0; it1 < (int)all_objects.size()-1; it1++) {
    double sp1[3] = { all_objects[it1].startpoint[0], all_objects[it1].startpoint[1], all_objects[it1].startpoint[2] };
    for (int it2 = it1+1; it2 < (int)all_objects.size(); it2++) {
      double sp2[3] = { all_objects[it2].startpoint[0], all_objects[it2].startpoint[1], all_objects[it2].startpoint[2] };
      if (sqrt(pow(sp1[0]-sp2[0],2)+pow(sp1[1]-sp2[1],2)+pow(sp1[2]-sp2[2],2)) < fVertexCut) {
        vertex_candidates.push_back(vertex(all_objects[it1]));
        break;
      }
    }
  }

  bool changed = true;
  while (changed) {
    changed = false;
    for (int it1 = 0; it1 < (int)vertex_candidates.size()-1; it1++) {
      double vp1[3] = { vertex_candidates[it1].position[0], vertex_candidates[it1].position[1], vertex_candidates[it1].position[2] };
      for (int it2 = it1+1; it2 < (int)vertex_candidates.size(); it2++) {
        double vp2[3] = { vertex_candidates[it2].position[0], vertex_candidates[it2].position[1], vertex_candidates[it2].position[2] };
        if (sqrt(pow(vp1[0]-vp2[0],2)+pow(vp1[1]-vp2[1],2)+pow(vp1[2]-vp2[2],2)) < fVertexCut) {
          for (minipart object : vertex_candidates[it2].objects)
            vertex_candidates[it1].add_object(object);
          vertex_candidates.erase(vertex_candidates.begin()+it2);
          changed = true;
          break;
        }
      }
      if (changed)
        break;
    }
  }

  for (vertex vtx : vertex_candidates) {
    double vertex_momentum[3] = {0,0,0};
    double vertex_energy = 0;
    for (minipart o : vtx.objects) {
      for (int d = 0; d < 3; d++)
        vertex_momentum[d] += o.momentum[d];
      double mass;
      if (o.object_type == "t")
        mass = 139.57;
      else
        mass = 0;
      double momentum_squared = pow(o.momentum[0],2) + pow(o.momentum[1],2) + pow(o.momentum[2],2);
      vertex_energy += sqrt(pow(mass,2) + momentum_squared);
    }
    fRecoEventEnergy.push_back(vertex_energy);
    fRecoEventMomentum.push_back(sqrt(pow(vertex_momentum[0],2)+pow(vertex_momentum[1],2)+pow(vertex_momentum[2],2)));
  }

// Fill event tree
  fTree->Fill();

// Clean up
  ClearData();

} // function nnbarEventAnalyzer::analyze

DEFINE_ART_MODULE(nnbarEventAnalyzer)

} // namespace microboone