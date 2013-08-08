#include "testApp.h"

#include "WavFile.h"

bool bWavSaved = false;

//--------------------------------------------------------------
void testApp::setup()
{
    ofSetVerticalSync(true);

//    videoPlayer.loadMovie("Casey_You_need_noise.mov");
//    videoPlayer.loadMovie("fingers.mov");
    videoPlayer.loadMovie("cat.mp4");
    videoPlayer.play();
    
	fftAnalyzer[0].setup(44100, BUFFER_SIZE/2, 2);
	fftAnalyzer[0].peakHoldTime = 15;         // hold longer
	fftAnalyzer[0].peakDecayRate = 0.95f;     // decay slower
	fftAnalyzer[0].linearEQIntercept = 0.9f;  // reduced gain at lowest frequency
	fftAnalyzer[0].linearEQSlope = 0.01f;     // increasing gain at higher frequencies
    
    fftAnalyzer[1].setup(44100, BUFFER_SIZE/2, 2);
	fftAnalyzer[1].peakHoldTime = 15;         // hold longer
	fftAnalyzer[1].peakDecayRate = 0.95f;     // decay slower
	fftAnalyzer[1].linearEQIntercept = 0.9f;  // reduced gain at lowest frequency
	fftAnalyzer[1].linearEQSlope = 0.01f;     // increasing gain at higher frequencies

}

//--------------------------------------------------------------
void testApp::update()
{
    videoPlayer.update();
    
    if (videoPlayer.isAudioLoaded()) {
        if (!bWavSaved) {
            float * interleavedBuffer = videoPlayer.getAllAmplitudes();
            numAmplitudesPerChannel = videoPlayer.getNumAmplitudes() / 2;
            leftBuffer  = new float[numAmplitudesPerChannel];
            rightBuffer = new float[numAmplitudesPerChannel];
            
            for (int i = 0; i < numAmplitudesPerChannel; i++) {
                leftBuffer[i]  = interleavedBuffer[i * 2 + 0];
                rightBuffer[i] = interleavedBuffer[i * 2 + 1];
            }
            
            // Save out a wav file.        
            WavFile wav;
            wav.setFormat(2, 44100, 16);
            wav.open(ofToDataPath("somefile.wav"), WAVFILE_WRITE);
            wav.write(videoPlayer.getAllAmplitudes(), videoPlayer.getNumAmplitudes());
            wav.close();
            
            cout << "Audio ready" << endl;
            bWavSaved = true;
        }
        
        // calculate fft
        float avgPower = 0.0f;
        
        int idx = (int)(videoPlayer.getPosition() * (numAmplitudesPerChannel - 1));
        fft[0].powerSpectrum(idx, BUFFER_SIZE/2, leftBuffer,  BUFFER_SIZE, &magnitude[0][0], &phase[0][0], &power[0][0], &avgPower);
        fft[1].powerSpectrum(idx, BUFFER_SIZE/2, rightBuffer, BUFFER_SIZE, &magnitude[1][0], &phase[1][0], &power[1][0], &avgPower);
        for (int i = 0; i < BUFFER_SIZE/2; i++) {
            freq[0][i] = magnitude[0][i];
            freq[1][i] = magnitude[1][i];
        }
        
        fftAnalyzer[0].calculate(freq[0]);
        fftAnalyzer[1].calculate(freq[1]);
    }
}

//--------------------------------------------------------------
void testApp::draw()
{
    ofSetColor(255);
    ofFill();
    
    videoPlayer.draw(20, 20);
    
    if (bWavSaved) {
        const float waveformWidth  = ofGetWidth() - 40;
        const float waveformHeight = 300;

        float top = ofGetHeight() - waveformHeight - 20;
        float left = 20;
        int slice = numAmplitudesPerChannel / waveformWidth;
        
        // draw the audio waveform
        ofNoFill();
        
        ofSetColor(ofColor::red);
        ofBeginShape();
        for (int i = 0; i < waveformWidth; i++) {
            ofVertex(left + i, top + waveformHeight / 2.f + leftBuffer[i * slice] * waveformHeight / 2.f);
        }
        ofEndShape();
        
        ofSetColor(ofColor::blue);
        ofBeginShape();
        for (int i = 0; i < waveformWidth; i++) {
            ofVertex(left + i, top + waveformHeight / 2.f + rightBuffer[i * slice] * waveformHeight / 2.f);
        }
        ofEndShape();
            
        // draw a playhead over the waveform
        ofSetColor(ofColor::white);
        ofLine(left + videoPlayer.getPosition() * waveformWidth, top, left + videoPlayer.getPosition() * waveformWidth, top + waveformHeight);
                
        // draw current amplitude on the edges
        ofFill();
        ofSetColor(ofColor::red);
        ofRect(left, top, waveformWidth * ABS(videoPlayer.getAmplitude(0)), 20);
        ofSetColor(ofColor::blue);
        ofRect(left, top + waveformHeight - 20, waveformWidth * ABS(videoPlayer.getAmplitude(1)), 20);
        
        // draw a dot at the playhead position for each channel
        ofSetColor(ofColor::red);
        ofCircle(left + videoPlayer.getPosition() * waveformWidth, top + waveformHeight / 2.f + videoPlayer.getAmplitude(0) * waveformHeight / 2.f, 5);
        ofSetColor(ofColor::blue);
        ofCircle(left + videoPlayer.getPosition() * waveformWidth, top + waveformHeight / 2.f + videoPlayer.getAmplitude(1) * waveformHeight / 2.f, 5);
        
        // draw a frame around the whole thing
        ofSetColor(ofColor::white);
        ofNoFill();
        ofRect(left, top, waveformWidth, waveformHeight);
    
        // draw fft bands
        ofFill();
    
        left += 320 + 20;
        slice = 320 / fftAnalyzer[0].nAverages;
        
        ofSetColor(255);
        for (int i = 0; i < fftAnalyzer[0].nAverages; i++) {
            ofRect(left + (i * slice), 20 + 240, slice, -fftAnalyzer[0].averages[i] * 6);
        }
        ofSetColor(ofColor::red);
        for (int i = 0; i < fftAnalyzer[0].nAverages; i++) {
            ofRect(left + (i * slice), 20 + 240 - fftAnalyzer[0].peaks[i] * 6, slice, -4);
        }
        
        left += 320 + 20;
        slice = 320 / fftAnalyzer[1].nAverages;

        ofSetColor(255);
        for (int i = 0; i < fftAnalyzer[1].nAverages; i++) {
            ofRect(left + (i * slice), 20 + 240, slice, -fftAnalyzer[1].averages[i] * 6);
        }
        ofSetColor(ofColor::blue);
        for (int i = 0; i < fftAnalyzer[1].nAverages; i++) {
            ofRect(left + (i * slice), 20 + 240 - fftAnalyzer[1].peaks[i] * 6, slice, -4);
        }
    }
}

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{

}

//--------------------------------------------------------------
void testApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo)
{

}
