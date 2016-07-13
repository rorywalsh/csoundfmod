<Cabbage>
form caption("Bells") size(430, 300), colour(58, 110, 182), pluginID("def1")
rslider bounds(8, 56, 100, 100), channel("modIndex"), range(0, 2, .3, 1, .01), text("Mod Index"), trackercolour("lime"), outlinecolour(0, 0, 0, 50), textcolour("black")

checkbox bounds(8, 16, 60, 25), channel("but1"), text("Push", "Push")
rslider bounds(112, 56, 100, 100), channel("crossFade"), range(0, 5, .3, 1, .01), text("Cross Fade"), trackercolour("lime"), outlinecolour(0, 0, 0, 50), textcolour("black")

rslider bounds(216, 56, 100, 100), channel("speed"), range(.01, 25, 10, 1, .01), text("Alarm Speed"), trackercolour("lime"), outlinecolour(0, 0, 0, 50), textcolour("black")
rslider bounds(320, 56, 100, 100), channel("duration"), range(.01, 20, .3, 1, .01), text("Duration"), trackercolour("lime"), outlinecolour(0, 0, 0, 50), textcolour("black")
rslider bounds(8, 160, 100, 100), channel("frequency"), range(10, 2000, 500, 1, .01), text("Frequency"), trackercolour("lime"), outlinecolour(0, 0, 0, 50), textcolour("black")
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


instr 1
kPartials = 0
if chnget:k("but1")==1 then
	if metro(chnget:k("speed"))==1 then
		event "i", 2, 0, chnget:k("duration"), chnget:k("modIndex"), chnget:k("crossFade")
	endif
endif
endin


instr 2
iamp = p4
kc1 = p5
kc2 = p6
kvdepth = 0.005
kvrate = 6
a1 expon p4, p3, 0.001
asig fmbell iamp, chnget:i("frequency"), kc1, kc2, kvdepth, kvrate
     outs asig*a1, asig*a1
endin

instr 3

kamp = p4
kfreq = 880
kc1 = p5
kc2 = p6
kvdepth = 0.005
kvrate = 6

asig fmbell kamp, kfreq, kc1, kc2, kvdepth, kvrate, 1, 1, 1, 1, 1, p7
     outs asig, asig
endin
</CsInstruments>
<CsScore>
f 1 0 32768 10 1 
;starts instrument 1 and runs it for a week
i1 0 [60*60*24*7] 
</CsScore>
</CsoundSynthesizer>
