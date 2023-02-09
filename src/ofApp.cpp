
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


#include "ofApp.h"
#include <string> 
#include <iostream>
#include <fstream>
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"

glm::mat4 ofApp::rotateToVector(glm::vec3 v1, glm::vec3 v2) {

	glm::vec3 axis = glm::cross(v1, v2);
	glm::quat q = glm::angleAxis(glm::angle(v1, v2), glm::normalize(axis));
	return glm::toMat4(q);
}

void ofApp::saveKeyFrame() {
	keyframes.push_back(aimPos);
	if (keyframes.size() > 1) {
		int numFrames = frames;
		frameCount.push_back(numFrames);
	}
	cout << "Keyframe Added" << endl;
}

void ofApp::playback() {
	float directionLength = 0.05;
	if (glm::distance(keyframes[atFrame], aimPos) < directionLength) {
		//calculate direction to move target to next keyframe
		direction = keyframes[atFrame + 1] - keyframes[atFrame];
		int nFrames = frameCount[atFrame];
		direction /= nFrames;
		directionLength = glm::length(direction);
	}
	if (glm::distance(aimPos, keyframes[atFrame + 1]) > directionLength) {
		//animate target
		aimPos += direction;
	}
	else {
		//shift target into position if close enough to keyframe position
		aimPos = keyframes[atFrame + 1];
		atFrame++;
	}
	if (glm::distance(aimPos, keyframes[keyframes.size() - 1]) < directionLength) {
		//end playback when last keyframe reached
		atFrame = 0;
		bPlayback = false;
	}
}

void ofApp::clearKeyFrames() {
	keyframes.clear();
	frameCount.clear();
	cout << "Key Frames Cleared" << endl;
}

void ofApp::undoHeirarchy() {
	glm::vec3 pos;
	for (int i = joints.size() - 1; i > 0; i--) {
		pos = joints[i]->position;
		joints[i]->parent = NULL;
		joints[i - 1]->childList.clear();
		joints[i]->position = pos;
		joints[i]->position += joints[i - 1]->getPosition();
	}
}

void ofApp::repairHeirarchy() {
	glm::vec3 pos;
	for (int i = 1; i < joints.size(); i++) {
		pos = joints[i]->getPosition();
		joints[i - 1]->addChild(joints[i]);
		joints[i]->parent = joints[i - 1];
		joints[i]->position = pos;
		joints[i]->position -= joints[i - 1]->getPosition();
	}
}

void ofApp::solveIk() {
	//check if skeleton is a single chain
	bool valid = false;
	if (joints.size() > 1) {
		for (int i = 0; i < joints.size() - 1; i++) {
			if (joints[i]->childList.size() == 1) {
				valid = true;
			}

		}
	}
	if (valid) {
		float length = 0;
		float tolerance = 0.01;
		float distFromGoal = glm::distance(joints[0]->getPosition(), aimPos);
		originalRoot = joints[0]->getPosition();

		//calculate length of skeleton	
		for (int i = 1; i < joints.size(); i++) {
			length += glm::distance(joints[i]->getPosition(), joints[i - 1]->getPosition());
		}

		//align all joints straight to target if target is out of range
		if (length <= distFromGoal) {
			glm::vec3 parentToChild;
			for (int i = 1; i < joints.size(); i++) {
				glm::vec3 straightToGoal = glm::normalize(aimPos - glm::vec3(joints[0]->getPosition()));
				float l = glm::distance(joints[i]->getPosition(), joints[i - 1]->getPosition());
				straightToGoal *= l;
				joints[i]->position = straightToGoal;
			}
		}

		else {
			//solve ik
			undoHeirarchy();
			int iterations = 0;
			float dif = glm::distance(joints[joints.size() - 1]->getPosition(), aimPos);
			while (dif > tolerance) {
				backwards();
				forwards();
				dif = glm::distance(joints[joints.size() - 1]->getPosition(), aimPos);
				iterations++;
				//break out if target is not reached within 15 iterations
				if (iterations > 15) {
					break;
				}
			}
			repairHeirarchy();
		}
	}
}

//backward FABRIK
void ofApp::backwards() {
	glm::vec3 target = aimPos;
	float l = glm::distance(joints[joints.size() - 1]->getPosition(), joints[joints.size() - 2]->getPosition());
	for (int i = joints.size() - 1; i >= 1; i--) {
		joints[i]->position = (target);
		newPath = (glm::normalize(glm::vec3(joints[i-1]->getPosition()) - glm::vec3(joints[i]->getPosition()))) * l + glm::vec3(joints[i]->getPosition());
		if (i > 1) {
			l = glm::distance(joints[i-1]->getPosition(), joints[i - 2]->getPosition());
		}
		joints[i-1]->position = newPath;
		target = joints[i-1]->position;
	}
}

//forward FABRIK
void ofApp::forwards() {
	glm::vec3 target = originalRoot;
	float l = glm::distance(joints[0]->getPosition(), joints[1]->getPosition());
	for (int i = 0; i < joints.size()-1; i++) {
		joints[i]->position = (target);
		newPath = (glm::normalize(glm::vec3(joints[i+1]->getPosition()) - glm::vec3(joints[i]->getPosition()))) * l + glm::vec3(joints[i]->getPosition());
		if (i < joints.size()-2) {
			l = glm::distance(joints[i+1]->getPosition(), joints[i + 2]->getPosition());
		}
		joints[i + 1]->position = newPath;
		target = joints[i+1]->position;
	}
}

void ofApp::createJoint() {
	
	Joint* J = new Joint(glm::vec3(1,1,1), 0.3, ofColor(rand() % 255, rand() % 255, rand() % 255), "joint" + std::to_string(jointCounter));
	
	jointCounter++;
	if (!selected.empty()) {
		selected[0]->addChild(J);
	}
	joints.push_back(J);
	scene.push_back(J);
	
}

void ofApp::deleteJoint() {
	glm::vec3 pos = selected[0]->position;
	if (selected[0]->parent != NULL) {
		for (int i = 0; i < selected[0]->childList.size(); i++) {
			
			selected[0]->parent->childList.push_back(selected[0]->childList[i]);
			selected[0]->childList[i]->parent = selected[0]->parent;
			selected[0]->childList[i]->position += pos;
		}
	}
	else {
		for (int i = 0; i < selected[0]->childList.size(); i++) {
			selected[0]->childList[i]->parent = NULL;
			selected[0]->childList[i]->position += pos;
		}
	}
	
	scene.erase(std::remove(scene.begin(), scene.end(), selected[0]), scene.end());
	joints.erase(std::remove(joints.begin(), joints.end(), selected[0]), joints.end());
	selected.clear();
	
}

void ofApp::saveSkeleton() {
	ofstream myfile;
	myfile.open("skeleton.txt");
	for (int i = 0; i < joints.size(); i++) {
		if (joints[i]->parent != NULL) {
			myfile << "create -joint " + joints[i]->name
				+ " -rotate <" + std::to_string(joints[i]->rotation.x) + ", " + std::to_string(joints[i]->rotation.y) + ", " + std::to_string(joints[i]->rotation.z)
				+ "> -translate <" + std::to_string(joints[i]->position.x) + ", " + std::to_string(joints[i]->position.y) + ", " + std::to_string(joints[i]->position.z)
				+ "> -parent " + joints[i]->parent->name +";\n";
		}
		else {
			myfile << "create -joint " + joints[i]->name
				+ " -rotate <" + std::to_string(joints[i]->rotation.x) + ", " + std::to_string(joints[i]->rotation.y) + ", " + std::to_string(joints[i]->rotation.z)
				+ "> -translate <" + std::to_string(joints[i]->position.x) + ", " + std::to_string(joints[i]->position.y) + ", " + std::to_string(joints[i]->position.z)
				+ ">;\n";
		}
	}
	myfile.close();
	cout << "skeleton saved" << endl;
}

//--------------------------------------------------------------
//
void ofApp::setup() {
	gui.setup();
	gui.add(frames.setup("frames", 60, 1, 300));

	aimPos = glm::vec3(8, 0, 0);
	ofSetBackgroundColor(ofColor::black);
	ofEnableDepthTest();
	mainCam.setDistance(15);
	mainCam.setNearClip(.1);
	
	sideCam.setPosition(40, 0, 0);
	sideCam.lookAt(glm::vec3(0, 0, 0));
	topCam.setNearClip(.1);
	topCam.setPosition(0, 16, 0);
	topCam.lookAt(glm::vec3(0, 0, 0));
	ofSetSmoothLighting(true);


	// setup one point light
	//
	light1.enable();
	light1.setPosition(5, 5, 0);
	light1.setDiffuseColor(ofColor(255.f, 255.f, 255.f));
	light1.setSpecularColor(ofColor(255.f, 255.f, 255.f));

	theCam = &mainCam;

	//  create a scene consisting of a ground plane with 2x2 blocks
	//  arranged in semi-random positions, scales and rotations
	//
	// ground plane
	//
	scene.push_back(new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0)));   
}

 
//--------------------------------------------------------------
void ofApp::update(){
	if (bSolveIk) {
		solveIk();
	}

	if (bPlayback) {
		if (keyframes.size() > 0) {
			playback();
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	if (!bGui) {
		ofDisableDepthTest();
		gui.draw();

	}
	ofEnableDepthTest();
	theCam->begin();
	ofNoFill();
	drawAxis();
	ofEnableLighting();

	if (bSolveIk) {
		ofDrawSphere(aimPos, .2);
	}

	//  draw the objects in scene
	//
	material.begin();
	ofFill();
	for (int i = 0; i < scene.size(); i++) {
		if (objSelected() && scene[i] == selected[0])
			ofSetColor(ofColor::white);
		else ofSetColor(scene[i]->diffuseColor);
		scene[i]->draw();
	}



	material.end();
	ofDisableLighting();
	theCam->end();

	
}



// 
// Draw an XYZ axis in RGB at transform
//
void ofApp::drawAxis(glm::mat4 m, float len) {

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	//ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(len, 0, 0, 1)));


	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	//ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, len, 0, 1)));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	//ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, 0, len, 1)));
}

// print C++ code for obj tranformation channels. (for debugging);
//
void ofApp::printChannels(SceneObject *obj) {
	cout << "position = glm::vec3(" << obj->position.x << "," << obj->position.y << "," << obj->position.z << ");" << endl;
	cout << "rotation = glm::vec3(" << obj->rotation.x << "," << obj->rotation.y << "," << obj->rotation.z << ");" << endl;
	cout << "scale = glm::vec3(" << obj->scale.x << "," << obj->scale.y << "," << obj->scale.z << ");" << endl;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

	switch (key) {
	case OF_KEY_ALT:
		bAltKeyDown = false;
		mainCam.disableMouseInput();
		break;
	case 'x':
		bRotateX = false;
		break;
	case 'y':
		bRotateY = false;
		break;
	case 'z':
		bRotateZ = false;
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case 'C':
	case 'c':
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case 'd':
		clearKeyFrames();
		break;
	case 'F':
	case 'b':
		for (int i = 0; i < joints.size(); i++) {
			cout << joints[i]->getPosition() << endl;
		}
		break;
	case 'f':
		ofToggleFullscreen();
		break;
	case 'g':
		if (bGui) {
			bGui = false;
		}
		else {
			bGui = true;
		}
		break;
	case 'h':
		bHide = !bHide;
		break;
	case 'i':
		if (bSolveIk) {
			bSolveIk = false;
		}
		else {
			bSolveIk = true;
		}
		break;
	case 'j':
		createJoint();
		break;
	case 'k':
		saveKeyFrame();
		break;
	case 'n':
		break;
	case 'p':
		//if (objSelected()) printChannels(selected[0]);
		
		
		if (bPlayback) {
			bPlayback = false;
		}
		else {
			aimPos = keyframes[0];
			atFrame = 0;
			bPlayback = true;
		}
		break;
	case 'r':
		break;
	case 's':
		saveSkeleton();
		break;
	case 'x':
		bRotateX = true;
		break;
	case 'y':
		bRotateY = true;
		break;
	case 'z':
		bRotateZ = true;
		break;
	case OF_KEY_F1: 
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		theCam = &sideCam;
		break;
	case OF_KEY_F3:
		theCam = &topCam;
		break;
	case OF_KEY_ALT:
		bAltKeyDown = true;
		if (!mainCam.getMouseInputEnabled()) mainCam.enableMouseInput();
		break;
	case OF_KEY_DEL:
		deleteJoint();
		break;

	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	if (objSelected() && bDrag) {
		glm::vec3 point; 
		mouseToDragPlane(x, y, point);
		if (bRotateX) {
			selected[0]->rotation += glm::vec3((point.x - lastPoint.x) * 20.0, 0, 0);

		}
		else if (bRotateY) {
			selected[0]->rotation += glm::vec3(0, (point.x - lastPoint.x) * 20.0, 0);
		}
		else if (bRotateZ) {
			selected[0]->rotation += glm::vec3(0, 0, (point.x - lastPoint.x) * 20.0);
			
		}
		else {
			selected[0]->position += glm::vec3(point - lastPoint);
			
		}
		lastPoint = point;
	}

	if (!bDrag) return;
	glm::vec3 point;
	mouseToDragPlane(x, y, point);
	aimPos += point - lastPoint;
	lastPoint = point;
}

//  This projects the mouse point in screen space (x, y) to a 3D point on a plane
//  normal to the view axis of the camera passing through the point of the selected object.
//  If no object selected, the plane passing through the world origin is used.
//
bool ofApp::mouseToDragPlane(int x, int y, glm::vec3 &point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	float dist;
	glm::vec3 pos;
	if (objSelected()) {
		pos = selected[0]->position;
	}
	else pos = glm::vec3(0, 0, 0);
	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}
	if (bTargetSelected) {
		if (glm::intersectRayPlane(p, dn, aimPos, glm::normalize(theCam->getZAxis()), dist)) {
			point = p + dn * dist;
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------
//
// Provides functionality of single selection and if something is already selected,
// sets up state for translation/rotation of object using mouse.
//
void ofApp::mousePressed(int x, int y, int button) {
	bTargetSelected = false;
	// if we are moving the camera around, don't allow selection
	//
	if (mainCam.getMouseInputEnabled()) return;

	// clear selection list
	//
	selected.clear();

	//
	// test if something selected
	//
	vector<SceneObject*> hits;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of scene objects
	//
	for (int i = 0; i < scene.size(); i++) {

		glm::vec3 point, norm;

		//  We hit an object
		//
		if (scene[i]->isSelectable && scene[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(scene[i]);
		}
	}


	// if we selected more than one, pick nearest
	//
	SceneObject* selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = std::numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(hits[n]->position - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}
		}
	}
	if (selectedObj) {
		selected.push_back(selectedObj);
		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
	}
	else {
		selected.clear();
	}
	glm::vec3 point, normal;
	if (glm::intersectRaySphere(p, dn, aimPos, .2, point, normal)) {
		bTargetSelected = true;
		bDrag = true;
		lastPoint = aimPos;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	bDrag = false;

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

void ofApp::dragEvent(ofDragInfo dragInfo) {
	//
	//
	joints.clear();
	for (int i = 1; i < scene.size(); i++) {
		scene.pop_back();
	}
	//scene.clear();
	skeleFile = ofFile(dragInfo.files[0]);
	readObjFile(dragInfo.files[0]);
}


glm::vec3 ofApp::findRotateVector(string rotateV) {
	unsigned first = rotateV.find("<");
	unsigned last = rotateV.find_first_of(",");
	float rotateX = std::stof(rotateV.substr(first + 1, last - first - 1));

	first = rotateV.find_first_of(" ");
	last = rotateV.find_last_of(",");
	float rotateY = std::stof(rotateV.substr(first + 1, last - first - 1));

	first = rotateV.find_last_of(" ");
	last = rotateV.find_last_of(">");
	float rotateZ = std::stof(rotateV.substr(first + 1, last - first - 1));

	glm::vec3 rotate = glm::vec3(rotateX, rotateY, rotateZ);
	return rotate;
}

glm::vec3 ofApp::findTransVector(string transV) {
	unsigned first = transV.find("<");
	unsigned last = transV.find_first_of(",");
	float transX = std::stof(transV.substr(first + 1, last - first - 1));

	first = transV.find_first_of(" ");
	last = transV.find_last_of(",");
	float transY = std::stof(transV.substr(first + 1, last - first - 1));

	first = transV.find_last_of(" ");
	last = transV.find_last_of(">");
	float transZ = std::stof(transV.substr(first + 1, last - first - 1));

	glm::vec3 translate = glm::vec3(transX, transY, transZ);
	return translate;
}

void ofApp::createJointFromScript(string name, glm::vec3 r, glm::vec3 t, string parentName) {

	Joint* J = new Joint(t, 0.5, ofColor(rand() % 255, rand() % 255, rand() % 255), name);

	J->rotation = r;

	for (int i = 0; i < joints.size(); i++) {
		if (joints[i]->name == parentName) {
			J->parent = joints[i];
			break;
		}
	}

	joints.push_back(J);
	scene.push_back(J);

}

void ofApp::createRootJointFromScript(string name, glm::vec3 r, glm::vec3 t) {

	Joint* J = new Joint(t, 0.3, ofColor(rand() % 255, rand() % 255, rand() % 255), name);

	J->rotation = r;

	joints.push_back(J);
	scene.push_back(J);

}

void ofApp::readObjFile(string name) {
	string line;
	ofFile file;
	file = skeleFile;


	// Open file, return if error
	//
	if (!file.open(name)) {
		cout << "can't open file: " << name << endl;
		return;
	}
	// read each line in file and process 
	//
	while (getline(file, line)) {
		// "line" is a string that contains the current line in the file
		//  suggest printing it out first to make sure you are reading it properly
		//  then parse the string "line" to create vertices and faces in mesh (refer to wavefront .obj wiki for format) 


		unsigned first = line.find("-joint ");
		unsigned last = line.find(" -rotate");
		string name = line.substr(first + 7, last - first - 7);
		

		first = line.find("-rotate ");
		last = line.find(" -translate");
		string rotateV = line.substr(first + 8, last - first - 8);
		
		glm::vec3 rotate = findRotateVector(rotateV);
		

		if (line.find("parent") != std::string::npos) {
			first = line.find("-translate ");
			last = line.find(" -parent");
			string transV = line.substr(first + 11, last - first - 11);
			
			glm::vec3 translate = findTransVector(transV);

			first = line.find("-parent ");
			last = line.find(";");
			string parentN = line.substr(first + 8, last - first - 8);
			
			createJointFromScript(name, rotate, translate, parentN);
			
			//createJointFromScript();
		}
		else {
			first = line.find("-translate ");
			last = line.find(";");
			string transV = line.substr(first + 11, last - first - 11);

			glm::vec3 translate = findTransVector(transV);

			createRootJointFromScript(name, rotate, translate);
		}

	
	}

	file.close();
}

