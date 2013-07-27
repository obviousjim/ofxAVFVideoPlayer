#pragma once

#include "ofMain.h"
#include "ofxAVFVideoPlayer.h"

#include "fft.h"
#include "FFTOctaveAnalyzer.h"

#define BUFFER_SIZE 512


class testApp : public ofBaseApp{

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
    
        std::vector<ofxAVFVideoPlayer *> videoPlayers;
        static const int N_VIDEO_PLAYERS = 1;
    
    float magnitude[BUFFER_SIZE];
    float phase[BUFFER_SIZE];
    float power[BUFFER_SIZE];
    float freq[BUFFER_SIZE/2];
    
    FFTOctaveAnalyzer FFTanalyzer;
    fft		myfft;
};
