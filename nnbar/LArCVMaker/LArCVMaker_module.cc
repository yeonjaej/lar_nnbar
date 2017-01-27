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

//  void ClearData();

  TTree* fTree;
  std::string fWireModuleLabel;

  int fNumberWires;
  int fNumberTicks;

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

//void LArCVMaker::ClearData() {

//} // function LArCVMaker::ClearData

void LArCVMaker::beginJob() {

  if (!fTree) {

    art::ServiceHandle<art::TFileService> tfs;
    fTree = tfs->make<TTree>("LArCV","LArCV tree");

    fTree->Branch("FirstWire",&fFirstWire,"FirstWire/I");
    fTree->Branch("LastWire",&fLastWire,"LastWire/I");
    fTree->Branch("FirstTick",&fFirstTick,"FirstTick/I");
    fTree->Branch("LastTick",&fFirstTick,"LastTick/I");
  }
} // function LArCVMaker::beginJob

void LArCVMaker::analyze(art::Event const & evt) {

//  ClearData();

  art::Handle<std::vector<recob::Wire>> wireh;
  evt.getByLabel(fWireModuleLabel,wireh);

  fNumberWires = wireh->size();

  int adc_cut = 20;

  fFirstWire = -1;
  fLastWire = -1;
  fFirstTick = -1;
  fLastTick = -1;

  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;

    if (wire.View() != 2) continue;

    for (int tick = 0; tick < (int)wire.Signal().size(); tick++) {
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

  fTree->Fill();

  std::cout << "Analyze function finished." << std::endl;
} // function LArCVMaker::analyze

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

