#pragma once

#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "ofxStrip.h"
#include "ofxAssimpModelLoader.h"
#include "ofVboMesh.h"

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
    void exit();
		
	ofxLeapMotion leap;
	vector <ofxLeapMotionSimpleHand> simpleHands;
    
	vector <int> fingersFound; 
	ofEasyCam cam;
	ofLight l1;
	ofLight l2;
	ofMaterial m1;
	
	map <int, ofPolyline> fingerTrails;
    
    
    vector <ofPoint> middleFinger;//this is the coordinates of the middle finger
    //model
    
    // ofMesh mesh;
    // ofLight	light;
    ofxAssimpModelLoader model;
    int nDraw;
    int nAdd;
    
   // vector <ofPoint> hPos;//this is the palm position
    
    //collision
    vector <ofPoint> handSpeed;
    int mXChange;//The mouse is changed for the flipped coordinates
    int mYChange;
    int ballSize;
    int cameraViewX; //This is the max area for the current camera view
    int cameraViewY;
    
    float 				px, py, vx, vy;
    float               prevx, prevy;
    
    int boundWidth, boundHeight, boundHPos, boundHBarrier;
    ofColor objectColor;
    ofColor drawColor;
    
    int nInteraction;//The object to interact
    
    //timer
    float timer;
    float startTime; // store when we start time timer

    bool tSwap;
};
