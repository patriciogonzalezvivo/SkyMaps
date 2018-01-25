#include "ofApp.h"

#include "GeoLoc/src/GeoLoc.h"
#include "Astro/src/AstroOps.h"

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
    
    double lng = 0.;
    double lat = 0.;
    geoLoc(lng, lat, ofToDataPath(GEOIP_DB), ofToDataPath(GEOLOC_FILE));
    obs = Observer(lng, lat);
    
    initial_jd = obs.getJulianDate();
    
    BodyId bodies_names[] = { SUN, MERCURY, VENUS, EARTH, MARS, JUPITER, SATURN, URANUS, NEPTUNE, PLUTO, LUNA };
    float bodies_sizes[] = { 5, 0.0561, 0.1377, 0.17, 0.255, 1.87*.5, 1.615*.5, 0.68, 0.68, 0.0306, 0.0459 };
    
    for (int i = 0; i < 11; i++) {
        bodies.push_back(ofxBody(bodies_names[i], bodies_sizes[i] * 10.));
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
    obs.setJuliaDay(initial_jd + ofGetElapsedTimef() * 4.);
#endif
    TimeOps::JDtoMDY(obs.getJulianDate(), month, day, year);
    date = ofToString(year) + "/" + ofToString(month,2,'0') + "/" + ofToString(int(day),2,'0');
    
    // Updating BODIES positions
    // --------------------------------
    
    // Update planets positions
    for ( int i = 0; i < bodies.size(); i++) {
        bodies[i].compute(obs);
    }
}

void drawString(std::string str, int x , int y) {
    ofSetColor(255);
    ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
    ofDrawBitmapStringHighlight(str, x - str.length() * 4, y);
}

//--------------------------------------------------------------
void ofApp::draw(){
    // Draw Date
    drawString(date, ofGetWidth()*.5, ofGetHeight()-30);
    
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
