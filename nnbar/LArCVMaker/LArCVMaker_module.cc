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

// external includes
#include "pngwriter.h"

namespace nnbar {

class LArCVMaker : public art::EDAnalyzer {
  
public:

  explicit LArCVMaker(fhicl::ParameterSet const & pset);
  void analyze(art::Event const & evt);

private:

  void ClearData();
  void Reset();
  void FindROI(int apa);
  int FindBestAPA(std::vector<int> apas);
  void GenerateImage();

  std::string fWireModuleLabel;
  int fADCCut;

  int fFirstWire;
  int fLastWire;
  int fFirstTick;
  int fLastTick;

  int fEvent;
  int fAPA;
  int fNumberWires;
  int fNumberTicks;

  std::map<int,std::vector<float>> fWireMap;
}; // class LArCVMaker

LArCVMaker::LArCVMaker(fhicl::ParameterSet const & pset) :
    EDAnalyzer(pset),
    fWireModuleLabel(pset.get<std::string>("WireModuleLabel")),
    fADCCut(pset.get<int>("ADCCut"))
{} // function LArCVMaker::LArCVMaker

void LArCVMaker::ClearData() {

  fWireMap.clear();
} // function LArCVMaker::ClearData

void LArCVMaker::Reset() {

  fFirstWire = -1;
  fLastWire = -1;
  fFirstTick = -1;
  fLastTick = -1;
} // function LArCVMaker::Reset

void LArCVMaker::FindROI(int apa) {

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

  fNumberWires = fLastWire - fFirstWire + 1;
  fNumberTicks = fLastTick - fFirstTick + 1;

  int order = 1;
  while (fNumberWires/order > 600 || fNumberTicks/order > 600)
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

  fNumberWires = fLastWire - fFirstWire + 1;
  fNumberTicks = fLastTick - fFirstTick + 1;

} // function LArCVMaker::FindROI

int LArCVMaker::FindBestAPA(std::vector<int> apas) {

  int best_apa = -1;
  float best_adc = -1;

  for (int apa : apas) {

    float max_adc = 0;

    FindROI(apa);

    for (int it_x = 0; it_x < fNumberWires; ++it_x) {
      for (int it_y = 0; it_y < fNumberTicks; ++it_y) {
        int wire_address = fFirstWire + it_x;
        int tick_address = fFirstTick + it_y;
        if (fWireMap.find(wire_address) != fWireMap.end() && tick_address < (int)fWireMap[wire_address].size())
          max_adc += fWireMap[wire_address][tick_address];
      }
    }

    if (max_adc > best_adc || best_apa == -1) {
      best_apa = apa;
      best_adc = max_adc;
    }
  }

  return best_apa;
} // function LArCVMaker::FindBestAPA

void LArCVMaker::GenerateImage() {

  FindROI(fAPA);

  if (fNumberWires < 100 || fNumberTicks < 100) return;

  float min = 0;
  float max = 0;

  for (int it_x = 0; it_x < fNumberWires; ++it_x) {
    std::vector<float> wire;
    for (int it_y = 0; it_y < fNumberTicks; ++it_y) {
      float adc = 0;
      int wire_address = fFirstWire + it_x;
      int tick_address = fFirstTick + it_y;
      if (fWireMap.find(wire_address) != fWireMap.end() && tick_address < (int)fWireMap[wire_address].size())
        adc = fWireMap[wire_address][tick_address];
      if (it_x == 0 && it_y == 0) {
        min = adc;
        max = adc;
      }
      else if (adc < min) min = adc;
      else if (adc > max) max = adc;
    }
  }

  float base_colour;
  if (min < 0) base_colour = 1 + (min / (max-min));
  else base_colour = 1;

  std::vector<std::vector<float>> image;
  for (int it_x = 0; it_x < fNumberWires; ++it_x) {
    std::vector<float> wire;
    for (int it_y = 0; it_y < fNumberTicks; ++it_y) {
      float adc = base_colour;
      int wire_address = fFirstWire + it_x;
      int tick_address = fFirstTick + it_y;
      if (fWireMap.find(wire_address) != fWireMap.end() && tick_address < (int)fWireMap[wire_address].size())
        adc = 1 - ((fWireMap[wire_address][tick_address]-min)/(max-min));
      wire.push_back(adc);
    }
    image.push_back(wire);
  }
/*
  std::string filename;
  char const * outdir = std::getenv("CONDOR_DIR_IMAGE");
  if (outdir == NULL) {
    std::cout << "Error! CONDOR_DIR_IMAGE is not set! Exiting..." << std::endl;
    exit(1);
  }
  else filename = std::string(outdir)+"/ImageZ_"+std::to_string(fEvent)+"_"+std::to_string(fAPA)+".png";
*/
  std::string filename = "./ImageZ_"+std::to_string(fEvent)+"_"+std::to_string(fAPA)+".png";
  pngwriter image_maker(fNumberWires, fNumberTicks, base_colour, filename.c_str());

  for (int it_x = 0; it_x < fNumberWires; ++it_x)
    for (int it_y = 0; it_y < fNumberTicks; ++it_y)
      image_maker.plotHSV(it_x+1, it_y+1, 0., 0., image[it_x][it_y]);

  image_maker.scale_kxky(4,1);
  double image_size;
  if (fNumberWires > fNumberTicks) image_size = fNumberWires;
  else image_size = fNumberTicks;
  double scale_factor = 600. / image_size;
  image_maker.scale_k(scale_factor);

  image_maker.write_png();
} // function LArCVMaker::GenerateImage

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
    int apa = std::floor(wire.Channel()/2560);
    if (std::find(apas.begin(),apas.end(),apa) == apas.end())
      apas.push_back(apa);
  }

  // find best APA and generate image
  int best_apa = FindBestAPA(apas);
  if (best_apa != -1) fAPA = best_apa;
  GenerateImage();
  ClearData();

} // function LArCVMaker::analyze

DEFINE_ART_MODULE(LArCVMaker)

} // namespace nnbar

