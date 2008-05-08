
for (<ARGV>) {
    #s/(.)(?=$1)/${1}_/g; # break non-shadda
    s/(.)[\211\212\213]/$1$1/og; # shadda

    s/\243\215/aN/og;
    s/(?<=[ \"])\243/a/og;
    s/y}/Y/og;

    s/\214//og; # (madda previous)
    s/\215/aN/og;
    s/\216/iN/og;
    s/\217/uN/og;

    s/\242/aa/og;		# ba madda
    s/\243/aa/og;		# alone
    s/\244/aa/og;		# final
    s/\245/b/og;
    s/\246/b/og;
    s/\247/b/og;
    s/\248/b/og;
    s/\249/p/og;
    s/\250/p/og;
    s/\251/p/og;
    s/\252/p/og;
    s/\253/t/og;
    s/\254/t/og;
    s/\255/t/og;
    s/\256/t/og;
    s/\257/th/og;
    s/\258/th/og;
    s/\259/th/og;
    s/\260/th/og;
    s/\261/j/og;
    s/\262/j/og;
    s/\263/ch/og;
    s/\264/ch/og;
    s/\265/.h/og;
    s/\266/.h/og;
    #s/\267//og;
    #s/\268//og;
    s/\269/kh/og;
    s/\270/kh/og;
    s/\271/d/og;		# final
    s/\272/d/og;
    s/\273/dh/og;
    s/\274/dh/og;
    s/\275/r/og;
    s/\276/r/og;
    s/\277/z/og;
    s/\278/z/og;
    s/\279/zh/og;
    s/\280/zh/og;
    #s/\281//og;
    #s/\282//og;
    #s/\283//og;
    #s/\284//og;
    #s/\285//og;
    #s/\286//og;
    #s/\287//og;
    #s/\288//og;
    #s/\289//og;
    #s/\290//og;
    #s/\291//og;
    #s/\292//og;
    #s/\293//og;
    #s/\294//og;
    #s/\295//og;
    #s/\296//og;
    #s/\297//og;
    #s/\298//og;
    #s/\299//og;
    s/\300/z/og;
    s/\301/s/og;
    s/\302/s/og;
    s/\303/s/og;
    s/\304/s/og;
    s/\305/s/og;
    s/\306/s/og;
    s/\307/sh/og;
    s/\308/sh/og;
    s/\309/sh/og;
    s/\310/sh/og;
    s/\311/sh/og;
    s/\312/sh/og;
    s/\313/.s/og;
    s/\314/.s/og;
    s/\315/.s/og;
    s/\316/.s/og;
    s/\317/.s/og;
    s/\318/.d/og;
    s/\319/.d/og;
    s/\320/.d/og;
    s/\321/.d/og;
    s/\322/.d/og;
    s/\323/.t/og;
    s/\324/.t/og;
    s/\325/.z/og;
    s/\326/.z/og;
    s/\327/\`/og;		# final
    s/\328/\`/og;
    s/\329/\`/og;
    s/\330/\`/og;
    s/\331/\`/og;
    s/\332/\`/og;		# initial
    s/\333/gh/og;
    s/\334/gh/og;
    s/\335/gh/og;
    s/\336/gh/og;
    s/\337/gh/og;
    s/\338/gh/og;
    s/\339/f/og;
    s/\340/f/og;
    s/\341/f/og;
    s/\342/f/og;
    s/\343/q/og;
    s/\344/q/og;
    s/\345/q/og;
    s/\346/k/og;
    s/\347/k/og;
    s/\348/k/og;
    s/\349/g/og;
    s/\350/g/og;
    s/\351/g/og;
    s/\352/l/og;
    s/\353/l/og;
    s/\354/l/og;
    s/\355/m/og;		# final
    s/\356/m/og;		# medial
    s/\357/m/og;		# initial
    s/\358/m/og;
    s/\359/n/og;
    s/\360/n/og;		# final
    s/\361/n/og;		# medial
    s/\362/n/og;		# initial
    s/\363/v/og;
    s/\364/v/og;
    s/(?<=[sctdkgz])\365/_h/og; # standalone
    s/\365/h/og;		# standalone
    s/(?<=[sctdkgz])\366/_h/og; # standalone
    s/\366/h/og;		# final
    s/(?<=[sctdkgz])\367/_h/og; # standalone
    s/\367/h/og;		# medial
    s/(?<=[sctdkgz])\368/_h/og; # standalone
    s/\368/h/og;
    s/(?<=[sctdkgz])\369/_h/og; # standalone
    s/\369/h/og;
    s/(?<=[sctdkgz])\370/_h/og; # standalone
    s/\370/h/og;
    s/\371/y/og;
    s/\372/y/og;
    s/\373/y/og;
    s/\374/y/og;		# final
    s/\375/y/og;
    s/\376/y/og;		# initial

    #s/\232/\'/og; # final hamze
    s/\233/\'/og; # final hamze
    s/aa\234/a\'/og; # hamze
    s/a\234/\'a/og; # hamze
    s/v\234/u\'/og; # hamze
    s/h\234/h-i/og; # hamze
    s/y\234/ii-i/og; # hamze
    #s/\235/\'/og; # hamze
    #s/\236/\'/og; # hamze
    s/\237/\'/og; # hamze
    #s/\238/\'/og; # hamze
    #s/\239/\'/og; # hamze
    s/\240/\'i/og; # hamze with carrier
    s/\241/\'i/og; # hamze with carrier

    s/\201/a/og;
    s/\202/a/og;
    s/\203/i/og;
    s/\204/i/og;
    s/\205/u/og;
    s/\206/u/og;
    s/\207//og;

    s/\|/a/og;

    s/‘/1/og;
    s/\225/5/og;

    s/\*/\n/og;
    s/ (uua?|v) / va /og;
    s/ uuu / uu /og;
    s/ aaa / aa /og;
    s/ iii / ii /og;

    s/ bh / bih /og;
    s/ jz / juz /og;
    s/ fy / fii /og;
    s/ k_h / kih /og;
    s/ khvd / khuwd /og;
    s/ my/ mii/og;

    s/_h /ih /og;
    s/(?<=[^auioy])h /ih /og;
    s/ dr / dar /og;
    s/ allh / all_ah /og;
    s/ alllh / al-ll_ah /og;
    s/\n +/\n/og;
    s/\n\n+/\n/og;
    s/\`\`/~\`/og;

    print;
}
