// art framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "fhiclcpp/ParameterSet.h"
#include "canvas/Persistency/Common/FindOneP.h"

// data container includes
#include "nusimdata/SimulationBase/MCTruth.h"
#include "lardataobj/MCBase/MCTrack.h"
#include "lardataobj/MCBase/MCShower.h"
#include "lardataobj/RecoBase/Wire.h"
#include "larcore/Geometry/Geometry.h"

// root includes
#include "TTree.h"
#include "TDatabasePDG.h"

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

  // Image functions
  void ClearData();
  void ResetROI();
  void SetROISize();
  int FindBestAPA(std::vector<int> apas);
  int FindROI(int apa, int plane);

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
  int fChargedPionMultiplicity;
  int fNeutralPionMultiplicity;
  int fLeptonMultiplicity;
  int fProtonMultiplicity;
  int fNeutronMultiplicity;
  int fVisibleProtonMultiplicity;

  int fMCTrackMultiplicity;
  int fMCShowerMultiplicity;

  double fInvariantMass;
  double fMomentum;
  double fMomentumX;
  double fMomentumY;
  double fMomentumZ;
  double fTotalEnergy;

  double fInvariantMassNoNucleons;
  double fInvariantMassAll;

  double fMomentumNoNucleons;
  double fMomentumAll;
  double fMomentumXNoNucleons;
  double fMomentumXAll;
  double fMomentumYNoNucleons;
  double fMomentumYAll;
  double fMomentumZNoNucleons;
  double fMomentumZAll;

  double fTotalEnergyNoNucleons;
  double fTotalEnergyAll;

  // Background variables
  bool fNC;
  int fInteractionType;
  double fNuEnergy;
  double fQSqr;
  int fNuPdg;
  double fLepEnergy;
  double fCosZ;

  // Image variables
  int fDownsamplingU;
  int fDownsamplingV;
  int fDownsamplingZ;

  int fMaxTick;
  int fADCCut;
  int fContainmentCut;
  int fEventType;

  int fFirstWire;
  int fLastWire;
  int fFirstTick;
  int fLastTick;

  const int fNumberChannels[3] = { 800, 800, 960 };
  const int fFirstChannel[3] = { 0, 800, 1600 };
  const int fLastChannel[3] = { 799, 1599, 2559 };

  int fAPA;
  int fNumberWires;
  int fNumberTicks;

}; // class DLTopology

DLTopology::DLTopology(fhicl::ParameterSet const& pset) :
    EDAnalyzer(pset),
    fTree(nullptr),
    fIsSignal(pset.get<bool>("IsSignal")),
    fMaxTick(pset.get<int>("MaxTick")),
    fADCCut(pset.get<int>("ADCCut"))
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

  fTree->Branch("ChargedPionMultiplicity",&fChargedPionMultiplicity,"ChargedPionMultiplicity/I");
  fTree->Branch("NeutralPionMultiplicity",&fNeutralPionMultiplicity,"NeutralPionMultiplicity/I");
  fTree->Branch("LeptonMultiplicity",&fLeptonMultiplicity,"LeptonMultiplicity/I");
  fTree->Branch("ProtonMultiplicity",&fProtonMultiplicity,"ProtonMultiplicity/I");
  fTree->Branch("NeutronMultiplicity",&fNeutronMultiplicity,"NeutronMultiplicity/I");
  fTree->Branch("VisibleProtonMultiplicity",&fVisibleProtonMultiplicity,"VisibleProtonMultiplicity/I");

  fTree->Branch("MCTrackMultiplicity",&fMCTrackMultiplicity,"MCTrackMultiplicity/I");
  fTree->Branch("MCShowerMultiplicity",&fMCShowerMultiplicity,"MCShowerMultiplicity/I");
/*
  fTree->Branch("InvariantMassNoNucleons",&fInvariantMassNoNucleons,"InvariantMassNoNucleons/D");
  fTree->Branch("InvariantMassAll",&fInvariantMassAll,"InvariantMassAll/D");

  fTree->Branch("MomentumNoNucleons",&fMomentumNoNucleons,"MomentumNoNucleons/D");
  fTree->Branch("MomentumAll",&fMomentumAll,"MomentumAll/D");
  fTree->Branch("MomentumXNoNucleons",&fMomentumXNoNucleons,"MomentumXNoNucleons/D");
  fTree->Branch("MomentumXAll",&fMomentumXAll,"MomentumXAll/D");
  fTree->Branch("MomentumYNoNucleons",&fMomentumYNoNucleons,"MomentumYNoNucleons/D");
  fTree->Branch("MomentumYAll",&fMomentumYAll,"MomentumYAll/D");
  fTree->Branch("MomentumZNoNucleons",&fMomentumZNoNucleons,"MomentumZNoNucleons/D");
  fTree->Branch("MomentumZAll",&fMomentumZAll,"MomentumZAll/D");

  fTree->Branch("TotalEnergyNoNucleons",&fTotalEnergyNoNucleons,"TotalEnergyNoNucleons/D");
  fTree->Branch("TotalEnergyAll",&fTotalEnergyAll,"TotalEnergyAll/D");
*/

  fTree->Branch("InvariantMass",&fInvariantMass,"InvariantMass/D");
  fTree->Branch("Momentum",&fMomentum,"Momentum/D");
  fTree->Branch("MomentumX",&fMomentumX,"MomentumX/D");
  fTree->Branch("MomentumY",&fMomentumY,"MomentumY/D");
  fTree->Branch("MomentumZ",&fMomentumZ,"MomentumZ/D");
  fTree->Branch("TotalEnergy",&fTotalEnergy,"TotalEnergy/D");

  if (!fIsSignal) {
    fTree->Branch("NC",&fNC,"NC/B");
    fTree->Branch("InteractionType",&fInteractionType,"InteractionType/I");
    fTree->Branch("NuEnergy",&fNuEnergy,"NuEnergy/D");
    fTree->Branch("QSqr",&fQSqr,"QSqr/D");
    fTree->Branch("NuPdg",&fNuPdg,"NuPdg/I");
    fTree->Branch("LepEnergy",&fLepEnergy,"LepEnergy/D");
    fTree->Branch("CosZ",&fCosZ,"CosZ/D");
  }

  fTree->Branch("DownsamplingU",&fDownsamplingU,"DownsamplingU/I");
  fTree->Branch("DownsamplingV",&fDownsamplingV,"DownsamplingV/I");
  fTree->Branch("DownsamplingZ",&fDownsamplingZ,"DownsamplingZ/I");

} // function DLTopology::InitializeBranches

void DLTopology::Clear() {

  // Clear wire map
  fWireMap.clear();

  // Clear tree variables at start of event
  fRun    = 0;
  fSubrun = 0;
  fEvent  = 0;

  fVertexX = 0;
  fVertexY = 0;
  fVertexZ = 0;

  fVertexAPA        = -1;
  fImageAPA         = -1;
  fChargeContained  = false;
  fTracksContained  = false;
  fShowersContained = false;

  fChargedPionMultiplicity   = 0;
  fNeutralPionMultiplicity   = 0;
  fLeptonMultiplicity        = 0;
  fProtonMultiplicity        = 0;
  fNeutronMultiplicity       = 0;
  fVisibleProtonMultiplicity = 0;

  fMCTrackMultiplicity     = 0;
  fMCShowerMultiplicity    = 0;

  fInvariantMassNoNucleons = 0;
  fInvariantMassAll        = 0;

  fMomentumNoNucleons      = 0;
  fMomentumAll             = 0;
  fMomentumXNoNucleons     = 0;
  fMomentumXAll            = 0;
  fMomentumYNoNucleons     = 0;
  fMomentumYAll            = 0;
  fMomentumZNoNucleons     = 0;
  fMomentumZAll            = 0;

  fTotalEnergyNoNucleons   = 0;
  fTotalEnergyAll          = 0;

  fInvariantMass = 0;
  fMomentum      = 0;
  fMomentumX     = 0;
  fMomentumY     = 0;
  fMomentumZ     = 0;
  fTotalEnergy   = 0;

  if (!fIsSignal) {
    fNC              = 0;
    fInteractionType = 0;
    fNuEnergy        = 0;
    fQSqr            = 0;
    fNuPdg           = 0;
    fLepEnergy       = 0;
    fCosZ            = 0;
  }

  ResetROI();
  fAPA = -1;
  fWireMap.clear();

  fDownsamplingU = false;
  fDownsamplingV = false;
  fDownsamplingZ = false;

} // function DLTopology::Clear

bool DLTopology::FindObject(int primary_trackid, art::Handle<std::vector<sim::MCTrack>> TrackHandle, art::Handle<std::vector<sim::MCShower>> ShowerHandle) {

  // Check if the primary has a track
  for (std::vector<sim::MCTrack>::const_iterator it_track = TrackHandle->begin(); it_track != TrackHandle->end(); ++it_track) {
    const sim::MCTrack & track = *it_track;
    double trackid = track.TrackID();
    if (trackid == primary_trackid) {
      fMomentumX += track.Start().Px();
      fMomentumY += track.Start().Py();
      fMomentumZ += track.Start().Pz();
      if (track.PdgCode() == 2212) fTotalEnergy += sqrt(pow(track.Start().E(),2)-pow(938,2));
      else fTotalEnergy += track.Start().E();
      std::cout << "Adding track with PDG " << track.PdgCode() << ", energy " << track.Start().E() << " MeV and momentum " << sqrt(pow(track.Start().Px(),2)+pow(track.Start().Py(),2)+pow(track.Start().Pz(),2)) << "MeV!" << std::endl;
      return true;
    }
  }

  // Check if the primary has a shower
  for (std::vector<sim::MCShower>::const_iterator it_shower = ShowerHandle->begin(); it_shower != ShowerHandle->end(); ++it_shower) {
    const sim::MCShower & shower = *it_shower;
    double trackid = shower.TrackID();
    if (trackid == primary_trackid) {
      fMomentumX += shower.Start().Px();
      fMomentumY += shower.Start().Py();
      fMomentumZ += shower.Start().Pz();
      fTotalEnergy += shower.Start().E();
      std::cout << "Adding shower with PDG " << shower.PdgCode() << ", energy " << shower.Start().E() << " MeV and momentum " << sqrt(pow(shower.Start().Px(),2)+pow(shower.Start().Py(),2)+pow(shower.Start().Pz(),2)) << "MeV!" << std::endl;
      return true;
    }
  }

  // Return false if no match found
  return false;

} // function DLTopology::FindObject

void DLTopology::ResetROI() {

  fFirstWire = -1;
  fLastWire = -1;
  fFirstTick = -1;
  fLastTick = -1;
  fNumberWires = -1;
  fNumberTicks = -1;
} // function DLTopology::ResetROI

void DLTopology::SetROISize() {

  fNumberWires = fLastWire - fFirstWire + 1;
  fNumberTicks = fLastTick - fFirstTick + 1;
} // function DLTopology::SetROISize

int DLTopology::FindBestAPA(std::vector<int> apas) {

  int best_apa = -1;
  float best_adc = -1;

  for (int apa : apas) {

    float max_adc = 0;

    for (int it_plane = 0; it_plane < 3; ++it_plane) {
      for (int it_x = 0; it_x < fNumberChannels[it_plane]; ++it_x) {
        for (int it_y = 0; it_y < fMaxTick; ++it_y) {
          int wire_address = (apa * 2560) + fFirstChannel[it_plane] + it_x;
          int tick_address = it_y;
          if (fWireMap.find(wire_address) != fWireMap.end() && tick_address < (int)fWireMap[wire_address].size())
            max_adc += fWireMap[wire_address][tick_address];
        }
      }
    }

    if (max_adc > best_adc || best_apa == -1) {
      best_apa = apa;
      best_adc = max_adc;
    }
  }

  return best_apa;
} // function DLTopology::FindBestAPA

int DLTopology::FindROI(int apa, int plane) {

  ResetROI();

  // find first & last channels in APA
  int first_channel = (2560*apa) + fFirstChannel[plane];
  int last_channel = (2560*apa) + fLastChannel[plane];

  // find first & last channel & tick with ADC above threshold
  for (int channel = first_channel; channel < last_channel; ++channel) {
    if (fWireMap.find(channel) != fWireMap.end()) {
      for (int tick = 0; tick < (int)fWireMap[channel].size(); ++tick) {
        if (fWireMap[channel][tick] > fADCCut) {
          if (fFirstWire == -1 || fFirstWire > channel) fFirstWire = channel;
          if (fLastWire == -1 || fLastWire < channel) fLastWire = channel;
          if (fFirstTick == -1 || fFirstTick > tick) fFirstTick = tick;
          if (fLastTick == -1 || fLastTick < tick) fLastTick = tick;
        }
      }
    }
  }

  if (fFirstWire == -1 || fLastWire == -1 || fFirstTick == -1 || fLastTick == -1) return -1;
  SetROISize();

  // figure out whether we need to downsample
  if (fNumberWires > 600 || fNumberTicks/4 > 600) return 1;
  else return 0;

} // function DLTopology::FindROI

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

  // MC particle information
  art::Handle<std::vector<simb::MCParticle>> ParticleHandle;
  evt.getByLabel("largeant",ParticleHandle);

  // MC track information
  art::Handle<std::vector<sim::MCTrack>> TrackHandle;
  evt.getByLabel("mcreco",TrackHandle);
  fMCTrackMultiplicity = TrackHandle->size();

  // MC shower information
  art::Handle<std::vector<sim::MCShower>> ShowerHandle;
  evt.getByLabel("mcreco",ShowerHandle);
  fMCShowerMultiplicity = ShowerHandle->size();

  // Wire information
  art::Handle<std::vector<recob::Wire>> WireHandle;
  evt.getByLabel("caldata",WireHandle);

  TVector3 vertex_position;

  // Signal topology
  if (fIsSignal) {

    double px_nonuc = 0;
    double px_all   = 0;
    double py_nonuc = 0;
    double py_all   = 0;
    double pz_nonuc = 0;
    double pz_all   = 0;
    double e_nonuc  = 0;
    double e_all    = 0;

    for (int it = 0; it < mct->NParticles(); ++it) {
      simb::MCParticle part = mct->GetParticle(it);
      if (part.StatusCode() == 1) {
        if (abs(part.PdgCode()) == 211 || part.PdgCode() == 111) {
          if (fVertexX == 0 && fVertexY == 0 && fVertexZ == 0) {
            vertex_position = part.Position(0).Vect();
            fVertexX = part.Position(0).X();
            fVertexY = part.Position(0).Y();
            fVertexZ = part.Position(0).Z();
          }

          if (abs(part.PdgCode()) == 211) ++fChargedPionMultiplicity;
          else ++fNeutralPionMultiplicity;

          px_nonuc += part.Px();
          px_all   += part.Px();
          py_nonuc += part.Py();
          py_all   += part.Py();
          pz_nonuc += part.Pz();
          pz_all   += part.Pz();
          e_nonuc  += part.E();
          e_all    += part.E();
        }

        else if (part.PdgCode() == 2112) {
          ++fProtonMultiplicity;
          double proton_energy = part.E() - 0.9383;
          if (proton_energy > 0.05) {
            ++fVisibleProtonMultiplicity;
            px_all += part.Px();
            py_all += part.Py();
            pz_all += part.Pz();
            e_all  += proton_energy;
          }
        }
        else if (part.PdgCode() == 2212) ++fNeutronMultiplicity;
        else std::cout << "Particle not accounted for, of type " << part.PdgCode() << std::endl;
      }
    }

    fMomentumNoNucleons      = sqrt(pow(px_nonuc,2)+pow(py_nonuc,2)+pow(pz_nonuc,2));
    fMomentumAll             = sqrt(pow(px_all,2)+pow(py_all,2)+pow(pz_all,2));
    fMomentumXNoNucleons     = px_nonuc;
    fMomentumXAll            = px_all;
    fMomentumYNoNucleons     = py_nonuc;
    fMomentumYAll            = py_all;
    fMomentumZNoNucleons     = pz_nonuc;
    fMomentumZAll            = pz_all;

    fTotalEnergyNoNucleons   = e_nonuc;
    fTotalEnergyAll          = e_all;

    if (e_nonuc < fMomentumNoNucleons) fInvariantMassNoNucleons = 0;
    else fInvariantMassNoNucleons = sqrt(pow(e_nonuc,2)-pow(fMomentumNoNucleons,2));

    if (e_all < fMomentumAll) fInvariantMassAll = 0;
    else fInvariantMassAll = sqrt(pow(e_all,2)-pow(fMomentumAll,2));
  }
  // Background topology
  else {

    double px_nonuc = 0;
    double px_all   = 0;
    double py_nonuc = 0;
    double py_all   = 0;
    double pz_nonuc = 0;
    double pz_all   = 0;
    double e_nonuc  = 0;
    double e_all    = 0;

    for (int it = 0; it < mct->NParticles(); ++it) {
      simb::MCParticle part = mct->GetParticle(it);
      if (part.StatusCode() == 1) {
        if (abs(part.PdgCode()) == 211 || part.PdgCode() == 111 || abs(part.PdgCode()) == 11
            || abs(part.PdgCode()) == 13 || abs(part.PdgCode()) == 15) {
          if (abs(part.PdgCode()) == 211) ++fChargedPionMultiplicity;
          else if (part.PdgCode() == 111) ++fNeutralPionMultiplicity;
          else ++fLeptonMultiplicity;
          px_nonuc += part.Px();
          px_all   += part.Px();
          py_nonuc += part.Py();
          py_all   += part.Py();
          pz_nonuc += part.Pz();
          pz_all   += part.Pz();
          e_nonuc  += part.E();
          e_all    += part.E();
        }
        else if (part.PdgCode() == 2112) {
          ++fProtonMultiplicity;
          double proton_energy = part.E() - 0.9383;
          if (proton_energy > 0.05) {
            ++fVisibleProtonMultiplicity;
            px_all += part.Px();
            py_all += part.Py();
            pz_all += part.Pz();
            e_all  += proton_energy;
          }
        }
        else if (part.PdgCode() == 2212) ++fNeutronMultiplicity;
        else std::cout << "Particle not accounted for, of type " << part.PdgCode() << std::endl;
      }
    }

    fMomentumNoNucleons      = sqrt(pow(px_nonuc,2)+pow(py_nonuc,2)+pow(pz_nonuc,2));
    fMomentumAll             = sqrt(pow(px_all,2)+pow(py_all,2)+pow(pz_all,2));
    fMomentumXNoNucleons     = px_nonuc;
    fMomentumXAll            = px_all;
    fMomentumYNoNucleons     = py_nonuc;
    fMomentumYAll            = py_all;
    fMomentumZNoNucleons     = pz_nonuc;
    fMomentumZAll            = pz_all;

    fTotalEnergyNoNucleons   = e_nonuc;
    fTotalEnergyAll          = e_all;

    if (e_nonuc < fMomentumNoNucleons) fInvariantMassNoNucleons = 0;
    else fInvariantMassNoNucleons = sqrt(pow(e_nonuc,2)-pow(fMomentumNoNucleons,2));

    if (e_all < fMomentumAll) fInvariantMassAll = 0;
    else fInvariantMassAll = sqrt(pow(e_all,2)-pow(fMomentumAll,2));

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
    fCosZ = mct->GetNeutrino().Nu().Momentum().Vect().Unit().Z();
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

  std::vector<int>  primary_trackid;

// Get the primaries
  for (std::vector<simb::MCParticle>::const_iterator it = ParticleHandle->begin(); it != ParticleHandle->end(); ++it) {
    const simb::MCParticle & part = *it;
    if (part.Process() == "primary") primary_trackid.push_back(part.TrackId());
  }
  for (unsigned int it = 0; it < primary_trackid.size(); ++it) {
// Check if the primary has a corresponding object
    if (FindObject(primary_trackid.at(it),TrackHandle,ShowerHandle)) continue;
// Check if the primary has children
    std::vector<int> child_trackid;
    const simb::MCParticle & part = ParticleHandle->at(primary_trackid.at(it)-1);
    if (part.NumberDaughters() > 0) {
      for (int it_child = 0; it_child < part.NumberDaughters(); ++it_child) {
        FindObject(part.Daughter(it_child),TrackHandle,ShowerHandle);
      }
    }
  }

  // Finish up with kinematics
  fTotalEnergy *= 0.001;
  fMomentumX *= 0.001;
  fMomentumY *= 0.001;
  fMomentumZ *= 0.001;
  fMomentum = sqrt(pow(fMomentumX,2)+pow(fMomentumY,2)+pow(fMomentumZ,2));
  fInvariantMass = sqrt(pow(fTotalEnergy,2)-pow(fMomentum,2));

  // Image variables
  for (std::vector<recob::Wire>::const_iterator it = WireHandle->begin();
      it != WireHandle->end(); ++it) {
    const recob::Wire & wire = *it;
    fWireMap.insert(std::pair<int,std::vector<float>>(wire.Channel(),std::vector<float>(wire.Signal())));
    int apa = std::floor(wire.Channel()/2560);
    if (std::find(apas.begin(),apas.end(),apa) == apas.end())
      apas.push_back(apa);
  }

  // find best APA
  if (apas.size() == 0) {
    std::cout << "Skipping event. No activity inside the TPC!" << std::endl;
    return;
  }
  best_apa = FindBestAPA(apas);
  if (best_apa != -1) fAPA = best_apa;
  else {
    std::cout << "Skipping event. Could not find good APA!" << std::endl;
    return;
  }

  // check for problems
  fDownsamplingU = FindROI(best_apa,0);
  fDownsamplingV = FindROI(best_apa,1);
  fDownsamplingZ = FindROI(best_apa,2);

  // Fill event tree
  fTree->Fill();

} // function DLTopology::analyze

DEFINE_ART_MODULE(DLTopology)

} // namespace nnbar

