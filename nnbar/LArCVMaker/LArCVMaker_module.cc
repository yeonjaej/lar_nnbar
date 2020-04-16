// framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDAnalyzer.h"
//#include "art/Framework/Services/Optional/TFileService.h"
#include "fhiclcpp/ParameterSet.h"

// data product includes
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RecoBase/Wire.h"

//#include "lardataobj/Simulation/SupernovaTruth.h"

// root includes
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"

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
#include "larcv/core/DataFormat/EventImage2D.h"
//#include "larcv/core/DataFormat/Image2D.h"
#include "larcv/core/DataFormat/EventParticle.h"
#include "larcv/core/DataFormat/Particle.h"
//#include "DataFormat/EventROI.h"
#include "larcv/core/DataFormat/IOManager.h"

#include <iostream>
#include <fstream>

namespace nnbar {

class LArCVMaker : public art::EDAnalyzer {
  
public:

  explicit LArCVMaker(fhicl::ParameterSet const & pset);
  void analyze(art::Event const& evt);
  void beginJob();
  void endJob();
  

private:

  void ClearData();
  void ResetROI();
  void SetROISize();
  int FindAPAWithNeutrino(std::vector<int> apas, art::Event const & evt);
  int FindROI(int apa, int plane);

  larcv::IOManager fMgr;

  std::string fWireModuleLabel;
  int fMaxTick;
  int fADCCut;
  int fEventType;

  int fFirstWire;
  int fLastWire;
  int fFirstTick;
  int fLastTick;

  const int fNumberChannels[3] = { 2400, 2400, 3456 };
  const int fFirstChannel[3] = { 0, 2400, 4800 };
  const int fLastChannel[3] = { 2399, 4799, 8255 };

  int fEvent;
  int fAPA;
  int fNumberWires;
  int fNumberTicks;

  std::map<int,std::vector<float>> fWireMap;
  std::vector<std::vector<float>> fImage;
  //std::ofstream pdg;
  TH1D* hADCSpectrum;
  TFile* SpectrumFile;
  std::ofstream pdg;
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
  std::ofstream pdg;
  
  std::string filename;
  if (std::getenv("PROCESS") != nullptr) filename = "larcv_" + std::string(std::getenv("PROCESS")) + ".root";
  else filename = "larcv.root";
  fMgr.set_out_file(filename);
  fMgr.initialize();
  SpectrumFile = new TFile("./SignalADCSpectrum.root","RECREATE");
  hADCSpectrum = new TH1D("hADCSpectrum","ADC Spectrum Collection; ADC; Entries",4096, 0, 4096);
} // function LArCVMaker::beginJob

void LArCVMaker::endJob() {
  
  SpectrumFile->cd();
  hADCSpectrum->Write();
  SpectrumFile->Close();
  fMgr.finalize();
} // function LArCVMaker::endJob

void LArCVMaker::ClearData() {

  //  ResetROI();
  //  fAPA = -1;
  fWireMap.clear();
} // function LArCVMaker::ClearData




void LArCVMaker::analyze(art::Event const & evt) {

  ClearData();


  fEvent = evt.event();

  fMgr.set_id(evt.id().run(),evt.id().subRun(),evt.id().event());

  // get wire objects
  art::Handle<std::vector<recob::Wire>> wireh;
  evt.getByLabel(fWireModuleLabel,wireh);

  // fill wire map
  for (std::vector<recob::Wire>::const_iterator it = wireh->begin();
      it != wireh->end(); ++it) {
    const recob::Wire & wire = *it;
    fWireMap.insert(std::pair<int,std::vector<float>>(wire.Channel(),std::vector<float>(wire.Signal())));

  }
  // get handle on larcv image
  //std::cout << "0.0" <<std::endl;
  auto images = (larcv::EventImage2D*)(fMgr.get_data("image2d", "tpc"));
  //std::cout << "0.1" << std::endl;
  size_t image_width[3] = { 2400, 2400, 3600 };
  //size_t ticks = 6400;
  //std::cout << "0.2" << std::endl;
  //auto const& image = new larcv::Image2D
  //  std::vector<larcv::Image2D> image_v;
  //  std::cout << "0.21" << std::endl;
  //larcv::Image2D image_0(400, 400);
  //std::cout << "0.22" << std::endl;
  //image_v.push_back(image_0);
  //std::cout << "0.221" << std::endl;  
  //larcv::Image2D image_1(2400, 400);
  //image_v.push_back(image_1);
  //std::cout << "0.222" << std::endl;
  //larcv::Image2D image_2(3600, 6400);
  //image_v.push_back(image_2);
  //larcv::Image2D temp_img_k(400,400);
  //std::cout << temp_img_k.size() << std::endl;
  
  //std::cout << "0.23" << std::endl;
  for (size_t it_plane = 0; it_plane < 3; ++it_plane) {

    larcv::Image2D image_temp(400,400);
    std::cout << "Want 400x400 images with correct meta" << std::endl;
    std::cout << "cols : "<< image_temp.meta().cols() << " , rows: "<< image_temp.meta().rows()  << std::endl;  
    //float px= image_temp.pixel(0,0);    
    std::cout << "is it 400 400"  << std::endl;
    float px= image_temp.pixel(0,0);
    std::cout << "px valuve at (0,0) : "<< px << std::endl; 

    for (size_t it_channel = 0; it_channel < image_width[it_plane]; ++it_channel) {
      std::cout << "0.5" << std::endl;
      int channel = it_channel + fFirstChannel[it_plane];
      std::cout << "0.6" << std::endl;
      for (int it_tick = 0; it_tick < fMaxTick; ++it_tick) {
	std::cout << "0.7" << std::endl;
	//wire_idx = it_channel/wire_f;
	std::cout << "0.8" << std::endl;
	//tick_idx = it_tick/tick_f;

	std::cout << "0.9" << std::endl;
	
	
	//if (fWireMap.find(channel) != fWireMap.end()) {
	// std::cout << "1.0" << std::endl;
	// pixel_update = temp_img.pixel(wire_idx, tick_idx)+fWireMap[channel][it_tick];
	//  std::cout << "1.1" << std::endl;
	//  temp_img.set_pixel(wire_idx,tick_idx, pixel_update);
	//  std::cout << "1.2" << std::endl;
	//	}
	//std::cout << "1.3" << std::endl;
        //else {
	  
	//}image_v.at(it_plane).set_pixel(it_channel,it_tick,0);
        if (fWireMap.find(channel) != fWireMap.end()) image_temp.set_pixel(it_channel,it_tick,fWireMap[channel][it_tick]);
        else image_temp.set_pixel(it_channel,it_tick,0);
	//std::cout << "1.4" << std::endl;
      }
      std::cout << "-1" <<std::endl;
    }
    std::cout << "0" <<std::endl;
    //images->emplace(std::move(image_temp));
    images->emplace(std::move(image_temp)); 
  }

  std::cout << "1" <<std::endl; 
  auto particles = (larcv::EventParticle*)(fMgr.get_data("particle", "tpc"));

  std::cout << "2" <<std::endl;

  larcv::Particle particle_temp; //:_pdg=fEventType;

  std::cout << "3" <<std::endl;
  particle_temp.id(fEventType);

  std::cout << "4" <<std::endl;
  particles->append(particle_temp);

  std::cout << "5" <<std::endl;
  std::vector<larcv::Particle> part_v_temp;

  std::cout << "6" <<std::endl;
  part_v_temp.push_back(particle_temp);
  //std::vector<larcv::Particle>&& part_v = part_v_temp;
  //  part_v.push_back(particle_temp);

  //pset.emplace(part_v);


  //auto roi = (larcv::EventROI*)(fMgr.get_data(larcv::kProductROI, "tpc"));

  //roi->Emplace(larcv::ROI((larcv::ROIType_t)fEventType));
  std::cout << "7" <<std::endl;
  fMgr.save_entry();
  std::cout << "8" <<std::endl;
} // function LArCVMaker::analyze

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

