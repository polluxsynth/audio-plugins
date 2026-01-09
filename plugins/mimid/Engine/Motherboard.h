/*
  ==============================================================================
  This file is part of the MiMi-d synthesizer,
  originally from Obxd synthesizer.

  Copyright © 2013-2014 Filatov Vadim
  Copyright 2023-2025 Ricard Wanderlof

  Contact original author via email :
  justdat_@_e1.ru

  This file may be licensed under the terms of of the
  GNU General Public License Version 2 (the ``GPL'').

  Software distributed under the License is distributed
  on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
  express or implied. See the GPL for the specific language
  governing rights and limitations.

  You should have received a copy of the GPL along with this
  program. If not, go to http://www.gnu.org/licenses/gpl.html
  or write to the Free Software Foundation, Inc.,  
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
  ==============================================================================
*/
#pragma once
#include <climits>
#include "VoiceAllocator.h"
#include "SynthEngine.h"
#include "Lfo.h"

class Motherboard
{
public:
	const static int MAX_VOICES = 32;
	const static int modRatio = 1;
private:
	int totalvc;
	Decimator17 leftDecim, rightDecim;

public:
	float volume;
	float panSpread[MAX_VOICES];
	float pannings[MAX_VOICES];
	Voice voices[MAX_VOICES];
	VoiceAllocator<MAX_VOICES> voiceAlloc;
	float sampleRate;
	bool oversample;
	int modCount;
	bool economyMode;
	Motherboard(): leftDecim(), rightDecim(), voiceAlloc(voices)
	{
		economyMode = true;
		oversample = false;
		modCount = 0;
		volume = 0;
		totalvc = MAX_VOICES;
		for (int i = 0; i < MAX_VOICES;++i) {
			voices[i].voiceNumber = i;
			panSpread[i] = SRandom::globalRandom().nextFloat()-0.5;
			pannings[i] = 0.5;
		}
	}
	~Motherboard()
	{
	}
	void setVoiceCount(int count)
	{
		// If the number of voices is increased, any free running
		// LFOs need to be synced with the rest. Since the first
		// voice is always available we simply use that.
		// Since we start att totalvc, the loop is never executed
		// for voice #0. Furthermore, it is not run at all if
		// the voice count is not increased over the current value.
		for (int i = totalvc; i < count; i++)
		{
			voices[i].lfo1.phaseSync(voices[0].lfo1);
			voices[i].lfo2.phaseSync(voices[0].lfo2);
			voices[i].lfo3.phaseSync(voices[0].lfo3);
		}
		voiceAlloc.reinit(count);
		totalvc = count;
	}
	void setSampleRate()
	{
		int oversampleRatio = oversample ? 2 : 1;
		for (int i = 0; i < MAX_VOICES; i++) {
			voices[i].setHQ(oversample);
			voices[i].setSampleRate(sampleRate, oversampleRatio, modRatio);
		}
	}
	void setSampleRate(float sr)
	{
		sampleRate = sr;
		setSampleRate();
	}
	void setOversample(bool over)
	{
		oversample = over;
		setSampleRate();
	}
	void sustainOn()
	{
		for (int i = 0; i < MAX_VOICES; i++)
			voices[i].sustOn();
	}
	void sustainOff()
	{
		for (int i = 0; i < MAX_VOICES; i++)
			voices[i].sustOff();
	}
	void setPanSpreadAmt(float val)
	{
		for (int i = 0; i < MAX_VOICES; i++)
			// pannings are 0..1, with 0.5 being center, whereas
			// panSpread is [-0.5..+0.5] and val is 0..1
			pannings[i] = panSpread[i] * val + 0.5;
	}
	inline float processSynthVoice(Voice& voice, bool processMod)
	{
		if (processMod) {
			voice.checkAdssrState();
			// Always update LFOs to keep them in phase even if
			// voice is not playing, as well as updating
			// aftertouch in case it continues to change
			// after voice has stopped playing, to avoid an
			// unexpected aftertouch value next time it triggers.
			voice.lfo1.update();
			voice.lfo2.update();
			voice.lfo3.update();
			voice.aftert = voice.afterTouchSmoother.smoothStep();
		}
		if (voice.shouldProcess || !economyMode) {
			if (processMod)
				voice.processModulation();
			return voice.processAudioSample();
		}
		return 0;
	}
	void processSample(float* sm1, float* sm2)
	{
		float vl = 0, vr = 0;
		float vlo = 0, vro = 0;

		// Run modulation at fraction of sample rate, up to 1:1
		if (++modCount >= modRatio) modCount = 0;
		bool processMod = (modCount == 0);

		for (int i = 0; i < totalvc; i++) {
			float x1 = processSynthVoice(voices[i], processMod);
			if (oversample) {
				float x2 = processSynthVoice(voices[i], false);
				vlo += x2 * (1 - pannings[i]);
				vro += x2 * pannings[i];
			}
			vl += x1 * (1 - pannings[i]);
			vr += x1 * pannings[i];
		}
		if (oversample) {
			vl = leftDecim.Calc(vl, vlo);
			vr = rightDecim.Calc(vr, vro);
		}
		*sm1 = vl * volume;
		*sm2 = vr * volume;
	}
};
