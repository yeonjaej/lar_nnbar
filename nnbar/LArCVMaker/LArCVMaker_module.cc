// framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "fhiclcpp/ParameterSet.h"

// data product includes
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RecoBase/Wire.h"

// root includes
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
  void ProcessWire(recob::Wire w);

  TTree* fTree;
  std::string fWireModuleLabel;

  int fNumberWires;
  int fNumberTicks;

  int fFirstAPA;
  int fLastAPA;
  int fFirstWire;
  int fLastWire;
  int fFirstTick;
  int fLastTick;

  std::vector<std::vector<std::vector<float>>> fImageZ;
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
  fImageZ.resize(fLastAPA - fFirstAPA + 1);
}

void LArCVMaker::ProcessWire(recob::Wire w) {

  if (w.View() != 2 || wire.Channel() < fFirstWire || wire.Channel() > fLastChannel) return;
  int channel_apa = std::floor(wire.Channel() / 2560.);
  int channel_it = wire.Channel() - (2560 * channel_apa) - 1600;
  int apa_it = channel_apa - fFirstAPA;

  // make placeholder adc data
  int n_ticks = fLastTick - fFirstWire + 1;
  std::vector<float> empty_channel;
  empty_channel.resize(n_ticks);

  // get adc data
  it_first = wire.Signal().begin() + fFirstTick;
  it_last = wire.Signal().begin() + fLastTick + 1;
  std::vector<float> this_channel(it_first,it_last);

  while ((int)fImageZ[apa_it].size() < channel_it-1) {
    fImageZ[apa_it].push_back(empty_channel);
  }
  fImageZ[apa_it].push_back(this_channel);
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

    fTree->Branch("ImageZ","std::vector<std::vector<std::vector<float>>>",&fImageZ);
  }
} // function LArCVMaker::beginJob

void LArCVMaker::analyze(art::Event const & evt) {

  art::Handle<std::vector<recob::Wire>> wireh;
  evt.getByLabel(fWireModuleLabel,wireh);

  fNumberWires = wireh->size();

  int adc_cut = 20;

  fFirstAPA = -1;
  fLastAPA = -1;
  fFirstWire = -1;
  fLastWire = -1;
  fFirstTick = -1;
  fLastTick = -1;

  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;

    std::cout << "Iterator value is " << it << "." << std::endl;

    if (wire.View() != 2) continue;

    for (int tick = 0; tick < (int)wire.Signal().size(); ++tick) {
      float adc = wire.Signal()[tick];
      if (adc > adc_cut) {
        if (fFirstWire == -1 || fFirstWire > (int)wire.Channel()) fFirstWire = wire.Channel();
        if (fLastWire == -1 || fLastWire < (int)wire.Channel()) fLastWire = wire.Channel();
        if (fFirstTick == -1 || fFirstTick > tick) fFirstTick = tick;
        if (fLastTick == -1 || fLastTick < tick) fLastTick = tick;
      }
    }
  }
  std::cout << "First tick is " << fFirstTick << ", last tick is " << fLastTick << "." << std::endl;
  std::cout << "First wire is " << fFirstWire << ", last wire is " << fLastWire << "." << std::endl;

  SetupAPAs();

  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;
    ProcessWire(wire);
  }

  fTree->Fill();
  ClearData();
} // function LArCVMaker::analyze

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

