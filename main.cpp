#include <vector>
#include<iostream>
#include <cmath>
#include <limits>
#include "rasterizer.h"
#include "Vex.h"
#include "shader.h"
#include<chrono>
using namespace std::chrono;
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255); //(R,G,B,A)
const TGAColor green = TGAColor(0, 255, 0, 255);
Model *model = NULL;
const int width  = 800;
const int height = 800;
Vector3f light_dir(-2, 3, 2); // define light_dir
Vector3f eye_pos(1, 1, 3);

int main(int argc, char** argv)
{
    auto start = steady_clock::now();

    


    Rasterizer rst;
    if (2 == argc)
    {
        model = new Model(argv[1]);
    }
    else
    {
        model = new Model("obj/african_head.obj");
    }
    
    TGAImage depth(width, height, TGAImage::RGB);
    DepthShader depthshader;
    depthshader.set_model_matrix('Z', -90);
    depthshader.set_view_matrix(light_dir);
    depthshader.set_projection_matrix(45, 1, 0.1, 50);
    depthshader.set_viewport_matrix(width, height);
    for (int i = 0; i < model->nfaces(); i++)
    {
        Vex vex;
        for (int j = 0; j < 3; j++)
        {
            Vector4f v = depthshader.vertex(i, j, vex);
    
          
            vex.screen_coords[j] = depthshader.get_viewport(v, width, height);
        }
        rst.triangle(vex, depth, model, depthshader);

    }
    depth.flip_vertically(); // to place the origin in the bottom left corner of the image
    depth.write_tga_file("depth.tga");

    light_dir.normalize();//ע�⣬����light_dirʵ������(light_dir-{0,0,0})����������շ�������Ҫ���ɵ�λ������
    TGAImage image(width, height, TGAImage::RGB);
    PhongShader shader;
    //�õ������ӽǵ����ͼ
    shader.shadowbuffer = depth.get_zbuffer();

    shader.set_model_matrix('Z', -90);
    shader.set_view_matrix(eye_pos);
    shader.set_projection_matrix(45, 1, 0.1, 50);
    shader.set_viewport_matrix(width, height);
    //�õ�ת������Դ�ӽǵ�  ת������
    shader.set_shadowMatrix(depthshader.get_MS());
    for (int i = 0; i < model->nfaces(); i++)
    {
        Vex vex;
        for (int j = 0; j < 3; j++)
        {
         
            Vector4f v = shader.vertex(i, j, vex);
            vex.screen_coords[j] = shader.get_viewport(v, width, height);
        }
        vex.set_TBN();
        Vector3f n = (vex.world_coords[1] - vex.world_coords[0]).cross(vex.world_coords[2] - vex.world_coords[0]);
        n.normalize();
        float backculling = n * light_dir;
        if (backculling>0)
        {
            rst.triangle(vex, image, model, shader);
        }
    }
    // ��¼����ʱ��
    auto end = steady_clock::now();
    // ����ʱ����
    auto duration = duration_cast<milliseconds>(end - start);
   std:: cout << "Elapsed time: " << duration.count() << " ms" << std::endl;
   
    image.flip_vertically();
    image.write_tga_file("framebuffer1.tga");
    delete model;
    return 0;
}