#include "ofApp.h"

#include "GeoLoc/src/GeoLoc.h"
#include "Astro/src/AstroOps.h"
#include "Astro/src/ProjOps.h"

#include "TimeOps.h"
double initial_jd;

const std::string monthNames[] = { "ENE", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
std::vector<string> zodiacSigns = { "Ari", "Tau", "Gem", "Cnc", "Leo", "Vir", "Lib", "Sco", "Sgr", "Cap", "Aqr", "Psc" };

bool in_array(const std::string &value, const std::vector<string> &array){
    return std::find(array.begin(), array.end(), value) != array.end();
}

ofPoint coord2EquatorialSphere(Star &_star, float _distance) {
    Vector eclip = _star.getEquatorialVector() * _distance;
    return ofPoint(eclip.x, eclip.y, eclip.z);
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();
    ofSetBackgroundColor(0);
    ofSetCircleResolution(36);
    
    // Initial Location
    geoLoc(lng, lat, ofToDataPath(GEOIP_DB), ofToDataPath(GEOLOC_FILE));
    obs = Observer(lng, lat);
    
    // Initial Time
    initial_jd = obs.getJulianDate();
    
    // Instanciate Bodies: Sun + 9 planets
    for (int i = 10; i >= 0; i--) {
        if (i != 3) {
            bodies.push_back( Body( BodyId(i) ) );
        }
    }
    
    // Instanciate Moon
    moon = Luna();
    moonShader.load("shaders/moon.vert","shaders/moon.frag");
    
    billboard.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
    billboard.addVertex(ofPoint(-1.,-1));
    billboard.addTexCoord(ofVec2f(0.,1.));
    billboard.addColor(ofFloatColor(1.));
    billboard.addVertex(ofPoint(-1.,1));
    billboard.addTexCoord(ofVec2f(0.,0.));
    billboard.addColor(ofFloatColor(1.));
    billboard.addVertex(ofPoint(1.,1));
    billboard.addTexCoord(ofVec2f(1.,0.));
    billboard.addColor(ofFloatColor(1.));
    billboard.addVertex(ofPoint(1.,-1));
    billboard.addTexCoord(ofVec2f(1.,1.));
    billboard.addColor(ofFloatColor(1.));
    
    // Instanciate Stars
    for (int i = 0; i < Star::TOTAL; i++) {
        stars.push_back(Star(i));
        starsPos.push_back(ofPoint(0.));
        starsSize.push_back(1. + pow(1.-stars[i].getMagnitud()/6., 1.2)*4.);
    }
    
    // Instanciate Constellations
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
    
    // Update moon position
    moon.compute(obs);
    
    // Update stars positions
    for (int i = 0; i < Star::TOTAL; i++) {
        stars[i].compute(obs);
        double x, y;
        ProjOps::horizontalToFisheye(stars[i], x, y);
        starsPos[i].x = x * ofGetHeight();
        starsPos[i].y = y * ofGetHeight();
    }
}

void drawString(std::string str, int x , int y) {
    ofSetColor(255);
    ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE);
    ofDrawBitmapStringHighlight(str, x - str.length() * 4, y);
}

void drawString(std::string str, ofPoint p) {
    drawString(str, p.x, p.y);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofPushMatrix();
    ofTranslate(ofGetWidth()*.5, ofGetHeight()*.5);
    
    // Draw Constellations
    for (auto& constellation : constellations) {
        vector<int> indices = constellation.getStarIndices();
        for (int i = 0; i < indices.size(); i+=2) {
            if ( starsPos[indices[i]].distance(ofPoint(0)) > ofGetHeight()*.5 &&
                starsPos[indices[i+1]].distance(ofPoint(0)) > ofGetHeight()*.5 ) {
                continue;
            }
            std::string name = constellation.getAbbreviation();
            if (in_array(name, zodiacSigns) ) {
                ofSetColor(255, 100);
            }
            else {
                ofSetColor(255, 50);
            }
            ofDrawLine(starsPos[indices[i]], starsPos[indices[i+1]]);
        }
    }
    
    // Draw Stars
    for (int i = 0; i < Star::TOTAL; i++) {
        if ( starsPos[i].distance(ofPoint(0)) > ofGetHeight()*.5) {
            continue;
        }
        ofSetColor(0);
        ofDrawCircle(starsPos[i], starsSize[i]);
        ofSetColor(255,200);
        ofDrawCircle(starsPos[i], starsSize[i]-1.);
    }
    
    // Draw Bodies
    for (int i = 0; i < bodies.size(); i++) {
        double x, y;
        ProjOps::horizontalToFisheye(bodies[i], x, y);
        ofPoint bodyPos = ofPoint(x, y) * ofGetHeight();
        
        if ( bodyPos.distance(ofPoint(0)) > ofGetHeight()*.5) {
            continue;
        }
        
        ofSetColor(255);
        drawString(bodies[i].getBodyName(), bodyPos + ofPoint(0.,20));
        if (bodies[i].getBodyId() == SUN) {
            ofDrawCircle(bodyPos, 10);
        }
        else {
            ofDrawCircle(bodyPos, 3);
        }
        
    }
    
    // Draw Moon
    {
        double x, y;
        ProjOps::horizontalToFisheye(moon, x, y);
        ofPoint moonPos = ofPoint(x, y) * ofGetHeight();
        
        if ( moonPos.distance(ofPoint(0)) < ofGetHeight()*.5) {
            float moonPhase = moon.getAge()/Luna::SYNODIC_MONTH;
            
            ofSetColor(255);
            drawString(moon.getBodyName(), moonPos + ofPoint(0.,25));
            
            moonShader.begin();
            ofPushMatrix();
            ofTranslate(moonPos);
            ofScale(10, 10);
            moonShader.setUniform1f("u_synodic_day", moonPhase);
            billboard.draw();
            ofPopMatrix();
            moonShader.end();
        }
    }
    
    ofPopMatrix();
    
    // Draw Date
    drawString(date, ofGetWidth()*.5, 30);
    drawString("lng: " + ofToString(lng,2,'0') + "  lat: " + ofToString(lat,2,'0'), ofGetWidth()*.5, 50);
    
    drawString("E", ofGetWidth()*.5-ofGetHeight()*.5, ofGetHeight()*.5);
    drawString("W", ofGetWidth()*.5+ofGetHeight()*.5, ofGetHeight()*.5);
    drawString("S", ofGetWidth()*.5, ofGetHeight());
    
    
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
