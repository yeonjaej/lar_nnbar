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

namespace nnbar {

class LArCVMaker : public art::EDAnalyzer {
  
public:

  explicit LArCVMaker(fhicl::ParameterSet const & pset);
  void beginJob();
  //void endJob();
  void analyze(art::Event const & evt);

private:

  void ClearData();
  void FillHist(float i);

  TTree* fTree;
  std::string fWireModuleLabel;

  int fNumberWires;
  int fNumberTicks;
  std::vector<float> fMaxADC;

  std::vector<int> fBins;

  int fFirstWire;
  int fLastWire;
  int fFirstTick;
  int fLastTick;
}; // class LArCVMaker

LArCVMaker::LArCVMaker(fhicl::ParameterSet const & pset) :
    EDAnalyzer(pset),
    fTree(nullptr),
    fWireModuleLabel(pset.get<std::string>("WireModuleLabel"))
{} // function LArCVMaker::LArCVMaker

void LArCVMaker::ClearData() {

  fMaxADC.clear();
  fBins.clear();
} // function LArCVMaker::ClearData

void LArCVMaker::beginJob() {

  if (!fTree) {

    art::ServiceHandle<art::TFileService> tfs;
    fTree = tfs->make<TTree>("LArCV","LArCV tree");

    fTree->Branch("NumberWires",&fNumberWires,"NumberWires/I");
    fTree->Branch("NumberTicks",&fNumberTicks,"NumberTicks/I");
    fTree->Branch("MaxADC","std::vector<float>",&fMaxADC);

    fTree->Branch("Bins","std::vector<int>",&fBins);

    fTree->Branch("FirstWire",&fFirstWire,"FirstWire/I");
    fTree->Branch("LastWire",&fLastWire,"LastWire/I");
    fTree->Branch("FirstTick",&fFirstTick,"FirstTick/I");
    fTree->Branch("LastTick",&fFirstTick,"LastTick/I");
  }
} // function LArCVMaker::beginJob

void LArCVMaker::analyze(art::Event const & evt) {

  ClearData();

  fFirstWire = 1e5;
  fLastWire = -1;
  fFirstTick = 1e5;
  fLastTick = -1;

  fBins.resize(22);

  art::Handle<std::vector<recob::Wire>> wireh;
  evt.getByLabel(fWireModuleLabel,wireh);

  fNumberWires = wireh->size();

  int fADCCut = 20;
  int wire_no = 0;

  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;

    if (it == wireh->begin())
      fNumberTicks = wire.Signal().size();
    else if (fNumberTicks != (int)wire.Signal().size())
      throw cet::exception("LArCVMaker") << "Number of time ticks is not consistent between wires!";

    float max_adc = -1;
    int tick_no = 0;
    //std::cout << "About to loop through time ticks on wire..." << std::endl;
    for (std::vector<float>::const_iterator adc = wire.Signal().begin();
        adc != wire.Signal().end(); ++adc) {
      if (*adc > max_adc) max_adc = *adc;
      FillHist(*adc);
      if (*adc > fADCCut) {
        if (fFirstWire > wire_no) fFirstWire = wire_no;
        if (fLastWire < wire_no) fLastWire = wire_no;
        if (fFirstTick > tick_no) fFirstTick = tick_no;
        if (fLastTick < tick_no) fLastTick = tick_no;
      }
      ++tick_no;
    }
    //std::cout << "Done looping through time ticks." << std::endl;
    fMaxADC.push_back(max_adc);
    ++wire_no;
  }
  std::cout << "Done looping through wires on this event." << std::endl;

  fTree->Fill();

  std::cout << "Analyze function finished." << std::endl;
} // function LArCVMaker::analyze

void LArCVMaker::FillHist(float i) {

  for (int it = 0; it < 20; ++it) {
    if (i < (10 * it)) {
      ++fBins[i];
      return;
    }
  }
  ++fBins[21];
} // function LArCVMaker::FillHist

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

