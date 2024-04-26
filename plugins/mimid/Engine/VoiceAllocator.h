/*
	==============================================================================
        This file is part of the MiMi-d synthesizer.

        Copyright 2023 Ricard Wanderlof

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

template <int S> class VoiceList : public PriorityQueue<Voice *, S>
{
public:
	int find_noteno(int noteNo)
	{
		// Scan from latest pushed note towards oldest,
		// so that if there are multiple notes with the same note
		// number we pick the latest one played.
		for (int i = this->tos - 1; i >= 0; i--)
			if (this->array[i]->midiIndx == noteNo)
				return i;
		return -1;
	}
	Voice *extract_noteno(int noteNo)
	{
		int pos = find_noteno(noteNo);
		if (pos < 0) return NULL;
		return this->_extract(pos);
	}
	int find_lowest_voice()
	{
		int lowestVoice = S;
		int lowest_pos = -1;
		for (int i = 0; i < this->tos; i++)
			if (this->array[i]->voiceNumber < lowestVoice) {
				lowestVoice = this->array[i]->voiceNumber;
				lowest_pos = i;
			}
		return lowest_pos;
	}
	Voice *extract_lowest_voice()
	{
		int pos = find_lowest_voice();
		if (pos < 0) return NULL;
		return this->_extract(pos);
	}
	int find_next_to_lowest_note()
	{
		int lowestNote = 128;
		int lowestPos = -1;
		int nextLowestNote = 128;
		int nextLowestPos = -1;
		for (int i = 0; i < this->tos; i++) {
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
	Voice *extract_next_to_lowest_note()
	{
		int pos = find_next_to_lowest_note();
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
	int totalvc;
	float velsave[128]; // one per note number

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

	VoiceAllocator()
	{
		rsz = mem = rob_oldest = rob_next_to_lowest = false;
		restore = strgNoteOn = strgNoteOff = false;
		uniPlaying = false;
		alwaysPorta = false;
		totalvc = 0;
	}
	~VoiceAllocator()
	{
	}
	void init(int voiceCount, Voice *voices[])
	{
		totalvc =  voiceCount;
		onpri.clear();
		offpri.init(totalvc, voices);
		restore_stack.clear();
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

		for (int i = 0; i < totalvc; i++) {
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
			voice->NoteOn(noteNo, velocity, !strgNoteOn, uniPlaying || alwaysPorta);
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
				for (int i = 0; i < totalvc; i++) {
					Voice *voice = onpri.peek(i);
					if (!voice) {
						voice = offpri.pop();
						onpri.push(voice);
					}
					voice->NoteOn(noteNo, velsave[noteNo], !strgNoteOff, true);
				}
				uniNote = noteNo;
			} else {
				for (int i = 0; i < totalvc; i++) {
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
	void setNoteOn(int noteNo,float velocity)
	{
		velsave[noteNo] = velocity; // needed for restore mode
		if (uni) {
			uniSetNoteOn(noteNo, velocity);
			return;
		}
		Voice *voice = NULL;
		if (offpri.size() > 0) { // exist voices in offpri
			if (mem)
				voice = offpri.extract_noteno(noteNo);
			if (!voice) {
				if (rsz)
					voice = offpri.extract_lowest_voice();
				else
					voice = offpri._extract(); // take first = oldest
			}
		} else {
			if (rob_oldest) {
				voice = onpri._extract(); // rob oldest
				// Since we've robbed a voice, push the note
				// it was playing onto the restore stack
				restore_stack.push(voice->midiIndx);
			}
			else if (rob_next_to_lowest) {
				voice = onpri.extract_next_to_lowest_note();
				// Since we've robbed a voice, push the note
				// it was playing onto the restore stack
				restore_stack.push(voice->midiIndx);
			}
		}
		if (voice) {
			onpri._push(voice);
			voice->NoteOn(noteNo, velocity, !strgNoteOn);
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
				voice->NoteOn(restoreNote, velsave[restoreNote], !strgNoteOff);
			} else {
				voice->NoteOff();
				offpri._push(voice);
			}
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
