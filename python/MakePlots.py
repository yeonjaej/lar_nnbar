import sys
from root_numpy import root2array
from matplotlib import pyplot as plt
from matplotlib import rc
from matplotlib.colors import colorConverter as cc
import PlotMaker as pm

def get_event_info(list_in,list_out):
  for n in list_in:
    list_out.append(n)

def get_object_info(list_in,list_out):
  for e in list_in:
    for n in e:
      list_out.append(n)

NumberPrimaries = []
NumberPrimariesTrackLike = []
NumberPrimariesShowerLike = []

NumberMCTracks = []
MCTrackLength = []
MCTrackMomentum = []

NumberMCShowers = []
MCShowerEnergy = []

NumberHits = []
HitWires = []
HitStartTime = []
HitPeakAmp = []
HitRMS = []
HitIntegral = []

NumberTracks = []
TrackLength = []
TrackMomentum = []

NumberShowers = []
ShowerEnergy = []

TrackMultiplicityDiff = []
ShowerMultiplicityDiff = []

TrueEventEnergy = []
TrueEventMomentum = []

MCRecoEventEnergy = []
MCRecoEventMomentum = []

RecoEventEnergy = []
RecoEventMomentum = []

sample = None
if len(sys.argv) == 2:
  sample = sys.argv[1]
else:
  print "Error: You must provide a sample as argument! Exiting..."
  exit(1)

for file_number in xrange(996):
  filename = './ana/{}/anahist_{}.root'.format(sample,file_number)
  array = root2array(filename,'nnbar/nnbar')

  # Primary information
  get_event_info(array['NumberPrimaries'],NumberPrimaries)
  get_event_info(array['NumberPrimariesTrackLike'],NumberPrimariesTrackLike)
  get_event_info(array['NumberPrimariesShowerLike'],NumberPrimariesShowerLike)

  # MC track information
  get_event_info(array['NumberMCTracks'],NumberMCTracks)
  get_object_info(array['MCTrackLength'],MCTrackLength)
  get_object_info(array['MCTrackMomentum'],MCTrackMomentum)

  # MC shower information
  get_event_info(array['NumberMCShowers'],NumberMCShowers)
  get_object_info(array['MCShowerEnergy'],MCShowerEnergy)

  # Hit information
  get_event_info(array['NumberHits'],NumberHits)
  get_event_info(array['HitWires'],HitWires)
  get_object_info(array['HitStartTime'],HitStartTime)
  get_object_info(array['HitPeakAmp'],HitPeakAmp)
  get_object_info(array['HitRMS'],HitRMS)
  get_object_info(array['HitIntegral'],HitIntegral)

  # Track information
  get_event_info(array['NumberTracks'],NumberTracks)
  get_object_info(array['TrackLength'],TrackLength)
  get_object_info(array['TrackMomentum'],TrackMomentum)

  # Shower information
  get_event_info(array['NumberShowers'],NumberShowers)
  get_object_info(array['ShowerEnergy'],ShowerEnergy)

  get_event_info(array['TrackMultiplicityDiff'],TrackMultiplicityDiff)
  get_event_info(array['ShowerMultiplicityDiff'],ShowerMultiplicityDiff)

  get_event_info(array['TrueEventEnergy'],TrueEventEnergy)
  get_event_info(array['TrueEventMomentum'],TrueEventMomentum)
  get_event_info(array['MCRecoEventEnergy'],MCRecoEventEnergy)
  get_event_info(array['MCRecoEventMomentum'],MCRecoEventMomentum)
  get_object_info(array['RecoEventEnergy'],RecoEventEnergy)
  get_object_info(array['RecoEventMomentum'],RecoEventMomentum)

  for n in array['TrueEventEnergy']:
    TrueEventEnergy.append(n)
  for n in array['TrueEventMomentum']:
    TrueEventMomentum.append(n)

font = {'family' : 'normal',
        'weight' : 'bold',
        'size'   : 16}

rc('font', **font)

# Primary information
pm.truth_only_plot(sample,'NumberPrimaries',NumberPrimaries,"Number of primary particles","Number of primaries","Number of events",[0,20],20,1,False)
pm.truth_only_plot(sample,'NumberPrimariesTrackLike',NumberPrimariesTrackLike,"Number of track-like primaries","Number of primaries","Number of events",[0,20],20,1,False)
pm.truth_only_plot(sample,'NumberPrimariesShowerLike',NumberPrimariesShowerLike,"Number of shower-like primaries","Number of primaries","Number of events",[0,20],20,1,False)

# Track information
pm.mcreco_reco_comparison(sample,'TrackMultiplicity',NumberMCTracks,NumberTracks,"Track multiplicity","Number of tracks","Number of events",[0,20],20,1,False)
pm.mcreco_reco_comparison(sample,'TrackLength',MCTrackLength,TrackLength,"Track length","Track length [cm]","Number of tracks",[0,50],80,1,False)
pm.mcreco_reco_comparison(sample,'TrackMomentum',MCTrackMomentum,TrackMomentum,"Track momentum","Track momentum [GeV]","Number of tracks",[0,1],50,1,False)

# Shower information
pm.mcreco_reco_comparison(sample,'ShowerMultiplicity',NumberMCShowers,NumberShowers,"Shower multiplicity","Number of showers","Number of events",[0,20],20,1,False)
pm.mcreco_reco_comparison(sample,'ShowerEnergy',MCShowerEnergy,ShowerEnergy,"Shower energy","Shower energy [GeV]","Number of showers",[0,1],50,1,False)

# Hit information
pm.reco_only_plot(sample,'NumberHits',NumberHits,"Number of hits","Number of hits","Number of events",[0,2000],50,1,False)
pm.reco_only_plot(sample,'HitWires',HitWires,"Number of hit wires","Number of hit wires","Number of events",[0,50],50,1,False)
pm.reco_only_plot(sample,'HitStartTime',HitStartTime,"Hit start time","Start time [TDC]","Number of hits",[3000,8000],50,1,False)
pm.reco_only_plot(sample,'HitPeakAmp',HitPeakAmp,"Hit peak amplitude","Peak amplitude [ADC]","Number of hits",[0,50],50,1,False)
pm.reco_only_plot(sample,'HitRMS',HitRMS,"Hit RMS","RMS [TDC]","Number of hits",[0,20],100,1,False)
pm.reco_only_plot(sample,'HitIntegral',HitIntegral,"Hit integral", "Integral [ADC x TDC]","Number of hits",[0,500],50,1,False)

# Analysis information
plt.hist(TrackMultiplicityDiff,bins=20,range=[-20,10],histtype='stepfilled',edgecolor=cc.to_rgba('k',1),facecolor=cc.to_rgba('k',0.4),label="mcreco-reco")
plt.title("Track multiplicity difference")
plt.xlabel("Track multiplicity (mcreco-reco)")
plt.ylabel("Number of events")
plt.legend(loc=1)
pm.draw_plot(sample,'TrackMultiplicityDiff')

plt.hist(ShowerMultiplicityDiff,bins=30,range=[-10,10],histtype='stepfilled',edgecolor=cc.to_rgba('k',1),facecolor=cc.to_rgba('k',0.4),label="mcreco-reco")
plt.title("Shower multiplicity difference")
plt.xlabel("Shower multiplicity (mcreco-reco)")
plt.ylabel("Number of events")
plt.legend(loc=1)
pm.draw_plot(sample,'ShowerMultiplicityDiff')

#pm.draw_comparison_plot('EventMomentum',TrueEventMomentum,MCRecoEventMomentum,RecoEventMomentum,"Event momentum","Momentum [GeV]","Number of events",[0,2],50,1,False)
#pm.draw_comparison_plot('EventEnergy',TrueEventEnergy,MCRecoEventEnergy,RecoEventEnergy,"Event energy","Energy [GeV]","Number of events",[0,2],50,1,False)

pm.truth_reco_comparison(sample,'EventMomentum',TrueEventMomentum,RecoEventMomentum,"Event momentum","Momentum [GeV]","Number of events",[0,2],50,1,False)
pm.truth_reco_comparison(sample,'EventEnergy',TrueEventEnergy,RecoEventEnergy,"Event energy","Energy [GeV]","Number of events",[0,2],50,1,False)

plt.hist2d(TrackLength,TrackMomentum,bins=[50,50])
plt.savefig("./plots/{}/TrackMomentumVsLength.png".format(sample))

