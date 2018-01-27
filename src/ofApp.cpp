#include "ofApp.h"

#include "GeoLoc/src/GeoLoc.h"
#include "Astro/src/AstroOps.h"
#include "Astro/src/Vector.h"

#include "TimeOps.h"
double initial_jd;

const std::string monthNames[] = { "ENE", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
std::vector<string> zodiacSigns = { "Ari", "Tau", "Gem", "Cnc", "Leo", "Vir", "Lib", "Sco", "Sgr", "Cap", "Aqr", "Psc" };
std::vector<string> projectionName = { "Polar", "Fisheye", "Ortho", "Stereo", "Lambert", "Equirectangular" };

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
        starsSize.push_back(0.8 * max(3-stars[i].getMagnitud()/2.1, 0.5));
    }
    
    // Instanciate Constellations
    for (int i = 0; i < Constellation::TOTAL; i++) {
        constellations.push_back(Constellation(i));
    }
    
    vector<std::string> direction = { "S", "E", "N", "W" };
    int step = 5;
    int total = 360/step;
    int labelstep = total/direction.size();
    for (int i = 0; i < 72; i++) {
        Line h;
        float a = i*step-180;
        float b = (i+1)*step-180;
        h.A = ofPoint(0.0, ofDegToRad(a));
        h.B = ofPoint(0.0, ofDegToRad(b));
        h.T = ofPoint(ofDegToRad(3.0), ofDegToRad(a));
        if (i%labelstep == 0) {
            h.text = direction[int(i/labelstep)];
        }
        lines.push_back(h);
    }
    
    proj = POLAR;
    
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
        
        PROJECT(stars[i], x, y);
        starsPos[i] = ofPoint(x, y);
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

    // Draw Constellations
    for (auto& constellation : constellations) {
        vector<int> indices = constellation.getStarIndices();

        bool bVisible = false;
        bool bZodiac = false;

        std::string name = constellation.getAbbreviation();
        if (in_array(name, zodiacSigns) ) {
            bZodiac = true;
        }

        if (bZodiac) {
            ofSetLineWidth(1);
            ofSetColor(255, 120);
        }
        else {
            ofSetLineWidth(2);
            ofSetColor(255, 70);
        }

        for (int i = 0; i < indices.size(); i+=2) {
            if (stars[indices[i]].getAltitud() < 0 &&
                stars[indices[i+1]].getAltitud() < 0 ) {
                continue;
            }
            
            if (starsPos[indices[i]].distance(starsPos[indices[i+1]]) > ofGetWidth() * .5) {
                continue;
            }

            bVisible = true;
            ofDrawLine(starsPos[indices[i]], starsPos[indices[i+1]]);
        }

        if (bZodiac) {
            ofSetLineWidth(1);
            ofSetColor(255, 0, 0, 100);
        }
        else {
            ofSetLineWidth(2);
            ofSetColor(255, 0, 0, 50);
        }

        if (bVisible) {
            vector<EqPoint> boundary = constellation.getBoundary();

            for (int i = 0; i < boundary.size()-1; i++ ) {
                boundary[i].compute(obs);
                boundary[i+1].compute(obs);

                double x1, y1, x2, y2;
                PROJECT(boundary[i], x1, y1);
                PROJECT(boundary[i+1], x2, y2);
                
                if (ofPoint(x1, y1).distance(ofPoint(x2, y2)) > ofGetWidth() * .5) {
                    continue;
                }
                
                ofDrawLine(ofPoint(x1, y1), ofPoint(x2, y2));
            }
        }
    }
    ofSetLineWidth(1);

    // Draw Stars
    for (int i = 0; i < Star::TOTAL; i++) {
        if ( stars[i].getAltitud() < 0 ) {
            continue;
        }
        float size = starsSize[i];
//        size *= exp(-(90-stars[i].getAltitud())*0.01);
        ofSetColor(0);
        ofDrawCircle(starsPos[i], size+1);
        ofSetColor(255,200);
        ofDrawCircle(starsPos[i], size);
    }

    // Draw Bodies
    for (int i = 0; i < bodies.size(); i++) {
        double x, y;

        PROJECT(bodies[i], x, y);
        ofPoint bodyPos = ofPoint(x, y);

        if ( bodies[i].getAltitudRadians() < 0 ) {
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

        PROJECT(moon, x, y);
        ofPoint moonPos = ofPoint(x, y);

        if ( moon.getAltitud() > 0 ) {
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
    
    ofSetColor(255);
    for (int i = 0; i < lines.size(); i++) {
        double x1, y1, x2, y2;
        PROJECTV(lines[i].A.x, lines[i].A.y, x1, y1);
        PROJECTV(lines[i].B.x, lines[i].B.y, x2, y2);
        ofDrawLine(x1, y1, x2, y2);
        if (lines[i].text != "") {
            double x3, y3;
            PROJECTV(lines[i].T.x, lines[i].T.y, x3, y3);
            drawString(lines[i].text, ofPoint(x3, y3));
        }
    }

    ofPopMatrix();
    
    // Draw Date
    drawString(projectionName[proj], ofGetWidth()*.5, 30);
    drawString(date, ofGetWidth()*.5, 50);
    drawString("lng: " + ofToString(lng,2,'0') + "  lat: " + ofToString(lat,2,'0'), ofGetWidth()*.5, 70);

    
    // Share screen through Syphon
    syphon.publishScreen();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    proj = ProjId((proj+1)%6);
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
