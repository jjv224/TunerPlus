// #include <Audio.h>
// #include <Wire.h>
// #include <SPI.h>
// #include <SerialFlash.h>
// #include <math.h>

// String frequencyToNote(float frequency, float &targetFreq);

// // Audio system setup
// AudioInputI2S           i2s1;
// AudioAnalyzeFFT1024     fft1024;
// AudioConnection         patchCord1(i2s1, 0, fft1024, 0);
// AudioControlSGTL5000    sgtl5000;

// void setup() {
//   Serial.begin(115200);
//   AudioMemory(12);

//   sgtl5000.enable();
//   sgtl5000.inputSelect(AUDIO_INPUT_MIC);
//   sgtl5000.volume(0.7);
//   sgtl5000.micGain(30);

//   fft1024.windowFunction(AudioWindowHanning1024);
// }

// void loop() {
//   delay(2000); //Respond every 2 seconds in the terminal

//   if (fft1024.available()) {
//     // Print first 10 FFT bins
//     // Serial.print("FFT bins: ");
//     // for (int i = 0; i < 10; i++) {
//     //   Serial.print(fft1024.read(i), 6);
//     //   Serial.print(" ");
//     // }
//     // Serial.println();

//     // Find peak bin
//     int maxIndex = 0;
//     float maxVal = 0;
//     for (int i = 1; i < 511; i++) {
//       float val = fft1024.read(i);
//       if (val > maxVal) {
//         maxVal = val;
//         maxIndex = i;
//       }
//     }

//     // Parabolic interpolation
//     float y1 = fft1024.read(maxIndex - 1);
//     float y2 = fft1024.read(maxIndex);
//     float y3 = fft1024.read(maxIndex + 1);
//     float correction = 0.5f * (y1 - y3) / (y1 - 2.0f * y2 + y3);
//     float trueIndex = maxIndex + correction;
//     float exactFreq = trueIndex * (44100.0f / 1024.0f);

//     float noteFreq;
//     String note = frequencyToNote(exactFreq, noteFreq);

//     Serial.printf("Detected Frequency: %.2f Hz — Note: %s (Ideal: %.2f Hz)\n",
//                   exactFreq, note.c_str(), noteFreq);

//     float diff = exactFreq - noteFreq;

//     if (fabs(diff) <= 10.0f) {
//       Serial.println("✅ In tune!");
//     } else if (diff < 0) {
//       Serial.println("🔼 Too low — tune up");
//     } else {
//       Serial.println("🔽 Too high — tune down");
//     }

//     Serial.println();
//   }
// }

// // Convert frequency to closest note, return ideal note frequency
// String frequencyToNote(float frequency, float &targetFreqOut) {
//   const char* noteNames[] = {
//     "C0", "C#0", "D0", "D#0", "E0", "F0", "F#0", "G0", "G#0", "A0", "A#0", "B0",
//     "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1",
//     "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2",
//     "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",
//     "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
//     "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",
//     "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6",
//     "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7", "A7", "A#7", "B7",
//     "C8"
//   };

//   int noteCount = sizeof(noteNames) / sizeof(noteNames[0]);
//   float baseFreq = 16.35; // C0
//   int closestNote = 0;
//   float minDiff = 1e6;
//   float closestFreq = 0;

//   for (int i = 0; i < noteCount; i++) {
//     float noteFreq = baseFreq * pow(2.0, i / 12.0);
//     float diff = fabs(frequency - noteFreq);
//     if (diff < minDiff) {
//       minDiff = diff;
//       closestNote = i;
//       closestFreq = noteFreq;
//     }
//   }

//   targetFreqOut = closestFreq;
//   return String(noteNames[closestNote]);
// }

#include <Audio.h>        //for audio functions: 
#include <Wire.h>
#include <SPI.h>
#include <SerialFlash.h>
#include <math.h>         // for FFT

//Prototypes for functions
String frequencyToNote(float frequency, float &targetFreqOut);
float interpFreq(int index);  

// Audio system setup
AudioInputI2S           i2s1;
AudioAnalyzeFFT1024     fft1024;
AudioConnection         patchCord1(i2s1, 0, fft1024, 0);
AudioControlSGTL5000    sgtl5000;

//setup lol
void setup() {
  Serial.begin(115200);
  AudioMemory(12);

  //Set up ADC/DSP chip 
  sgtl5000.enable();
  sgtl5000.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000.volume(0.7);
  sgtl5000.micGain(30);

  //Hanning window is used to reduce "leakage" of the audio to help find the true pitch 
  fft1024.windowFunction(AudioWindowHanning1024); 
}

void loop() {   //Continuous listen/ feedback loop
  delay(2000);  //Delay for better readability on terminal

  if (fft1024.available()) {  //if sound
    //Print first 10 FFT bins, left in for debugging
    Serial.print("FFT bins: ");
    for (int i = 0; i < 10; i++) {
      Serial.print(fft1024.read(i), 6);
      Serial.print(" ");
    }
    Serial.println();

    // Find two peaks from input wave
    int peak1 = 0, peak2 = 0;
    float val1 = 0, val2 = 0;
    //Go through the Bin Values to discern Two freq.
    for (int i = 1; i < 511; i++) { 
      float val = fft1024.read(i);
      if (val > val1) {
        val2 = val1;
        peak2 = peak1;
        val1 = val;
        peak1 = i;
      } else if (val > val2) {
        val2 = val;
        peak2 = i;
      }
    }

    // find frequency from final peaks
    float freq1 = interpFreq(peak1); 
    float freq2 = interpFreq(peak2);

    // find musical pitch from freqeuncy
    float ideal1, ideal2;
    String note1 = frequencyToNote(freq1, ideal1);
    String note2 = frequencyToNote(freq2, ideal2);

    //Tuning feedback
    auto tuningStatus = [](float actual, float ideal) { //auto = generic class
      float diff = actual - ideal;
      if (fabs(diff) <= 10.0f) return "✅ In tune!";   // fabs = float absolute value
      else if (diff < 0) return "🔼 Too low";
      else return "🔽 Too high";
    };

    //Print results
    Serial.printf("Detected Freq 1: %.2f Hz — Note: %s (Ideal: %.2f Hz) — %s\n", 
                  freq1, note1.c_str(), ideal1, tuningStatus(freq1, ideal1));

    Serial.printf("Detected Freq 2: %.2f Hz — Note: %s (Ideal: %.2f Hz) — %s\n",
                  freq2, note2.c_str(), ideal2, tuningStatus(freq2, ideal2));

    Serial.println();
  }
}

// frequency estimation using parabolic interpolation
float interpFreq(int index) {
  float y1 = fft1024.read(index - 1);
  float y2 = fft1024.read(index);
  float y3 = fft1024.read(index + 1);
  float correction = 0.5f * (y1 - y3) / (y1 - 2.0f * y2 + y3);
  float trueIndex = index + correction;
  return trueIndex * (44100.0f / 1024.0f);
}

// frequency to note name and return target freq
String frequencyToNote(float frequency, float &targetFreqOut) {
  const char* noteNames[] = {
    "C0", "C#0", "D0", "D#0", "E0", "F0", "F#0", "G0", "G#0", "A0", "A#0", "B0",
    "C1", "C#1", "D1", "D#1", "E1", "F1", "F#1", "G1", "G#1", "A1", "A#1", "B1",
    "C2", "C#2", "D2", "D#2", "E2", "F2", "F#2", "G2", "G#2", "A2", "A#2", "B2",
    "C3", "C#3", "D3", "D#3", "E3", "F3", "F#3", "G3", "G#3", "A3", "A#3", "B3",
    "C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4", "G#4", "A4", "A#4", "B4",
    "C5", "C#5", "D5", "D#5", "E5", "F5", "F#5", "G5", "G#5", "A5", "A#5", "B5",
    "C6", "C#6", "D6", "D#6", "E6", "F6", "F#6", "G6", "G#6", "A6", "A#6", "B6",
    "C7", "C#7", "D7", "D#7", "E7", "F7", "F#7", "G7", "G#7", "A7", "A#7", "B7",
    "C8"
  };

  int noteCount = sizeof(noteNames) / sizeof(noteNames[0]); //gives number of notes
  float baseFreq = 16.35; // C0, lowest frequency 
  float closestFreq = 0;
  int closestNote = 0;
  float minDiff = 1e6;    //Smallest difference to compare other possible smallest differences

  //Find lowest note
  for (int i = 0; i < noteCount; i++) { //Check pitch against ideal freq of all 88 keys of a piano
    float noteFreq = baseFreq * pow(2.0, i / 12.0); //Calculate theoretical frequency 
    float diff = fabs(frequency - noteFreq); //Compare against calculated frequency
    if (diff < minDiff) { //if difference is smaller than mindiff, replace mindiff as closest pitch
      minDiff = diff;
      closestNote = i;
      closestFreq = noteFreq;
    }
  }

  targetFreqOut = closestFreq;           //Final freq
  return String(noteNames[closestNote]); //Output :)
}
