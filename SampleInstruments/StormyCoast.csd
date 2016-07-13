<Cabbage>
form caption("Stormy Coast") size(400, 150), 
rslider bounds(280, 16, 100, 100), channel("gain"), range(0, 1, .5, 1, .01), text("Gain"), trackercolour("lime"), outlinecolour(0, 0, 0, 50), textcolour("black")
rslider bounds(16, 0, 60, 69), channel("band1"), range(.1, 4, .5), text("Freq 1 ")
rslider bounds(80, 0, 60, 69), channel("band2"), range(.1, 4, .2), text("Freq 2")
rslider bounds(144, 0, 60, 69), channel("band3"), range(.1, 4, .4), text("Freq 3")
rslider bounds(208, 0, 60, 69), channel("band4"), range(.1, 4, .3), text("Freq 4")
rslider bounds(16, 72, 60, 69), channel("amp1"), range(0, 1, 1), text("Amp 1")
rslider bounds(80, 72, 60, 69), channel("amp2"), range(0, 1, .6), text("Amp 2")
rslider bounds(144, 72, 60, 69), channel("amp3"), range(0, 1, .15), text("Amp 3")
rslider bounds(208, 72, 60, 69), channel("amp4"), range(0, 1, .1), text("Amp 4")
</Cabbage>
<CsoundSynthesizer>
<CsOptions>
-n -d -+rtmidi=NULL -M0 -m0d 
</CsOptions>
<CsInstruments>
; Initialize the global variables. 
sr = 44100
ksmps = 32
nchnls = 2
0dbfs = 1

seed 0

instr 1
kGain chnget "gain"

aNoise pinker

kPan1 jitter .5, .1, 1 
kPan1 = kPan1+.5
kPan2 jitter .5, .1, 1 
kPan2 = kPan2+.5
kPan3 jitter .5, .1, 1
kPan3 = kPan3+.5 
kPan4 jitter .5, .1, 1
kPan4 = kPan4+.5 
 
aLow butterlp aNoise, 100+randi:k(50, chnget:k("band1"), 1)
aMid1 butterbp aNoise, 200+randi:k(100, chnget:k("band2"), 1), 100
aMid2 butterbp aNoise, 800+randi:k(300, chnget:k("band3"), 1), 400
aHigh butterbp aNoise, 2000+randi:k(500, chnget:k("band4"), 1), 500 

aLowL, aLowR pan2 aLow*chnget:k("amp1"), random(0, 1) 
aMid1L, aMid1R pan2 aMid1*chnget:k("amp2"), random(0, 1) 
aMid2L, aMid2R pan2 aMid2*chnget:k("amp3"), random(0, 1) 
aHighL, aHighR pan2 aHigh*chnget:k("amp4"), random(0, 1) 

aLeft = aLowL+aMid1L+aMid2L+aHighL
aRight = aLowR+aMid1R+aMid2R+aHighR
outs aLeft*kGain, aRight*kGain

endin

</CsInstruments>
<CsScore>
;causes Csound to run for about 7000 years...
f0 z
;starts instrument 1 and runs it for a week
i1 0 [60*60*24*7] 
</CsScore>
</CsoundSynthesizer>
