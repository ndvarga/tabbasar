
#include "Filter.h"

void Filter::setup(float sampleRate, float frequency, float q)
{
	sample_rate = sampleRate;
	this->frequency = frequency;
	this->q = q;
	calculate_coefficients(sampleRate, frequency, q);
}

void Filter::calculate_coefficients(float sampleRate, float frequency, float q)
{
    float k = tanf(M_PI * frequency / sampleRate);
    float norm = 1.0 / (1 + k / q + k * k);
    
    B0 = k * k * norm;
    B1 = 2.0 * B0;
    B2 = B0;
    A1 = 2 * (k * k - 1) * norm;
    A2 = (1 - k / q + k * k) * norm;	
}

float Filter::process(float in)
{
    float out = B0*in + B1*lastX1 + B2*lastX2 - A1*lastY1 - A2*lastY2;
    
    //update the sample that is now 2 samples ago
    lastX2 = lastX1;
    
    //update the previous sample
    lastX1 = in;
    lastY2 = lastY1;
    lastY1 = out;
    
    return out;

}

void Filter::set_frequency(float new_frequency)
{
	calculate_coefficients(sample_rate, new_frequency, q);
}
