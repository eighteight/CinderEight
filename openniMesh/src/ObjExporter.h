//
//  ObjExporter.h
//  openniMesh
//
//  Created by Gusev, Vladimir on 9/19/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef openniMesh_ObjExporter_h
#define openniMesh_ObjExporter_h

#include "ConcurrentQueue.h"
#include "cinder/ip/Flip.h"
#include "boost/unordered/unordered_map.hpp"
#include <fstream>

class Vertex{
public:
	Vertex(float x, float y, float z, float xT, float yT):x(x), y(y), z(z), xT(xT), yT(yT), i(0){};
	~Vertex(){};
	bool operator < (const Vertex &other) const {
		if (x != other.x) return x < other.x;
		if (y != other.y) return y < other.y;
		else return z < other.z;
	}
	bool operator == (const Vertex &other) const { return x == other.x && y == other.y && z == other.z; }
    float x, y, z, xT, yT;
    size_t i;
};

class VertexComp {
public:
	bool operator()(Vertex s1, Vertex s2) {
		return (! (s1.x == s2.x && s1.y==s2.y && s1.z == s2.z));
	}
};

class Face{
public:
	Face(Vertex a, Vertex b, Vertex c): a(a), b(b), c(c){};
	~Face(){};
	Vertex a,b,c;
};

struct VertexHash: std::unary_function<Vertex, std::size_t>
{
    std::size_t operator()(Vertex const& v) const
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, v.x);
        boost::hash_combine(seed, v.y);
        boost::hash_combine(seed, v.z);
        return seed;
    }
};

struct ObjData{
    ObjData(){};
    ObjData(XnPoint3D* d, XnPoint3D* d2, cinder::Surface t, cinder::Surface t2):d(d), d2(d2), t(t), t2(t2){};
    XnPoint3D* d, *d2;
    cinder::Surface t, t2;
};

typedef ph::ConcurrentQueue<ObjData> ExportQueue;
typedef std::shared_ptr<ExportQueue> ExportQueueRef;
class ObjExporter{
public:
    ObjExporter(std::string filePrefix, size_t sizeX, size_t sizeY):filePrefix(filePrefix), fileFrameCounter(0),sizeX(sizeX), sizeY(sizeY), size(sizeX*sizeY){
        averagedDepth = new XnPoint3D[size];
        averagedDepth2 = new XnPoint3D[size];
    };
    ObjExporter(std::string filePrefix, ExportQueueRef queue, size_t sizeX, size_t sizeY):filePrefix(filePrefix), queue(queue), fileFrameCounter(0),sizeX(sizeX), sizeY(sizeY), size(sizeX*sizeY){
        averagedDepth = new XnPoint3D[size];
        averagedDepth2 = new XnPoint3D[size];
    };

    ~ObjExporter(){
    	if (queue){
    		queue->empty();
    	}

    	if (averagedDepth) delete [] averagedDepth;
    	if (averagedDepth2) delete [] averagedDepth2;
    }


    void run(){
        
        ObjData data;
        
        while(true){
            if (queue->try_pop(data)){
                boost::mutex::scoped_lock lock(mMutex);
                exportDepthToObj(data.d, data.d2, data.t, data.t2);
                lock.unlock();
                mCondition.notify_one();
            }
        }
    }

private:
    ExportQueueRef queue;
    size_t sizeX, sizeY, size;
    std::string filePrefix;
    XnPoint3D*              averagedDepth, *averagedDepth2;
    size_t fileFrameCounter;
    std::vector<Face> faces, faces2;
    std::vector<XnPoint3D*> depthQueue, depthQueue2;
    std::vector<Vertex> indV, indV2;

    mutable boost::mutex		mMutex;
    std::condition_variable	mCondition;
    
    boost::unordered_map<Vertex, size_t, VertexHash> vrtxMap;
    boost::unordered_map<Vertex, size_t, VertexHash> vrtxMap2;
public:
    void exportDepthToObj(XnPoint3D* depth, XnPoint3D* depth2, cinder::Surface colorSurface, cinder::Surface colorSurface2){
        double start = cinder::app::getElapsedSeconds();
        
        computeFaces(faces, depth, averagedDepth, depthQueue);
        computeFaces(faces2, depth2, averagedDepth2, depthQueue2);

        std::string myPath = getHomeDirectory().string()+"eliot2/"+filePrefix+"."+getFileNameSuffix();
        std::cout << "Exporting OBJ: " << myPath << " "<<std::endl;
        std::ofstream oStream( (myPath+".obj").c_str() );	
        oStream << "# -----------------" << std::endl;
        oStream << "# Start of obj file" << std::endl;
        oStream << "g depth" << std::endl;
        
        oStream << "mtllib " + myPath + ".mat" << std::endl;
        
        computeAndWriteVertices(faces, vrtxMap, indV, oStream);
        computeAndWriteVertices(faces2, vrtxMap2, indV2, oStream);
        
        //	for (boost::unordered_map <Vertex, size_t, VertexComp>::iterator vrt = vrtxMap.begin(); vrt != vrtxMap.end(); vrt++) {
        //		oStream << "vt "<<vrt->first.xT << " " << vrt->first.yT << endl;
        //	}
        //	for (boost::unordered_map <Vertex, size_t, VertexComp>::iterator vrt = vertexMap2.begin(); vrt != vertexMap2.end(); vrt++) {
        //		oStream << "vt "<<vrt->first.xT << " " << vrt->first.yT << endl;
        //	}
        for (std::vector <Vertex>::iterator ind = indV.begin(); ind != indV.end(); ind++) {
            oStream << "vt "<<ind->xT << " " <<ind->yT << std::endl;
        }
        
        for (std::vector <Vertex>::iterator ind = indV.begin(); ind != indV.end(); ind++) {
            oStream << "vt "<<ind->xT << " " <<ind->yT << std::endl;
        }
        for (std::vector <Vertex>::iterator ind = indV2.begin(); ind != indV2.end(); ind++) {
            oStream << "vt "<<ind->xT << " " <<ind->yT << std::endl;
        }
        
        oStream << "usemtl rgb" << std::endl;
        for (std::vector <Face>::iterator f = faces.begin(); f != faces.end(); f++) {
            oStream << "f " << toObjFaceIndex(vrtxMap[f->a])  << " " << toObjFaceIndex(vrtxMap[f->b])<< " " << toObjFaceIndex(vrtxMap[f->c])<< std::endl;
        }
        oStream << "usemtl rgb2" << std::endl;
        for (std::vector <Face>::iterator f = faces2.begin(); f != faces2.end(); f++) {
            oStream << "f " << toObjFaceIndex(vrtxMap2[f->a]+vrtxMap.size())  << " " << toObjFaceIndex(vrtxMap2[f->b]+vrtxMap.size())<< " " << toObjFaceIndex(vrtxMap2[f->c]+vrtxMap.size())<< std::endl;
        }
        
        oStream << std::endl << std::endl;
        oStream.close();
        ///MTL FILE WRITE//////////////////////////////////////////////
        std::ofstream mtl((myPath+".mat").c_str() );
        mtl << "newmtl rgb" << std::endl;
        mtl << "Ka 1.000000 1.000000 1.000000" << std::endl;
        mtl << "Kd 1.000000 1.000000 1.000000" << std::endl;
        mtl << "Ks 0.000000 0.000000 0.000000" << std::endl;
        
        mtl << "illum 0" << std::endl;
        
        mtl << "map_Kd " + myPath + ".jpg" << std::endl;
        
        //        ///SECOND POINT CLOUD
        mtl << "newmtl rgb2" << std::endl;
        mtl << "Ka 1.000000 1.000000 1.000000" << std::endl;
        mtl << "Kd 1.000000 1.000000 1.000000" << std::endl;
        mtl << "Ks 0.000000 0.000000 0.000000" << std::endl;
        
        mtl << "illum 0" << std::endl;
        mtl << "map_Kd " + myPath + "_.jpg" << std::endl;
        mtl.close();
        
        
        ///SAVE RGB TEXTURE//////////////////////////////////////////////

        ip::flipVertical(&colorSurface);
        writeImage( fs::path(myPath+".jpg"), colorSurface, ImageTarget::Options().colorModel( ImageIo::CM_RGB ).quality( 1.0f ) );

        ip::flipVertical(&colorSurface2);
        writeImage( fs::path(myPath+"_.jpg"), colorSurface2, ImageTarget::Options().colorModel( ImageIo::CM_RGB ).quality( 1.0f ) );
        std::cout << "Export "<<fileFrameCounter<< " finished in " << (cinder::app::getElapsedSeconds() - start)<< " s. Accum: "<<depthQueue.size()<<std::endl;

        fileFrameCounter++;

    }

    void computeFaces(std::vector<Face> &faces, XnPoint3D* depthNew, XnPoint3D* avD, std::vector<XnPoint3D*>& depthQueue){
        computeDepthAverage(depthNew, avD, depthQueue);
        float maxDist = 2500.0;
        faces.clear();
        uint step = 2;
        for (size_t y = 0; y < sizeY - 1; y+=step) { // WRITE VERTICES which have faces
            for (size_t x = 0; x < sizeX - 1; x+=step) {
                size_t b = y * sizeX + x ;
                if (Vec3f(avD[b].X, avD[b].Y, avD[b].Z).lengthSquared() > 0.0) {
                    int a = b + step;
                    int c = (y + step) * sizeX + x;
                    int d = c + step;
                    Vec3f av(avD[a].X, avD[a].Y, avD[a].Z);
                    Vec3f bv(avD[b].X, avD[b].Y, avD[b].Z);
                    Vec3f cv(avD[c].X, avD[c].Y, avD[c].Z);
                    Vec3f dv(avD[d].X, avD[d].Y, avD[d].Z);
                    
                    float ab = av.distanceSquared(bv);
                    float bc = bv.distanceSquared(cv);
                    float ad = av.distanceSquared(dv);
                    float cd = cv.distanceSquared(dv);
                    
                    if (ab<maxDist && bc<maxDist && cd<maxDist&& ad<maxDist){
                        if (avD[a].Z > 0 && avD[b].Z > 0 && avD[c].Z > 0) {
                            Vertex v1(avD[a].X, avD[a].Y, avD[a].Z, float(x+step)/(float)sizeX, float(y)/(float)sizeY);
                            Vertex v2(avD[b].X, avD[b].Y, avD[b].Z, float(x)/(float)sizeX, float(y)/(float)sizeY);
                            Vertex v3(avD[c].X, avD[c].Y, avD[c].Z, float(x)/(float)sizeX, float(y+step)/(float)sizeY);
                            faces.push_back(Face(v1, v2, v3));
                        }
                        if (avD[a].Z > 0 && avD[d].Z > 0 && avD[c].Z > 0){
                            Vertex v1(avD[a].X, avD[a].Y, avD[a].Z, float(x+step)/(float)sizeX, float(y)/(float)sizeY);
                            Vertex v2(avD[c].X, avD[c].Y, avD[c].Z, float(x)/(float)sizeX, float(y+step)/(float)sizeY);
                            Vertex v3(avD[d].X, avD[d].Y, avD[d].Z, float(x+step)/(float)sizeX, float(y+step)/(float)sizeY);
                            faces.push_back(Face(v1, v2, v3));
                        }
                    }
                }
            }
        }
    }

    void computeAndWriteVertices(std::vector<Face> &faces, boost::unordered_map<Vertex, size_t, VertexHash>& vertexMap, std::vector<Vertex>&indV, std::ofstream& oStream){
        vertexMap.clear();
        indV.clear();
        size_t cn = 0;
        for (std::vector <Face>::iterator fc = faces.begin(); fc != faces.end(); fc++) {
            if (vertexMap.insert(std::make_pair(fc->a,0)).second){
                indV.push_back(fc->a);
                vertexMap[fc->a] = ++cn;
                oStream << "v "<<fc->a.x << " " << fc->a.y << " " << fc->a.z << std::endl;
            }
            
            if (vertexMap.insert(std::make_pair(fc->b,0)).second){
                indV.push_back(fc->b);
                vertexMap[fc->b] = ++cn;
                oStream << "v "<<fc->b.x << " " << fc->b.y << " " << fc->b.z << std::endl;
            }
            
            if (vertexMap.insert(std::make_pair(fc->c,0)).second){
                indV.push_back(fc->c);
                vertexMap[fc->c] = ++cn;
                oStream << "v "<<fc->c.x << " " << fc->c.y << " " << fc->c.z << std::endl;
            }
        }
    }
    void computeDepthAverage(XnPoint3D* depthNew, XnPoint3D* avD, std::vector<XnPoint3D*>& depthQueue ){
        depthQueue.push_back(depthNew);
        uint limit = 10;
        if (depthQueue.size()>limit){
           depthQueue.erase (depthQueue.begin()+0 );
        }
        XnPoint3D zr;
        zr.X=zr.Y=zr.Z = 0.0f;

        std::fill(avD, avD+size, zr);

        for (std::vector<XnPoint3D*>::iterator dpth = depthQueue.begin(); dpth != depthQueue.end(); dpth ++){
            for (size_t i = 0; i < size; i++){
                avD[i].X +=(*dpth)[i].X;
                avD[i].Y +=(*dpth)[i].Y;
                avD[i].Z +=(*dpth)[i].Z;
            }
        }
        float oneOverSize = 1.0/(float)depthQueue.size();
        for (size_t i = 0; i < size; i++){
            avD[i].X *= oneOverSize;
            avD[i].Y *= oneOverSize;
            avD[i].Z *= oneOverSize;
        }
    }
    std::string toObjFaceIndex(size_t ind){
        std::string temp = boost::lexical_cast<std::string>(ind);
        return temp + "/" +temp;// + "/"+temp;
    }
    std::string getFileNameSuffix(){
        if (fileFrameCounter<10)
            return "000000"+boost::lexical_cast<std::string>(fileFrameCounter);
        if (fileFrameCounter<100)
            return "00000"+boost::lexical_cast<std::string>(fileFrameCounter);
        if (fileFrameCounter<1000)
            return "0000"+boost::lexical_cast<std::string>(fileFrameCounter);
        if (fileFrameCounter<10000)
            return "000"+boost::lexical_cast<std::string>(fileFrameCounter);
        if (fileFrameCounter<100000)
            return "00"+boost::lexical_cast<std::string>(fileFrameCounter);
        if (fileFrameCounter<1000000)
            return "0"+boost::lexical_cast<std::string>(fileFrameCounter);
        return boost::lexical_cast<std::string>(fileFrameCounter);
    }
};

#endif
