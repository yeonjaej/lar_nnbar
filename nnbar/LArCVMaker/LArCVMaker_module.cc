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
  void Reset();
  void GenerateImage();

  TTree* fTree;
  std::string fWireModuleLabel;
  int fADCCut;

  int fFirstWire;
  int fLastWire;
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

void LArCVMaker::Reset() {

  fFirstWire = -1;
  fLastWire = -1;
  fFirstTick = -1;
  fLastTick = -1;
} // function LArCVMaker::Reset

void LArCVMaker::GenerateImage() {

  int order = 1;
  while (fNumberWiresOriginal/order > 600 || fNumberTicksOriginal/order > 600)
    ++order;

  int margin = 10 * order;
  if (fFirstWire-margin < (fAPA*2560)+1600) fFirstWire = (fAPA*2560)+1600;
  else fFirstWire -= margin;
  if (fLastWire+margin > ((fAPA+1)*2560)-1) fLastWire = ((fAPA+1)*2560)-1;
  else fLastWire += margin;
  if (fFirstTick-margin < 0) fFirstTick = 0;
  else fFirstTick -= margin;
  if (fLastTick+margin > 4492) fLastTick = 4492;
  else fLastTick += margin;

  fNumberWiresOriginal = fLastWire - fFirstWire + 1;
  fNumberTicksOriginal = fLastTick - fFirstTick + 1;

  fNumberWiresDownsampled = std::ceil(fNumberWiresOriginal/order);
  fNumberTicksDownsampled = std::ceil(fNumberTicksOriginal/order);

  if (order > 6) {
    std::cout << "Downsampling order is " << order << ", original resolution is "
        << fNumberWiresOriginal << "x" << fNumberTicksOriginal << ". Skipping this event..." << std::endl;
    return;
  }

  for (int it_x = 0; it_x < fNumberWiresDownsampled; ++it_x) {
    for (int it_y = 0; it_y < fNumberTicksDownsampled; ++it_y) {
      float pixel = 0;
      for (int x = 0; x < order; ++x) {
        for (int y = 0; y < order; ++y) {
          float adc = 0;
          int wire_address = fFirstWire + (order*it_x) + x;
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
} // function LArCVMaker::GenerateImage

void LArCVMaker::beginJob() {

  if (!fTree) {

    art::ServiceHandle<art::TFileService> tfs;
    fTree = tfs->make<TTree>("LArCV","LArCV tree");

    fTree->Branch("FirstWire",&fFirstWire,"FirstWire/I");
    fTree->Branch("LastWire",&fLastWire,"LastWire/I");
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

  // get event number
  fEvent = evt.event();

  // get wire objects
  art::Handle<std::vector<recob::Wire>> wireh;
  evt.getByLabel(fWireModuleLabel,wireh);

  // initialize ROI finding variables
  Reset();
  std::vector<int> apas;

  // fill wire map
  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;
    if (wire.View() != 2) continue;
    fWireMap.insert(std::pair<int,std::vector<float>>(wire.Channel(),std::vector<float>(wire.Signal())));

    std::cout << "Number of ticks is " << wire.Signal().size() << "." << std::endl;

    int apa = std::floor(wire.Channel()/2560);
    if (std::find(apas.begin(),apas.end(),apa) == apas.end())
      apas.push_back(apa);
  }

  // identify ROI in each APA
  for (int apa : apas) {
    for (int channel = (2560*apa)+1600; channel < (2560*(apa+1))-1; ++channel) {
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

    // generate image
    if (fFirstWire != -1) {
      fAPA = apa;
      fNumberWiresOriginal = fLastWire - fFirstWire + 1;
      fNumberTicksOriginal = fLastTick - fFirstTick + 1;
      GenerateImage();
    }
    else std::cout << "Couldn't find any wires in this APA above the threshold." << std::endl;
    Reset();
  }

  ClearData();
} // function LArCVMaker::analyze

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

