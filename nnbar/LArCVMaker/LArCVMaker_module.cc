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
#include "DataFormat/EventImage2D.h"
#include "DataFormat/EventROI.h"
#include "DataFormat/IOManager.h"

namespace nnbar {

class LArCVMaker : public art::EDAnalyzer {
  
public:

  explicit LArCVMaker(fhicl::ParameterSet const & pset);
  void analyze(art::Event const & evt);
  void beginJob();
  void endJob();

private:

  void ClearData();
  void ResetROI();
  void SetROISize();
  int FindBestAPA(std::vector<int> apas);
  int FindROI(int apa, int plane);

  larcv::IOManager fMgr;

  std::string fWireModuleLabel;
  int fMaxTick;
  int fADCCut;
  int fEventType;

  const int fNumberChannels[3] = { 2400, 2400, 3456 };
  const int fFirstChannel[3] = { 0, 2400, 4800 };
  const int fLastChannel[3] = { 2399, 4799, 8255 };

  int fEvent;
  int fAPA;
  int fNumberWires;
  int fNumberTicks;

  std::map<int,std::vector<float> > fWireMap;
  std::vector<std::vector<float> > fImage;

}; // class LArCVMaker

LArCVMaker::LArCVMaker(fhicl::ParameterSet const & pset) :
    EDAnalyzer(pset),
    fMgr(larcv::IOManager::kWRITE),
    fWireModuleLabel(pset.get<std::string>("WireModuleLabel")),
    fMaxTick(pset.get<int>("MaxTick")),
    fADCCut(pset.get<int>("ADCCut")),
    fEventType(pset.get<int>("EventType"))
{} // function LArCVMaker::LArCVMaker

void LArCVMaker::beginJob() {

  std::string filename;
  if (std::getenv("PROCESS") != nullptr) filename = "larcv_" + std::string(std::getenv("PROCESS")) + ".root";
  else filename = "larcv.root";
  fMgr.set_out_file(filename);
  fMgr.initialize();
} // function LArCVMaker::beginJob

void LArCVMaker::endJob() {

  fMgr.finalize();
} // function LArCVMaker::endJob

void LArCVMaker::ClearData() {

  fWireMap.clear();
} // function LArCVMaker::ClearData

void LArCVMaker::analyze(art::Event const & evt) {

  ClearData();

  // get event number
  fEvent = evt.event();

  fMgr.set_id(evt.id().run(),evt.id().subRun(),evt.id().event());

  // get wire objects
  art::Handle<std::vector<recob::Wire>> wireh;
  evt.getByLabel(fWireModuleLabel,wireh);

  // loop over each wire and add it to the wire map
  for (std::vector<recob::Wire>::const_iterator it = wireh->begin(); it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;
    fWireMap.insert(std::pair<int,std::vector<float> >(wire.Channel(),std::vector<float>(wire.Signal())));
  }

  // get handle on larcv image
  auto images = (larcv::EventImage2D*)(fMgr.get_data(larcv::kProductImage2D, "tpc"));

  int image_width[3] = { 2400, 2400, 3600 };
  // create images from the wire map
  for (int it_plane = 0; it_plane < 3; ++it_plane) {

//    larcv::Image2D image(fNumberChannels[it_plane],fMaxTick);
    larcv::Image2D image_temp(image_width[it_plane],6400);
    for (int it_channel = 0; it_channel < image_width[it_plane]; ++it_channel) {
      int channel = it_channel + fFirstChannel[it_plane];
      for (int it_tick = 0; it_tick < fMaxTick; ++it_tick) {
        if (fWireMap.find(channel) != fWireMap.end()) image_temp.set_pixel(it_channel,it_tick,fWireMap[channel][it_tick]);
        else image_temp.set_pixel(it_channel,it_tick,0);
      }
    }
    if (it_plane < 2)
      image_temp.compress(600,640);
    else
      image_temp.compress(600,640);
    larcv::Image2D image(600,640);
    for (int it_x = 0; it_x < 600; ++it_x) {
      for (int it_y = 0; it_y < 640; ++it_y) {
        image.set_pixel(it_x,it_y,image_temp.pixel(it_x,it_y));
      }
    }
    images->Emplace(std::move(image));
  }

  auto roi = (larcv::EventROI*)(fMgr.get_data(larcv::kProductROI, "tpc"));
  roi->Emplace(larcv::ROI((larcv::ROIType_t)fEventType));

  fMgr.save_entry();
} // function LArCVMaker::analyze

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

