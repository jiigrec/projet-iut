/*
 *  Code pris depuis http://stackoverflow.com/questions/29249657/playing-wav-file-with-portaudio-and-sndfile
 *
 *  Modifié par Jean-Yves Roda pour être utilisé de façon simple dans le code
 *
 */


#include <stdio.h>
#include <string.h>
#include "portaudio.h"
#include <string>
#include <stdint.h>
#include <unistd.h>


PaStream* stream;
FILE* wavfile;
int numChannels;
int sampleRate;
PaSampleFormat sampleFormat;
int bytesPerSample, bitsPerSample;

int paStreamCallback(
    const void *input, void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData )
{
    size_t numRead = fread(output, bytesPerSample * numChannels, frameCount, wavfile);
    output = (uint8_t*)output + numRead * numChannels * bytesPerSample;
    frameCount -= numRead;

    if(frameCount > 0) {
        memset(output, 0, frameCount * numChannels * bytesPerSample);
        return paComplete;
    }

    return paContinue;
}

bool portAudioOpen() {
    Pa_Initialize();
    PaStreamParameters outputParameters;

    outputParameters.device = Pa_GetDefaultOutputDevice();

    outputParameters.channelCount = numChannels;
    outputParameters.sampleFormat = sampleFormat;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultHighOutputLatency;

    PaError ret = Pa_OpenStream(
        &stream,
        NULL, // no input
        &outputParameters,
        sampleRate,
        paFramesPerBufferUnspecified, // framesPerBuffer
        0, // flags
        &paStreamCallback,
        NULL //void *userData
        );

    if(ret != paNoError) {
        if(stream)
            Pa_CloseStream(stream);
        return false;
    }
    Pa_StartStream(stream);
    return true;
}

std::string freadStr(FILE* f, size_t len) {
    std::string s(len, '\0');
    fread(&s[0], 1, len, f);
    return s;
}

template<typename T>
T freadNum(FILE* f) {
    T value;
    fread(&value, sizeof(value), 1, f);
    return value; // no endian-swap for now... WAV is LE anyway...
}

void readFmtChunk(uint32_t chunkLen) {
    uint16_t fmttag = freadNum<uint16_t>(wavfile); // 1: PCM (int). 3: IEEE float
    numChannels = freadNum<uint16_t>(wavfile);
    sampleRate = freadNum<uint32_t>(wavfile);
    uint32_t byteRate = freadNum<uint32_t>(wavfile);
    uint16_t blockAlign = freadNum<uint16_t>(wavfile);
    bitsPerSample = freadNum<uint16_t>(wavfile);
    bytesPerSample = bitsPerSample / 8;
    if(fmttag == 1 /*PCM*/) {
        switch(bitsPerSample) {
            case 8: sampleFormat = paInt8; break;
            case 16: sampleFormat = paInt16; break;
            case 32: sampleFormat = paInt32; break;
            default: return;
        }

    } else {

        sampleFormat = paFloat32;

    }
    if(chunkLen > 16) {
        uint16_t extendedSize = freadNum<uint16_t>(wavfile);
        fseek(wavfile, extendedSize, SEEK_CUR);
    }
}

void playSound(std::string sound) {
    if (Pa_IsStreamActive(stream) > 0)
        return;
    wavfile = fopen(sound.c_str(), "r");
    freadStr(wavfile, 4);
    uint32_t wavechunksize = freadNum<uint32_t>(wavfile);
    freadStr(wavfile, 4);

    while(true) {
        std::string chunkName = freadStr(wavfile, 4);
        uint32_t chunkLen = freadNum<uint32_t>(wavfile);
        if(chunkName == "fmt ")
            readFmtChunk(chunkLen);
        else if(chunkName == "data") {;
            break; // start playing now
        } else {
            // skip chunk
            fseek(wavfile, chunkLen, SEEK_CUR);
        }
    }



}


void stopSound() {
    if (Pa_IsStreamActive(stream) > 0) {
        fclose(wavfile);
        Pa_CloseStream(stream);
        Pa_Terminate();
    }
}






