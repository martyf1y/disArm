#include "testApp.h"

/* Note on OS X, you must have this in the Run Script Build Phase of your project. 
where the first path ../../../addons/ofxLeapMotion/ is the path to the ofxLeapMotion addon. 

cp -f ../../../addons/ofxLeapMotion/libs/lib/osx/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/libLeap.dylib"; install_name_tool -change ./libLeap.dylib @executable_path/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/$PRODUCT_NAME";

   If you don't have this you'll see an error in the console: dyld: Library not loaded: @loader_path/libLeap.dylib
*/

//--------------------------------------------------------------
void testApp::setup(){

    ofSetFrameRate(60);
    ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);

	leap.open(); 

	l1.setPosition(-1200, 0, 0);
	l2.setPosition(200, 200, -50);

	cam.setOrientation(ofPoint(-90, 0, 180));
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    
    
    //Model stuff
    ofDisableArbTex(); // we need GL_TEXTURE_2D for our models coords.
    
    model.loadModel("Hammerz/Hammer.3ds");
    ofPoint modelPosition(ofGetWidth() * 0.5, ofGetHeight() * 0.5);
    model.setPosition(-280, 0, 900);
    model.setRotation(0, 270, 1, 0, 0);
    nDraw = 1;
    
    middleFinger.push_back(ofPoint(0, 0, 0));
    middleFinger.push_back(ofPoint(0, 0, 0));
    middleFinger.push_back(ofPoint(0, 0, 0));
    handSpeed.push_back(ofPoint(0, 0, 0));
    handSpeed.push_back(ofPoint(0, 0, 0));
    handSpeed.push_back(ofPoint(0, 0, 0));

    //Sphere
    // we will bounce a circle using these variables:
    px = 0;
    py = 0;
    vx = 0;
    vy = 0;

    cameraViewX = 815;
    cameraViewY = 455;
    ballSize = 50;
    boundWidth = 600;
    boundHeight = 300;
    boundHPos = 50;
    boundHBarrier = 50;
    objectColor = ofColor(200, 200, 0);
    drawColor = ofColor(0, 0, 255);
    nInteraction = 1;
    tSwap = false;
}


//--------------------------------------------------------------
void testApp::update(){
    
    timer = ofGetElapsedTimeMillis() - startTime;
    
    if(timer >= 90000)//every minute
    {
        nInteraction = ofRandom(1, 8);
        startTime = ofGetElapsedTimeMillis();
        tSwap = true;
        keyPressed(nInteraction + 48);
    }
    mXChange = ofMap(mouseX, 0, ofGetWidth(), cameraViewX, -cameraViewX);
    mYChange = ofMap(mouseY, 0, ofGetHeight(), -cameraViewY, cameraViewY);
  //  vx = ofMap(vx, -cameraViewX, cameraViewX, cameraViewX, -cameraViewX);
  //  vy = ofMap(vy, -cameraViewY, cameraViewY, -cameraViewY, cameraViewY);
  //  px = ofMap(px, -cameraViewX, cameraViewX, ofGetWidth()/2, -ofGetWidth()/2);
    //py = ofMap(py, -cameraViewY, cameraViewY, -cameraViewY, cameraViewY);

    
	fingersFound.clear();
	
	//here is a simple example of getting the hands and using them to draw trails from the fingertips. 
	//the leap data is delivered in a threaded callback - so it can be easier to work with this copied hand data
	
	//if instead you want to get the data as it comes in then you can inherit ofxLeapMotion and implement the onFrame method. 
	//there you can work with the frame data directly. 



    //Option 1: Use the simple ofxLeapMotionSimpleHand - this gives you quick access to fingers and palms. 
    
    simpleHands = leap.getSimpleHands();
    
    if( leap.isFrameNew() && simpleHands.size() ){
    
        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
		leap.setMappingY(90, 490, -150, 200);
        leap.setMappingZ(-150, 150, -ofGetHeight()/2, ofGetHeight()/2);
    
        for(int i = 0; i < simpleHands.size(); i++){
        
            //for(int j = 0; j < simpleHands[i].fingers.size(); j++){
                int id = simpleHands[i].fingers[2].id;
            
                ofPolyline & polyline = fingerTrails[id];
                ofPoint pt = simpleHands[i].fingers[2].pos;
                
                //if the distance between the last point and the current point is too big - lets clear the line 
                //this stops us connecting to an old drawing
                if( polyline.size() && (pt-polyline[polyline.size()-1] ).length() > 100 ){
                    polyline.clear(); 
                }
                if(pt.y >= 100)
                {
                //add our point to our trail
                polyline.addVertex(pt); 
                }
                middleFinger[i] = pt;
                handSpeed[i] = simpleHands[i].handVelocity;
                //store fingers seen this frame for drawing
                fingersFound.push_back(id);
            
                //trying to get hand boundary
                //hPos[i] = simpleHands[i].handPos;
           // }
        }
    }

    // (1) we increase px and py by adding vx and vy
    px += vx;
    py += vy;
    
    // (2) check for collision, and trigger sounds:
    // horizontal collisions:
    if (px > cameraViewX){
        px = cameraViewX;
        vx *= -1;
    } else if (px < -cameraViewX){
        px = -cameraViewX;
        vx *= -1;
    }
    // vertical collisions:
    if (py < -cameraViewY){
        py = -cameraViewY;
        vy *= -1;
    } else if (py > cameraViewY){
        py = cameraViewY;
        vy *= -1;
    }
    // (3) slow down velocity:
    vx 	*= 0.946f;
    vy 	*= 0.946f;
    
    // if(mXChange >= px - 25 && mXChange <= px + 25
    //  && mYChange >= py - 25 && mYChange <= py + 25)
    for(int i = 0; i < simpleHands.size(); i++)
    {
        if(middleFinger[i].x >= px - ballSize && middleFinger[i].x -boundWidth <= px + ballSize
           && middleFinger[i].z + boundHeight >= py - ballSize && middleFinger[i].z  - boundHPos - boundHBarrier <= py + ballSize)
        {
            // add into vx and vy a small amount of the change in mouse:
            vx += handSpeed[i].x/20;
            vy += handSpeed[i].z/20;
            // store the previous mouse position:
            // prevx = mXChange;
            // prevy = mYChange;
        }
    }
    // Option 2: Work with the leap data / sdk directly - gives you access to more properties than the simple approach  
    // uncomment code below and comment the code above to use this approach. You can also inhereit ofxLeapMotion and get the data directly via the onFrame callback. 
    
//	vector <Hand> hands = leap.getLeapHands(); 
//	if( leap.isFrameNew() && hands.size() ){
//
//		//leap returns data in mm - lets set a mapping to our world space. 
//		//you can get back a mapped point by using ofxLeapMotion::getMappedofPoint with the Leap::Vector that tipPosition returns  
//		leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
//		leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
//		leap.setMappingZ(-150, 150, -200, 200);
//				
//		for(int i = 0; i < hands.size(); i++){
//            for(int j = 0; j < hands[i].fingers().count(); j++){
//				ofPoint pt; 
//			
//				const Finger & finger = hands[i].fingers()[j];
//								
//				//here we convert the Leap point to an ofPoint - with mapping of coordinates
//				//if you just want the raw point - use ofxLeapMotion::getofPoint 
//				pt = leap.getMappedofPoint( finger.tipPosition() );
//                								
//				//lets get the correct trail (ofPolyline) out of our map - using the finger id as the key 
//				ofPolyline & polyline = fingerTrails[finger.id()]; 
//				
//				//if the distance between the last point and the current point is too big - lets clear the line 
//				//this stops us connecting to an old drawing
//				if( polyline.size() && (pt-polyline[polyline.size()-1] ).length() > 50 ){
//					polyline.clear(); 
//				}
//				
//				//add our point to our trail
//				polyline.addVertex(pt); 
//				
//				//store fingers seen this frame for drawing
//				fingersFound.push_back(finger.id());
//			}
//		}	
//	}
//    

	//IMPORTANT! - tell ofxLeapMotion that the frame is no longer new. 
	leap.markFrameAsOld();	
}

//--------------------------------------------------------------
void testApp::draw(){
    
	ofDisableLighting();
    ofBackgroundGradient(ofColor(120, 120, 120), ofColor(80, 80, 80),  OF_GRADIENT_BAR);
    //ofBackground(0);
   	ofSetColor(200, 20, 40);
    if(nInteraction == 0)
    {
	ofDrawBitmapString("nInteraction " + ofToString(nInteraction), 20, 20);
    ofDrawBitmapString("Y Height " + ofToString(middleFinger[0].y), 20, 60);
    ofDrawBitmapString("Timer " + ofToString(timer), 20, 80);
    ofDrawBitmapString("Xpos " + ofToString(px - 50) + " " + ofToString(middleFinger[0].x), 20, 100);
    ofDrawBitmapString("Ypos " + ofToString(py) + " " + ofToString(mYChange), 20, 120);
    ofDrawBitmapString("Hand Speed " + ofToString(handSpeed[0]), 20, 140);
    }
    else if (nInteraction > 0 && nInteraction < 5)
    {
    ofDrawBitmapStringHighlight("HIT THE BALL!", 20, 20);
    }
    else if (nInteraction > 5 )
    {
    ofDrawBitmapStringHighlight("Lower hand to draw!", 20, 20);
    }

	cam.begin();
    ofEnableLighting();
    l1.enable();
    l2.enable();
    
    
	ofPushMatrix();
		ofRotate(90, 0, 0, 1);
		ofSetColor(20);
		//ofDrawGridPlane(800, 20, false);
	ofPopMatrix();
   
    //Sphere
  /*  if(mXChange >= px - 25 && mXChange <= px + 25
       && mYChange >= py - 25 && mYChange <= py + 25)
    {
        ofSetColor(0, 200, 0);
    }
    else
    {
        ofSetColor(200, 0, 0);
    }*/
    if(nInteraction < 6)
    {
    for(int i = 0; i < simpleHands.size(); i++)
    {
        if(middleFinger[i].x >= px - ballSize && middleFinger[i].x -boundWidth <= px + ballSize
           && middleFinger[i].z + boundHeight >= py - ballSize && middleFinger[i].z  - boundHPos - boundHBarrier <= py + ballSize)
        {
     ofSetColor(0, 200, 0);
     }
     else
     {
     ofSetColor(200, 0, 0);
     }
       // ofDrawPlane(middleFinger[i].x, 0, middleFinger[i].z + 200, 200, 100);

    }
    ofDrawSphere(px, 0, py, 50); //This is the interactive ball
    }

	m1.begin(); 
	m1.setShininess(0.6);
	if(nInteraction > 5)
    {
	for(int i = 0; i < fingersFound.size(); i++){
		ofxStrip strip;
		int id = fingersFound[i];
		
		ofPolyline & polyline = fingerTrails[id];
		strip.generate(polyline.getVertices(), 5, ofPoint(0, 0.5, 0.5) );
		
		//ofSetColor(255 - id * 15, 0, id * 25);
        ofSetColor(drawColor);
        strip.getMesh().draw();
	}
    }
    l2.disable();
    
    // model.setPosition(mouseX, mouseY, 0);
    
        for(int i = 0; i < simpleHands.size(); i++){
        
        simpleHands[i].debugDraw(model, objectColor);
        ofNoFill();
        ofSetColor(230, 0, 0);
       // ofDrawBox(middleFinger[i].x-boundWidth/2, middleFinger[i].y, middleFinger[i].z+boundHPos, boundWidth, 200, boundHeight);
        ofFill();
    }
    
	m1.end();
	cam.end();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    tSwap = true;
    
    //nDraw = 1;
    nInteraction = key;
    
    if(tSwap)
    {
        model.setRotation(0, 0, 0, 0, 0);
        model.setScale(1, 1, 1);
        model.setPosition(0, 0, 0);
        switch (nInteraction) {
                
            case '1':
                model.loadModel("Hammerz/Hammer.3ds");
                //model.setScale(.7, .7, .7);
                model.setPosition(-280, 0, 900);
                model.setRotation(0, 270, 1, 0, 0);
                objectColor = ofColor(0, 0, 0);
                boundWidth = 600;
                boundHeight = 300;
                boundHPos = 50;
                boundHBarrier = 50;
                //ofEnableSeparateSpecularLight();
                break;
            case '2':
                model.loadModel("Whisk/Whisk.3DS");
                model.setPosition(-100, 0,900);
                model.setRotation(0, 90, 1, 0, 0);
                boundWidth = 200;
                boundHeight = 300;
                boundHPos = 50;
                boundHBarrier = 50;
                objectColor = ofColor(200, 200, 0);
                break;
            case '3':
                model.loadModel("PoliceCar/PoliceCar.3DS");
                model.setPosition(-250, 0,550);
                model.setScale(1.3, 1.3, 1.3);
                boundWidth = 600;
                boundHeight = 1000;
                boundHPos = 400;
                boundHBarrier = -300;
                // model.setRotation(0, 0, 0, 0, 0);
                break;
            case '5':
                model.loadModel("paw/bear.stl");
                model.setPosition(-100, 0,870);
                //model.setScale(0.5, 0.5, 1);
                model.setRotation(0, 180, 0, 1, 0);
                boundWidth = 400;
                boundHeight = 800;
                boundHPos = 300;
                boundHBarrier = -200;
                objectColor = ofColor(139, 69, 19);
                
                break;

            case '4':
                model.loadModel("Wheel/Wheel.3DS");
                model.setPosition(-450, 0,170);
                model.setScale(0.5, 0.5, 0.5);
                model.setRotation(0, 90, 0, 0, 1);
                boundWidth = 600;
                boundHeight = 800;
                boundHPos = 300;
                boundHBarrier = -200;
                objectColor = ofColor(0, 0, 0);
                break;
            case '6':
                model.loadModel("Pencil/Pencil.obj");
                model.setPosition(-110, 400, 480);
                objectColor = ofColor(0, 0, 200);
                drawColor = ofColor(0, 0, 200);
                // model.setRotation(0, 50, 1, 0, 0);
                //  ofEnableSeparateSpecularLight();
                break;
            case '7':
                model.loadModel("Airbrush/Airbrush.3ds");
                model.setPosition(460, -160, 20);
                model.setRotation(0, 90, 0, 1, 0);
                objectColor = ofColor(50, 50, 50);
                drawColor = ofColor(0, 200, 0);
                //  model.setRotation(0, -90, 0, 0, 1);
                break;
        }
        tSwap = false;
    }
    nInteraction -= 48;

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
   // x = ofMap(x, 0, ofGetWidth(), cameraViewX, -cameraViewX);
   // y = ofMap(y, 0, ofGetHeight(), -cameraViewY, cameraViewY);
    
    }

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
  //  cam.disableMouseInput();
  //  x = ofMap(x, 0, ofGetWidth(), cameraViewX, -cameraViewX);
  //  y = ofMap(y, 0, ofGetHeight(), -cameraViewY, cameraViewY);
    prevx = mXChange;
    prevy = mYChange;
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

//--------------------------------------------------------------
void testApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
}
