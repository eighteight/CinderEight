//
//  Alembic.h
//  openniMesh
//
//  Created by Vladimir Gusev on 9/4/12.
//
//

#ifndef openniMesh_Alembic_h
#define openniMesh_Alembic_h

//-*****************************************************************************
//-*****************************************************************************
// EXAMPLE1 - INTRODUCTION
//
// Hello Alembic User! This is the first Example Usage file, and so we'll
// start by targeting the thing you'd most often want to do - write and read
// animated, geometric primitives. To do this, we will be using two main
// libraries: Alembic::Abc, which provides the basic Alembic Abstractions,
// and Alembic::AbcGeom, which implements specific Geometric primitives
// on top of Alembic::Abc.
//-*****************************************************************************
//-*****************************************************************************

//-*****************************************************************************
//-*****************************************************************************
// INCLUDES
//
// Each Library includes the entirety of its public self in a file named "All.h"
// file. So, you can typically just do include lines like the following.
//-*****************************************************************************
//-*****************************************************************************

// Alembic Includes
#include <Alembic/AbcGeom/All.h>
// This is required to tell Alembic which implementation to use.  In this case,
// the HDF5 implementation, currently the only one available.
#include <Alembic/AbcCoreHDF5/All.h>
#include "cinder/Rand.h"

// Other includes
#include <iostream>
#include <assert.h>

// We include some global mesh data to test with from an external source
// to keep this example code clean.
#include "MeshData.h"

//-*****************************************************************************
//-*****************************************************************************
// NAMESPACES
//
// Each library has a namespace which is the same as the name of the library.
// We shorten those here for brevity.
//-*****************************************************************************
//-*****************************************************************************

using namespace std;
using namespace Alembic::AbcGeom; // Contains Abc, AbcCoreAbstract

//-*****************************************************************************
//-*****************************************************************************
// WRITING OUT AN ANIMATED MESH
//
// Here we'll create an "Archive", which is Alembic's term for the actual
// file on disk containing all of the scene geometry. The Archive will contain
// a single animated Transform with a single static PolyMesh as its child.
//-*****************************************************************************
//-*****************************************************************************
void Example1_MeshOut()
{
    // Create an OArchive.
    // Like std::iostreams, we have a completely separate-but-parallel class
    // hierarchy for output and for input (OArchive, IArchive, and so on). This
    // maintains the important abstraction that Alembic is for storage,
    // representation, and archival (as opposed to being a dynamic scene
    // manipulation framework).
    OArchive archive(
                     
                     // The hard link to the implementation.
                     Alembic::AbcCoreHDF5::WriteArchive(),
                     
                     // The file name.
                     // Because we're an OArchive, this is creating (or clobbering)
                     // the archive with this filename.
                     "polyMesh1.abc" );
    ci::Rand _random;
    TimeSamplingPtr ts( new TimeSampling( 1.0 / 24.0, 0.0 ) );

    // Create a PolyMesh class.
    // An OPolyMesh is-an OObject that has-an OPolyMeshSchema
    // An OPolyMeshSchema is-an OCompoundProperty
        OPolyMesh meshyObj( OObject( archive, kTop ), "meshy", ts );
    OPolyMeshSchema &mesh = meshyObj.getSchema();
    
    // UVs and Normals use GeomParams, which can be written or read
    // as indexed or not, as you'd like.
    
    // The typed GeomParams have an inner Sample class that is used
    // for setting and getting data.
    OV2fGeomParam::Sample uvsamp( V2fArraySample( (const V2f *)g_uvs,
                                                 g_numUVs ),
                                 kFacevaryingScope );
    // indexed normals
    ON3fGeomParam::Sample nsamp( N3fArraySample( (const N3f *)g_normals,
                                                g_numNormals ),
                                kFacevaryingScope );
    
    // Set a mesh sample.
    // We're creating the sample inline here,
    // but we could create a static sample and leave it around,
    // only modifying the parts that have changed.
    
    // Alembic is strongly typed. P3fArraySample is for an array
    // of 32-bit points, which are the mesh vertices. g_verts etc.
    // are defined in MeshData.cpp.
         for (int i = 0; i<100; i++){
    OPolyMeshSchema::Sample mesh_samp(
                                      P3fArraySample( ( const V3f * )g_verts, g_numVerts ),
                                      Int32ArraySample( g_indices, g_numIndices ),
                                      Int32ArraySample( g_counts, g_numCounts ),
                                      uvsamp, nsamp );
    
    // not actually the right data; just making it up

        Box3d cbox;

        cbox.extendBy( V3d(_random.nextFloat(1.0), _random.nextFloat(-1.0), 0.0 ) );
        cbox.extendBy( V3d( -1.0, 1.0, 3.0 ) );
        mesh_samp.setChildBounds( cbox );
    
    // Set the sample twice.
    // Because the data is the same in both samples, Alembic will
    // store only one copy, but note that two samples have been set.
        mesh.set( mesh_samp );
    }
    //mesh.set( mesh_samp );
    
    
    // Alembic objects close themselves automatically when they go out
    // of scope. So - we don't have to do anything to finish
    // them off!
    std::cout << "Writing: " << archive.getName() << std::endl;
}

void meshUnderXformOut()
{
    OArchive archive( Alembic::AbcCoreHDF5::WriteArchive(), "polyMesh1.abc"  );
    
    TimeSamplingPtr ts( new TimeSampling( 1.0 / 24.0, 0.0 ) );
    
    OXform xfobj( archive.getTop(), "xf", ts );
    
    OPolyMesh meshobj( xfobj, "mesh", ts );
    
    OPolyMeshSchema::Sample mesh_samp(
                                      V3fArraySample( ( const V3f * )g_verts, g_numVerts ),
                                      Int32ArraySample( g_indices, g_numIndices ),
                                      Int32ArraySample( g_counts, g_numCounts ) );
    
    XformSample xf_samp;
    XformOp rotOp( kRotateYOperation );

    Box3d childBounds;
    childBounds.makeEmpty();
    childBounds.extendBy( V3d( 1.0, 1.0, 1.0 ) );
    childBounds.extendBy( V3d( -1.0, -1.0, -1.0 ) );
    
    xf_samp.setChildBounds( childBounds );
    
    double rotation = 0.0;
    
    for ( std::size_t i = 0 ; i < 100 ; ++i )
    {
        xf_samp.addOp( rotOp, rotation );
        xfobj.getSchema().set( xf_samp );
        meshobj.getSchema().set( mesh_samp );
        rotation += 30.0;
    }
}


#endif
