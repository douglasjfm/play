function [posi nega gp gn] = scores(teta,spos,sneg)
posi = 0;
nega = 0;
gn = 0;
gp = 0;
countp = 1:length(spos);
countn = 1:length(sneg);
pos2 = sort(spos,'descend');
neg2 = sort(sneg);
tetap = ones(1,length(spos));
tetan = ones(1,length(sneg));
tetap = tetap * teta;
tetan = tetan * teta;
boolp = pos2 >= tetap;
booln = neg2 < tetan;
countp = countp .* boolp;
countn = countn .* booln;
[~,posi] = max(countp);
[~,nega] = max(countn);
posi = posi * boolp(1);
nega = nega * booln(1);
gp = posi + length(sneg) - nega;
gn = nega + length(spos) - posi;
%for i=1:length(spos)
%	if (spos(i) >= teta)
%		posi = 1 + posi;
%		gp = gp + 1;
%	else
%		gn = gn + 1;
%	endif
%endfor

%for i=1:length(sneg)
%	if (sneg(i) < teta)
%		nega = 1 + nega;
%		gn = gn + 1;
%	else
%		gp = gp + 1;
%	endif
%endfor
endfunction
