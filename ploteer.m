function [far frr frr2 eer]=ploteer(exp,k,a,teta)
spos = [];
sneg = [];
for s=1:13
[sposi snegi] = getscores(exp,k,a,s);
spos = [spos sposi];
sneg = [sneg snegi];
endfor
p = zeros(1,length(teta));
n = zeros(1,length(teta));
far = zeros(1,length(teta));
frr = zeros(1,length(teta));
frr2 = zeros(1,length(teta));
eer = -1;
pkg load geometry;
for i=1:length(teta)
	[acceptedPos rejectedNeg tp tn] = scores(teta(i),spos,sneg);
	p(i) = acceptedPos;
	n(i) = rejectedNeg;
	far(i) = (length(sneg) - rejectedNeg)/length(sneg);
	frr(i) = (length(spos) - acceptedPos)/length(spos);
	frr2(i) = 1 - frr(i);
  if ((far(i) < frr(i)) && (eer < 0))
    l1 = createLine([teta(i-1) far(i-1)],[teta(i) far(i)]);
    l2 = createLine([teta(i-1) frr(i-1)],[teta(i) frr(i)]);
    pi = intersectLines(l1,l2);
    eer = pi(2);
  endif
endfor
#amb = ['office';'hall';'intersection';'global'];
#titulo = sprintf('EER: K = %d A = %s SPK = %d',k,amb(a+1,:),spkr);
#plot(teta,far,teta,frr)
#title(titulo)
#xlabel('teta');
endfunction
