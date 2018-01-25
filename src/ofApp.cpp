#include "ofApp.h"

#include "GeoLoc/src/GeoLoc.h"
#include "Astro/src/AstroOps.h"
#include "Astro/src/ProjOps.h"

#include "TimeOps.h"
double initial_jd;

const std::string month_names[] = { "ENE", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

ofPoint coord2EquatorialSphere(Star &_star, float _distance) {
    Vector eclip = _star.getEquatorialVector() * _distance;
    return ofPoint(eclip.x, eclip.y, eclip.z);
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();
    ofSetBackgroundColor(0);
    ofSetCircleResolution(36);
    
    geoLoc(lng, lat, ofToDataPath(GEOIP_DB), ofToDataPath(GEOLOC_FILE));
    obs = Observer(lng, lat);
    
    initial_jd = obs.getJulianDate();
    
    BodyId bodies_names[] = { SUN, MERCURY, VENUS, EARTH, MARS, JUPITER, SATURN, URANUS, NEPTUNE, PLUTO, LUNA };
    float bodies_sizes[] = { 5, 0.0561, 0.1377, 0.17, 0.255, 1.87*.5, 1.615*.5, 0.68, 0.68, 0.0306, 0.0459 };
    
    for (int i = 0; i < 11; i++) {
        bodies.push_back(ofxBody(bodies_names[i], bodies_sizes[i] * 10.));
    }
    
    for (int i = 0; i < Star::TOTAL; i++) {
        stars.push_back(Star(i));
        starsPos.push_back(ofPoint(0.));
        starsSize.push_back(1. + pow(1.-stars[i].getMagnitud()/6., 1.5)*4.);
    }
    
    for (int i = 0; i < Constellation::TOTAL; i++) {
        constellations.push_back(Constellation(i));
    }
    
    syphon.setName("SkyMaps");
}

//--------------------------------------------------------------
void ofApp::update(){

    // TIME CALCULATIONS
    // --------------------------------
#ifndef TIME_ANIMATION
    obs.setTime();
#else
    obs.setJuliaDay(initial_jd + ofGetElapsedTimef() * .01);
#endif
    TimeOps::JDtoMDY(obs.getJulianDate(), month, day, year);
    TimeOps::toHMS(day, hour, min, sec);
    date = ofToString(year) + "/" + ofToString(month,2,'0') + "/" + ofToString(int(day),2,'0');
    date += " " + ofToString(hour,2,'0') + ":" + ofToString(min,2,'0') + ":" + ofToString(int(sec),2,'0');
    
    // Updating BODIES positions
    // --------------------------------
    
    // Update bodies positions
    for ( int i = 0; i < bodies.size(); i++) {
        bodies[i].compute(obs);
    }
    
    // Update stars positions
    
    for (int i = 0; i < Star::TOTAL; i++) {
        stars[i].compute(obs);
        double x, y;
        ProjOps::horizontalToPolar(stars[i], 1, 1, x, y);
        x -= .5;
        y -= .5;
        starsPos[i].x = x * ofGetHeight();
        starsPos[i].y = y * ofGetHeight();
    }
}

void drawString(std::string str, int x , int y) {
    ofSetColor(255);
    ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
    ofDrawBitmapStringHighlight(str, x - str.length() * 4, y);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofPushMatrix();
    ofTranslate(ofGetWidth()*.5, ofGetHeight()*.5);
    
    ofSetColor(255, 100);
    for (auto& constellation : constellations) {
        vector<int> indices = constellation.getStarIndices();
        for (int i = 0; i < indices.size(); i+=2) {
            if ( starsPos[indices[i]].distance(ofPoint(0)) > ofGetHeight()*.5 ||
                starsPos[indices[i+1]].distance(ofPoint(0)) > ofGetHeight()*.5 ) {
                continue;
            }
            ofDrawLine(starsPos[indices[i]], starsPos[indices[i+1]]);
        }
    }
    
    
    for (int i = 0; i < Star::TOTAL; i++) {
        if ( starsPos[i].distance(ofPoint(0)) > ofGetHeight()*.5) {
            continue;
        }
        ofSetColor(0);
        ofDrawCircle(starsPos[i], starsSize[i]);
        ofSetColor(255);
        ofDrawCircle(starsPos[i], starsSize[i]-1.);
    }
    
   
    
    ofPopMatrix();
    
    // Draw Date
    drawString(date, ofGetWidth()*.5, ofGetHeight()-30);
    
    drawString("N", ofGetWidth()*.5, 20);
    drawString("E", ofGetWidth()*.5-ofGetHeight()*.5, ofGetHeight()*.5);
    drawString("W", ofGetWidth()*.5+ofGetHeight()*.5, ofGetHeight()*.5);
    
    drawString("lng: " + ofToString(lng,2,'0') + "  lat: " + ofToString(lat,2,'0'), ofGetWidth()*.5, ofGetHeight()-50);
    
    // Share screen through Syphon
    syphon.publishScreen();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
