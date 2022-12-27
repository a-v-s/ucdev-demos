#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


// fbase = ( (fxo) / ( (B / 2) * D) ) * (SYNTH /2^20)

// B = 4 for high band, 8 for middle band 
// High band = 826 ... 1055 MHz
// Middle band = 412 ... 527 MHz
// B/2 , thus 2 or 4 

// D is internal reference dividor, 1 for disabled, 2 for enabled
// REFDIV in XO_RXO_CONFIG0 
// We need to calculate SYNTH

// So we get fxo = 26000000, D = 1




 // Default values 
// SYNTH3	0x42
// SYNTH2	0x16
// SYNTH1	0x27
// SYNTH0	0x62

// 0x6227162, middle band
// 26000000/4 = 6500000
// 0x6227162 = 102920546
// 102920546 / 2^20 = 98,1526813507
// 98,1526813507 * 6500000 =637992428,78 = 638 MHz ???
 


// Note: It appears the crystal in the module is 50 MHz instead



double S2LP_RF_CalculateBaseFrequency( uint32_t synth_value) {
	//double const pll_div = (S2LP_RF_GetSynthBand(handle) == S2LP_SYNTH_BAND_HIGH ? 4.0 : 8.0);
	//double const ref_div = (S2LP_IsRefDivEnabled(handle) ? 2.0 : 1.0);
	//double const fxo = S2LP_GetClockFrequency(handle);
	double const pll_div = 8.0;
	double const ref_div = 2.0;
	double const fxo = 50000000.0;

	double const synth_divided = synth_value / ((double) (1ull << 20ull));
	double const fxo_denominator = (pll_div / 2.0) * ref_div;
	return round((fxo / fxo_denominator) * synth_divided);
}


uint32_t S2LP_RF_CalculateSyncForBaseFrequency(uint32_t frequency) {
	double const fbase = frequency;
	double const pll_div = 8.0;
//	double const ref_div = 1.0;
//	double const fxo = 26000000.0;
	double const ref_div = 2.0;
	double const fxo = 50000000.0;
	double const pow20 = (1 << 20);

	double const div_const = ((pll_div / 2.0) * ref_div) / fxo;
	return (uint32_t) (fbase * div_const * pow20);
}


int calculate(int freq) {
	bool high_band;
//	if (freq >= 826000000 && freq <= 1055000000) high_band = true; else 
//	if (freq >= 412000000 && freq <=  527000000) high_band = false; else
//	return -1;

	high_band = freq > 469500000;

	uint64_t  val = (uint64_t)freq << 20;

	//val /= (26000000/ (high_band ? 2 : 4));
	// RadioControlli Modules use 50 MHz crystal instead
	val /= (50000000/ (high_band ? 4 : 8));
	return val;
	
}


int main(int argc, char* argv[]) {
	if (argc != 2) return -1;
	int f= atoi(argv[1]);

	uint32_t test1 = calculate(f);
	uint32_t test2 = S2LP_RF_CalculateSyncForBaseFrequency(f);

	printf("%d\n",test1);
	printf("%d\n",test2);
	printf("%f\n", S2LP_RF_CalculateBaseFrequency(0x6227162));
	return 0;

}
