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
  //void endJob();
  void analyze(art::Event const & evt);

private:

  void ClearData();
  void Downsample(int order);

  TTree* fTree;
  std::string fWireModuleLabel;
  int fADCCut;

  int fFirstAPA;
  int fLastAPA;
  int fFirstWire;
  int fLastWire;
  int fFirstTick;
  int fLastTick;

  int fEvent;
  int fAPA;
  int fNumberWires;
  int fNumberTicks;

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

void LArCVMaker::Downsample(int order) {

  std::cout << "Function Downsample called with order " << order << "." << std::endl;

  fNumberWires = std::ceil(fNumberWires/order);
  fNumberTicks = std::ceil(fNumberTicks/order);

  std::cout << "New resolution is " << fNumberWires << "x" << fNumberTicks << "." << std::endl;

  int apa_count = 0;
  for (int it_x = 0; it_x < fNumberWires; ++it_x) {
    for (int it_y = 0; it_y < fNumberTicks; ++it_y) {
      float pixel = 0;
      for (int x = 0; x < order; ++x) {
        for (int y = 0; y < order; ++y) {

          float adc;

          int wire_address = fFirstWire + (1600*apa_count) + (order*it_x) + x;
          if (wire_address%2560 < 1600) {
            ++apa_count;
            wire_address += 1600;
          }

          int time_address = fFirstTick + (order*it_y) + y;

          if (fWireMap.find(wire_address) != fWireMap.end())
            adc = fWireMap[wire_address][time_address];
          else adc = 0;
          pixel += adc;
        }
      }
      pixel /= pow(order,2);
      fImageZ.push_back(pixel);
    }
  }
} // function LArCVMaker::Downsample

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
    fTree->Branch("NumberWires",&fNumberWires,"NumberWires/I");
    fTree->Branch("NumberTicks",&fNumberTicks,"NumberTicks/I");

    fTree->Branch("ImageZ","std::vector<float>",&fImageZ);
  }
} // function LArCVMaker::beginJob

void LArCVMaker::analyze(art::Event const & evt) {

  fEvent = evt.event();
  auto const * geom = lar::providerFrom<geo::Geometry>();

  art::Handle<std::vector<recob::Wire>> wireh;
  evt.getByLabel(fWireModuleLabel,wireh);

  fFirstAPA = -1;
  fLastAPA = -1;
  fFirstWire = -1;
  fLastWire = -1;
  fFirstTick = -1;
  fLastTick = -1;

  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;
    if (wire.View() != 2) continue;

    int apa = std::floor(wire.Channel()/2560);
    double * wire_start = nullptr;
    double * wire_end = nullptr;
    geo::WireID wire_id;
    std::vector<geo::WireID> geo_wires = geom->ChannelToWire(wire.Channel());
    if (geo_wires.size() == 1) wire_id = geo_wires[0];
    else {
      std::cout << "Error! More than one wire per channel???" << std::endl;
      exit(1);
    }
    geom->WireEndPoints(wire_id,wire_start,wire_end);
    std::cout << "APA " << apa << " - Start [" << wire_start[0] << "," << wire_start[1]
        << "," << wire_start[2] << "] - End [" << wire_end[0] << "," << wire_end[1]
        << "," << wire_end[2] << "]" << std::endl;

    fWireMap.insert(std::pair<int,std::vector<float>>(wire.Channel(),std::vector<float>(wire.Signal())));

    for (int tick = 0; tick < (int)wire.Signal().size(); ++tick) {
      float adc = wire.Signal()[tick];
      if (adc > fADCCut) {
        if (fFirstWire == -1 || fFirstWire > (int)wire.Channel()) fFirstWire = wire.Channel();
        if (fLastWire == -1 || fLastWire < (int)wire.Channel()) fLastWire = wire.Channel();
        if (fFirstTick == -1 || fFirstTick > tick) fFirstTick = tick;
        if (fLastTick == -1 || fLastTick < tick) fLastTick = tick;
      }
    }
  }

  fFirstAPA = std::floor(fFirstWire / 2560.);
  fLastAPA = std::floor(fLastWire / 2560.);
  int NAPABoundaries = fLastAPA - fFirstAPA;
  fNumberWires = fLastWire - fFirstWire + 1 - (1600*NAPABoundaries);
  fNumberTicks = fLastTick - fFirstTick + 1;

  std::cout << "Original resolution of image is " << fNumberWires << "x" << fNumberTicks << "." << std::endl;
  if (fNumberWires > 2400 || fNumberTicks > 2400) Downsample(3);
  else if (fNumberWires > 600 || fNumberTicks > 600) Downsample(2);
  else Downsample(1);

  fTree->Fill();
  ClearData();
} // function LArCVMaker::analyze

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

