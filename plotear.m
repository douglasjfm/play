function plotear(exp,k,a,spkr)
[spos sneg] = getscores(exp,k,a,spkr);
teta = 0:0.005:1;
p = zeros(1,length(teta));
n = zeros(1,length(teta));
far = zeros(1,length(teta));
frr = zeros(1,length(teta));
frr2 = zeros(1,length(teta));
for i=1:length(teta)
	[acceptedPos rejectedNeg tp tn] = scores(teta(i),spos,sneg);
	p(i) = acceptedPos/length(spos);
	n(i) = rejectedNeg/length(sneg);
	far(i) = (length(sneg) - rejectedNeg)/length(sneg);
	frr(i) = (length(spos) - acceptedPos)/length(spos);
	frr2(i) = 1 - frr(i);
endfor
amb = ['office';'hall';'intersection';'global'];
titulo = sprintf('Equal Hit Rate: K = %d A = %s spk = %d',k,amb(a+1,:),spkr);
plot(teta,p*100,teta,n*100)
title(titulo)
xlabel('teta')
ylabel('%');
endfunction