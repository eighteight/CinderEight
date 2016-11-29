
#include "ofNode.h"
//#include "ofMath.h"
//#include "ofLog.h"
//#include "of3dGraphics.h"

using namespace cinder;
ofNode::ofNode() : 
	parent(NULL) {
	setPosition(Vec3f(0, 0, 0));
	setOrientation(Vec3f(0, 0, 0));
	setScale(1);
}

//----------------------------------------
void ofNode::setParent(ofNode& parent, bool bMaintainGlobalTransform) {
    if(bMaintainGlobalTransform) {
        cinder::Matrix44f globalTransform(getGlobalTransformMatrix());
        this->parent = &parent;
        setTransformMatrix(globalTransform);
    } else {
        this->parent = &parent;
    }
}

//----------------------------------------
void ofNode::clearParent(bool bMaintainGlobalTransform) {
    if(bMaintainGlobalTransform) {
        Matrix44f globalTransform(getGlobalTransformMatrix());
        this->parent = NULL;
        setTransformMatrix(globalTransform);
    } else {
        this->parent = NULL;
    }
}

//----------------------------------------
ofNode* ofNode::getParent() const {
	return parent;
}

//----------------------------------------
void ofNode::setTransformMatrix(const Matrix44f &m44) {
	localTransformMatrix = m44;

	Quatf so;
//	localTransformMatrix.decompose(position, orientation, scale, so);
	
	onPositionChanged();
	onOrientationChanged();
	onScaleChanged();
}

//----------------------------------------
void ofNode::setPosition(float px, float py, float pz) {
	setPosition(Vec3f(px, py, pz));
}

//----------------------------------------
void ofNode::setPosition(const Vec3f& p) {
	position = p;
	localTransformMatrix.setTranslate(position);
	onPositionChanged();
}

//----------------------------------------
void ofNode::setGlobalPosition(float px, float py, float pz) {
	setGlobalPosition(Vec3f(px, py, pz));
}

//----------------------------------------
void ofNode::setGlobalPosition(const Vec3f& p) {
	if(parent == NULL) {
		setPosition(p);
	} else {
//		setPosition(p * Matrix44f::getInverseOf(parent->getGlobalTransformMatrix()));
	}
}

//----------------------------------------
Vec3f ofNode::getPosition() const {
	return position;
}

//----------------------------------------
float ofNode::getX() const {
	return position.x;
}

//----------------------------------------
float ofNode::getY() const {
	return position.y;
}

//----------------------------------------
float ofNode::getZ() const {
	return position.z;
}

//----------------------------------------
void ofNode::setOrientation(const Quatf& q) {
	orientation = q;
	createMatrix();
	onOrientationChanged();
}

//----------------------------------------
void ofNode::setOrientation(const Vec3f& eulerAngles) {
//	setOrientation(Quatf(eulerAngles.x, Vec3f(1, 0, 0), eulerAngles.z, Vec3f(0, 0, 1), eulerAngles.y, Vec3f(0, 1, 0)));
    setOrientation(Quatf(eulerAngles.x, eulerAngles.y, eulerAngles.z));
}

//----------------------------------------
void ofNode::setGlobalOrientation(const Quatf& q) {
//	if(parent == NULL) {
//		setOrientation(q);
//	} else {
//		Matrix44f invParent(Matrix44::getInverseOf(parent->getGlobalTransformMatrix()));
//		Matrix44f m44(Matrix44f(q) * invParent);
//		setOrientation(m44.getRotate());
//	}
}

////----------------------------------------
//Quatf ofNode::getOrientationQuat() const {
//	return orientation;
//}
//
////----------------------------------------
Vec3f ofNode::getOrientationEuler() const {
    return Vec3f(0.f,0.f,0.f);//orientation.get());
}
//
////----------------------------------------
void ofNode::setScale(float s) {
	setScale(s, s, s);
}
//
////----------------------------------------
void ofNode::setScale(float sx, float sy, float sz) {
	setScale(Vec3f(sx, sy, sz));
}
//
////----------------------------------------
void ofNode::setScale(const Vec3f& s) {
	this->scale = s;
	createMatrix();
	onScaleChanged();
}
//
////----------------------------------------
//Vec3f ofNode::getScale() const {
//	return scale;
//}
//
////----------------------------------------
//void ofNode::move(float x, float y, float z) {
//	move(Vec3f(x, y, z));
//}
//
////----------------------------------------
//void ofNode::move(const Vec3f& offset) {
//	position += offset;
//	localTransformMatrix.setTranslate(position);
//	onPositionChanged();
//}
//
////----------------------------------------
//void ofNode::truck(float amount) {
//	move(getXAxis() * amount);
//}
//
////----------------------------------------
//void ofNode::boom(float amount) {
//	move(getYAxis() * amount);
//}
//
////----------------------------------------
//void ofNode::dolly(float amount) {
//	move(getZAxis() * amount);
//}
//
////----------------------------------------
//void ofNode::tilt(float degrees) {
//	rotate(degrees, getXAxis());
//}
//
////----------------------------------------
//void ofNode::pan(float degrees) {
//	rotate(degrees, getYAxis());
//}
//
////----------------------------------------
//void ofNode::roll(float degrees) {
//	rotate(degrees, getZAxis());
//}
//
////----------------------------------------
//void ofNode::rotate(const Quatf& q) {
//	orientation *= q;
//	createMatrix();
//}
//
////----------------------------------------
//void ofNode::rotate(float degrees, const Vec3f& v) {
//	//rotate(Quatf(degrees, v));
//}
//
////----------------------------------------
//void ofNode::rotate(float degrees, float vx, float vy, float vz) {
//	//rotate(Quatf(degrees, Vec3f(vx, vy, vz)));
//}
//
////----------------------------------------
//void ofNode::rotateAround(const Quatf& q, const Vec3f& point) {
//	//	ofLogVerbose("ofNode") << "rotateAround(const Quatf& q, const Vec3f& point) not implemented yet";
//	//	Matrix44 m = getLocalTransformMatrix();
//	//	m.setTranslation(point);
//	//	m.rotate(q);
//	
//	setGlobalPosition((getGlobalPosition() - point)* q + point); 
//	
////	onOrientationChanged();
//	onPositionChanged();
//}
//
////----------------------------------------
//void ofNode::rotateAround(float degrees, const Vec3f& axis, const Vec3f& point) {
////	rotateAround(Quatf(degrees, axis), point);
//}
//
////----------------------------------------
//void ofNode::lookAt(const Vec3f& lookAtPosition, Vec3f upVector) {
////	if(parent) upVector = upVector * Matrix44::getInverseOf(parent->getGlobalTransformMatrix());	
////	Vec3f zaxis = (getGlobalPosition() - lookAtPosition).normalized();	
////	if (zaxis.length() > 0) {
////		Vec3f xaxis = upVector.getCrossed(zaxis).normalized();	
////		Vec3f yaxis = zaxis.getCrossed(xaxis);
////		
////		Matrix44f m;
////		m._mat[0].set(xaxis.x, xaxis.y, xaxis.z, 0);
////		m._mat[1].set(yaxis.x, yaxis.y, yaxis.z, 0);
////		m._mat[2].set(zaxis.x, zaxis.y, zaxis.z, 0);
////		
////		setGlobalOrientation(m.getRotate());
////	}
//}
//
////----------------------------------------
//void ofNode::lookAt(const ofNode& lookAtNode, const Vec3f& upVector) {
//	lookAt(lookAtNode.getGlobalPosition(), upVector);
//}
//
////----------------------------------------
Vec3f ofNode::getXAxis() const {
	return axis[0];
}
//
////----------------------------------------
Vec3f ofNode::getYAxis() const {
	return axis[1];
}
//
////----------------------------------------
Vec3f ofNode::getZAxis() const {
	return axis[2];
}
//
////----------------------------------------
//Vec3f ofNode::getSideDir() const {
//	return getXAxis();
//}
//
////----------------------------------------
//Vec3f ofNode::getLookAtDir() const {
//	return -getZAxis();
//}
//
////----------------------------------------
//Vec3f ofNode::getUpDir() const {
//	return getYAxis();
//}
//
////----------------------------------------
//float ofNode::getPitch() const {
//	return getOrientationEuler().x;
//}
//
////----------------------------------------
//float ofNode::getHeading() const {
//	return getOrientationEuler().y;
//}
//
////----------------------------------------
//float ofNode::getRoll() const {
//	return getOrientationEuler().z;
//}
//
////----------------------------------------
//const Matrix44f& ofNode::getLocalTransformMatrix() const {
//	return localTransformMatrix;
//}
//
////----------------------------------------
//Matrix44f ofNode::getGlobalTransformMatrix() const {
//	if(parent) return getLocalTransformMatrix() * parent->getGlobalTransformMatrix();
//	else return getLocalTransformMatrix();
//}
//
////----------------------------------------
//Vec3f ofNode::getGlobalPosition() const {
//    return Vec3f(0);//getGlobalTransformMatrix().getTranslate();
//}
//
////----------------------------------------
//Quatf ofNode::getGlobalOrientation() const {
//	return Quatf(0);//getGlobalTransformMatrix().getRotate();
//}
//
////----------------------------------------
//Vec3f ofNode::getGlobalScale() const {
//	if(parent) return getScale()*parent->getGlobalScale();
//	else return getScale();
//}
//
////----------------------------------------
//void ofNode::orbit(float longitude, float latitude, float radius, const Vec3f& centerPoint) {
////	Matrix44f m;
////
////	// find position
////	Vec3f p(0, 0, radius);
////	p.rotate(ofClamp(latitude, -89, 89), Vec3f(1, 0, 0));
////	p.rotate(longitude, Vec3f(0, 1, 0));
////	p += centerPoint;
////	setPosition(p);
////	
////	lookAt(centerPoint);//, v - centerPoint);
//}
//
////----------------------------------------
//void ofNode::orbit(float longitude, float latitude, float radius, ofNode& centerNode) {
//	orbit(longitude, latitude, radius, centerNo`de.getGlobalPosition());
//}
//
////----------------------------------------
void ofNode::resetTransform() {
	setPosition(Vec3f());
	setOrientation(Vec3f());
}
//
////----------------------------------------
void ofNode::draw() {
	transformGL();
	customDraw();
	restoreTransformGL();
}
//
////----------------------------------------
void ofNode::customDraw() {
//	ofDrawBox(10);
//	ofDrawAxis(20);
}
//
////----------------------------------------
void ofNode::transformGL() const {
    gl::pushMatrices();
	//ofMultMatrix( getGlobalTransformMatrix() );
	//glMultMatrixf( getGlobalTransformMatrix().getPtr() );
}

//----------------------------------------
void ofNode::restoreTransformGL() const {
    gl::popMatrices();
}
//
////----------------------------------------
void ofNode::createMatrix() {
	//if(isMatrixDirty) {
	//	isMatrixDirty = false;
//	localTransformMatrix.makeScaleMatrix(scale);
//	localTransformMatrix.rotate(orientation);
//	localTransformMatrix.setTranslation(position);
//	
//	if(scale[0]>0) axis[0] = getLocalTransformMatrix().getRowAsVec3f(0)/scale[0];
//	if(scale[1]>0) axis[1] = getLocalTransformMatrix().getRowAsVec3f(1)/scale[1];
//	if(scale[2]>0) axis[2] = getLocalTransformMatrix().getRowAsVec3f(2)/scale[2];
}


