MidpointOS : UGen {
	*ar { arg freq = 440.0, depth = 2.0, spread = 0.5, reduction = 0.5, lerp = 1.0, oversample=1;
		if(freq.rate!='audio'){freq = K2A.ar(freq)};
		if(depth.rate!='audio'){depth = K2A.ar(depth)};
		if(spread.rate!='audio'){spread = K2A.ar(spread)};
		if(reduction.rate!='audio'){reduction = K2A.ar(reduction)};
		if(lerp.rate!='audio'){lerp = K2A.ar(lerp)};
		^this.multiNew('audio', freq, depth, spread, reduction, lerp, oversample)
	}
	checkInputs {
		/* TODO */
		^this.checkValidInputs;
	}
}