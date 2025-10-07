#include "header.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <fstream>
using namespace std;

// Function to print the help message
void printHelpMessage() {
    cout << "Project 2: Image Processing, Fall 2024\n\n";
    cout << "Usage:\n\t./project2.out [output] [firstImage] [method] [...]\n";
}

// Function to validate that a file name ends with ".tga"
bool validateFileName(const string& filename) {
    return filename.size() >= 4 && filename.substr(filename.size() - 4) == ".tga";
}

// Function to check if a file exists
bool fileExists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

// Function to load a TGA image
vector<Pixel> loadImage(const string& filename, TGAHeader& header) {
    if (!validateFileName(filename)) {
        cerr << "Invalid argument, invalid file name.\n";
        exit(1);
    }
    if (!fileExists(filename)) {
        cerr << "Invalid argument, file does not exist.\n";
        exit(1);
    }
    return readTGA(filename, header);
}

// Function to save a TGA image
void saveImage(const string& filename, const TGAHeader& header, const vector<Pixel>& pixels) {
    writeTGA(filename, header, pixels);
}

int main(int argc, char* argv[]) {
    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        printHelpMessage();
        return 0;
    }

    // Validate output file name
    if (argc < 2 || !validateFileName(argv[1])) {
        cerr << "Invalid file name.\n";
        return 1;
    }
    string outputFile = argv[1];

    // Validate source file for tracking image
    if (argc < 3 || !validateFileName(argv[2])) {
        cerr << "Invalid file name.\n";
        return 1;
    }
    if (!fileExists(argv[2])) {
        cerr << "File does not exist.\n";
        return 1;
    }

    TGAHeader header;
    vector<Pixel> trackingImage = loadImage(argv[2], header);

    for (int i = 3; i < argc; ++i) {
        string method = argv[i];

        if (method == "multiply" || method == "subtract" || method == "screen" || method == "overlay") {
            // Check for missing second image argument
            if (i + 1 >= argc) {
                cerr << "Missing argument.\n";
                return 1;
            }
            // Check if second argument is a valid .tga file
            if (!validateFileName(argv[i + 1])) {
                cerr << "Invalid argument, invalid file name.\n";
                return 1;
            }
            if (!fileExists(argv[i + 1])) {
                cerr << "Invalid argument, file does not exist.\n";
                return 1;
            }
            vector<Pixel> secondImage = loadImage(argv[++i], header);
            if (method == "multiply") {
                transform(trackingImage.begin(), trackingImage.end(), secondImage.begin(), trackingImage.begin(), multiply);
            } else if (method == "subtract") {
                transform(trackingImage.begin(), trackingImage.end(), secondImage.begin(), trackingImage.begin(), subtract);
            } else if (method == "screen") {
                transform(trackingImage.begin(), trackingImage.end(), secondImage.begin(), trackingImage.begin(), screen);
            } else if (method == "overlay") {
                transform(trackingImage.begin(), trackingImage.end(), secondImage.begin(), trackingImage.begin(), overlay);
            }

        } else if (method == "addred" || method == "addgreen" || method == "addblue" ||
                   method == "scalered" || method == "scalegreen" || method == "scaleblue") {
            if (i + 1 >= argc) {
                cerr << "Missing argument.\n";
                return 1;
            }
            int value;
            try {
                value = stoi(argv[++i]);
            } catch (exception&) {
                cerr << "Invalid argument, expected number.\n";
                return 1;
            }
            if (method == "addred") addChannel(trackingImage, value, 'r');
            else if (method == "addgreen") addChannel(trackingImage, value, 'g');
            else if (method == "addblue") addChannel(trackingImage, value, 'b');
            else if (method == "scalered") scaleChannel(trackingImage, value, 'r');
            else if (method == "scalegreen") scaleChannel(trackingImage, value, 'g');
            else if (method == "scaleblue") scaleChannel(trackingImage, value, 'b');

        } else if (method == "onlyred" || method == "onlygreen" || method == "onlyblue") {
            vector<Pixel> outputChannel;
            char channel = method == "onlyred" ? 'r' : (method == "onlygreen" ? 'g' : 'b');
            extractChannel(trackingImage, outputChannel, channel);
            trackingImage = outputChannel;

        } else if (method == "combine") {
            if (i + 2 >= argc || !validateFileName(argv[i + 1]) || !validateFileName(argv[i + 2])) {
                cerr << "Missing or invalid arguments for combine.\n";
                return 1;
            }
            if (!fileExists(argv[i + 1]) || !fileExists(argv[i + 2])) {
                cerr << "Invalid argument, file does not exist.\n";
                return 1;
            }
            vector<Pixel> greenChannel = loadImage(argv[++i], header);
            vector<Pixel> blueChannel = loadImage(argv[++i], header);
            vector<Pixel> combined(header.width * header.height);
            combineChannels(trackingImage, greenChannel, blueChannel, combined);
            trackingImage = combined;

        } else if (method == "flip") {
            rotate180(trackingImage, header.width, header.height);

        } else {
            cerr << "Invalid method name.\n";
            return 1;
        }
    }

    saveImage(outputFile, header, trackingImage);
    cout << "Processing completed. Output saved to: " << outputFile << endl;

    return 0;
}