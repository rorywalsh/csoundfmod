<Cabbage>
form caption("Electricity") size(270, 200), 
button bounds(10, 10, 250, 50), channel("trigger"), text("Off", "On")
rslider bounds(16, 72, 68, 67), channel("sparkTime"), range(0, 1, 0.01), text("Spark Time")
rslider bounds(88, 72, 68, 67), channel("idtTime1"), range(0.00001, 0.005, 0.0001, 1, .0001), text("IDT Time 1")
rslider bounds(162, 72, 68, 67), channel("idtTime2"), range(0.00001, 0.005, 0.0002, 1, .0001), text("IDT Time 1")
</Cabbage>
<CsoundSynthesizer>
<CsOptions>
-n -d -+rtmidi=NULL -M0 -m0d 
</CsOptions>
<CsInstruments>
sr = 44100
ksmps = 32
nchnls = 2
0dbfs = 1
seed 0
; Electricity.csd
; Written by Iain McCurdy, 2015
; Modified for FMOD by Rory Walsh, 2016


gasendL,gasendR	init	0					; reverb send variables
;===============================================
; utility UDO: i-rate version of scale opcode
;===============================================
opcode	scale_i,i,iii
 ival,imax,imin	xin
 ival	=	(ival * (imax-imin)) + imin
	xout	ival
endop


;===============================================
; This instrument is always on. When the channel
; "trigger" changes to 1 it will trigger 
; instrument 1, when it changes to 0 it will stop it.
instr TRIGGER_INSTRUMENT
	kTrigger chnget "trigger"
	kTrigger = int(kTrigger)
	if changed(kTrigger)==1 then
		if kTrigger==1 then
			event "i", 1, 0, 1000
		else 
			turnoff2 1, 0, 1
		endif
	endif	
endin

;===============================================
; creates the low hum, and triggers instrument 2
;===============================================
instr 1
	kTime line 0, p3, p3
	kDelta randi 1, 2
	if kTime<chnget:i("sparkTime") then  
	  	ktrig	metro	300*abs(kDelta)				; generate a trigger
	  	ky_ratio chnget "idtTime1"
	  	kx_ratio chnget "idtTime2"
	  	schedkwhen	ktrig,0,0,2,0,0.01,p4,p5	; spark sound
	endif  
	 
	; 50 Hz mains hum
	aamp	interp	(1-(ktrig*0.5)) * (0.005 + (0.03))	; amplitude derives from mouse down position and inversely to trigger impulses
	kpw	rspline	0.89-(ky_ratio*0.88),0.99-(ky_ratio*0.98),0.2,0.4	; slight pulse width shift
	kjit	rspline	-10,10,1,5					; frequency jitter
	asq	vco2	1, 50*cent(kjit), 4, kpw, 0, 0.125		; square wave
	asq	*=	aamp						; scale amplitude
	kpan	rspline	kx_ratio*0.5,0.5+(kx_ratio*0.5),0.5,5		; panning function
	aL1,aR1	pan2	asq,kpan				; create stereo output
	; a second hum signal
	kpw	rspline	0.89-(ky_ratio*0.88),0.99-(ky_ratio*0.98),0.2,0.4	; slight pulse width shift
	kjit	rspline	-10,10,1,5					; frequency jitter
	asq	vco2	1, 50*cent(kjit), 4, kpw, 0, 0.125		; square wave
	asq	*=	aamp                                        	; scale amplitude
	kpan	rspline	kx_ratio*0.5,0.5+(kx_ratio*0.5),0.5,5      	; panning function 
	aL2,aR2	pan2	asq,kpan                    		; create stereo output
	
	outs	aL1+aL2, aR1+aR2				; mix hum signals and send to output	
endin

;==================================================
; creates the sparks. THis instrument is triggered
; in bursts from instruent 1 using schedkwhen
;==================================================
instr	2
iamp	exprand	1						; random amplitude
asig	mpulse	iamp,0						; a click impulse
asig	buthp	asig,500					; highpass filter it
icfoct	random	9,14						; random freq (oct format)
asig	wguide1	asig,cpsoct(icfoct),sr/4,0.5			; send click through a waveguide filter
iDT1	scale_i	p4,0.00001,0.005				; delay time for a comb filter derived from mouse x position
a1	comb	asig,0.01,iDT1					; comb filter (1)
iDT2	scale_i	p5,0.00001,0.005				; delay time for a comb filter derived from mouse y position
a2	comb	asig,0.005,iDT2					; comb filter (2)
asig	sum	a1,a2						; mix the two comb filter outputs
aL,aR	pan2	asig,rnd(1)					; random pan click
	outs	aL,aR						; send stereo audio to outputs
gasendL	+=	aL/(2+rnd(3))				; send some audio to the reverb send channels
gasendR	+=	aR/(2+rnd(3))
endin

;==================================================
; add reverb to the sparks
;==================================================
instr	REVERB_INSTRUMENT	; reverb
	aL,aR	reverbsc	gasendL,gasendR,0.7,7000
	outs		aL,aR
	clear		gasendL,gasendR
endin


</CsInstruments>
<CsScore>
i"REVERB_INSTRUMENT" 0 [3600*24*7]
i"TRIGGER_INSTRUMENT" 0 [3600*24*7]
</CsScore>
</CsoundSynthesizer>
