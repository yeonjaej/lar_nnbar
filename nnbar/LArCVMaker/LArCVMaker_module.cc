// framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "fhiclcpp/ParameterSet.h"

// data product includes
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RecoBase/Wire.h"

// root includes
#include "TFile.h"
#include "TTree.h"

// c++ includes
#include <vector>
#include <iterator>
#include <typeinfo>
#include <memory>
#include <string>
#include <algorithm>
#include <cmath>

namespace nnbar {

class LArCVMaker : public art::EDAnalyzer {
  
public:

  explicit LArCVMaker(fhicl::ParameterSet const & pset);
  void beginJob();
  //void endJob();
  void analyze(art::Event const & evt);

private:

  void ClearData();
  void SetupAPAs();
  void PadVector(std::vector<float> & vec, int new_size, int n_ticks);
  void ProcessWire(recob::Wire w);

  TTree* fTree;
  std::string fWireModuleLabel;

  int fFirstAPA;
  int fLastAPA;
  int fFirstWire;
  int fLastWire;
  int fFirstTick;
  int fLastTick;

  int fEvent;
  int fAPA;
  int fNumberTicks;
  std::vector<float> fImageZ;
}; // class LArCVMaker

LArCVMaker::LArCVMaker(fhicl::ParameterSet const & pset) :
    EDAnalyzer(pset),
    fTree(nullptr),
    fWireModuleLabel(pset.get<std::string>("WireModuleLabel"))
{} // function LArCVMaker::LArCVMaker

void LArCVMaker::ClearData() {

  fImageZ.clear();
} // function LArCVMaker::ClearData

void LArCVMaker::SetupAPAs() {

  fFirstAPA = std::floor(fFirstWire / 2560.);
  fLastAPA = std::floor(fLastWire / 2560.);
}  // function LArCVMaker::setupAPAs

void LArCVMaker::PadVector(std::vector<float> & vec, int new_size, int n_ticks) {

  std::vector<float> empty_channel;
  empty_channel.resize(n_ticks);

  int n_wires = vec.size() / n_ticks;
  if (n_wires < new_size) {
    vec.insert(vec.end(),empty_channel.begin(),empty_channel.end());
    n_wires = vec.size() / n_ticks;
    std::cout << "Resizing vector, adding " << new_size-n_wires << " new wires with " << n_ticks
        << " new time ticks, total " << n_ticks*(new_size-n_wires) << " new entries." << std::endl;
    vec.resize(vec.size() + (n_ticks*(new_size-n_wires)));
  }
} // function LArCVMaker::PadVector

void LArCVMaker::ProcessWire(recob::Wire w) {

  if (w.View() != 2 || (int)w.Channel() < fFirstWire || (int)w.Channel() > fLastWire) return;
  int channel_apa = std::floor(w.Channel() / 2560.);
  int channel_it = w.Channel() - (2560 * channel_apa) - 1600;

  int n_ticks = fLastTick - fFirstTick + 1;

  // get adc data
  std::vector<float>::const_iterator it_first = w.Signal().begin() + fFirstTick;
  std::vector<float>::const_iterator it_last = w.Signal().begin() + fLastTick + 1;

  PadVector(fImageZ, channel_it-1, n_ticks);
  fImageZ.insert(fImageZ.end(),it_first,it_last);
} // function LArCVMaker::GetNumberOfWires

void LArCVMaker::beginJob() {

  if (!fTree) {

    art::ServiceHandle<art::TFileService> tfs;
    fTree = tfs->make<TTree>("LArCV","LArCV tree");

    fTree->Branch("FirstAPA",&fFirstAPA,"FirstAPA/I");
    fTree->Branch("LastAPA",&fLastAPA,"LastAPA/I");
    fTree->Branch("FirstWire",&fFirstWire,"FirstWire/I");
    fTree->Branch("LastWire",&fLastWire,"LastWire/I");
    fTree->Branch("FirstTick",&fFirstTick,"FirstTick/I");
    fTree->Branch("LastTick",&fFirstTick,"LastTick/I");

    fTree->Branch("Event",&fEvent,"Event/I");
    fTree->Branch("APA",&fAPA,"APA/I");
    fTree->Branch("NumberTicks",&fNumberTicks,"NumberTicks/I");
    fTree->Branch("ImageZ","std::vector<float>",&fImageZ);
  }
} // function LArCVMaker::beginJob

void LArCVMaker::analyze(art::Event const & evt) {

  float max_adc = -1;

  fEvent = evt.event();

  std::cout << "Beginning analyze function..." << std::endl;

  art::Handle<std::vector<recob::Wire>> wireh;
  evt.getByLabel(fWireModuleLabel,wireh);

  int adc_cut = 50;

  fFirstAPA = -1;
  fLastAPA = -1;
  fFirstWire = -1;
  fLastWire = -1;
  fFirstTick = -1;
  fLastTick = -1;

  std::cout << "Looping over wires to find ROI...";

  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;

    if (wire.View() != 2) continue;

    for (int tick = 0; tick < (int)wire.Signal().size(); ++tick) {
      float adc = wire.Signal()[tick];
      if (adc > max_adc) max_adc = adc;
      if (adc > adc_cut) {
        if (fFirstWire == -1 || fFirstWire > (int)wire.Channel()) fFirstWire = wire.Channel();
        if (fLastWire == -1 || fLastWire < (int)wire.Channel()) fLastWire = wire.Channel();
        if (fFirstTick == -1 || fFirstTick > tick) fFirstTick = tick;
        if (fLastTick == -1 || fLastTick < tick) fLastTick = tick;
      }
    }
  }

  std::cout << " done." << std::endl;
  std::cout << "Max ADC value in this event is " << max_adc << "." << std::endl;
  std::cout << "First tick is " << fFirstTick << ", last tick is " << fLastTick << "." << std::endl;
  std::cout << "First wire is " << fFirstWire << ", last wire is " << fLastWire << "." << std::endl;
  std::cout << "Setting up APAs...";

  SetupAPAs();

  std::cout << " done." << std::endl;
  std::cout << "Processing wires..." << std::endl;

  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;
    int current_apa = std::floor(wire.Channel() / 2560.);
    if (it == wireh->begin()) fAPA = current_apa;
    else if (current_apa != fAPA) {
      fAPA = current_apa;
      if (fImageZ.size()/fNumberTicks < 960) PadVector(fImageZ,960,fNumberTicks);
      std::cout << "Size of fImageZ is " << fImageZ.size() << "." << std::endl;
      fTree->Fill();
      ClearData();
    }
    std::cout << "About to call ProcessWire...";
    ProcessWire(wire);
    std::cout << " done." << std::endl;
  }
  if (fImageZ.size()/fNumberTicks < 960) PadVector(fImageZ,960,fNumberTicks);
  fTree->Fill();
  ClearData();

  std::cout << " done." << std::endl;
} // function LArCVMaker::analyze

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

