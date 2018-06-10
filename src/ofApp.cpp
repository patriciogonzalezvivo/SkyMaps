#include "ofApp.h"

#include "GeoLoc/src/GeoLoc.h"
#include "Astro/src/CoordOps.h"
#include "Astro/src/primitives/Vector.h"

#include "TimeOps.h"

const std::string monthNames[] = { "ENE", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
std::vector<string> zodiacSigns = { "Ari", "Tau", "Gem", "Cnc", "Leo", "Vir", "Lib", "Sco", "Sgr", "Cap", "Aqr", "Psc" };
std::vector<string> projectionName = { "Polar", "Fisheye", "Ortho", "Stereo", "Lambert", "Equirectangular" };
const ofFloatColor palette[] = {
    ofFloatColor(0.020, 0.051, 0.090), // 0 Dark
    ofFloatColor(0.188, 0.349, 0.412), // 1 Dark Blue
    ofFloatColor(0.498, 0.773, 0.843), // 2 Blue
    ofFloatColor(0.733, 0.957, 0.976), // 3 Light Blue
    ofFloatColor(1.000, 1.000, 0.886), // 4 Light Yellow
    ofFloatColor(0.933, 0.937, 0.937), // 5 WHITE
    ofFloatColor(0.976, 0.373, 0.349)  // 6 Red
};

bool in_array(const std::string &value, const std::vector<string> &array){
    return std::find(array.begin(), array.end(), value) != array.end();
}

ofPoint coord2EquatorialSphere(Star &_star, float _distance) {
    Vector eclip = _star.getEquatorial().getVector() * _distance;
    return ofPoint(eclip.x, eclip.y, eclip.z);
}

//--------------------------------------------------------------
void ofApp::setup(){
    ofDisableArbTex();
    ofSetBackgroundColor(0);
    ofSetCircleResolution(36);
    ofSetFrameRate(30);
    ofSetVerticalSync(true);
    
    // Initial Location
    geoLoc(lng, lat, ofToDataPath(GEOLOC_FILE));
    obs = Observer(lng, lat);
    
    // Initial Time
    time_offset = 0.;
    time_step = 0.0005;
    time_play = false;
    
    // Instanciate Bodies: Sun + 9 planets
    for (int i = 10; i >= 0; i--) {
        if (i != 3) {
            bodies.push_back( Body( BodyId(i) ) );
        }
    }
    
    // Instanciate Moon
    moon = Luna();
    moonShader.load("shaders/moon");
    
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
    
#ifdef SATELLITES
    // Satellites
    TLE sats[] = {
         TLE("HOBBLE",
             "1 20580U 90037B   18154.57093887 +.00000421 +00000-0 +14812-4 0  9997",
             "2 20580 028.4684 205.1197 0002723 359.7851 153.4291 15.09046689343324"),
         TLE("TERRA",
             "1 25994U 99068A   18154.24441102 -.00000021  00000-0  53030-5 0  9998",
             "2 25994  98.2062 229.3170 0001386  97.9233 262.2105 14.57104269981794"),
        TLE("GOES 16",
            "1 41866U 16071A   18154.50918000 -.00000249  00000-0  00000-0 0  9998",
            "2 41866   0.0024 323.4828 0001042 138.0782 258.4507  1.00269829  5675"),
        TLE("GOES 17",
            "1 43226U 18022A   18153.55154698 -.00000184 +00000-0 +00000-0 0  9995",
            "2 43226 000.0506 131.5682 0001494 306.5382 281.9134 01.00275070000988"),
//        TLE("SUOMI",
//            "1 37849U 11061A   18154.59022466  .00000019  00000-0  29961-4 0  9994",
//            "2 37849  98.7369  93.2509 0000790 115.8241 296.7478 14.19549859341951"),
         TLE("NOAA 19",
             "1 33591U 09005A   18154.53769778  .00000063  00000-0  59621-4 0  9992",
             "2 33591  99.1410 132.2940 0014182   9.6985 350.4457 14.12282740480248"),
         TLE("NOAA 20",
             "1 43013U 17073A   18154.54421336  .00000003  00000-0  22344-4 0  9998",
             "2 43013  98.7249  93.0462 0000870  77.9803 282.1471 14.19559862 27975"),
        TLE("ISS",
            "1 25544U 98067A   18151.37845806  .00001264  00000-0  26359-4 0  9999",
            "2 25544  51.6399 102.5027 0003948 138.3660   3.9342 15.54113216115909")
    };
    
    int N = sizeof(sats)/sizeof(sats[0]);
    for (int i = 0; i < N; i++) {
        satellites.push_back(Satellite(sats[i]));
    }
#endif
    
    
    vector<std::string> direction = { "S", "W", "N", "E" };
    int step = 5;
    int total = 360/step;
    int labelstep = total/direction.size();
    for (int i = 0; i < 72; i++) {
        HorLine h1, h2, v1;
        float a = i*step-180;
        float b = (i+1)*step-180;
        h1.A = Horizontal(0., a, DEGS);
        h1.B = Horizontal(0., b, DEGS);
        h2.A = Horizontal(1., a, DEGS);
        h2.B = Horizontal(1., b, DEGS);

        v1.A = Horizontal(0., a, DEGS);
        
        if (i%labelstep == 0) {
            h1.T = Horizontal(6., a, DEGS);
            h1.text = direction[int(i/labelstep)];
            v1.B = Horizontal(4., a, DEGS);
        }
        else {
            v1.B = Horizontal(3., a, DEGS);
        }
        hlines.push_back(h1);
        hlines.push_back(h2);
        hlines.push_back(v1);
        
        for (int j = 1; j < step; j++) {
            HorLine v2;
            v2.A = Horizontal(1., a+j, DEGS);
            v2.B = Horizontal(2., a+j, DEGS);
            hlines.push_back(v2);
        }
    }
    
    proj = POLAR;
    
#ifdef TARGET_OSX
    syphon.setName("SkyMaps");
#endif
    
#ifdef FULLSCREEN
    ofSetFullscreen(true);
    ofHideCursor();
#endif
    
    updateLines();
}

//--------------------------------------------------------------
void ofApp::update(){

    // TIME CALCULATIONS
    // --------------------------------
    if (time_play) {
        time_offset += time_step;
    }
    obs.setJD(TimeOps::now(UTC) + time_offset);
    
    date = TimeOps::formatDateTime(obs.getJD(), Y_MON_D);
    date += " " + std::string(TimeOps::formatTime(obs.getJD() + 0.1666666667, true));;
    
    // Updating BODIES positions
    // --------------------------------
    
    // Update bodies positions
    for ( int i = 0; i < bodies.size(); i++ ) {
        bodies[i].compute(obs);
    }
    
    // Update moon position
    moon.compute(obs);
    
    // Update stars positions
    for (int i = 0; i < Star::TOTAL; i++) {
        stars[i].compute(obs);
        double x, y;
        
        PROJECT(stars[i].getHorizontal(), x, y);
        starsPos[i] = ofPoint(x, y);
    }
    
#ifdef SATELLITES
    for ( unsigned int i = 0; i < satellites.size(); i++) {
        satellites[i].compute(obs);
    }
#endif
}

void drawString(std::string str, int x , int y, const ofColor& frg = ofColor(255)) {
    ofDrawBitmapStringHighlight(str, x - str.length() * 4, y, ofColor(0.), frg);
}

void drawString(std::string str, ofPoint p, const ofColor& frg = ofColor(255)) {
    drawString(str, p.x + str.length() * .5, p.y + 6., frg);
}

void ofApp::updateLines() {
    slines.clear();
    
//    // Meridian
//    double x1, y1, x2, y2, x3, y3;
//    PROJECT(Horizontal(90., 0., DEGS), x1, y1);
//    PROJECT(Horizontal(180., 0., DEGS), x2, y2);
//    PROJECT(Horizontal(180., 180., DEGS), x3, y3);
//    ofSetColor(palette[3]);
//    ScrLine nl1;
//    nl1.A = ofPoint(x1, y1);
//    nl1.B = ofPoint(x2, y2);
//    slines.push_back(nl1);
//    
//    ScrLine nl2;
//    nl2.A = ofPoint(x1, y1);
//    nl2.B = ofPoint(x3, y3);
//    slines.push_back(nl2);

    // Borders
    ofSetColor(palette[5]);
    for (int i = 0; i < hlines.size(); i++) {
        double x1, y1, x2, y2;
        PROJECT(hlines[i].A, x1, y1);
        PROJECT(hlines[i].B, x2, y2);
        ScrLine nl;
        nl.A = ofPoint(x1, y1);
        nl.B = ofPoint(x2, y2);
        
        if (hlines[i].text != "") {
            double x3, y3;
            PROJECT(hlines[i].T, x3, y3);
            nl.T = ofPoint(x3, y3);
            nl.text = hlines[i].text;
        }
        
        slines.push_back(nl);
    }
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
        
//        for ( unsigned int i = 0; i < satellites.size(); i++) {
//            Constellation c = Constellation( satellites[i].getEquatorial() );
//            if (constellation.getId() == c.getId()) {
//                bZodiac = true;
//                break;
//            }
//        }

        if (bZodiac) {
            ofSetLineWidth(2);
            ofSetColor(palette[2], 120);
        }
        else {
            ofSetLineWidth(1);
            ofSetColor(palette[2], 70);
        }

        for (int i = 0; i < indices.size(); i+=2) {
            if (stars[indices[i]].getHorizontal().getAltitud(RADS) < 0 &&
                stars[indices[i+1]].getHorizontal().getAltitud(RADS) < 0 ) {
                continue;
            }
            
            if (starsPos[indices[i]].distance(starsPos[indices[i+1]]) > ofGetWidth() * .5) {
                continue;
            }

            bVisible = true;
            ofDrawLine(starsPos[indices[i]], starsPos[indices[i+1]]);
        }

        if (bZodiac) {
            ofSetLineWidth(2);
            ofSetColor(palette[6], 200);
        }
        else {
            ofSetLineWidth(1);
            ofSetColor(palette[6], 70);
        }

        if (bVisible) {
            vector<Equatorial> boundary = constellation.getBoundary();

            for (int i = 0; i < boundary.size()-1; i++ ) {
                Horizontal A = CoordOps::toHorizontal(obs, boundary[i]);
                Horizontal B = CoordOps::toHorizontal(obs, boundary[i+1]);

                double x1, y1, x2, y2;
                PROJECT(A, x1, y1);
                PROJECT(B, x2, y2);
                
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
        if ( stars[i].getHorizontal().getAltitud(RADS) < 0 ) {
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
        if ( bodies[i].getHorizontal().getAltitud(RADS) < 0 ) {
            continue;
        }

        double x, y;
        PROJECT(bodies[i].getHorizontal(), x, y);
        ofPoint bodyPos = ofPoint(x, y);
        
        ofPushStyle();
        ofSetColor(palette[5]);
        drawString(string(bodies[i].getName()), bodyPos + ofPoint(0.,20));
#ifdef DEBUG_HOUR_ANGLE
        drawString(MathOps::formatAngle(bodies[i].getHourAngle(RADS), RADS, Dd) , bodyPos + ofPoint(0.,40), palette[3]);
#endif
        if (bodies[i].getId() == SUN) {
            ofDrawCircle(bodyPos, 10);
        }
        else {
            ofDrawCircle(bodyPos, 3);
        }
        ofPopStyle();
    }

    // Draw Moon
    {
        if ( moon.getHorizontal().getAltitud(RADS) > 0 ) {
            double x, y;
            
            PROJECT(moon.getHorizontal(), x, y);
            ofPoint moonPos = ofPoint(x, y);
            
            float moonPhase = moon.getAge()/Luna::SYNODIC_MONTH;

            ofSetColor(255);
            drawString(moon.getName(), moonPos + ofPoint(0.,25));

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
    
#ifdef SATELLITES
    for ( unsigned int i = 0; i < satellites.size(); i++) {
        if ( satellites[i].getHorizontal().getAltitud(RADS) < 0 ) {
            continue;
        }

        double x, y;
        PROJECT(satellites[i].getHorizontal(), x, y);
        ofPoint bodyPos = ofPoint(x, y);
        
        ofPushStyle();
        ofSetColor(palette[3]);
        drawString(string(satellites[i].getName()), bodyPos + ofPoint(0.,20));
#ifdef DEBUG_HOUR_ANGLE
        drawString(MathOps::formatAngle(satellites[i].getHourAngle(RADS), RADS, Dd) , bodyPos + ofPoint(0.,40), palette[3]);
#endif
        ofDrawRectangle(bodyPos, 5, 5);
        ofPopStyle();
        
    }
#endif
    
    // HUD
    ofSetColor(palette[3]);
    for (int i = 0; i < slines.size(); i++) {
        ofDrawLine(slines[i].A, slines[i].B);
        if (slines[i].text != "") {
            drawString(slines[i].text, slines[i].T);
        }
    }

    ofPopMatrix();
    
    // Draw Date
#ifdef PROJECT_SHOW
    drawString(projectionName[proj], ofGetWidth()*.5, 30);
#endif
    drawString(date, ofGetWidth()*.5, 50);
    drawString("lng: " + ofToString(lng,2,'0') + "  lat: " + ofToString(lat,2,'0'), ofGetWidth()*.5, 70);

    
#ifdef TARGET_OSX
    syphon.publishScreen();
#endif
    
#ifdef FPS_DEBUG
    ofDrawBitmapString(ofToString(ofGetFrameRate()), 5, 15);
#endif
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if ( key == 'r' ) {
        time_offset -= time_step;
    }
    else if ( key == 'f' ) {
        time_offset += time_step;
    }
    if ( key == '-' ) {
        time_step -= 0.0001;
    }
    else if ( key == '=' ) {
        time_step += 0.0001;
    }
    else if ( key == 'v' ) {
        time_offset = 0;
    }
    else if ( key == 'p' ) {
        proj = ProjId((proj+1)%6);
        updateLines();
    }
    else if ( key == 'f' ) {
        ofToggleFullscreen();
    }
    else {
        time_play = !time_play;
    }
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
    updateLines();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
