
#include <pcl/point_types.h>
#define nil Boost_nil
#define Nil Boost_Nil
//#include <pcl/io/pcd_io.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/ros/conversions.h>
#undef Nil
#undef nil


#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class PCLVoxelGridApp : public AppNative {
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
private:
    //sensor_msgs::PointCloud2::Ptr cloud;// (new sensor_msgs::PointCloud2 ());
        pcl::VoxelGrid<sensor_msgs::PointCloud2> sor;
};

void PCLVoxelGridApp::setup()
{

}

void PCLVoxelGridApp::mouseDown( MouseEvent event )
{
}

void PCLVoxelGridApp::update()
{
    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud <pcl::PointXYZ>);
    cloud->height = 480;
    cloud->width = 640;
    cloud->is_dense = false;
    cloud->points.resize (cloud->height * cloud->width);
    register int depth_idx = 0;
    for (int v = -240; v < 240; ++v)
    {
        for (register int u = -320; u < 320; ++u, ++depth_idx)
        {
            pcl::PointXYZ& pt = cloud->points[depth_idx];
            // Check for invalid measurements
//            if (depth_map[depth_idx] == 0 ||
//                depth_map[depth_idx] == depth_image->getNoSampleValue () ||
//                depth_map[depth_idx] == depth_image->getShadowValue ())
//            {
//                // not valid
//                pt.x = pt.y = pt.z = bad_point;
//                continue;
//            }
            float constant = 1.0f;
            pt.z = 0.0f;//depth_map[depth_idx] * 0.001f;
            pt.x = static_cast<float> (u) * pt.z * constant;
            pt.y = static_cast<float> (v) * pt.z * constant;
        }
    }
    
    //sensor_msgs::PointCloud2::Ptr cloud2 (new sensor_msgs::PointCloud2 ());
    sensor_msgs::PointCloud2 cloud2;
    pcl::toROSMsg(*cloud, cloud2);
    size_t cc = cloud2.width*cloud2.height;
    sensor_msgs::PointCloud2::Ptr cloud2Ptr (new sensor_msgs::PointCloud2(cloud2));
    sensor_msgs::PointCloud2::Ptr cloud_filtered (new sensor_msgs::PointCloud2 ());

    for (int d = 0; d < static_cast<int>(cloud2Ptr->fields.size ()); ++d)
    {
        std::cout<< cloud2Ptr->fields[d].name<<" "<<d<<endl;
    }
    pcl::VoxelGrid<sensor_msgs::PointCloud2> sor;
    try{
    sor.setInputCloud (cloud2Ptr);
    }catch (Exception e){
        cout<<e.what()<<endl;
    }
    sor.setLeafSize (0.01f, 0.01f, 0.01f);
    sor.filter (*cloud_filtered);
}

void PCLVoxelGridApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( PCLVoxelGridApp, RendererGl )
