<Cabbage>
form caption("Police Siren") size(400, 350), colour(58, 110, 182), pluginID("def1")
rslider bounds(296, 162, 100, 100), channel("gain"), range(0, 1, 0.5, 1, .01), text("Gain"), trackercolour("lime"), outlinecolour(0, 0, 0, 50), textcolour("black")

gentable bounds(8, 8, 260, 160), tablenumber(1), identchannel("table")
rslider bounds(8, 248, 77, 72), channel("overdrive"), text("overdrive"), range(1, 100, 0)
rslider bounds(88, 248, 74, 72), channel("freq"), text("speed"), range(0.1, 3, 0.2)
rslider bounds(8, 168, 65, 65), channel("attackShape"), range(-10, 1, -6), text("Att.")
rslider bounds(136, 168, 65, 65), channel("decayShape"), range(-10, 1, -3), text("Dec.")
rslider bounds(72, 168, 65, 65), channel("attackDur"), range(0, 4096, 2048), text("Att Dur")
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

;RW, 2016

instr 1
k1 oscil 800, chnget:k("freq"), 1
a1 vco2 .5*chnget:k("overdrive"), 300+k1
a1 clip a1, 1, 1
outs (a1)*chnget:k("gain"), (a1)*chnget:k("gain")

kAtt chnget "attackShape"
kDec chnget "decayShape"
kDur chnget "attackDur"

kTrig changed kAtt, kDec, kDur
if kTrig ==1 then
	event "i", 2, 0, 0, kDur, kAtt, kDec, p
endif

endin

instr 2
gi1 ftgen 1, 0, 4096, 16, 0, p4, p5, 1, 4096-p4, p6, 0 
chnset	"tablenumber(1)", "table"	; update table display	
endin

</CsInstruments>
<CsScore>
f1 0 4096 16 0 2048 -6 1 2048 -3 0
;causes Csound to run for about 7000 years...
;starts instrument 1 and runs it for a week
i1 0 [60*60*24*7] 
</CsScore>
</CsoundSynthesizer>
