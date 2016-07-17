<Cabbage>
form caption("Gunshot") size(300, 260)
button bounds(24, 8, 69, 25), channel("oneshot"), text("Fire"), colour:0("white"), fontcolour:0("black"), colour:1("white"), fontcolour:1("black")
checkbox bounds(104, 8, 171, 25), channel("automatic"), text("Enable Automatic Fire")
rslider bounds(32, 40, 75, 65), channel("shellFreqUpper"), range(0, 20000, 10000), text("Sweep 1")
rslider bounds(112, 40, 75, 65), channel("shellFreqLower"), range(0, 5000, 100), text("Sweep 2")
rslider bounds(192, 40, 75, 65), channel("fireRate"), range(0, 20, 10), text("Fire Rate")
rslider bounds(112, 112, 75, 65), channel("reverbLevel"), range(0, 1, .3), text("LF Reverb")
rslider bounds(32, 112, 75, 65), channel("noiseLevel"), range(0, 1, .200), text("Noise Level")
rslider bounds(192, 112, 75, 65), channel("noiseFilter"), range(0, 1, 0), text("Noise Beta")

</Cabbage>
<CsoundSynthesizer>
<CsOptions>
-n -d -+rtmidi=NULL -M0
</CsOptions>
<CsInstruments>
; Initialize the global variables. 
sr = 44100
ksmps = 32
nchnls = 2
0dbfs = 1
seed 0

; GunShot.csd
; Written by Rory Walsh, 2016. 

; instrument 1 is always on. It waits for channel data to change
; and then acts accordingly. 
instr 1
	kTrigger chnget "oneshot"
	if changed(kTrigger)==1 && kTrigger==1 then
		event "i", "FIRE_GUN", 0, 10
	endif

	if chnget:k("automatic")==1 then
	 if metro(chnget:k("fireRate"))==1 then
	 	event "i", "FIRE_GUN", 0, 10
	 endif
	endif
endin

; very simple implementation of a gunshot... 
instr FIRE_GUN
	iDur1 = 0.020
	k1 line chnget:i("shellFreqUpper"), iDur1, 100
	aShellBurst oscil .25, k1
	kEnv1 linseg 1, iDur1, 1, 0, 0, 1, 0
	aShellBurst = aShellBurst*kEnv1

	iDur2 = 0.030
	k1 line chnget:i("shellFreqLower"), iDur2, 100
	aExitBurst oscil .25, k1
	kEnv2 linseg 1, iDur2, 1, 0, 0, 1, 0
	aExitBurst = aExitBurst*kEnv2

	iDur3 chnget "noiseLevel"
	aEnv expon .4, iDur3, 0.0001
	aEnv2 expon 1, 10*chnget:i("reverbLevel"), 0.0001
	aNoiseBurst noise 1, chnget:i("noiseFilter")	
	aLowpass butterlp aNoiseBurst, 300
	aLowNoise = aLowpass*aEnv2
	
	aNoiseBurst = aNoiseBurst*aEnv
	
	outs aNoiseBurst+aLowNoise, aNoiseBurst+aLowNoise
	aBurstMix = aShellBurst+aExitBurst	
	outs aBurstMix, aBurstMix

	a1 reson aNoiseBurst, 300+rnd31:i(200, 1, 1), 200
	a2 reson aNoiseBurst, 500+rnd31:i(200, 1, 1), 400
	a3 reson aNoiseBurst, 700+rnd31:i(200, 1, 1), 600
	a4 reson aNoiseBurst, 900+rnd31:i(200, 1, 1), 800

	a5 reson aNoiseBurst, 2200+rnd31:i(1000, 1, 1), 2200
	a6 reson aNoiseBurst, 2400+rnd31:i(1000, 1, 1), 2400
	a7 reson aNoiseBurst, 2600+rnd31:i(1000, 1, 1), 2600
	a8 reson aNoiseBurst, 2800+rnd31:i(1000, 1, 1), 2800
	
	aResonanceMix = (a1+a2+a3+a4+a5+a6+a7+a8)
	aBalance balance aResonanceMix, aNoiseBurst
	outs aBalance, aBalance
endin
</CsInstruments>
<CsScore>
;causes Csound to run for about 7000 years...
i1 0 36000
</CsScore>
</CsoundSynthesizer>
