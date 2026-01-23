/*
	==============================================================================
        This file is part of the MiMi-d synthesizer.

        Copyright 2023-2025 Ricard Wanderlof

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
#include "PriorityQueue.h"
#include "Voice.h"

typedef void (*syncfunc)(Voice *masterVoice, Voice *slaveVoice);

template <int S> class VoiceList : public PriorityQueue<Voice *, S>
{
public:
	int find_noteno(int noteNo, bool single = false)
	{
		// Scan from latest pushed note towards oldest,
		// so that if there are multiple notes with the same note
		// number we pick the latest one played.
		for (int i = this->tos - 1; i >= 0; i--) {
			if (single && this->array[i]->buddy) continue;
			if (this->array[i]->midiIndx == noteNo)
				return i;
		}
		return -1;
	}
	Voice *extract_noteno(int noteNo, bool single = false)
	{
		int pos = find_noteno(noteNo, single);
		if (pos < 0) return NULL;
		return this->_extract(pos);
	}
	int find(bool single = false)
	{
		for (int pos = 0; pos < this->tos; pos++) {
			if (!single || !this->array[pos]->buddy)
				return pos;
		}
		return -1; // no voice found
	}
	Voice *extract(bool single = false)
	{	int pos = find(single);
		if (pos < 0) return NULL;
		return this->_extract(pos);
	}
	int find(Voice *voice)
	{
		for (int pos = 0; pos < this->tos; pos++) {
			if (voice == this->array[pos])
				return pos;
		}
		return -1; // voice not found
	}
	int find_lowest_voice(bool single = false)
	{
		int lowestVoice = S;
		int lowest_pos = -1;
		for (int i = 0; i < this->tos; i++)
			if (this->array[i]->voiceNumber < lowestVoice) {
				if (single && this->array[i]->buddy) continue;
				lowestVoice = this->array[i]->voiceNumber;
				lowest_pos = i;
			}
		return lowest_pos;
	}
	Voice *extract_lowest_voice(bool single = false)
	{
		int pos = find_lowest_voice(single);
		if (pos < 0) return NULL;
		return this->_extract(pos);
	}
	int find_next_to_lowest_note(bool single = false)
	{
		int lowestNote = 128;
		int lowestPos = -1;
		int nextLowestNote = 128;
		int nextLowestPos = -1;
		for (int i = 0; i < this->tos; i++) {
			if (single && this->array[i]->buddy) continue;
			int this_note = this->array[i]->midiIndx;
			if (this_note < lowestNote) {
				nextLowestNote = lowestNote;
				nextLowestPos = lowestPos;
				lowestNote = this_note;
				lowestPos = i;
			} else if (this_note < nextLowestNote) {
				nextLowestNote = this_note;
				nextLowestPos = i;
			}
		}
		// If there is no next-to-lowest, go with the lowest, so we
		// at least potentially get something
		if (nextLowestPos < 0)
			return lowestPos;
		return nextLowestPos;
	}
	Voice *extract_next_to_lowest_note(bool single = false)
	{
		int pos = find_next_to_lowest_note(single);
		if (pos < 0) return NULL;
		return this->_extract(pos);
	}
};

template <int S> class NoteStack : public PriorityQueue<int, S>
{
public:
	int find_noteno(int noteNo)
	{
		for (int i = 0; i < this->tos; i++)
			if (this->array[i] == noteNo)
				return i;
		return -1;
	}
	void remove_noteno(int noteNo)
	{
		for (int i = 0; i < this->tos; i++)
			if (this->array[i] == noteNo) {
				this->_remove(i);
				return;
			}
	}
};

template <int S> class VoiceAllocator
{
private:
	VoiceList<S> offpri;
	VoiceList<S> onpri;
	NoteStack<10> restore_stack;
	int totalvc; // Initial voice count at init time
	int voicecount; // Current total voice count
	float velsave[128]; // one per note number
	float atsave[128]; // poly aftertouch
	float chat; // channel aftertouch
	bool usingPolyAfterTouch;

	// Unison mode
	int uniNote;
	bool uniPlaying;
public:
	bool uni; // Unison mode: play all voices from single note
	bool rsz;
	bool mem;
	bool rob_oldest;
	bool rob_next_to_lowest;
	bool strgNoteOn;
	bool strgNoteOff;
	bool restore;
	bool alwaysPorta;
	bool dual;

	VoiceAllocator()
	{
		rsz = mem = rob_oldest = rob_next_to_lowest = false;
		restore = strgNoteOn = strgNoteOff = false;
		uniPlaying = false;
		alwaysPorta = false;
		usingPolyAfterTouch = false;
		totalvc = voicecount = 0;
	}
	~VoiceAllocator()
	{
	}
	// Initialize allocator from list of pointers to voices
	void init(int voiceCount, Voice *voices[])
	{
		totalvc = voicecount = voiceCount;
		onpri.clear();
		offpri.init(totalvc, voices);
		restore_stack.clear();
	}
	// Reinitialize allocator when voice count changed runtime
	// Voice list must be the same as passed to init method
	void reinit(int voiceCount, Voice *voices[],
		    syncfunc sync, Voice *masterVoice)
	{
		int voiceDelta = voiceCount - voicecount;
		int voiceNo = totalvc - 1;
		// If we are increasing the voice count, push
		// non-usable (and non playing) voices onto offpri.
		// Non-usable voices will never be buddies so no need
		// to have any special handling for debuddification here.
		while (voiceDelta > 0) {
			while (voices[voiceNo]->usable)
				voiceNo--;
			offpri._insert(voices[voiceNo]);
			voices[voiceNo]->usable = true;
			sync(masterVoice, voices[voiceNo]);
			voiceNo--;
			voiceDelta--;
		}
		// If we are decreasing the voice count, extract
		// voices from offpri if possible, and as a last resort
		// from onpri.
		while (voiceDelta < 0) {
			Voice *voice;

			if (offpri.size() > 0)
				voice = offpri._extract();
			else
				voice = onpri._extract();
			voice->NoteOffImmediately();
			voice->usable = false;
			voiceDelta++;
			Voice *buddy = voice->buddy;
			if (buddy) {
				// Debuddify
				voice->buddy = NULL;
				// We turn the voice off immediately so we
				// don't get half of a buddy pair sounding,
				// but we only mark it unusable if voiceDelta
				// would go above 0.
				buddy->NoteOffImmediately();
				// Making the buddy unusable could cause the
				// voiceDelta to go above 0, but we don't
				// need the value for anything else than
				// terminating the loop, so ride with it.
				if (voiceDelta < 0) {
					// Still more voices to disable
					buddy->usable = false;
					voiceDelta++;
				} else {
					// Reached end of voiceDelta count,
					// so push it onto offpri.
					offpri._push(buddy);
				}
			}
		}
		voicecount = voiceCount;
	}
	void setAfterTouch(float ATvalue)
	{
		(void) ATvalue;
		usingPolyAfterTouch = false;
	}
	void setAfterTouch(int noteNo, float afterTouchValue)
	{
		atsave[noteNo] = afterTouchValue;
		usingPolyAfterTouch = true;
	}
	void setVoiceAfterTouch(Voice *voice, int noteNo)
	{
		if (usingPolyAfterTouch)
			// TODO: Bypass smoother here? This isn't a continuous
			// update, this is a newly allocated voice.
			voice->afterTouchSmoother.setSteep(atsave[noteNo]);
		// Else we don't need to set anything as channel aftertouch
		// is set directly in voices when message arrives.
		// It's only poly aftertouch that we need restore to the value
		// last noted for the note in question.
	}
private:
	void uniSetNoteOn(int noteNo, float velocity)
	{
		if (uniPlaying) {
			if (rob_oldest || rob_next_to_lowest) {
				// Push robbed note onto restore stack
				restore_stack.push(uniNote);
			} else {
				// Already playing and no rob set,
				// so push into restore stack and return.
				restore_stack.push(noteNo);
				return;
			}
		}

		int i = 0;
		while (i < voicecount) {
			// We manipulate onpri/offpri here to get a seamless
			// transition between unison on and off, and to avoid
			// having to manage the voice * list outside of the
			// priority queues.
			// Since we're going to access all voices, we don't
			// care about the priority order, so we optimize
			// slightly by using peek() and pop() instead of
			// extract() (the latter causing memmove).
			Voice *voice = onpri.peek(i);
			if (!voice) {
				voice = offpri.pop();
				onpri.push(voice);
			}
			noteOn(voice, noteNo, velocity, !strgNoteOn, uniPlaying || alwaysPorta);
			i++;
			if (voice->buddy) {
				// Debuddify and allocate
				Voice *buddy = voice->buddy;
				voice->buddy = NULL;
				onpri.push(buddy);
				noteOn(buddy, noteNo, velocity, !strgNoteOn, uniPlaying || alwaysPorta);
				i++;
			}
		}
		uniPlaying = true;
		uniNote = noteNo;
	}
	void uniSetNoteOff(int noteNo)
	{
		if (uniPlaying && noteNo == uniNote) {
			// If restore mode is enabled, and we have notes
			// stacked up, pop the latest one played, and
			// play it.
			if (restore && restore_stack.size() > 0) {
				noteNo = restore_stack._pop();
				for (int i = 0; i < voicecount; i++) {
					Voice *voice = onpri.peek(i);
					if (!voice) {
						voice = offpri.pop();
						onpri.push(voice);
					}
					noteOn(voice, noteNo, velsave[noteNo], !strgNoteOff, true);
				}
				uniNote = noteNo;
			} else {
				for (int i = 0; i < voicecount; i++) {
					Voice *voice = onpri.pop();
					if (!voice)
						break;
					offpri.push(voice);
					voice->NoteOff();
				}
				uniPlaying = false;
			}
		}
	}
	/* Grab a voice according to priority rules (offpri if available,
	 * honoring mem and rsz, else onpri if rob enabled or force is set).
	 * Note that in extreme cases, such as when attempting to grab
	 * a voice buddy when the voice count is 1, there might not be
	 * any voice available neither in offpri nor onpri, so we need to
	 * consider that.
	 * If single is set, only look for single (non-buddified) voices
	 */
	Voice *grabVoice(int noteNo, bool single = false)
	{
		Voice *voice = NULL;

		if (offpri.size() > 0) { // exist voices in offpri
			if (mem)
				voice = offpri.extract_noteno(noteNo, single);
			if (!voice) {
				if (rsz)
					voice = offpri.extract_lowest_voice(single);
				else
					voice = offpri.extract(single); // take first = oldest
			}
		} else {
			if (rob_oldest) {
				voice = onpri.extract(single); // rob oldest
				// If we've robbed a voice, push the note
				// it was playing onto the restore stack.
				if (voice)
					restore_stack.push(voice->midiIndx);
			}
			else if (rob_next_to_lowest) {
				voice = onpri.extract_next_to_lowest_note(single);
				// If we've robbed a voice, push the note
				// it was playing onto the restore stack.
				if (voice)
					restore_stack.push(voice->midiIndx);
			}
		}
		return voice;
	}
	void noteOn(Voice *voice, int noteNo, float velocity, bool multitrig, bool porta = true)
	{
		setVoiceAfterTouch(voice, noteNo);
		voice->NoteOn(noteNo, velocity, multitrig, porta);
	}
public:
	void setNoteOn(int noteNo, float velocity)
	{
		velsave[noteNo] = velocity; // needed for restore mode
		if (uni) {
			uniSetNoteOn(noteNo, velocity);
			return;
		}
		Voice *voice = grabVoice(noteNo);
		Voice *buddy = NULL;
		if (voice) {
			if (dual) {
				buddy = voice->buddy;
				if (!buddy) {
					// We don't have a buddy yet. Hopefully,
					// we'll be able to grab a single voice.
					buddy = grabVoice(noteNo, true);
					// Only look for single voices. If there
					// aren't any, just run with it, and
					// let us go unbuddified.
					voice->buddy = buddy; // May be NULL
				}
			} else if (voice->buddy) { // single mode
				// Debuddify, and forcibly gate buddy off
				// (or else we might hear the buddy release)
				Voice *defunctBuddy = voice->buddy;
				voice->buddy = NULL;
				defunctBuddy->NoteOffImmediately();
				offpri._push(defunctBuddy);
			}
			// Now finally, gate on the voice(s)
			onpri._push(voice);
			noteOn(voice, noteNo, velocity, !strgNoteOn);
			if (buddy) {
				noteOn(buddy, noteNo, velocity, !strgNoteOn);
			}
			// If we switch to unison, we want to know the
			// last note played.
			uniNote = noteNo;
			uniPlaying = true;
		} else {
			// No free voice, queue it
			restore_stack.push(noteNo);
		}
	}
	void setNoteOff(int noteNo)
	{
		// If this note was queued as held for possible future play,
		// remove it.
		restore_stack.remove_noteno(noteNo);
		if (uni) {
			uniSetNoteOff(noteNo);
			return;
		}
		// Now try and locate note if a voice is playing it
		Voice *voice = onpri.extract_noteno(noteNo);
		// If more than one voice is playing the note, extract
		// all of them, hence a while loop.
		while (voice) {
			// If restore mode is enabled, and we have notes
			// stacked up, pop the latest one played, and
			// trigger the voice with that note
			if (restore && restore_stack.size() > 0) {
				int restoreNote = restore_stack._pop();
				onpri._push(voice);
				noteOn(voice, restoreNote, velsave[restoreNote], !strgNoteOff);
				// If the voice has a buddy, we gate it on too,
				// but we don't buddify any single voices, as
				// that might lead to an unrelated voice getting
				// robbed when we release a note.
				Voice *buddy = voice->buddy;
				if (buddy) {
					noteOn(buddy, restoreNote, velsave[restoreNote], !strgNoteOff);
				}
			} else { // Not restore mode, just gate off the voice
				voice->NoteOff();
				offpri._push(voice);
				Voice *buddy = voice->buddy;
				if (buddy) {
					// Don't potentially debuddify here,
					// do it in note on when we actually
					// need the voices again
					buddy->NoteOff();
				}
			}
			// Next voice playing same note
			voice = onpri.extract_noteno(noteNo);
		}
		// If no voice playing, remember that for potential change
		// to unison mode
		if (onpri.size() == 0)
			uniPlaying = false;
	}
	void sustainOn()
	{
	}
	void sustainOff()
	{
	}
};
