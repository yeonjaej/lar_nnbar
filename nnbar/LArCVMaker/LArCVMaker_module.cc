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

// local includes
#include "Image2D.h"
#include "IOManager.h"

namespace nnbar {

class LArCVMaker : public art::EDAnalyzer {
  
public:

  explicit LArCVMaker(fhicl::ParameterSet const & pset);
  void analyze(art::Event const & evt);
  void beginJob();
  void endJob();

private:

  void ClearData();
  int FindBestAPA(std::vector<int> apas);

  larcv::IOManager fMgr;

  std::string fWireModuleLabel;
  int fMaxTick;
  int fADCCut;

  int fFirstWire;
  int fLastWire;
  int fFirstTick;
  int fLastTick;

  const int fNumberChannels[3] = ( 800, 800, 960 };
  const int fFirstChannel[3] = { 0, 800, 1600 };
  const int fLastChannel[3] = { 800, 1600, 2560 };

  int fEvent;
  int fAPA;
  int fNumberWires;
  int fNumberTicks;

  std::map<int,std::vector<float>> fWireMap;

}; // class LArCVMaker

LArCVMaker::LArCVMaker(fhicl::ParameterSet const & pset) :
    EDAnalyzer(pset),
    fMgr(pset),
    fWireModuleLabel(pset.get<std::string>("WireModuleLabel")),
    fMaxTick(pset.get<int>("MaxTick")),
    fADCCut(pset.get<int>("ADCCut"))
{} // function LArCVMaker::LArCVMaker

void LArCVMaker::beginJob() {

  fMgr.initialize();
} // function LArCVMaker::beginJob

void LArCVMaker::endJob() {

  fMgr.finalize();
} // function LArCVMaker::endJob

void LArCVMaker::ClearData() {

  fWireMap.clear();
} // function LArCVMaker::ClearData

int LArCVMaker::FindBestAPA(std::vector<int> apas) {

  int best_apa = -1;
  float best_adc = -1;

  for (int apa : apas) {

    float max_adc = 0;

    FindROI(apa);

    for (int it_plane = 0; it_plane < 3; ++it_plane) {
      for (int it_x = fFirstChannel[it_plane]; it_x < fLastChannel[it_plane]; ++it_x) {
        for (int it_y = 0; it_y < fMaxTick; ++it_y) {
          int wire_address = (apa * 2560) + it_x;
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
} // function LArCVMaker::FindBestAPA

void LArCVMaker::analyze(art::Event const & evt) {

  // get event number
  fEvent = evt.event();

  // get wire objects
  art::Handle<std::vector<recob::Wire>> wireh;
  evt.getByLabel(fWireModuleLabel,wireh);

  // initialize ROI finding variables
  std::vector<int> apas;

  // fill wire map
  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;
    fWireMap.insert(std::pair<int,std::vector<float>>(wire.Channel(),std::vector<float>(wire.Signal())));
    int apa = std::floor(wire.Channel()/2560);
    if (std::find(apas.begin(),apas.end(),apa) == apas.end())
      apas.push_back(apa);
  }

  // find best APA
  int best_apa = FindBestAPA(apas);
  if (best_apa != -1) fAPA = best_apa;

  // produce image
  auto images = (larcv::EventImage2D*)(fMgr.get_data(::larcv::kProductImage2D, "tpc"))
  for (int it_plane = 0; it_plane < 3; ++it_plane) {
    larcv::Image2D image(fNumberChannels,fMaxTick);
    for (int it_channel = fFirstChannel[it_plane]; it_channel < fLastChannel[it_plane]; ++it_channel) {
      int channel = it_channel + (fAPA * 2560);
      for (int it_tick = 0; it_tick < fMaxTick; ++it_tick) {
        if (fWireMap.find(channel) != fWireMap.end()) image.set_pixel(it_channel,it_tick,fWireMap[channel][it_tick]);
      }
    }
    image.compress(600,600);
    images->Emplace(std::move(image));
  }

  fMgr->save_entry();
} // function LArCVMaker::analyze

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

