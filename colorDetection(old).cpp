
#include <ap_fixed.h>
#include <ap_int.h>
#include <stdint.h>
#include <assert.h>
//#include <vector>
#include <map>
#include <cmath>

typedef ap_uint<8> pixel_type;
typedef ap_int<8> pixel_type_s;
typedef ap_uint<96> u96b;
typedef ap_uint<32> word_32;
typedef ap_ufixed<8,0, AP_RND, AP_SAT> comp_type;
typedef ap_fixed<10,2, AP_RND, AP_SAT> coeff_type;

// H V S
double blue [5] = {199, 0.23, 1, 0.40, 0.6}; // {200, 0.30, 1, 0.4, 1};
double orange [5] = {9,   0.4,  1, 0.8, 1};
double pink [5] = {339,   0.4,  1, 0.75, 1};
double green [5] = {116,   0.36,  0.7, 0.4, 0.6};
double yellow [5] = {72,   0.75,  1, 0.30, 0.80};//{64,   0.30,  1, 0.40, 1};


double bluecounter =  0;
double orangecounter = 0;
double pinkcounter = 0;
double greencounter = 0;
double yellowcounter = 0;




double blueposition[2][2] 	= {{306, 382},{0, 0}}; // desired Position , actual position
double orangeposition[2][2] = {{118, 315},{0, 0}};
double pinkposition[2][2]	= {{412, 443},{0, 0}};
double greenposition[2][2] 	= {{833, 277},{0, 0}};
double yellowposition[2][2] = {{215, 329},{0, 0}};
//
//std::map<std::string, std::vector<double> > colorhsv = {
//    { "blue",   {200, 0.30, 1, 0.4, 1}},
//    { "orange", {11,   0.70,  1, 0.75, 1}},
//    { "pink", {330,   0.60,  1, 0.60, 1}},
//    { "green", {140,   0.30,  1, 0.40, 1}},
//    { "yellow", {64,   0.30,  1, 0.40, 1}}
//};

struct pixel_data {
    pixel_type blue;
    pixel_type green;
    pixel_type red;
};

void template_filter(volatile uint32_t* in_data, volatile uint32_t* out_data, int w, int h, int parameter_1){
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE s_axilite port=parameter_1 //if parameter_l = 1, game = OVER
#pragma HLS INTERFACE s_axilite port=w
#pragma HLS INTERFACE s_axilite port=h
	double precisionorange = 6;
	w = 1080; //REMOVE WHEN EXPORTING
	h = 720;
	double maxh, minh, avh = 0, maxv, minv, avv = 0, maxs, mins, avs = 0, countvals= 0;
	int x = 0; //x coordinate variable
	int y = 0;//y coordinate variable

#pragma HLS INTERFACE m_axi depth=2073600 port=in_data offset=slave // This will NOT work for resolutions higher than 1080p
#pragma HLS INTERFACE m_axi depth=2073600 port=out_data offset=slave

	//initialization of array coords needed throughout image processing
	int colorsav [5][3] = {{0, 0, 0},// sum of x coord, sum y coord, count
						   {0, 0, 0},
						   {0, 0, 0},
						   {0, 0, 0}};


	unsigned char first = *in_data;
	unsigned int current;
	double precision;
	unsigned char in_r;
	unsigned char in_g;
	unsigned char in_b;
	unsigned char out_r;
	unsigned char out_b;
	unsigned char out_g;
	unsigned char in_t;
	int r;
	int g;
	int b;
	int in_total;// in_red + in_green + in_blue;
	int test_red;
    double h_val = 0,s_val = 0, v_val = 0;
	double max, min, delta;


	//iterates through all pixels of inData
	InData_Flattened_Loop: for (int i = 0; i < w*h; i++) {
		x = i%w;
		y = i/w;

		//	#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_FLATTEN off

			current = *in_data++;

			precision = 0.075;

			in_r = current & 0xFF;
			in_g = (current >> 8) & 0xFF;
			in_b = (current >> 16) & 0xFF;

			out_r = 0;
			out_b = 0;
			out_g = 0;

			in_t = in_r + in_g + in_b;
			r = in_r;
			g = in_g;
			b = in_b;
			in_total =  r + g + b;// in_red + in_green + in_blue;

			if (r > g && r > b){
				max = r;
			} else if (b > r && b > g){
				max = b;
			} else {
				max = g;
			}

			if (r < g && r < b){
				min = r;
			} else if (b < r && b < g){
				min = b;
			} else {
				min = g;
			}

			delta = max - min;

			if (delta > 0){
				if (max == r){
					h_val = 60 * (fmod(((g-b)/ delta), 6));
				} else if ( max == g){
					h_val = 60 * (((b-r)/delta) + 2);
				} else if ( max == b){
					h_val = 60 * (((r- g) / delta) + 4);
				}

				if (max > 0){
					s_val = delta / max;
				} else {
					s_val = 0;
				}

				v_val = max/255;
			} else {
				h_val = 0;
				s_val = 0;
				v_val = max/255;
			}

			if (h_val < 0){
				h_val = 360 + h_val;
			}
           //std::cout << "h: " << h_val << "\t" << "v: " << v_val << "\t" << "s: " << s_val  << "\t" << "R: " << r  << "\t" << "G: " << g <<"\t"  << "B: " << b << std::endl;
           if (countvals == 0){
        	   maxh = h_val;
        	   minh = h_val;
        	   avh += h_val;
        	   maxv = v_val;
        	   minv = v_val;
        	   avv += v_val;
        	   maxs = s_val;
        	   mins = s_val;
        	   avs  += s_val;
        	   countvals++;
           } else {
        	   if (h_val > maxh){
        		   maxh = h_val;
        	   }
        	   if (s_val > maxs){
        	        maxs = s_val;
           	   }
        	   if (v_val > maxv){
        	        maxv = v_val;
          	   }
        	   if (h_val < minh){
        		   minh = h_val;
        	   }
        	   if (s_val < mins){
        		   mins = s_val;
        	   }
        	   if (v_val < minv){
        		   minv = v_val;
        	   }
        	   avh += h_val;
        	   avv += v_val;
        	   avs += s_val;
        	   countvals++;
           }

        if (h_val < (1+precision)*blue[0] &&
            h_val > (1-precision)*blue[0] &&
            v_val > blue[1] &&
            v_val < blue[2] &&
            s_val > blue[3] &&
            s_val < blue[4]
            ){

            out_r = 0xFF; // showing the pixel as red.
            out_g = 0x00; // no green component
            out_b = 0x00; // no blue  component
            blueposition[1][0] += x; // summing all the x coordinates for all pixels detected
            blueposition[1][1] += y; // summing all the y
            bluecounter++; // counting all the pixel detected in this color

        } else if (h_val > -precisionorange+orange[0] &&
                   h_val < precisionorange+orange[0]&&
                   v_val > orange[1] &&
                   v_val < orange[2] &&
                   s_val > orange[3] &&
                   s_val < orange[4]
                   ){

            out_r = 0x00;
            out_g = 0xFF;
            out_b = 0x00;
            orangeposition[1][0] += x;
            orangeposition[1][1] += y;
            orangecounter++;
        } else if (h_val < (1+precision)*pink[0] &&
                h_val > (1-precision)*pink[0] &&
                v_val > pink[1] &&
                v_val < pink[2] &&
                s_val > pink[3] &&
                s_val < pink[4]
                   ){

            out_r = 0x00;
            out_g = 0x00;
            out_b = 0xFF;
            pinkposition[1][0] += x; // summing all the x coordinates for all pixels detected
            pinkposition[1][1] += y; // summing all the y
            pinkcounter++;
        } else if (h_val < (1+precision)*green[0] &&
                   h_val > (1-precision)*green[0] &&
                   v_val > green[1] &&
                   v_val < green[2] &&
                   s_val > green[3] &&
                   s_val < green[4]
                   ){

            out_r = 0x00;
            out_g = 0xFF;
            out_b = 0xFF;
            greenposition[1][0] += x;
            greenposition[1][1] += y;
            greencounter++;
         } else if (h_val < (1+precision)*yellow[0] &&
                   h_val > (1-precision)*yellow[0] &&
                   v_val > yellow[1] &&
                   v_val < yellow[2] &&
                   s_val > yellow[3] &&
                   s_val < yellow[4]
                   ){

            out_r = 0xFF;
            out_g = 0xFF;
            out_b = 0xFF;
            yellowposition[1][0] += x;
            yellowposition[1][1] += y;
            yellowcounter++;
        } else {
            out_r = in_r;
            out_g = in_g;
            out_b = in_b;
        }

			unsigned int output = out_r | (out_g << 8) | (out_b << 16);
			*out_data++ = output;


}

if (bluecounter > 0){
	blueposition[1][0] =  blueposition[1][0]/bluecounter;
	blueposition[1][1] =  blueposition[1][1]/bluecounter;
	bluecounter = sqrt(pow((blueposition[0][0] - blueposition[1][0]), 2) +
						      pow((blueposition[0][1] - blueposition[1][1]), 2));

}

if (orangecounter > 0){
	orangeposition[1][0] =  orangeposition[1][0]/orangecounter;
	orangeposition[1][1] =  orangeposition[1][1]/orangecounter;
	orangecounter = sqrt(pow((orangeposition[0][0] - orangeposition[1][0]), 2) +
						      pow((orangeposition[0][1] - orangeposition[1][1]), 2));

}
if (pinkcounter > 0){
	pinkposition[1][0] =  pinkposition[1][0]/pinkcounter;
	pinkposition[1][1] =  pinkposition[1][1]/pinkcounter;
	pinkcounter = sqrt(pow((pinkposition[0][0] - pinkposition[1][0]), 2) +
						      pow((pinkposition[0][1] - pinkposition[1][1]), 2));

}
if (greencounter > 0){
	greenposition[1][0] =  greenposition[1][0]/greencounter;
	greenposition[1][1] =  greenposition[1][1]/greencounter;
	greencounter = sqrt(pow((greenposition[0][0] - greenposition[1][0]), 2) +
						      pow((greenposition[0][1] - greenposition[1][1]), 2));

}

if (yellowcounter > 0){
	yellowposition[1][0] =  yellowposition[1][0]/yellowcounter;
	yellowposition[1][1] =  yellowposition[1][1]/yellowcounter;
	yellowcounter = sqrt(pow((yellowposition[0][0] - yellowposition[1][0]), 2) +
						      pow((yellowposition[0][1] - yellowposition[1][1]), 2));

}


std::cout << "blue coodinates \t x: "  << blueposition[1][0] << "\t y: " << blueposition[1][1] << std::endl;
std::cout << "orange coodinates \t x: "  << orangeposition[1][0] << "\t y: " << orangeposition[1][1] << std::endl;
std::cout << "pink coodinates \t x: "  << pinkposition[1][0] << "\t y: " << pinkposition[1][1] << std::endl;
std::cout << "yellow coodinates \t x: "  << yellowposition[1][0] << "\t y: " << yellowposition[1][1] << std::endl;
std::cout << "green coodinates \t x: "  << greenposition[1][0] << "\t y: " << greenposition[1][1] << std::endl;

std::cout << "blue distance: " << bluecounter << std::endl;
std::cout << "orange distance: " << orangecounter << std::endl;
std::cout << "pink distance: " << pinkcounter << std::endl;
std::cout << "yellow distance: " << yellowcounter << std::endl;
std::cout << "green distance: " << greencounter << std::endl;

int distance = bluecounter +
			   orangecounter +
			   pinkcounter +
			   greencounter +
			   yellowcounter;

distance = distance/5;

//std::cout << "average blue x is " << colorsav[0][0] << std::endl;
//std::cout << "average blue y is " << colorsav[0][1] << std::endl;
//std::cout << "average orange x is " << colorsav[2][0] << std::endl;
//std::cout << "average orange y is " << colorsav[2][1] << std::endl;
//
//int distance = 10000;//sqrt((colorsav[0][0]-colorsav[2][0])*(colorsav[0][0]-colorsav[2][0])+(colorsav[0][1]-colorsav[2][1])*(colorsav[0][1]-colorsav[2][1]));
//std::cout << "distance between blue and orange " << distance << std::endl;
std::cout << std::endl << std::endl << std::endl;
std::cout << "max h: " << maxh << "\t" << "min h: " << minh << "\t" << "av h:" << avh/countvals << std::endl;
std::cout << "max s: " << maxs << "\t" << "min s: " << mins << "\t" << "av s:" << avs/countvals << std::endl;
std::cout << "max v: " << maxv << "\t" << "min v: " << minv << "\t" << "av v:" << avv/countvals << std::endl;
std::cout << std::endl << std::endl << std::endl;

/////////////////////////////////////////////////////////////////////////
    //START OF HEATMAP
    /////////////////////////////////////////////////////////////////////////

//    double threshold = 0.1*w;
//    // Map distance to intensity
//    double intensity = 0;
//    if (distance > threshold) {
//    	intensity = 0;
//    } else if (distance <= 0) {
//    	intensity = 1;
//    } else {
//    	// Linear relationship between intensity and d
//    	intensity = 1-(distance/threshold);
//    }
    int count =0;
   // double rgb_transform = 1-intensity;

    // Display heatmap (whole screen)
    unsigned int output;
//    if (distance <= 50){
//    	parameter_1= 1;
//    } else {
    OutData_Pixel_Loop: //for (int i = 0; i < h*w; ++i) {
    	for (int y = h; y>0; y--){
    		for (int x = w ; x>0; x-- ) {


#pragma HLS PIPELINE II=1

#pragma HLS LOOP_FLATTEN off
//    	x = w - fmod(i,w)  ;
//    	y = h - i/w ;
    	//std::cout << x << std::endl;

    	current = *in_data--;
    	in_r = current & 0xFF;
    	in_g = (current >> 8) & 0xFF;
    	in_b = (current >> 16) & 0xFF;
    	out_r = in_r;
    	out_g = in_g;
    	out_b = in_b;
    	//out_g = in_g*rgb_transform;
    	//out_b = in_b*rgb_transform;

    	//std::cout << "x: \t" << x << "\t y: \t" << y << std::endl;


    	//std::cout <<  x*x + y*y << std::endl;



//    	if (i <= 365940 && i > 365040){
//    		std::cout << "pixel \t x: " << x << "\t y: " << y << std::endl;
//    		std::cout << "\t x: " << blueposition[1][0] << "\t y:" << blueposition[1][1] << std::endl;
//    		std::cout << "distance: " << distanceblue << std::endl;
//    	}
//    	count++;
    	double distanceblue = sqrt(pow((x - blueposition[1][0]), 2) +
    	    	    				pow((y - blueposition[1][1]), 2));



    	int distanceorange =  sqrt(pow((x - orangeposition[1][0]), 2) +
			      pow((y - orangeposition[1][1]), 2));
    	int distanceyellow = sqrt(pow((x - yellowposition[1][0]), 2) +
			      pow((y - yellowposition[1][1]), 2));
    	int distancegreen =  sqrt(pow((x - greenposition[1][0]), 2) +
    				      pow((y - greenposition[1][1]), 2));
    	int distancepink = sqrt(pow((x - pinkposition[1][0]), 2) +
			      pow((y - pinkposition[1][1]), 2));

    	int radius = 50;
    	int mindistance = 10;

    		double threshold = 0.1*w;
    	    // Map distance to intensity
    	    double intensity = 0;

    	    if (yellowcounter > threshold) {
    	    	intensity = 0;
    	    } else if (yellowcounter <= mindistance) {
    	    	intensity = 1;
    	    } else {
    	    	// Linear relationship between intensity and d
    	    	intensity = 1-(yellowcounter/threshold);
    	    }

    	    double yellowtransform = 1-intensity;

    	    if (bluecounter > threshold) {
    	    	intensity = 0;
    	    } else if (bluecounter <= mindistance) {
    	    	intensity = 1;
    	    } else {
    	    	// Linear relationship between intensity and d
    	    	intensity = 1-(bluecounter/threshold);
    	    }

    	    double bluetransform = 1-intensity;

    	    if (orangecounter > threshold) {
    	    	intensity = 0;
    	    } else if (orangecounter <= mindistance) {
    	    	intensity = 1;
    	    } else {
    	    	// Linear relationship between intensity and d
    	    	intensity = 1-(orangecounter/threshold);
    	    }

    	    double orangetransform = 1-intensity;

    	    if (pinkcounter > threshold) {
    	    	intensity = 0;
    	    } else if (pinkcounter <= mindistance) {
    	    	intensity = 1;
    	    } else {
    	    	// Linear relationship between intensity and d
    	    	intensity = 1-(pinkcounter/threshold);
    	    }

    	    double pinktransform = 1-intensity;

    	    if (greencounter > threshold) {
    	    	intensity = 0;
    	    } else if (greencounter <= mindistance) {
    	    	intensity = 1;
    	    } else {
    	    	// Linear relationship between intensity and d
    	    	intensity = 1-(greencounter/threshold);
    	    }

    	    double greentransform = 1-intensity;

    	   // std::cout << distanceyellow/radius << std::endl;

    	    double fade = 0;

    	if (distanceyellow < radius) {
    		fade = distanceyellow/double(radius);
    		out_g = in_g*yellowtransform*(fade);
    		out_b = in_b*yellowtransform*(fade);
    	}
    	if (distanceblue < radius ){
    		fade = distanceblue/double(radius);
    		out_g = in_g*bluetransform*(fade);
      		out_b = in_b*bluetransform*(fade);
    	}
    	if (distancegreen < radius) {
    		fade = distancegreen/double(radius);
       		out_g = in_g*greentransform*(fade);
    		out_b = in_b*greentransform*(fade);
    	}
    	if (distancepink < radius) {
    		fade = distancepink/double(radius);
    		out_g = in_g*pinktransform*(fade);
    		out_b = in_b*pinktransform*(fade);
    	}
    	if (distanceorange < radius) {
    		fade = distanceorange/double(radius);
    		out_g = in_g*orangetransform*(fade);
    		out_b = in_b*orangetransform*(fade);
    	}
    	output = out_r | (out_g << 8) | (out_b << 16);
    	*out_data-- = output;

    }
}
//    }
    ////////////////////////////////////////////////////////////////////////
    //END OF HEATMAP
    ////////////////////////////////////////////////////////////////////////
}

//int distancef(int refx, int refy, int px, int py){ // use custom precision
//	return sqrt((refx-px)*(refx-px)+(refy-py)*(refy-py));
//}


//int oned_to_twod(int w, int h, int index)
// int x = (index)%w
// int y = (index)/w
