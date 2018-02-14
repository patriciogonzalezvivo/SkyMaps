#pragma once

#include "ofMain.h"

#define GEOIP_DB "GeoLiteCity.dat"
#define GEOLOC_FILE "geoLoc.csv"

#include "Astro/src/Observer.h"
#include "Astro/src/Luna.h"
#include "Astro/src/Star.h"
#include "Astro/src/Body.h"
#include "Astro/src/Constellation.h"

#include "Astro/src/ProjOps.h"

#if defined(_WIN32) || defined(_WIN64)
#define FULLSCREEN
#else
#include "ofxSyphon.h"
#endif

//#define TIME_ANIMATION 0.01
#define PROJECT(S,X,Y) ProjOps::toXY(proj, S, ofGetWidth(), ofGetHeight(), X, Y)
//#define PROJECT_SHOW

#define FPS_DEBUG

struct Line {
    Horizontal A;
    Horizontal B;
    Horizontal T;
    std::string text;
};

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
#if defined(_WIN32) || defined(_WIN64)
#else
    ofxSyphonServer syphon;
#endif
    
    // Observers
    Observer        obs;
    ProjId          proj;
    
    // Bodies
    vector<Body>    bodies;
    
    // Moon
    Luna            moon;
    ofShader        moonShader;
    
    ofVboMesh       billboard;
    
    // Stars
    vector<Star>    stars;
    vector<ofPoint> starsPos;
    vector<float>   starsSize;
    
    // Constellations
    vector<Constellation> constellations;

    // HUD
    vector<Line>    lines;
    
    // Time
    std::string     date;
    double          lng, lat;
};
