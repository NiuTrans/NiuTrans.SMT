@echo off

set task=en_zh_news_trans

set translist=1best.dev.out

set reflist=ref.dev.E2C.txt

set nref=4

::perl evaluation.pl -flist %translist% -task %task% -1best  -evaluate -rlist %reflist% -nref %nref% -ci
perl generate-xml.pl -flist %translist% -task %task% -1best  -evaluate -rlist %reflist% -nref %nref%

pause