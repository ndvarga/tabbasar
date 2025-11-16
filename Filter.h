#pragma once

#include <cmath>

class Filter
{
	public:
		Filter() {}
		
		float process(float);
		void set_frequency(float);
		void setup(float, float, float);
		
		~Filter() {}
	protected:
	
		void calculate_coefficients(float, float, float);

		float lastX1 = 0;
		float lastX2 = 0;
		float lastY1 = 0;
		float lastY2 = 0;
		
		float A1 = 0, A2 = 0;
		float B0 = 1, B1 = 0, B2 = 0;
		float sample_rate;
		float frequency;
		float q;

};