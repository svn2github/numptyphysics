#!/bin/bash

NUM=15
NPTPROOT='nptp://numptyphysics.garage.maemo.org/levels/'
HTTPROOT='http://numptyphysics.garage.maemo.org/levels/'

echo '<?xml version="1.0"?><rss version="2.0"><channel>' \
     '<title>NP-complete</title>' \
     '<description>User-contributed NumptyPhysics levels as they happen</description>' \
     '<link>'$NPTPROOT$'</link>'

for level in `svn log -v | grep A\ .*\.nph\$ | head -$NUM | cut -d/ -f4-`; do
  THUMB=thumbs/__`echo $level | tr /. __`.jpg
  echo '<item><title>'`basename $level .nph`'</title>'\
       '<description>&lt;a href=&quot;'$NPTPROOT$level'&quot;&gt;&lt;img' \
       'src=&quot;'$HTTPROOT$THUMB'&quot;&gt;&lt;/a&gt;</description>' \
       '<link>'$NPTPROOT$level'</link></item>'
done

echo '</channel></rss>'
