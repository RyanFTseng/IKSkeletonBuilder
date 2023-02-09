
//
//  Starter file for Project 3 - Skeleton Builder
//
//  This file includes functionality that supports selection and translate/rotation
//  of scene objects using the mouse.
//
//  Modifer keys for rotatation are x, y and z keys (for each axis of rotation)
//
//  (c) Kevin M. Smith  - 24 September 2018
//

#include "ofMain.h"
#include "box.h"
#include "Primitives.h"
#include "ofxGui.h"


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
		static void drawAxis(glm::mat4 transform = glm::mat4(1.0), float len = 1.0);
		bool mouseToDragPlane(int x, int y, glm::vec3 &point);
		void printChannels(SceneObject *);
		bool objSelected() { return (selected.size() ? true : false ); };
		void createJoint();
		
		void deleteJoint();
		glm::vec3 findRotateVector(string rotateV);
		glm::vec3 findTransVector(string transV);
		void createJointFromScript(string name, glm::vec3 r, glm::vec3 t, string parent);
		void createRootJointFromScript(string name, glm::vec3 r, glm::vec3 t);
		void readObjFile(string name);
		void saveSkeleton();

		void saveKeyFrame();
		void playback();
		void clearKeyFrames();

		glm::mat4 rotateToVector(glm::vec3 v1, glm::vec3 v2);

		void undoHeirarchy();
		void repairHeirarchy();

		void solveIk();
		void backwards();
		void forwards();

		// Lights
		//
		ofLight light1;
	
		// Cameras
		//
		ofEasyCam  mainCam;
		ofCamera sideCam;
		ofCamera topCam;
		ofCamera  *theCam;    // set to current camera either mainCam or sideCam

		// Materials
		//
		ofMaterial material;


		// scene components
		//
		vector<SceneObject *> scene;
		vector<Joint*> joints;
		vector<SceneObject *> selected;
		vector<glm::vec3> keyframes;
		vector<int> frameCount;
		ofPlanePrimitive plane;

		// state
		bool bDrag = false;
		bool bHide = true;
		bool bAltKeyDown = false;
		bool bRotateX = false;
		bool bRotateY = false;
		bool bRotateZ = false;
		glm::vec3 lastPoint;

		int jointCounter = 0;
		glm::mat4 trans, rot;
		ofFile skeleFile;

		bool bTargetSelected = false;
		glm::vec3 newPath;
		glm::vec3 aimPos;
		glm::vec3 originalRoot;
		bool bSolveIk = false;

		ofxIntSlider frames;
		ofxPanel gui;
		bool bGui;
		int numFrames;
		bool bPlayback = false;
		glm::vec3 direction;
		int atFrame = 0;
};