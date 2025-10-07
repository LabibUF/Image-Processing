//
// Created by labib on 10/24/2024.
//

#include "header.h"
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;

unsigned char roundFloatToChar(float value) {
    return static_cast<unsigned char>(min(255.0f, max(0.0f, value * 255.0f + 0.5f)));
}

vector<Pixel> readTGA(const string& filename, TGAHeader& header) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: Could not open " << filename << endl;
        exit(1);
    }

    file.read(&header.idLength, sizeof(header.idLength));
    file.read(&header.colorMapType, sizeof(header.colorMapType));
    file.read(&header.dataTypeCode, sizeof(header.dataTypeCode));
    file.read(reinterpret_cast<char*>(&header.colorMapOrigin), sizeof(header.colorMapOrigin));
    file.read(reinterpret_cast<char*>(&header.colorMapLength), sizeof(header.colorMapLength));
    file.read(&header.colorMapDepth, sizeof(header.colorMapDepth));
    file.read(reinterpret_cast<char*>(&header.xOrigin), sizeof(header.xOrigin));
    file.read(reinterpret_cast<char*>(&header.yOrigin), sizeof(header.yOrigin));
    file.read(reinterpret_cast<char*>(&header.width), sizeof(header.width));
    file.read(reinterpret_cast<char*>(&header.height), sizeof(header.height));
    file.read(&header.bitsPerPixel, sizeof(header.bitsPerPixel));
    file.read(&header.imageDescriptor, sizeof(header.imageDescriptor));

    size_t pixelCount = header.width * header.height;
    vector<Pixel> pixels(pixelCount);
    file.read(reinterpret_cast<char*>(pixels.data()), pixelCount * sizeof(Pixel));
    file.close();
    return pixels;
}

void writeTGA(const string& filename, const TGAHeader& header, const vector<Pixel>& pixels) {
    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error: Could not open " << filename << " for writing." << endl;
        return;
    }

    file.write(&header.idLength, sizeof(header.idLength));
    file.write(&header.colorMapType, sizeof(header.colorMapType));
    file.write(&header.dataTypeCode, sizeof(header.dataTypeCode));
    file.write(reinterpret_cast<const char*>(&header.colorMapOrigin), sizeof(header.colorMapOrigin));
    file.write(reinterpret_cast<const char*>(&header.colorMapLength), sizeof(header.colorMapLength));
    file.write(&header.colorMapDepth, sizeof(header.colorMapDepth));
    file.write(reinterpret_cast<const char*>(&header.xOrigin), sizeof(header.xOrigin));
    file.write(reinterpret_cast<const char*>(&header.yOrigin), sizeof(header.yOrigin));
    file.write(reinterpret_cast<const char*>(&header.width), sizeof(header.width));
    file.write(reinterpret_cast<const char*>(&header.height), sizeof(header.height));
    file.write(&header.bitsPerPixel, sizeof(header.bitsPerPixel));
    file.write(&header.imageDescriptor, sizeof(header.imageDescriptor));

    file.write(reinterpret_cast<const char*>(pixels.data()), pixels.size() * sizeof(Pixel));
    file.close();
}

unsigned char clamp(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return static_cast<unsigned char>(value);
}

Pixel multiply(const Pixel& p1, const Pixel& p2) {
    Pixel result;
    result.r = roundFloatToChar((p1.r / 255.0f) * (p2.r / 255.0f));
    result.g = roundFloatToChar((p1.g / 255.0f) * (p2.g / 255.0f));
    result.b = roundFloatToChar((p1.b / 255.0f) * (p2.b / 255.0f));
    return result;
}

Pixel subtract(const Pixel& p1, const Pixel& p2) {
    Pixel result;
    result.r = roundFloatToChar((p1.r / 255.0f) - (p2.r / 255.0f));
    result.g = roundFloatToChar((p1.g / 255.0f) - (p2.g / 255.0f));
    result.b = roundFloatToChar((p1.b / 255.0f) - (p2.b / 255.0f));
    return result;
}

Pixel overlay(const Pixel& p1, const Pixel& p2) {
    auto overlayChannel = [](float n1, float n2) -> unsigned char {
        if (n2 <= 0.5f) {
            return static_cast<unsigned char>(2 * n1 * n2 * 255 + 0.5f);
        } else {
            return static_cast<unsigned char>((1 - 2 * (1 - n1) * (1 - n2)) * 255 + 0.5f);
        }
    };

    float r1 = p1.r / 255.0f, g1 = p1.g / 255.0f, b1 = p1.b / 255.0f;
    float r2 = p2.r / 255.0f, g2 = p2.g / 255.0f, b2 = p2.b / 255.0f;

    return {
            overlayChannel(b1, b2),
            overlayChannel(g1, g2),
            overlayChannel(r1, r2)
    };
}

Pixel screen(const Pixel& p1, const Pixel& p2) {
    Pixel result;
    result.r = roundFloatToChar(1.0f - (1.0f - p1.r / 255.0f) * (1.0f - p2.r / 255.0f));
    result.g = roundFloatToChar(1.0f - (1.0f - p1.g / 255.0f) * (1.0f - p2.g / 255.0f));
    result.b = roundFloatToChar(1.0f - (1.0f - p1.b / 255.0f) * (1.0f - p2.b / 255.0f));
    return result;
}

void addChannel(vector<Pixel>& pixels, int value, char channel) {
    for (auto& pixel : pixels) {
        if (channel == 'r') pixel.r = clamp(pixel.r + value);
        else if (channel == 'g') pixel.g = clamp(pixel.g + value);
        else if (channel == 'b') pixel.b = clamp(pixel.b + value);
    }
}

void scaleChannel(vector<Pixel>& pixels, int factor, char channel) {
    for (auto& pixel : pixels) {
        if (channel == 'r') pixel.r = static_cast<unsigned char>(min(pixel.r * factor, 255));
        else if (channel == 'g') pixel.g = static_cast<unsigned char>(min(pixel.g * factor, 255));
        else if (channel == 'b') pixel.b = static_cast<unsigned char>(min(pixel.b * factor, 255));
    }
}

void extractChannel(const vector<Pixel>& pixels, vector<Pixel>& output, char channel) {
    output.clear();
    output.reserve(pixels.size());

    for (const auto& pixel : pixels) {
        unsigned char value = 0;

        // Choose the correct channel value to create a grayscale effect
        if (channel == 'r') {
            value = pixel.r;
        } else if (channel == 'g') {
            value = pixel.g;
        } else if (channel == 'b') {
            value = pixel.b;
        } else {
            cerr << "Invalid channel specified: " << channel << endl;
            return;
        }

        // For grayscale, set R, G, and B to the same intensity level
        output.push_back({ value, value, value });
    }

}


void combineChannels(const vector<Pixel>& r, const vector<Pixel>& g, const vector<Pixel>& b, vector<Pixel>& result) {
    for (size_t i = 0; i < r.size(); ++i) {
        result[i] = { b[i].b, g[i].g, r[i].r };
    }
}

void rotate180(vector<Pixel>& pixels, int width, int height) {
    reverse(pixels.begin(), pixels.end());
}
