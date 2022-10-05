#include <vector>
#include <iostream>
#include <cmath>
#include <limits>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "simple_gl.h"

// 设定模型
Model *model = NULL;
float *shadowbuffer = NULL;
// 设定zbuffer
int *zbuffer = NULL;
// 设定宽高深
const int width = 800;
const int height = 800;
// 各种方向
Vec3f light_dir(1, 1, 0);
Vec3f eye(1, 1, 4);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

// Shader
struct DepthShader : public IShader
{
    mat<3, 3, float> varying_tri;

    DepthShader() : varying_tri() {}

    virtual Vec4f vertex(int iface, int nthvert)
    {
        // 从OBJ文件中读取顶点数据
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        // 将其转换为屏幕坐标
        gl_Vertex = Viewport * Projection * ModelView * gl_Vertex;
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color)
    {
        Vec3f p = varying_tri * bar;
        color = TGAColor(255, 255, 255) * (p.z / depth);
        return false;
    }
};

struct Shader : public IShader
{
    // Projection*ModelView
    mat<4, 4, float> uniform_M;
    // (Projection*ModelView).invert_transpose()
    mat<4, 4, float> uniform_MIT;
    // 将帧缓冲区的屏幕坐标转换为阴影缓冲区的屏幕坐标
    mat<4, 4, float> uniform_Mshadow;
    // 三角形的uv坐标，由顶点着色器写入，由片段着色器读取
    mat<2, 3, float> varying_uv;
    // 视口变换前的三角坐标，由顶点着色器写入，由片段着色器读取
    mat<3, 3, float> varying_tri;

    Shader(Matrix M, Matrix MIT, Matrix MS) : uniform_M(M), uniform_MIT(MIT), uniform_Mshadow(MS), varying_uv(), varying_tri() {}

    virtual Vec4f vertex(int iface, int nthvert)
    {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = Viewport * Projection * ModelView * embed<4>(model->vert(iface, nthvert));
        varying_tri.set_col(nthvert, proj<3>(gl_Vertex / gl_Vertex[3]));
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor &color)
    {
        // 影子缓冲区中的相应点
        Vec4f sb_p = uniform_Mshadow * embed<4>(varying_tri * bar);
        sb_p = sb_p / sb_p[3];
        // 影子缓冲器阵列中的索引
        int idx = int(sb_p[0]) + int(sb_p[1]) * width;
        // float shadow = .3 + .7 * (shadowbuffer[idx] < sb_p[2]);
        float shadow = .3+.7*(shadowbuffer[idx]<sb_p[2]+43.34);
        // 为当前像素插值uv
        Vec2f uv = varying_uv * bar;
        // 法向量
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
        // 光向量
        Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();
        // 反射光向量
        Vec3f r = (n * (n * l * 2.f) - l).normalize();
        float spec = pow(std::max(r.z, 0.0f), model->specular(uv));
        float diff = std::max(0.f, n * l);
        TGAColor c = model->diffuse(uv);
        for (int i = 0; i < 3; i++)
            color[i] = std::min<float>(5 + c[i] * shadow * (1.5 * diff + 2 * spec), 255);
        // 我们不丢弃这个像素
        return false;
    }
};

// 主函数

int main(int argc, char **argv)
{
    // 导入模型
    if (2 > argc)
    {
        std::cerr << "Usage: " << argv[0] << "obj/model.obj" << std::endl;
        return 1;
    }

    // 初始化buffer
    float *zbuffer = new float[width * height];
    shadowbuffer = new float[width * height];
    for (int i = width * height; --i;)
    {
        zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max();
    }

    // 初始化模型
    model = new Model(argv[1]);
    // 标准化光向量
    light_dir.normalize();

    // 渲染Shadow-buffer
    {
        TGAImage depth(width, height, TGAImage::RGB);
        lookat(light_dir, center, up);
        viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
        projection(0);

        DepthShader depthshader;
        Vec4f screen_coords[3];
        for (int i = 0; i < model->nfaces(); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                screen_coords[j] = depthshader.vertex(i, j);
            }
            triangle(screen_coords, depthshader, depth, shadowbuffer);
        }
        depth.flip_vertically();
        depth.write_tga_file("depth.tga");
    }

    Matrix M = Viewport * Projection * ModelView;

    // 渲染frame-buffer
    {
        TGAImage frame(width, height, TGAImage::RGB);
        lookat(eye, center, up);
        viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
        projection(-1.f / (eye - center).norm());

        Shader shader(ModelView, (Projection * ModelView).invert_transpose(), M * (Viewport * Projection * ModelView).invert());
        Vec4f screen_coords[3];
        for (int i = 0; i < model->nfaces(); i++)
        {
            for (int j = 0; j < 3; j++)
            {
                screen_coords[j] = shader.vertex(i, j);
            }
            triangle(screen_coords, shader, frame, zbuffer);
        }
        frame.flip_vertically();
        frame.write_tga_file("framebuffer.tga");
    }

    delete model;
    delete[] zbuffer;
    delete[] shadowbuffer;
    return 0;
}