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
#include "Panning.h"
#include "PriorityQueue.h"
#include "Voice.h"

template <int S> class VoiceList : public PriorityQueue<Voice *, S>
{
public:
	VoiceList(Voice (&voices)[S])
	{
		for (int i = 0; i < S; i++)
			this->array[i] = &voices[i];
		this->tos = S;
	}
	VoiceList()
	{
	}
	void init(int nvoices, Voice (&voices)[S])
	{
		for (int i = 0; i < nvoices; i++)
			this->array[i] = &voices[i];
		this->tos = nvoices;
	}
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
			if (voice == this->array[pos] ||
			    voice == this->array[pos]->buddy)
				return pos;
		}
		return -1; // voice not found
	}
	// Technically we could just return a bool or int since when
	// the extract is successful we just return the input parameter.
	// Howver, all other extract functions return the actual item,
	// so let's be consistent and do the same. The overhead should be
	// minimal.
	Voice *extract(Voice *voice) {
		int pos = find(voice);
		if (pos < 0) return NULL;
		return this->_extract(pos);
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
	Voice (&voices)[S];
	Pannings<S> &pannings;
	int totalvc;
	float velsave[128]; // one per note number
	float atsave[128]; // poly aftertouch
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

	VoiceAllocator(Voice (&initVoices)[S], Pannings<S> &initPannings):
		offpri(initVoices), onpri(), restore_stack(),
		voices(initVoices), pannings(initPannings)
	{
		rsz = mem = rob_oldest = rob_next_to_lowest = false;
		restore = strgNoteOn = strgNoteOff = false;
		uniPlaying = false;
		alwaysPorta = false;
		usingPolyAfterTouch = false;
		totalvc = S;
	}
	~VoiceAllocator()
	{
	}
	// Reinitialize allocator when voice count changed runtime
	void reinit(int voiceCount)
	{
		// If we are increasing the voice count, push our newly
		// found (and non playing) voices onto offpri.
		for (int i = totalvc; i < voiceCount; i++) {
			offpri.push(&voices[i]);
		}
		// If we are decreasing the voice count, extract
		// superfluous voices from onpri if they are playing,
		// else extract them from offpri.
		for (int i = voiceCount; i < totalvc; i++) {
			Voice *voice = onpri.extract(&voices[i]);
			if (!voice)
				voice = offpri.extract(&voices[i]);
			if (voice) {
				voice->NoteOffImmediately();
				Voice *buddy = voice->buddy;
				if (buddy) {
					// Debuddify
					buddy->buddy = NULL;
					voice->buddy = NULL;
					// We turn the voice off immediately so
					// we don't get half of a buddy pair
					// sounding.
					buddy->NoteOffImmediately();
					// extract() pulls out the complete
					// buddy pair. If the voice is not
					// one to be cut off, put it (back)
					// in offpri.
					if (buddy->voiceNumber < voiceCount)
						offpri._push(buddy);
				}
				// Same here
				if (voice->voiceNumber < voiceCount)
					offpri._push(voice);
			}
		}
		totalvc = voiceCount;
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
	// Handle note on in mono mode:
	// Go through onpri, and gate on all notes, including potential
	// buddies, buddifying as we go through.
	// Then gate on all voices in offpri, again including potential
	// buddies, and debuddification.
	void uniNoteOn(int noteNo, float vel, bool multitrig, bool porta)
	{
		// Fixate onpri_size to the size of onpri before starting,
		// in order to push potential debuddified voices to onpri
		// without the pushed voices interfering with the scan.
		int onpri_size = onpri.size();
		for (int i = 0; i < onpri_size; i++) {
			// Use peek instead of extract to avoid unnecessarily
			// shifting the data around in onpri.
			Voice *voice = onpri.peek(i); // We know it exists
			noteOn(voice, noteNo, PAN_CENTER, vel, multitrig, porta);
			Voice *buddy = voice->buddy;
			if (buddy) {
				onpri.push(buddy);
				noteOn(buddy, noteNo, PAN_CENTER, vel, multitrig, porta);
				// Debuddify
				buddy->buddy = NULL;
				voice->buddy = NULL;
			}
		}
		Voice *voice = offpri.pop();
		while (voice) {
			onpri.push(voice);
			noteOn(voice, noteNo, PAN_CENTER, vel, multitrig, porta);
			Voice *buddy = voice->buddy;
			if (buddy) {
				onpri.push(buddy);
				noteOn(buddy, noteNo, PAN_CENTER, vel, multitrig, porta);
				// Debuddyify
				buddy->buddy = NULL;
				voice->buddy = NULL;
			}
			voice = offpri.pop();
		}
	}
	// Handle note off in mono mode:
	// Turn off all voices in onpri, including potential buddies and
	// consequent debuddification.
	void uniNoteOff()
	{
		// We manipulate onpri/offpri here to get a seamless
		// transition between unison on and off, and to avoid
		// having to manage the voice list outside of the
		// priority queues.
		// Since we're going to access all voices, we don't
		// care about the priority order, so we optimize
		// slightly by using peek() and pop() instead of
		// extract() (the latter causing memmove).
		Voice *voice = onpri.pop();
		while (voice) {
			offpri.push(voice);
			voice->NoteOff();
			Voice *buddy = voice->buddy;
			if (buddy) {
				offpri.push(buddy);
				buddy->NoteOff();
				// Debuddify
				buddy->buddy = NULL;
				voice->buddy = NULL;
			}
			voice = onpri.pop();
		}
	}
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
		uniNoteOn(noteNo, velocity, !strgNoteOn, uniPlaying || alwaysPorta);
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
				uniNoteOn(noteNo, velsave[noteNo], !strgNoteOff, true);
				uniNote = noteNo;
			} else {
				uniNoteOff();
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
		}
		// We might not have found a voice, if there were voices
		// in offpri, but we were only looking for single voices
		// and there were none.
		if (!voice) {
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
	void noteOn(Voice *voice, int noteNo, int position, float velocity, bool multitrig, bool porta = true)
	{
		setVoiceAfterTouch(voice, noteNo);
		pannings.setPosition(voice->voiceNumber, position);
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
		if (voice) {
			Voice *buddy = NULL;
			if (dual) {
				buddy = voice->buddy;
				if (!buddy) {
					// We don't have a buddy yet. Hopefully,
					// we'll be able to grab a single voice.
					// If there aren't any, just run with
					// it, and it remain unbuddified.
					buddy = grabVoice(noteNo, true);
					if (buddy) {
						// Buddification
						voice->buddy = buddy;
						buddy->buddy = voice;
					}
				}
			} else if (voice->buddy) { // single mode
				// Debuddify, and forcibly gate buddy off
				// (or else we might hear the buddy release)
				Voice *defunctBuddy = voice->buddy;
				defunctBuddy->buddy = NULL;
				voice->buddy = NULL;
				defunctBuddy->NoteOffImmediately();
				offpri._push(defunctBuddy);
			}
			// Now finally, gate on the voice(s)
			onpri._push(voice);
			noteOn(voice, noteNo, buddy ? PAN_LEFT : PAN_CENTER, velocity, !strgNoteOn);
			if (buddy) {
				noteOn(buddy, noteNo, PAN_RIGHT, velocity, !strgNoteOn);
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
				Voice *buddy = voice->buddy;
				int restoreNote = restore_stack._pop();
				onpri._push(voice);
				noteOn(voice, restoreNote, buddy ? PAN_LEFT : PAN_CENTER, velsave[restoreNote], !strgNoteOff);
				// If the voice has a buddy, we gate it on too,
				// but we don't buddify any single voices, as
				// that might lead to an unrelated voice getting
				// robbed when we release a note.
				if (buddy) {
					noteOn(buddy, restoreNote, PAN_RIGHT, velsave[restoreNote], !strgNoteOff);
				}
				uniNote = restoreNote;
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
