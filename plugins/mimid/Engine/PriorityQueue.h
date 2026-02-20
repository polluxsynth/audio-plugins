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

#include <iostream>
#include <cstring>
using namespace std;

// Priority queue / stack structure
//
//                               +--------> pop off top of stack
//                               |     +--- push onto top of stack
//                               |     v
//  +---+---+---+---+---+-//-+-------+---+
//  | 0 | 1 | 2 | 3 | 4 |    | tos-1 |   |
//  +---+---+---+---+---+-//-+-------+---+
//            |   ^
//            |   +------------------ insert, moving elements above up
//            +---------------------> extract, moving elemnts above down
//
// - insert(0) is like push but to the bottom of the stack (start of queue)
// - extract(0) is like pop but from the bottom of the stack (start of queue)
// - when pushing to a full stack (tos == S), bottom object is lost, i.e.
//   the stack is the same size after the operation
// - when inserting to a full stack, the top object is lost, i.e. the stack
//   size is the same after the operation
//
// When objects are pushed onto the stack, they are considered the  newest
// ones, so extractremove is used to get the oldest object.


template <typename T, int S> class PriorityQueue {
protected:
	T array[S];
	int tos; // top of stack, always 1 more than the highest item on stack
	// Unsafe remove: Remove specified item from queue with no bounds check
	void _remove(int pos)
	{
		tos--;
		memmove(&array[pos], &array[pos+1], sizeof(T) * (tos-pos));
	}

public:
	// Constructor: initialize queue from initializer of size S
	PriorityQueue(const T initializer[])
	{
		for (int i = 0; i < S; i++)
			array[i] = initializer[i];
		tos = S;
	}
	// Constructor: initialize empty queue
	PriorityQueue()
	{
		tos = 0;
	}
	// Clear queue
	void clear()
	{
		tos = 0;
	}
	// Return number of items in queue
	int size()
	{
		return tos;
	}
	// Init queue from initializer of specified size
	void init(int count, const T initializer[])
	{
		for (int i = 0; i < count; i++)
			array[i] = initializer[i];
		tos = count;
	}
	// Non-safe push: push onto end of queue without bounds check
	// or check for overflow
	void _push(T item)
	{
		array[tos++] = item;
	}
	// Non-safe pop: pop off end of queue without bounds check
	T _pop()
	{
		return array[--tos];
	}
	// Safe push: push onto end of queue; loose first object if
	// queue is full
	void push(T item)
	{
		// If stack would overflow, loose bottom object
		if (tos >= S) _remove(0);
		array[tos++] = item;
	}
	// Safe pop: pop off end of queue with bounds check; returns (T) 0
	// if queue is emtpty
	T pop()
	{
		if (tos == 0) return 0;
		return array[--tos];
	}
	// Unsafe peek: read specified item in queue without bounds check
	T _peek(int pos)
	{
		return array[pos];
	}
	// Shorthand for unsafe peek: [] operator
	T& operator[](int pos)
	{
		return array[pos];
	}
	// Safe peek: Read specified item in queue; return 0 if out of bounds
	// (Note overlap of 0 with potential values of T type)
	T peek(int pos)
	{
		if (pos < 0 || pos >= tos) return 0;

		return array[pos];
	}
	// Unsafe insert at pos, pushing whatever is above up one step,
	// with no bounds check, or check for full stack
	void _insert(T item, int pos)
	{
		memmove(&array[pos+1], &array[pos], sizeof(T) * (tos-pos));
		tos++;
		array[pos] = item;
	}
	// Unsafe insert at 0
	void _insert(T item)
	{
		_insert(item, 0);
	}
	// Insert at pos, pushing whatever is above up one step, with
	// bounds check. If stack is already full, top object is silently lost.
	void insert(T item, int pos)
	{
		if (pos < 0 || pos >= tos) return;
		// If queue would overflow, loose top object
		if (tos >= S) tos--;
		_insert(item, pos);
	}
	// Safe remove: Remove specified item from queue with bounds check
	void remove(int pos)
	{
		if (pos < 0 || pos >= tos) return;
		_remove(pos);
	}
	// Safe remove(0) = Dequeue = Remove from front of queue
	void remove()
	{
		if (tos == 0) return;
		_remove(0);
	}
	// Unsafe extract: Remove specified item from queue without bounds
	// check, and return it
	T _extract(int pos)
	{
		T item = array[pos];
		_remove(pos);
		return item;
	}
	// Unsafe extract(0) = Dequeue = remove and return item from front
	// of queue
	T _extract()
	{
		return _extract(0);
	}
#if 0
	// Remove specified item from queue, with bounds check.
	// If out of bounds, return (T) 0
	T extract(int pos)
	{
		if (pos < 0 || pos >= tos) return 0;

		return _extract(pos);
	}
#endif
	T extract()
	{
		if (tos <= 0) return 0;  // queue empty

		return _extract(0);
	}
#if 0
	typedef int (*Finder)(T *, int);
	int find(Finder F)
	{
		return F(array, tos);
	}
	T extract(Finder F)
	{
		int pos = F(array, tos);
		if (pos < 0)
			return 0; // null if pointer
		T extraction = array[pos];
		remove(pos);
		return extraction;
	}
	typedef int (*FinderArg)(T *, int, int);
	int find(FinderArg F, int arg)
	{
		return F(array, tos, arg);
	}
	T extract(FinderArg F, int arg)
	{
		int pos = F(array, tos, arg);
		if (pos < 0)
			return 0; // null if pointer
		T extraction = array[pos];
		remove(pos);
		return extraction;
	}
	// Call examiner function F with arguments item and arg for each
	// item in the array until a match is found; if so, return the position
	// else return -1;
	typedef bool (*Examiner)(T, int);
	bool find(Examiner F, int arg, int &pos)
	{
		for (int i = 0; i < tos; i++)
			if (F(array[i], arg)) {
				pos = i;
				return true;
			}
		return false;
	}
	// find and extract item, returning it if found
	bool extract(Examiner F, int arg, T &extraction)
	{
		int pos;
		if (!find(F, arg, pos)) return false;

		extraction = array[pos];
		remove(pos);
		return true;
	}
	// find and extract item, returning it if found
	T extract(Examiner F, int arg)
	{
		int pos;
		if (!find(F, arg, pos)) return 0; // null if pointer

		T extraction = array[pos];
		remove(pos);
		return extraction;
	}
#endif
	void print()
	{
		cout << "Size is " << size() << ":" << endl;
		for (int i = 0; i < tos; i++)
			cout << " " << array[i];
		cout << endl;
	}
};

