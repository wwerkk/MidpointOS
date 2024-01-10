MidpointOS : UGen {
	*ar { arg freq = 440.0, phase = 0.0, depth = 2.0, spread = 0.5, reduction = 0.5, lerp = 1.0, oversample=1;
		^this.multiNew('audio', freq, phase, depth, spread, reduction, lerp, oversample)
	}
	checkInputs {
		/* TODO */
		^this.checkValidInputs;
	}
}