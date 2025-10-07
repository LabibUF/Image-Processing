
#ifndef PROJECT2_HEADER_H
#define PROJECT2_HEADER_H

#include <vector>
#include <string>
using namespace std;

#pragma pack(push, 1)
struct TGAHeader {
    char idLength;
    char colorMapType;
    char dataTypeCode;
    short colorMapOrigin;
    short colorMapLength;
    char colorMapDepth;
    short xOrigin;
    short yOrigin;
    short width;
    short height;
    char bitsPerPixel;
    char imageDescriptor;
};
#pragma pack(pop)

struct Pixel {
    unsigned char b, g, r;
};

// Function declarations
vector<Pixel> readTGA(const string& filename, TGAHeader& header);
void writeTGA(const string& filename, const TGAHeader& header, const vector<Pixel>& pixels);
Pixel multiply(const Pixel& p1, const Pixel& p2);
Pixel subtract(const Pixel& p1, const Pixel& p2);
Pixel overlay(const Pixel& p1, const Pixel& p2);
Pixel screen(const Pixel& p1, const Pixel& p2);
void addChannel(vector<Pixel>& pixels, int value, char channel);
void scaleChannel(vector<Pixel>& pixels, int factor, char channel);
void extractChannel(const vector<Pixel>& pixels, vector<Pixel>& output, char channel);
void combineChannels(const vector<Pixel>& r, const vector<Pixel>& g, const vector<Pixel>& b, vector<Pixel>& result);
void rotate180(vector<Pixel>& pixels, int width, int height);

#endif //PROJECT2_HEADER_H
