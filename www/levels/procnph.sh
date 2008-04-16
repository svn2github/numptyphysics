#!/bin/bash
BASE=$1
NP='echo' #cd ../../Scratchbox/numptyphysics-0.1 && ./x86_64/Game -bmp'
NPTPROOT='nptp://numptyphysics.garage.maemo.org/levels/'
GEOM='200x120'

echo '<html><head><link rel="stylesheet" href="default.css" type="text/css"/></head><body>'
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
	echo "<div class=\"level\"><a href=\"${NPTPROOT}$f\"><img src=\"${THUMB}\"/><br>`basename $f .nph`</a></div>"
    done
    echo "<div class="spacer">&nbsp;</div>"
    echo "</div>"
  fi
done
echo '</body></html>'
