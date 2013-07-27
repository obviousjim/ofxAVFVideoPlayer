#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    for(int i=0; i<N_VIDEO_PLAYERS; i++) {
        videoPlayers.push_back(new ofxAVFVideoPlayer());
        videoPlayers[i]->loadMovie("/Users/focus/Desktop/RGBD_Compiled/Casey_tangle.mov");
    }
    
    
    ofSetVerticalSync(true);
    
    
	FFTanalyzer.setup(44100, BUFFER_SIZE/2, 2);
	
	FFTanalyzer.peakHoldTime = 15; // hold longer
	FFTanalyzer.peakDecayRate = 0.95f; // decay slower
	FFTanalyzer.linearEQIntercept = 0.9f; // reduced gain at lowest frequency
	FFTanalyzer.linearEQSlope = 0.01f; // increasing gain at higher frequencies

    
}

//--------------------------------------------------------------
void testApp::update(){
    int i=0;
    for(auto p : videoPlayers) {
        p->update();
        if(true || p->isLoaded()) {
            if(ofGetElapsedTimef() > i++ * 0.5)
                p->play();
        }
    }
        
    //cout << ofGetFrameRate() << endl;
}

//--------------------------------------------------------------
void testApp::draw(){
    int i=0;
    for(auto p : videoPlayers) {
        // draw video
        ofSetColor(ofColor::white);
        ofFill();
		p->draw(0,0);
        
        // draw audio waveform
        ofSetColor(ofColor::red);
        ofNoFill();
        ofBeginShape();
        for (int i = 0; i < ofGetWidth(); i++) {
            ofVertex(i, ofGetHeight() / 2 + p->getAmplitudeAt(i / (float)ofGetWidth()) * ofGetHeight() / 2);
        }
        ofEndShape();
        ofRect(0, ofGetHeight() - 20, p->getAmplitude() * ofGetWidth(), 20);
        
        // draw playhead over the waveform
        ofSetColor(ofColor::white);
        ofLine(p->getPosition() * ofGetWidth(), 0, p->getPosition() * ofGetWidth(), ofGetHeight());
        
        // draw current amplitude at the bottom
        ofFill();
        ofRect(0, ofGetHeight() - 20, ofGetWidth() * ABS(p->getAmplitude()), 20);
        
        // calculate fft
        float avg_power = 0.0f;
        
        int idx = MIN(floor(p->getPosition() * p->getNumAmplitudes()), p->getNumAmplitudes() - 1);
        myfft.powerSpectrum(idx, (int)BUFFER_SIZE/2, p->getAllAmplitudes(), BUFFER_SIZE, &magnitude[0], &phase[0], &power[0], &avg_power);
        
        for (int i = 0; i < (int)(BUFFER_SIZE/2); i++) {
            freq[i] = magnitude[i];
        }
        
        FFTanalyzer.calculate(freq);
        
        // draw fft bands
        ofSetHexColor(0xffffff);
        // This draws frequency bands (lots of em!)
//        for (int i = 0; i < (int)(BUFFER_SIZE/2 - 1); i++) {
//            ofRect(20 + (i*4), ofGetHeight() - 40, 4, -freq[i] * 10.0f);
//        }
        
        for (int i = 0; i < FFTanalyzer.nAverages; i++) {
            ofRect(ofGetWidth() / 2 + (i*20), ofGetHeight() - 40, 20, -FFTanalyzer.averages[i] * 6);
        }
        
        ofSetHexColor(0xff0000);
        for (int i = 0; i < FFTanalyzer.nAverages; i++) {
            ofRect(ofGetWidth() / 2 + (i*20), ofGetHeight() - 40 - FFTanalyzer.peaks[i] * 6 - 100, 20, -4);
        }
    }

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key) {
        case '1':
            videoPlayers[0]->loadMovie("IntroVideo7.mov");
            break;
        case '2':
            videoPlayers[1]->loadMovie("TheLumineers_1.mov");
            break;
        case '3':
            videoPlayers[2]->loadMovie("EmeliSande_NextToMe.mov");
            break;
        case '4':
            videoPlayers[3]->loadMovie("iHRMF2012_SwedishHouseMafia_DontWorryChild.mov");
            break;
    }
//    videoPlayer2.loadMovie("IntroVideo7.mov");
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}