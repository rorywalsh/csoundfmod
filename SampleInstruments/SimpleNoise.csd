<Cabbage>
form caption("SimpleNoise") size(500, 300), colour(58, 110, 182), pluginID("def1")

rslider bounds(24, 24, 161, 151), channel("bw"), range(0, 20000, 10000)
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
a1 expon 1, 2, 0.001
a2 randi a1, chnget:k("bw")
outs a2, a2
endin

</CsInstruments>
<CsScore>
;starts instrument 1 and runs it for a week
i1 0 [60*60*24*7] 
</CsScore>
</CsoundSynthesizer>
