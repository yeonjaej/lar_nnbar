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
  void produce(art::Event const & evt);

private:

  void ClearData();

  TTree* fTree;
  std::string fWireModuleLabel;

  int fNumberWires;
  int fNumberTicks;
  std::vector<float> fMaxADC;

}; // class nnbar::LArCVMaker

void LArCVMaker::LArCVMaker(fhicl::ParameterSet const & pset) :
    EDAnalyzer(pset),
    fTree(nullptr),
    fWireModuleLabel(pst.get<std::string>("WireModuleLabel"))
{} // function LArCVMaker::LArCVMaker

void LArCVMaker::ClearData() {

  fMaxADC.clear();
} // function LArCVMaker::ClearData

void LArCVMaker::beginJob() {

  if (!fTree) {

    art::ServiceHandle<art::TFileService> tfs;
    fTree = tfs->make<TTree>("LArCV","LArCV tree");

    fTree->Branch("NumberWires",&fNumberWires,"NumberWires/I");
    fTree->Branch("NumberTicks",&fNumberTicks,"NumberTicks/I");
    fTree->Branch("MaxADC","std::vector<float>",&fMaxADC);
  }
} // LArCVMaker::beginJob

void LArCVMaker::analyze(art::Event const & evt) {

  art::Handle<std::vector<recob::Wire>> wireh;
  evt.getByLabel(fWireModuleLabel,wireh);

  fNumberWires = wireh->size();

  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;
    if (it == wireh.begin())
      fNumberTicks = wire.Signal().size();
    else {
      if (wire.Signal().size() != fNumberTicks)
        throw cet::exception("LArCVMaker") << "Number of time ticks is not consistent between wires!"
    }
    float max_adc = -1;
    for (std::vector<float>::const_iterator adc = wire.Signal().begin();
        adc != wire.Signal().end(); ++adc)
      if (*adc > max_adc) max_adc = *adc;
    fMaxADC.push_back(max_adc);
  }

  fTree->Fill();
  ClearData();
} // LArCVMaker::analyze

} // namespace nnbar

