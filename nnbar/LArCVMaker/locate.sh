#!/bin/bash
filename='prod_bnb_optfilter_mcc9.0_reco1_C1_files.list'
while read p; do 
    PLINE=$(samweb locate-file $p)
    PLINE=${PLINE#enstore:}
    #echo ${#PLINE}
    #PLINE=${PLINE#$X)}
    PLINE=${PLINE:0:95}
    echo $PLINE'/'$p
done < $filename 
