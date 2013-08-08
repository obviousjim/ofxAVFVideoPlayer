#pragma once

#include "ofMain.h"
#include "ofxAVFVideoPlayer.h"

#include "fft.h"
#include "FFTOctaveAnalyzer.h"

#define BUFFER_SIZE 512

//--------------------------------------------------------------
//--------------------------------------------------------------
class testApp : public ofBaseApp
{
	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        ofxAVFVideoPlayer videoPlayer;
    
        float * leftBuffer;
        float * rightBuffer;
        int numAmplitudesPerChannel;
    
        float magnitude[2][BUFFER_SIZE];
        float phase[2][BUFFER_SIZE];
        float power[2][BUFFER_SIZE];
        float freq[2][BUFFER_SIZE/2];
        fft	fft[2];
        FFTOctaveAnalyzer fftAnalyzer[2];
};
