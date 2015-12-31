function [spos sneg] = getscores(exp,k,A,s)
pathexp = sprintf('testes/scrs/%s/K%d/',exp,k);
spos = [];
sneg = [];
if (A < 3)
	env = ((1+(18*A)):(1+(18*A)+17));
else
	env = 1:54;
endif
flneg = sprintf('scores_neg_%d.bin',s);
flpos = sprintf('scores_pos_%d.bin',s);
fid2 = fopen([pathexp flneg],'r');
fid1 = fopen([pathexp flpos],'r');
sposs = fscanf(fid1,'%f');
snegs = fscanf(fid2,'%f');
spos = [spos sposs(env)'];
k = 1;
for j=1:40
	tmp = snegs(k:(k+53));
	sneg = [sneg tmp(env)'];
	k = k + 54;
endfor
fclose(fid1);
fclose(fid2);
printf('%d %d\n', length(spos),length(sneg));
smin = min([spos sneg]);
spos = bsxfun(@plus,spos,abs(smin));
sneg = bsxfun(@plus,sneg,abs(smin));
smax = max([spos sneg]);
spos = spos / abs(smax);
sneg = sneg / abs(smax);
endfunction