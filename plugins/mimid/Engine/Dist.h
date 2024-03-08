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

class Dist
{
private:
	const float distTarget = 1.5;

	//JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Dist)
public:
	float distAmount, distLimit, distPeak, distGainComp;

	Dist()
	{
		distAmount=0.001;
		distLimit=10;
		distPeak=10;
		distGainComp=1;
	}
	~Dist()
	{
	}
private:
	// Cubic distortion, originally from atan(x * amount) / amount,
	// converted to x + amount * x^3 (truncated Taylor around x=0)
	inline float cubeDist(float sample)
	{
		return sample * (1 - sample * sample * distAmount);
	}
	// Square distortion, to get distortion with minimal bandwidth expansion
	// Essentially, x*x is x ring modulated with itself
	// We change the sign when x < 0 to get symmetry
	inline float squareDist(float sample)
	{
		return sample * (sample >= 0 ? (1 - distAmount * sample) :
					       (1 + distAmount * sample));
	}
public:
	void setSquareDistAmt(float val)
	{
		distAmount = val + 0.001;
		// distLimit: solve distortion derivative equation f'(x) = 0
		// to get limit for when the signal flatlines
		distLimit = 1/(2 * distAmount); // max input before flatline
		distPeak = squareDist(distLimit); // max output
		if (distLimit > distTarget)
			distGainComp = distTarget/squareDist(distTarget);
		else
			distGainComp = distTarget/squareDist(distLimit);
	}
	void setCubeDistAmt(float val)
	{
		distAmount = val + 0.001;
		// distLimit: solve distortion derivative equation f'(x) = 0
		// to get limit for when the signal flatlines
		distLimit = sqrt(1/(3 * distAmount)); // max before flatline
		distPeak = cubeDist(distLimit); // max output
		if (distLimit > distTarget)
			distGainComp = distTarget/cubeDist(distTarget);
		else
			distGainComp = distTarget/cubeDist(distLimit);
	}
	inline float applyCubeDist(float sample)
	{
		if (sample > distLimit) sample = distPeak;
		else if (sample < -distLimit) sample = -distPeak;
		else sample = cubeDist(sample);
		sample *= distGainComp; // compensate for amplitude drop
		return sample;
	}
	inline float applySquareDist(float sample)
	{
		if (sample > distLimit) sample = distPeak;
		else if (sample < -distLimit) sample = -distPeak;
		else sample = squareDist(sample);
		sample *= distGainComp; // compensate for amplitude drop
		return sample;
	}
};
