#!/bin/bash

NUM=15
NPTPROOT='nptp://numptyphysics.garage.maemo.org/levels/'
HTTPROOT='http://numptyphysics.garage.maemo.org/levels/'
DATE=`date`

echo '<?xml version="1.0"?><rss version="2.0"><channel>' \
     '<title>NP-complete</title>' \
     '<description>User-contributed NumptyPhysics levels as they happen</description>' \
     '<link>'$NPTPROOT$'</link>'

# set $IFS to end-of-line
IFS=`echo -en "\n\b"`

for line in `svn log -v | egrep '(A .*\.nph$|r[0-9]+.*line)'`; do
  TESTDATE=`echo "$line" | egrep 'r[0-9]+.*line' | cut -d\| -f3 | cut -d\( -f1`
  if [ -n "$TESTDATE" ]; then      
    DATE=$TESTDATE
  else
    LEVEL=`echo "$line" | head -$NUM | cut -d/ -f4-`
    THUMB=thumbs/__`echo $LEVEL | tr /. __`.jpg
    echo '<item><title>'`dirname $LEVEL`': '`basename $LEVEL .nph`'</title>'\
         '<description>&lt;a href=&quot;'$NPTPROOT$level'&quot;&gt;' \
         '&lt;img src=&quot;'$HTTPROOT$THUMB'&quot;&gt;&lt;/a&gt;</description>' \
         '<link>'$NPTPROOT$LEVEL'</link>' \
         '<guid>'$NPTPROOT$LEVEL'</guid>' \
	 '<pubDate>'$DATE'</pubDate></item>'
  fi
done

echo '</channel></rss>'
