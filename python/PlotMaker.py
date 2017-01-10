from matplotlib import pyplot as plt
from matplotlib.colors import colorConverter as cc

def draw_plot(sample,name):
  print "Making plot \"{}\"...".format(name)
  plt.tight_layout()
  plt.savefig('./plots/{}/{}.png'.format(sample,name))
  plt.clf()

def draw_comparison_plot(sample,ame,truth,mcreco,title,reco,xlabel,ylabel,rangex,bins,location,log):
  print "Making plot \"{}\"...".format(title)
  plt.hist(truth,bins=bins,log=log,range=rangex,histtype='stepfilled',edgecolor=cc.to_rgba('g',1),facecolor=cc.to_rgba('g',0.4),label="MC Truth")
  plt.hist(mcreco,bins=bins,log=log,range=rangex,histtype='stepfilled',edgecolor=cc.to_rgba('b',1),facecolor=cc.to_rgba('b',0.4),label="MC Reco")
  plt.hist(reco,bins=bins,log=log,range=rangex,histtype='stepfilled',edgecolor=cc.to_rgba('r',1),facecolor=cc.to_rgba('r',0.4),label="Reconstructed")
  plt.title(title)
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  plt.legend(loc=location)
  plt.tight_layout()
  # plt.show()
  plt.savefig('./plots/{}/{}.png'.format(sample,name))
  plt.clf()

def mcreco_reco_comparison(sample,name,mcreco,reco,title,xlabel,ylabel,rangex,bins,location,log):
  print "Making plot \"{}\"...".format(title)
  plt.hist(mcreco,bins=bins,log=log,range=rangex,histtype='stepfilled',edgecolor=cc.to_rgba('b',1),facecolor=cc.to_rgba('b',0.4),label="MC Reco")
  plt.hist(reco,bins=bins,log=log,range=rangex,histtype='stepfilled',edgecolor=cc.to_rgba('r',1),facecolor=cc.to_rgba('r',0.4),label="Reconstructed")
  plt.title(title)
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  plt.legend(loc=location)
  plt.tight_layout()
  # plt.show()
  plt.savefig('./plots/{}/{}.png'.format(sample,name))
  plt.clf()

def truth_reco_comparison(sample,name,truth,reco,title,xlabel,ylabel,rangex,bins,location,log):
  print "Making plot \"{}\"...".format(title)
  plt.hist(truth,bins=bins,log=log,range=rangex,histtype='stepfilled',edgecolor=cc.to_rgba('g',1),facecolor=cc.to_rgba('g',0.4),label="MC Truth")
  plt.hist(reco,bins=bins,log=log,range=rangex,histtype='stepfilled',edgecolor=cc.to_rgba('r',1),facecolor=cc.to_rgba('r',0.4),label="Reconstructed")
  plt.title(title)
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  plt.legend(loc=location)
  plt.tight_layout()
  # plt.show()
  plt.savefig('./plots/{}/{}.png'.format(sample,name))
  plt.clf()

def truth_only_plot(sample,name,truth,title,xlabel,ylabel,rangex,bins,location,log):
  print "Making plot \"{}\"...".format(title)
  plt.hist(truth,bins=bins,log=log,range=rangex,histtype='stepfilled',edgecolor=cc.to_rgba('g',1),facecolor=cc.to_rgba('g',0.4),label="MC Truth")
  plt.title(title)
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  plt.legend(loc=location)
  plt.tight_layout()
  # plt.show()
  plt.savefig('./plots/{}/{}.png'.format(sample,name))
  plt.clf()

def reco_only_plot(sample,name,reco,title,xlabel,ylabel,rangex,bins,location,log):
  print "Making plot \"{}\"...".format(title)
  plt.hist(reco,bins=bins,log=log,range=rangex,histtype='stepfilled',edgecolor=cc.to_rgba('r',1),facecolor=cc.to_rgba('r',0.4),label="Reconstructed")
  plt.title(title)
  plt.xlabel(xlabel)
  plt.ylabel(ylabel)
  plt.legend(loc=location)
  plt.tight_layout()
  # plt.show()
  plt.savefig('./plots/{}/{}.png'.format(sample,name))
  plt.clf()