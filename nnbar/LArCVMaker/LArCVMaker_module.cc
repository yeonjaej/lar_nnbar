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
#include <map>

namespace nnbar {

class LArCVMaker : public art::EDAnalyzer {
  
public:

  explicit LArCVMaker(fhicl::ParameterSet const & pset);
  void beginJob();
  void analyze(art::Event const & evt);

private:

  void ClearData();
  void GenerateImages();

  TTree* fTree;
  std::string fWireModuleLabel;
  int fADCCut;

  int fFirstAPA;
  int fLastAPA;
  int fFirstTick;
  int fLastTick;

  int fEvent;
  int fAPA;
  int fNumberWiresOriginal;
  int fNumberTicksOriginal;
  int fNumberWiresDownsampled;
  int fNumberTicksDownsampled;

  std::map<int,std::vector<float>> fWireMap;
  std::vector<float> fImageZ;
}; // class LArCVMaker

LArCVMaker::LArCVMaker(fhicl::ParameterSet const & pset) :
    EDAnalyzer(pset),
    fTree(nullptr),
    fWireModuleLabel(pset.get<std::string>("WireModuleLabel")),
    fADCCut(pset.get<int>("ADCCut"))
{} // function LArCVMaker::LArCVMaker

void LArCVMaker::ClearData() {

  fWireMap.clear();
  fImageZ.clear();
} // function LArCVMaker::ClearData

void LArCVMaker::GenerateImages() {

  int order = 1;
  while (fNumberWiresOriginal/order > 600 || fNumberTicksOriginal/order > 600)
    ++order;

  fNumberWiresDownsampled = fNumberWiresOriginal/order;
  fNumberTicksDownsampled = std::ceil(fNumberTicksOriginal/order);

  if (order > 6) {
    std::cout << "Downsampling order is " << order << ", number of ticks in ROI is " << fNumberTicksOriginal << ". Skipping this event..." << std::endl;
    return;
  }

  for (int it_apa = fFirstAPA; it_apa < fLastAPA+1; ++it_apa) {
    fAPA = it_apa;
    int first_wire = (2560*it_apa) + 1600;
    for (int it_x = 0; it_x < fNumberWiresDownsampled; ++it_x) {
      for (int it_y = 0; it_y < fNumberTicksDownsampled; ++it_y) {
        float pixel = 0;
        for (int x = 0; x < order; ++x) {
          for (int y = 0; y < order; ++y) {
            float adc = 0;
            int wire_address = first_wire + (order*it_x) + x;
            int time_address = fFirstTick + (order*it_y) + y;
            if (fWireMap.find(wire_address) != fWireMap.end() && time_address < (int)fWireMap[wire_address].size())
              adc = fWireMap[wire_address][time_address];
            pixel += adc;
          }
        }
        pixel /= pow(order,2);
        fImageZ.push_back(pixel);
      }
    }
    fTree->Fill();
    fImageZ.clear();
  }
} // function LArCVMaker::GenerateImages

void LArCVMaker::beginJob() {

  if (!fTree) {

    art::ServiceHandle<art::TFileService> tfs;
    fTree = tfs->make<TTree>("LArCV","LArCV tree");

    fTree->Branch("FirstAPA",&fFirstAPA,"FirstAPA/I");
    fTree->Branch("LastAPA",&fLastAPA,"LastAPA/I");
    fTree->Branch("FirstTick",&fFirstTick,"FirstTick/I");
    fTree->Branch("LastTick",&fFirstTick,"LastTick/I");

    fTree->Branch("Event",&fEvent,"Event/I");
    fTree->Branch("APA",&fAPA,"APA/I");
    fTree->Branch("NumberWiresOriginal",&fNumberWiresOriginal,"NumberWiresOriginal/I");
    fTree->Branch("NumberTicksOriginal",&fNumberTicksOriginal,"NumberTicksOriginal/I");
    fTree->Branch("NumberWiresDownsampled",&fNumberWiresDownsampled,"NumberWiresDownsampled/I");
    fTree->Branch("NumberTicksDownsampled",&fNumberTicksDownsampled,"NumberTicksDownsampled/I");

    fTree->Branch("ImageZ","std::vector<float>",&fImageZ);
  }
} // function LArCVMaker::beginJob

void LArCVMaker::analyze(art::Event const & evt) {

  fEvent = evt.event();

  art::Handle<std::vector<recob::Wire>> wireh;
  evt.getByLabel(fWireModuleLabel,wireh);

  fFirstAPA = -1;
  fLastAPA = -1;
  fFirstTick = -1;
  fLastTick = -1;

  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;
    if (wire.View() != 2) continue;

    fWireMap.insert(std::pair<int,std::vector<float>>(wire.Channel(),std::vector<float>(wire.Signal())));

    for (int tick = 0; tick < (int)wire.Signal().size(); ++tick) {
      float adc = wire.Signal()[tick];
      int apa = std::floor(wire.Channel()/2560);
      if (adc > fADCCut) {
        if (fFirstAPA == -1 || fFirstAPA > apa) fFirstAPA = apa;
        if (fLastAPA == -1 || fLastAPA < apa) fLastAPA = apa;
        if (fFirstTick == -1 || fFirstTick > tick) fFirstTick = tick;
        if (fLastTick == -1 || fLastTick < tick) fLastTick = tick;
      }
    }
  }

  fNumberWiresOriginal = 960;
  fNumberTicksOriginal = fLastTick - fFirstTick + 1;

  GenerateImages();
  ClearData();
} // function LArCVMaker::analyze

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

