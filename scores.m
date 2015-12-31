function [posi nega gp gn] = scores(teta,spos,sneg)
posi = 0;
nega = 0;
gn = 0;
gp = 0;
for i=1:length(spos)
	if (spos(i) >= teta)
		posi = 1 + posi;
		gp = gp + 1;
	else
		gn = gn + 1;
	endif
endfor	
for i=1:length(sneg)
	if (sneg(i) < teta)
		nega = 1 + nega;
		gn = gn + 1;
	else
		gp = gp + 1;
	endif
endfor
endfunction