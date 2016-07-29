#include <cmath>
#include <vector>
#include <complex>
#include <iostream>

#include <fftw3.h>
#include <sndfile.hh>

using namespace std;

template <class T>
void printVector (const vector<T> &v) {
    cout << "[ ";
    for (const auto &i : v) {
        cout << i << ' ';
    }
    cout << ']' << endl;
}

inline
double hann(double value) {
    double x = cos(value * M_PI / 2.0);
    return x*x;
}

vector<double> fft (vector<double> &audioData) {
    // http://www.fftw.org/doc/Complex-numbers.html
    // http://www.jimmo.org/example-of-one-dimensional-dft-of-real-data-with-fftw/
    // https://stackoverflow.com/questions/15949833/usage-of-complex-numbers-in-c
    int counter;
    double real, imag;

    vector<double> output(audioData.size());
    fftw_complex* fftResult = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * audioData.size());
    fftw_plan plan = fftw_plan_dft_r2c_1d((int) audioData.size(), audioData.data(), fftResult, FFTW_ESTIMATE);
    fftw_execute(plan);

    for (counter = 0; counter < audioData.size(); counter++) {
        real = fftResult[counter][0];
        imag = fftResult[counter][1];
        output[counter] = sqrt((real * real) + (imag * imag));
    }

    fftw_destroy_plan(plan);
    free(fftResult);

    return output;
}

vector<double> getAudioDataFromWavFile (const std::string &filePath) {
    double* audioData;
    size_t audioDataSize;
    vector<double> output;
    sf_count_t readFrames;
    SndfileHandle audioFile;

    // Open audio file in read-only
    audioFile = SndfileHandle(filePath);

    if (audioFile.error()) {
        throw string("Failed to open audio file. Reason: ") + audioFile.strError();
    }

    cout << audioFile.frames() << " frames, " << audioFile.samplerate() << " samplerate, " << audioFile.channels() << " channels, " << endl;

    audioDataSize = (size_t) audioFile.frames() * audioFile.channels();
    output = vector<double>(audioDataSize);
    readFrames = audioFile.read(output.data(), audioFile.frames());

    if (readFrames != audioFile.frames()) {
        throw "Number of read frames don't match all frames in audio file.";
    }

    return output;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Usage " << argv[0] << ": <WAV file>" << endl;
        return -1;
    }

    vector<double> audioData = getAudioDataFromWavFile(argv[1]);

    printVector<double>(audioData);

    //Apply windowing function to vector
    transform(audioData.begin(), audioData.end(), audioData.begin(), &hann);

    printVector<double>(audioData);

    vector<double> transformedData = fft(audioData);

    printVector<double>(transformedData);

    return 0;
}