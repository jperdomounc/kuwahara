#include <iostream>
#include <algorithm>
#include <vector>
#include <numeric>
#include <cmath>
#define NUM_COLUMNS 1200

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void kuwahara(int i, int j, int k, unsigned char* data, std::vector<unsigned char>& output);

int main()
{
    int x, y, n;

    
    unsigned char* data = stbi_load("cat.png", &x ,&y, &n, 0);

    if (data == nullptr) {
        std::cerr << "error loading image " << stbi_failure_reason() << std::endl;
        throw std::runtime_error("didn't even load lmao");
    }
    
    std::vector<unsigned char> output; 
    // passing pointer to vector data works for image write
    // image is 1200 x 1200 pixels
    
    for (int i = 0; i < y; i++) {
        for (int j = 0; j < x; j++) {
            for (int k = 0; k < 4; k++) { // k is offset into color channels r,g,b,a
                kuwahara(i, j, k, data, output);
            }
        }
    }
    
    if (stbi_write_png("new_cat.png", x, y, n, output.data(), x*n) == 0) {
        std::cerr << "error saving image";
    }
    
        
    stbi_image_free(data);
    
}

void kuwahara(int i, int j, int k, unsigned char* data, std::vector<unsigned char>& output) {
    // draw quadrants
    // calculate standard deviation for each quadrant
    // write to output buffer
    // formula for pixel at index i,j -> (i*x [1200] + j) * n [number of channels, 4] 
    std::vector<int> quadrant1;
    std::vector<int> quadrant2;
    std::vector<int> quadrant3;
    std::vector<int> quadrant4;
    if (k == 3) {
        output.push_back(data[((j*NUM_COLUMNS + i)*4) + k]); // ignore alpha channel
    } else {
        for (int a = 0; a < 3; a++) { // quadrant 1 ;a is x b is y
            for (int b = 0; b < 3; b++) {
                int pixel = ((std::clamp((j + a), 0, 1199) * NUM_COLUMNS + std::clamp((i + b), 0, 1199)) * 4) + k; // grabs pixel at i + a, j + b and offsets into color channel with k
                quadrant1.push_back(pixel); // pixel is actually just the index of the specific color channel
            }
        }

        for (int c = 0; c < 3; c++) { // quadrant 2
            for (int d = 0; d < 3; d++) {
                int pixel = ((std::clamp((j - c), 0, 1199) * NUM_COLUMNS + std::clamp((i + d), 0, 1199)) * 4) + k; // grabs pixel at i + a, j + b and offsets into color channel with k
                quadrant2.push_back(pixel);
            }
        }

        for (int e = 0; e < 3; e++) { // quadrant 3
            for (int f = 0; f < 3; f++) {
                int pixel = ((std::clamp((j - e), 0, 1199) * NUM_COLUMNS + std::clamp((i - f), 0, 1199)) * 4) + k; // grabs pixel at i + a, j + b and offsets into color channel with k
                quadrant3.push_back(pixel);
            }
        }

        for (int g = 0; g < 3; g++) { // quadrant 4
            for (int h = 0; h < 3; h++) {
                int pixel = ((std::clamp((j + g), 0, 1199) * NUM_COLUMNS + std::clamp((i - h), 0, 1199)) * 4) + k; // grabs pixel at i + a, j + b and offsets into color channel with k
                quadrant4.push_back(pixel);
            }
        }
    }

    std::vector<std::pair<int, double>> quadrant_avg_and_deviance;

    std::vector<unsigned char> q1_values;
    for (const auto& elem : quadrant1) {
        q1_values.push_back(data[elem]);
    }
    double mean1 = std::accumulate(q1_values.begin(), q1_values.end(), 0.0) / q1_values.size();
    double sq_sum1 = std::inner_product(q1_values.begin(), q1_values.end(), q1_values.begin(), 0.0);
    double stdev1 = std::sqrt(sq_sum1 / q1_values.size() - mean1 * mean1);
    quadrant_avg_and_deviance.push_back(std::make_pair(mean1, stdev1));

    std::vector<unsigned char> q2_values;
    for (const auto& elem : quadrant2) {
        q2_values.push_back(data[elem]);
    }
    double mean2 = std::accumulate(q2_values.begin(), q2_values.end(), 0.0) / q2_values.size();
    double sq_sum2 = std::inner_product(q2_values.begin(), q2_values.end(), q2_values.begin(), 0.0);
    double stdev2 = std::sqrt(sq_sum2 / q2_values.size() - mean2 * mean2);
    quadrant_avg_and_deviance.push_back(std::make_pair(mean2, stdev2));

    std::vector<unsigned char> q3_values;
    for (const auto& elem : quadrant3) {
        q3_values.push_back(data[elem]);
    }
    double mean3 = std::accumulate(q3_values.begin(), q3_values.end(), 0.0) / q3_values.size();
    double sq_sum3 = std::inner_product(q3_values.begin(), q3_values.end(), q3_values.begin(), 0.0);
    double stdev3 = std::sqrt(sq_sum3 / q3_values.size() - mean3 * mean3);
    quadrant_avg_and_deviance.push_back(std::make_pair(mean3, stdev3));

    std::vector<unsigned char> q4_values;
    for (const auto& elem : quadrant4) {
        q4_values.push_back(data[elem]);
    }
    double mean4 = std::accumulate(q4_values.begin(), q4_values.end(), 0.0) / q4_values.size();
    double sq_sum4 = std::inner_product(q4_values.begin(), q4_values.end(), q4_values.begin(), 0.0);
    double stdev4 = std::sqrt(sq_sum4 / q4_values.size() - mean4 * mean4);
    quadrant_avg_and_deviance.push_back(std::make_pair(mean4, stdev4));

    // i don't know... line 128
    auto lowest_stdev = std::min_element(quadrant_avg_and_deviance.begin(), quadrant_avg_and_deviance.end(), [](const auto& a, const auto& b){return a.second < b.second;});
    output.push_back(static_cast<unsigned char>(lowest_stdev->first));
    
    /* DEBUGGING
    for (const int& elem : quadrant1) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    for (const int& elem : quadrant2) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    for (const int& elem : quadrant3) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    for (const int& elem : quadrant4) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
    std::cout << "NUM_COLUMNS: " << NUM_COLUMNS << " i: " << i << " j: " << j << std::endl;
    */
} // kuwahara function end