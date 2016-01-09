function [far frr frr2 eer] = grafos(exp,k,a)
teta = 0:0.001:1;

[far frr frr2 eer] = ploteer(exp,k,a,teta);

amb = ['office';'hall';'intersection';'global'];
figure(1)
plot(far,frr2,'.')
title(sprintf('ROC: K = %d A = %s EER = %.1f',k,amb(a+1,:),100*eer))
xlabel('FAR')
ylabel('1-FRR')
saveas(1,sprintf('res%s/roc_%d_%d.png',exp,k,a),'png')
close(1);
figure(2)
plot(teta,far,teta,frr,'.')
title(sprintf('EER: K = %d A = %s SPK = %d',k,amb(a+1,:),48))
xlabel('teta')
ylabel('FRR(verde) FAR(Azul)')
saveas(2,sprintf('res%s/eer_%d_%d.png',exp,k,a),'png')
close(2);
figure(3)
plot(far,frr,'.')
title(sprintf('ROC: K = %d A = %s EER = %.1f',k,amb(a+1,:),100*eer))
xlabel('FAR')
ylabel('FRR')
saveas(3,sprintf('res%s/roc2_%d_%d.png',exp,k,a),'png')
close(3);

endfunction
