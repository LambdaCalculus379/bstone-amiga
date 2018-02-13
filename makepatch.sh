#!/bin/sh
cwd=$(pwd)
xdelta3 -e -s $1/AUDIOHED.BS6 $2/AUDIOHED.BS6
xdelta3 -e -s $1/AUDIOT.BS6 $2/AUDIOT.BS6
xdelta3 -e -s $1/EANIM.BS6 $2/EANIM.BS6
xdelta3 -e -s $1/GANIM.BS6 $2/GANIM.BS6
xdelta3 -e -s $1/IANIM.BS6 $2/IANIM.BS6
xdelta3 -e -s $1/MAPHEAD.BS6 $2/MAPHEAD.BS6
xdelta3 -e -s $1/MAPTEMP.BS6 $2/MAPTEMP.BS6
xdelta3 -e -s $1/SANIM.BS6 $2/SANIM.BS6
xdelta3 -e -s $1/SVSWAP.BS6 $2/SVSWAP.BS6
xdelta3 -e -s $1/VGADICT.BS6 $2/VGADICT.BS6
xdelta3 -e -s $1/VGAGRAPH.BS6 $2/VGAGRAPH.BS6
xdelta3 -e -s $1/VGAHEAD.BS6 $2/VGAHEAD.BS6
xdelta3 -e -s $1/VSWAP.BS6 $2/VSWAP.BS6
