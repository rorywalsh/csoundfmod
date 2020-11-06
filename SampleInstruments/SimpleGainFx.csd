<Cabbage>
form caption("Simple Gain") size(430, 300), colour(58, 110, 182), pluginID("def1")
rslider bounds(8, 56, 100, 100), channel("gain"), range(0, 1, .3, 1, .01), text("Gain")
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

; Written by Rory Walsh, 2015


instr 1
a1 inch 1
a2 inch 2

kGain chnget "gain"

outs a1*kGain, a2*kGain
endin


</CsInstruments>
<CsScore>
;starts instrument 1 and runs it for a week
i1 0 [60*60*24*7] 
</CsScore>
</CsoundSynthesizer>