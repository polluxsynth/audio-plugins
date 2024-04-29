/*
	==============================================================================
	This file is part of the MiMi-d synthesizer.

	Copyright 2024 Ricard Wanderlof

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

#include <time.h>

// Simple random class, using only int32's and floats.
// Linear Congruent Generator is basically the same as used in Java and other
// places, but shortened to 32 bits. The resulting output is audibly
// acceptable from bit 18 and up, but the more significant bits mask the
// less significant ones, and especially when used in contexts where the
// noise level is low the lack of randomness in the lower bits should be
// completely inconsequential.

class SRandom
{

private:
	int32_t r;

public:
	// Constructor with specified seed to use
	SRandom(uint32_t seed): r(seed)
	{
	}

	// Constructor with no arguments: Use more-or-less random seed
	// by using system clock_gettime() function.
	SRandom(): r(1)
	{
		randomizeSeed();
	}

	inline uint32_t nextInt32() noexcept
	{
		// This results in audibly random noise from bit 18 and up.
		// Below that, the bits have noticably short cycles
		// (a couple of seconds or less), at bit 11 there is
		// a noticable pitch, and bits 7 and downwards are
		// deterministic square waves.
		// Normally we convert to a float, so we only use the top
		// 24 bits.
		r = r * 0xdeece66d + 11;

		return r;
	}

	inline float nextFloat() noexcept
	{
		static const float m =  1.0f / (std::numeric_limits<uint32_t>::max() + 1.0f);
		return nextInt32() * m;
	}

	// Randomize seed with current time of day and instance address.
	void randomizeSeed()
	{
		timespec t;

		// We don't really care about the return value, as there
		// is not much we can do if there is a failure. Likely in that
		// case, t will contain arbitrary uninitiated data from the
		// stack which will do fine as a seed.
		clock_gettime(CLOCK_REALTIME, &t);
		r ^= t.tv_sec ^ t.tv_nsec ^ (uint64_t)this;
	}

	// Global PRNG instance. Used for one-offs or to seed other PRNG's.
	static SRandom &globalRandom()
	{
		// A randomizeSeed (because of no parameters) seeded generator.
		static SRandom globalNg;

		return globalNg;
	}
};
