#!/bin/bash
BASE=$1
NP='echo' #cd ../../Scratchbox/numptyphysics-0.1 && ./x86_64/Game -bmp'
NPTPROOT='nptp://numptyphysics.garage.maemo.org/levels/'
GEOM='200x120'

header()
{
I=0
J=$1
N=$2
echo '<html><head>' \
    '<link rel="stylesheet" href="default.css" type="text/css"/></head><body>' \
    '<div class=\"header\">' \
    '<h1><a href="http://numptyphysics.garage.maemo.org">NP</a>-complete</h1>'
while [ $I -lt $N ] ; do    
  #echo ' '$I
  I=$[$I+1];
done;
echo '</div>'
}

header 1 6
for d in ${BASE}/*; do
  if [ `find $d -name \*.nph | wc -l` -gt 0 ]; then
    echo "<div class=\"collection\"><h2>"`basename $d`"</h2><p>"
    echo "<div class="spacer">&nbsp;</div>"
    for f in `find $d -name \*.nph`; do
	THUMB=thumbs/`echo $f | tr /. __`.jpg
	if [ ! -e ${THUMB} ]; then
	    #(${NP} $f) && \
	    convert $f.bmp -geometry ${GEOM} ${THUMB} && rm $f.bmp
	fi
	echo "<div class=\"level\"><a href=\"${NPTPROOT}$f\">" \
	    "<img src=\"${THUMB}\"/><br>" \
	    "<span class=\"name\">"`basename $f .nph`"</span></a>" \
	    "<span class=\"buttons\">" \
	    "<a href=\"${NPTPROOT}$f\"><img alt=\"play\" src=\"/images/play.gif\"/></a>" \
	    "<a href=\"$f\"><img alt=\"save\" src=\"/images/save.gif\"/></a>" \
	    "</span></div>"
    done
    echo "<div class="spacer">&nbsp;</div>"
    echo "</div>"
  fi
done
echo '</body></html>'
