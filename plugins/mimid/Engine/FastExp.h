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
// The original Schaudolph algorithm was designed for double precision floats,
// but in the interest of avoiding type conversions and keeping everything
// as floats it has been reworked by Ricard Wanderlof.

float ExpAdjustment[256] = {
	1.040389835,
	1.039159306,
	1.037945888,
	1.036749401,
	1.035569671,
	1.034406528,
	1.033259801,
	1.032129324,
	1.031014933,
	1.029916467,
	1.028833767,
	1.027766676,
	1.02671504,
	1.025678708,
	1.02465753,
	1.023651359,
	1.022660049,
	1.021683458,
	1.020721446,
	1.019773873,
	1.018840604,
	1.017921503,
	1.017016438,
	1.016125279,
	1.015247897,
	1.014384165,
	1.013533958,
	1.012697153,
	1.011873629,
	1.011063266,
	1.010265947,
	1.009481555,
	1.008709975,
	1.007951096,
	1.007204805,
	1.006470993,
	1.005749552,
	1.005040376,
	1.004343358,
	1.003658397,
	1.002985389,
	1.002324233,
	1.001674831,
	1.001037085,
	1.000410897,
	0.999796173,
	0.999192819,
	0.998600742,
	0.998019851,
	0.997450055,
	0.996891266,
	0.996343396,
	0.995806358,
	0.995280068,
	0.99476444,
	0.994259393,
	0.993764844,
	0.993280711,
	0.992806917,
	0.992343381,
	0.991890026,
	0.991446776,
	0.991013555,
	0.990590289,
	0.990176903,
	0.989773325,
	0.989379484,
	0.988995309,
	0.988620729,
	0.988255677,
	0.987900083,
	0.987553882,
	0.987217006,
	0.98688939,
	0.98657097,
	0.986261682,
	0.985961463,
	0.985670251,
	0.985387985,
	0.985114604,
	0.984850048,
	0.984594259,
	0.984347178,
	0.984108748,
	0.983878911,
	0.983657613,
	0.983444797,
	0.983240409,
	0.983044394,
	0.982856701,
	0.982677276,
	0.982506066,
	0.982343022,
	0.982188091,
	0.982041225,
	0.981902373,
	0.981771487,
	0.981648519,
	0.981533421,
	0.981426146,
	0.981326648,
	0.98123488,
	0.981150798,
	0.981074356,
	0.981005511,
	0.980944219,
	0.980890437,
	0.980844122,
	0.980805232,
	0.980773726,
	0.980749562,
	0.9807327,
	0.9807231,
	0.980720722,
	0.980725528,
	0.980737478,
	0.980756534,
	0.98078266,
	0.980815817,
	0.980855968,
	0.980903079,
	0.980955475,
	0.981017942,
	0.981085714,
	0.981160303,
	0.981241675,
	0.981329796,
	0.981424634,
	0.981526154,
	0.981634325,
	0.981749114,
	0.981870489,
	0.981998419,
	0.982132873,
	0.98227382,
	0.982421229,
	0.982575072,
	0.982735318,
	0.982901937,
	0.983074902,
	0.983254183,
	0.983439752,
	0.983631582,
	0.983829644,
	0.984033912,
	0.984244358,
	0.984460956,
	0.984683681,
	0.984912505,
	0.985147403,
	0.985388349,
	0.98563532,
	0.98588829,
	0.986147234,
	0.986412128,
	0.986682949,
	0.986959673,
	0.987242277,
	0.987530737,
	0.987825031,
	0.988125136,
	0.98843103,
	0.988742691,
	0.989060098,
	0.989383229,
	0.989712063,
	0.990046579,
	0.990386756,
	0.990732574,
	0.991084012,
	0.991441052,
	0.991803672,
	0.992171854,
	0.992545578,
	0.992924825,
	0.993309578,
	0.993699816,
	0.994095522,
	0.994496677,
	0.994903265,
	0.995315266,
	0.995732665,
	0.996155442,
	0.996583582,
	0.997017068,
	0.997455883,
	0.99790001,
	0.998349434,
	0.998804138,
	0.999264107,
	0.999729325,
	1.000199776,
	1.000675446,
	1.001156319,
	1.001642381,
	1.002133617,
	1.002630011,
	1.003131551,
	1.003638222,
	1.00415001,
	1.004666901,
	1.005188881,
	1.005715938,
	1.006248058,
	1.006785227,
	1.007327434,
	1.007874665,
	1.008426907,
	1.008984149,
	1.009546377,
	1.010113581,
	1.010685747,
	1.011262865,
	1.011844922,
	1.012431907,
	1.013023808,
	1.013620615,
	1.014222317,
	1.014828902,
	1.01544036,
	1.016056681,
	1.016677853,
	1.017303866,
	1.017934711,
	1.018570378,
	1.019210855,
	1.019856135,
	1.020506206,
	1.02116106,
	1.021820687,
	1.022485078,
	1.023154224,
	1.023828116,
	1.024506745,
	1.025190103,
	1.02587818,
	1.026570969,
	1.027268461,
	1.027970647,
	1.02867752,
	1.029389072,
	1.030114973,
	1.030826088,
	1.03155163,
	1.032281819,
	1.03301665,
	1.033756114,
	1.034500204,
	1.035248913,
	1.036002235,
	1.036760162,
	1.037522688,
	1.038289806,
	1.039061509,
	1.039837792,
	1.040618648
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
// in the float version we use here (roughly an eighth, so about -87..+87,
// but in our case, we're happy with a range of +/- 60 semitones, corresponding
// to +/- 5 octaves, i.e. corresponding to x being in the range -5..+5 .
//
// Note that standard C++ does not permit type punning (using a union
// to convert the binary representation between int32 and float), but GCC
// (and certain other compilers) do, so we use that here to avoid resporting
// to more esoteric methos of converting between int32 and float.

static inline float fast_exp2f12(float x)
{
	union { float f; int32_t i; } u;
	int32_t tmp = (int32_t)((0x800000/12.0f) * x) + 0x3f7893f8;
	int index = (int)(tmp >> 15) & 0xff;

	u.i = tmp;
	return u.f * ExpAdjustment[index];
}
