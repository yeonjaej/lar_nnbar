// framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDAnalyzer.h"
//#include "art/Framework/Services/Optional/TFileService.h"
#include "fhiclcpp/ParameterSet.h"

// data product includes
#include "larcore/Geometry/Geometry.h"
#include "lardataobj/RecoBase/Wire.h"

#include "nusimdata/SimulationBase/MCTruth.h"//for signal


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
#include "DataFormat/EventImage2D.h"
#include "DataFormat/EventROI.h"
#include "DataFormat/IOManager.h"

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

  ResetROI();
  //  fAPA = -1;
  fWireMap.clear();
} // function LArCVMaker::ClearData

void LArCVMaker::ResetROI() {

  fFirstWire = -1;
  fLastWire = -1;
  fFirstTick = -1;
  fLastTick = -1;
  fNumberWires = -1;
  fNumberTicks = -1;
} // function LArCVMaker::ResetROI

void LArCVMaker::SetROISize() {

  fNumberWires = fLastWire - fFirstWire + 1;
  fNumberTicks = fLastTick - fFirstTick + 1;
}

int LArCVMaker::FindROI(int best_apa, int plane) {
  
  ResetROI();

  std::cout << "setting ROI as full APA" <<std::endl;

  fFirstTick=0;
  fLastTick=4487;

  if (plane==0) {
    fFirstWire = (2560*best_apa);
    fLastWire = (2560*best_apa)+799;
  }

  if (plane==1) {
    fFirstWire = (2560*best_apa)+800;
    fLastWire = (2560*best_apa)+1599;
  }

  if (plane==2) {
    fFirstWire = (2560*best_apa)+1600;
    fLastWire = (2560*best_apa)+2559;
  }
  std::cout <<"ROI set!"<<std::endl; 
 
  
  SetROISize();

  int downsample = 1;
  return downsample;
} // function LArCVMaker::FindROI

void LArCVMaker::analyze(art::Event const & evt) {

  ClearData();


  fEvent = evt.event();

  fMgr.set_id(evt.id().run(),evt.id().subRun(),evt.id().event());

  //nnbar signal only
  /* art::Handle<std::vector<simb::MCTruth>> TruthListHandle;
  std::vector<art::Ptr<simb::MCTruth>> TruthList;
  if (evt.getByLabel("nnbar",TruthListHandle))

  art::fill_ptr_vector(TruthList,TruthListHandle);
  art::Ptr<simb::MCTruth> mct = TruthList.at(0);

  TVector3  vertex_position = mct->GetParticle(0).Position(0).Vect();
  art::ServiceHandle<geo::Geometry> geo;
  std::cout << "n tpc in uboone? " << geo->NTPC() << std::endl;
  const geo::TPCGeo & tpc = geo->TPC(0);
  std::cout <<"ver_x : "<<vertex_position.X()<<" , ver_y : "<<vertex_position.Y()<<" , ver_z : "<<vertex_position.Z()<< std::endl;
  std::cout << "vertex contained in tpc ? " << tpc.ContainsPosition(vertex_position) << std::endl;
  */
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
  auto images = (larcv::EventImage2D*)(fMgr.get_data(larcv::kProductImage2D, "tpc"));

  int image_width[3] = { 2400, 2400, 3600 };
  
  for (int it_plane = 0; it_plane < 3; ++it_plane) {
    larcv::Image2D image_temp(image_width[it_plane],6400);
    for (int it_channel = 0; it_channel < image_width[it_plane]; ++it_channel) {
      int channel = it_channel + fFirstChannel[it_plane];
      for (int it_tick = 0; it_tick < fMaxTick; ++it_tick) {
        if (fWireMap.find(channel) != fWireMap.end()) {
	  image_temp.set_pixel(it_channel,it_tick,fWireMap[channel][it_tick]);
	  if (it_plane==2 && fWireMap[channel][it_tick]!=0) hADCSpectrum->Fill(fWireMap[channel][it_tick]);
	}
        else image_temp.set_pixel(it_channel,it_tick,0);
      }
    }
    images->Emplace(std::move(image_temp));
  }
  
  auto roi = (larcv::EventROI*)(fMgr.get_data(larcv::kProductROI, "tpc"));

  roi->Emplace(larcv::ROI((larcv::ROIType_t)fEventType));

  fMgr.save_entry();

} // function LArCVMaker::analyze

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

