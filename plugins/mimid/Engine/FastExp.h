// Fast exponentiation with semitone based input and relative frequency
// output: exp2f12()
//
// This file is part of the MiMi-d synthesizer.
//
// From https://stackoverflow.com/questions/10552280/fast-exp-calculation-possible-to-improve-accuracy-without-losing-too-much-perfo/10792321:
// Originally based on a paper by Nic Schraudolph
// (http://nic.schraudolph.org/pubs/Schraudolph99.pdf)
// with additional multiplicative adjustment table by Ben Voigt.
// The simple multiplication of a single lookup proves to provide good
// accuracy without the time needed for linear interpolation or higher order
// curve fitting.
//
// The resulting accuracy is about 0.060%, or +/- 0.85 cents, which is
// entirely sufficient for its use in the MiMi-d:
// Upon comparing with expf() in MiMi-d, there is no disernable difference,
// neither with simple detuned oscillators, nor when using more critical
// oscillator FM.
//
// Licensing: from https://stackoverflow.com/help/licensing:
// CC BY-SA 3.0 (post created in 2012, and edited in 2014).
//
// The original Schraudolph algorithm was designed for double precision
// floats, but in the interest of avoiding type conversions and keeping
// everything as floats it has been reworked by Ricard Wanderlof 2024.
// The rework is licensed under CC BY-SA 3.0 for simplicity.

#pragma once

constexpr float ExpAdjustment[256] = {
	1.040389835f,
	1.039159306f,
	1.037945888f,
	1.036749401f,
	1.035569671f,
	1.034406528f,
	1.033259801f,
	1.032129324f,
	1.031014933f,
	1.029916467f,
	1.028833767f,
	1.027766676f,
	1.02671504f,
	1.025678708f,
	1.02465753f,
	1.023651359f,
	1.022660049f,
	1.021683458f,
	1.020721446f,
	1.019773873f,
	1.018840604f,
	1.017921503f,
	1.017016438f,
	1.016125279f,
	1.015247897f,
	1.014384165f,
	1.013533958f,
	1.012697153f,
	1.011873629f,
	1.011063266f,
	1.010265947f,
	1.009481555f,
	1.008709975f,
	1.007951096f,
	1.007204805f,
	1.006470993f,
	1.005749552f,
	1.005040376f,
	1.004343358f,
	1.003658397f,
	1.002985389f,
	1.002324233f,
	1.001674831f,
	1.001037085f,
	1.000410897f,
	0.999796173f,
	0.999192819f,
	0.998600742f,
	0.998019851f,
	0.997450055f,
	0.996891266f,
	0.996343396f,
	0.995806358f,
	0.995280068f,
	0.99476444f,
	0.994259393f,
	0.993764844f,
	0.993280711f,
	0.992806917f,
	0.992343381f,
	0.991890026f,
	0.991446776f,
	0.991013555f,
	0.990590289f,
	0.990176903f,
	0.989773325f,
	0.989379484f,
	0.988995309f,
	0.988620729f,
	0.988255677f,
	0.987900083f,
	0.987553882f,
	0.987217006f,
	0.98688939f,
	0.98657097f,
	0.986261682f,
	0.985961463f,
	0.985670251f,
	0.985387985f,
	0.985114604f,
	0.984850048f,
	0.984594259f,
	0.984347178f,
	0.984108748f,
	0.983878911f,
	0.983657613f,
	0.983444797f,
	0.983240409f,
	0.983044394f,
	0.982856701f,
	0.982677276f,
	0.982506066f,
	0.982343022f,
	0.982188091f,
	0.982041225f,
	0.981902373f,
	0.981771487f,
	0.981648519f,
	0.981533421f,
	0.981426146f,
	0.981326648f,
	0.98123488f,
	0.981150798f,
	0.981074356f,
	0.981005511f,
	0.980944219f,
	0.980890437f,
	0.980844122f,
	0.980805232f,
	0.980773726f,
	0.980749562f,
	0.9807327f,
	0.9807231f,
	0.980720722f,
	0.980725528f,
	0.980737478f,
	0.980756534f,
	0.98078266f,
	0.980815817f,
	0.980855968f,
	0.980903079f,
	0.980955475f,
	0.981017942f,
	0.981085714f,
	0.981160303f,
	0.981241675f,
	0.981329796f,
	0.981424634f,
	0.981526154f,
	0.981634325f,
	0.981749114f,
	0.981870489f,
	0.981998419f,
	0.982132873f,
	0.98227382f,
	0.982421229f,
	0.982575072f,
	0.982735318f,
	0.982901937f,
	0.983074902f,
	0.983254183f,
	0.983439752f,
	0.983631582f,
	0.983829644f,
	0.984033912f,
	0.984244358f,
	0.984460956f,
	0.984683681f,
	0.984912505f,
	0.985147403f,
	0.985388349f,
	0.98563532f,
	0.98588829f,
	0.986147234f,
	0.986412128f,
	0.986682949f,
	0.986959673f,
	0.987242277f,
	0.987530737f,
	0.987825031f,
	0.988125136f,
	0.98843103f,
	0.988742691f,
	0.989060098f,
	0.989383229f,
	0.989712063f,
	0.990046579f,
	0.990386756f,
	0.990732574f,
	0.991084012f,
	0.991441052f,
	0.991803672f,
	0.992171854f,
	0.992545578f,
	0.992924825f,
	0.993309578f,
	0.993699816f,
	0.994095522f,
	0.994496677f,
	0.994903265f,
	0.995315266f,
	0.995732665f,
	0.996155442f,
	0.996583582f,
	0.997017068f,
	0.997455883f,
	0.99790001f,
	0.998349434f,
	0.998804138f,
	0.999264107f,
	0.999729325f,
	1.000199776f,
	1.000675446f,
	1.001156319f,
	1.001642381f,
	1.002133617f,
	1.002630011f,
	1.003131551f,
	1.003638222f,
	1.00415001f,
	1.004666901f,
	1.005188881f,
	1.005715938f,
	1.006248058f,
	1.006785227f,
	1.007327434f,
	1.007874665f,
	1.008426907f,
	1.008984149f,
	1.009546377f,
	1.010113581f,
	1.010685747f,
	1.011262865f,
	1.011844922f,
	1.012431907f,
	1.013023808f,
	1.013620615f,
	1.014222317f,
	1.014828902f,
	1.01544036f,
	1.016056681f,
	1.016677853f,
	1.017303866f,
	1.017934711f,
	1.018570378f,
	1.019210855f,
	1.019856135f,
	1.020506206f,
	1.02116106f,
	1.021820687f,
	1.022485078f,
	1.023154224f,
	1.023828116f,
	1.024506745f,
	1.025190103f,
	1.02587818f,
	1.026570969f,
	1.027268461f,
	1.027970647f,
	1.02867752f,
	1.029389072f,
	1.030114973f,
	1.030826088f,
	1.03155163f,
	1.032281819f,
	1.03301665f,
	1.033756114f,
	1.034500204f,
	1.035248913f,
	1.036002235f,
	1.036760162f,
	1.037522688f,
	1.038289806f,
	1.039061509f,
	1.039837792f,
	1.040618648f
};

// Fast float exponentition.
// Based on the algorithm devised by Nic Schraudolph, converted by Ricard
// Wanderlof to use floats instead of doubles, together with an adjustment
// table increasing the accuracy.
//
// The two constants involved are derived thus (see above for the reference
// to Schraudolph's original paper):
// The algorithm is based on a very simple approximation of 2**x, x = 0..1:
// exp_approx(x) = 1 + x. This holds true for 2**0 = 1 and 2**1 = 2, and
// the curve between these points is a straight line. For larger values of
// x, the higher bits are shifted into the exponent, which thus automatically
// become exponentiated. In its raw form, thus, the algorithm approximates
// 2**x with piecewise linear segments.
// For an exp2 function, the multiplicative constant would be 1048576
// or 0x100000, which shifts the mantissa so that a number between 0 and 1
// ends up shifted into the top of mantissa (20 bits shift, since the mantissa
// of a double is 52 bits, and only the top 32 bits are used in the original
// algorithm). The corresponding shift value for a float is 0x800000 or
// 8388608, since the mantissa is 23 bits.
// Now, for any other base than 2, we multiply this factor with ln2 of the
// new base. In our case, the argument is a pitch in semitones, and with
// 12 notes to an octave (i.e. a power of two) we need to divide by 12,
// resulting in 699050.667 .
// The term added afterwards has two functions: a) it adds the exponent
// offset for the floating point representation, which is 0x3fe for double
// (1022), and the top 9 bits of 0x3f0 for float (62), and b) it adds the
// equivalent of the 1 in the original approximation, together with a
// 'fudge factor' with controls the scope of the resulting inaccuracy.
// In our case, the original constant was 1072632447, 0x3fef127f, which was
// designed to be used with double precision floats, the 12 top bits
// representing the 1 bit sign and 11 bit exponent, and the lower 20
// representing the mantissa of the 'fudge factor'; the associated
// ExpAdjustment table is calculated based on this particular fudge factor,
// so changing it requires recalculating the table.
// (The lower 32 bits of the double, representing the lower 32 bits of the
// 52 mantissa, were not used and set to 0, the upper 20 bits offering
// enough precision for the accuracy we are aiming for).
// The corresponding equivalent for float (8 bit exponent and 23 bit mantissa)
// is 0x3f7893f8 consisting of a 1 bit sign and 8 bit exponent, hence 0x3f,
// and the rest is the mantissa 0x0f127f shifted up 3 bits to compensate for
// the exponent being 3 bits smaller in a float compared to a double.
//
// The multiplicative adjustment table is 256 entries long and is indexed from
// the highest 8 bits of the mantissa, hence the shift by 15 to move the
// bits into the LSBs of the index.
//
// The algorithm breaks down for very large or small values of x, in the
// original version roughly outside the range -700..+700, when used with an
// 11 bit double floating point representation exponent; and considerably less
// in the float version we use here; theoretically, roughly an eighth, so
// about -87..+87, but in practice (empirically verified) the value x can
// between -126 and +128, and 12 times that if the argument is in semitones.
//
// In our case, we're happy with a range of +/- 60 semitones, corresponding
// to +/- 5 octaves, i.e. corresponding to x being in the range -5..+5 .
//
// Note that standard C++ does not permit type punning (using a union
// to convert the binary representation between int32 and float), but GCC
// (and certain other compilers) do, so we use that here to avoid resorting
// to more esoteric methos of converting between int32 and float.

// semitones -> frequency factor
static inline float fast_exp2f12(float x)
{
	union { float f; int32_t i; } u;
	int32_t tmp = (int32_t)((0x800000/12.0f) * x) + 0x3f7893f8;
	int index = (int)(tmp >> 15) & 0xff;

	u.i = tmp;
	return u.f * ExpAdjustment[index];
}


// shape parameter (-10..+10) -> gradient factor
// Multiplier is empirically compared against MiMi-a sawtooth shape
// control. It gives roughly a halving of the sawtooth slope (or "one
// octave" of shape change, if you will) per parameter value whole step
// in the range 0..10 .
// Skip adjustment table as we don't need the precision for this case.
// The fudge term is set to 0x3f800000 to get a shape value of 0 to result
// in a gradient of 1.0f, at the expense of a higher maximum deviation
// (double in fact) from a true exponential.
static inline float superfast_exp2f_shape(float x)
{
	union { float f; int32_t i; } u;
	int32_t tmp = (int32_t)((0x800000 * 8.0f) * x) + 0x3f800000;

	u.i = tmp;
	return u.f;
}
