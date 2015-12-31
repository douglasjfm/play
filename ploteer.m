function ploteer(exp,k,a,spkr)
[spos sneg] = getscores(exp,k,a,spkr);
teta = 0:0.005:1;
p = zeros(1,length(teta));
n = zeros(1,length(teta));
far = zeros(1,length(teta));
frr = zeros(1,length(teta));
frr2 = zeros(1,length(teta));
for i=1:length(teta)
	[acceptedPos rejectedNeg tp tn] = scores(teta(i),spos,sneg);
	p(i) = acceptedPos;
	n(i) = rejectedNeg;
	far(i) = (length(sneg) - rejectedNeg)/length(sneg);
	frr(i) = (length(spos) - acceptedPos)/length(spos);
	frr2(i) = 1 - frr(i);
endfor
amb = ['office';'hall';'intersection';'global'];
titulo = sprintf('EER: K = %d A = %s SPK = %d',k,amb(a+1,:),spkr);
plot(teta,far,teta,frr)
title(titulo)
xlabel('teta');
endfunction