#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"

//========================================================================
int main()
{
    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1040, 600, OF_WINDOW);
	ofRunApp( new testApp());
}
