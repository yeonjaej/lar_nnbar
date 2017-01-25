// framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"

// data product includes
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RecoBase/Wire.h"

// root includes
#include "TTree.h"

// c++ includes
#include <memory>
#include <string>

namespace nnbar {

class LArCVMaker : public art::EDAnalyzer {
  
public:

  explicit LArCVMaker(fhicl::ParameterSet const & pset);
  void beginJob();
  //void endJob();
  void analyze(art::Event const & evt);

private:

  void ClearData();
  void FillHist(int i);

  TTree* fTree;
  std::string fWireModuleLabel;

  int fNumberWires;
  int fNumberTicks;
  std::vector<float> fMaxADC;

  std::vector<int> fBins;

  int fFirstWire = 1e5;
  int fLastWire = -1;
  int fFirstTick = 1e5;
  int fLastTick = -1;
}; // class nnbar::LArCVMaker

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

  fFirstWire = 1e5;
  fLastWire = -1;
  fFirstTick = 1e5;
  fLastTick = -1;

  bins.resize(22);

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
} // LArCVMaker::beginJob

void LArCVMaker::analyze(art::Event const & evt) {

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
    else {
      if (fNumberTicks != (int)wire.Signal().size())
        throw cet::exception("LArCVMaker") << "Number of time ticks is not consistent between wires!";
    }
    float max_adc = -1;
    int tick_no = 0;
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
    fMaxADC.push_back(max_adc);
    ++wire_no;
  }

  fTree->Fill();
  ClearData();
} // LArCVMaker::analyze

void LArCVMaker::FillHist(int i) {

  for (int it = 0; it < 21; ++it) {
    if (i < (10 * it)) {
      ++fBins[i];
      return;
    }
  }
  ++fBins[22];
} // LArCVMaker::FillHist

} // namespace nnbar

