#include <vector>
#include <iostream>
#include <cmath>
#include <limits>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "simple_gl.h"

// 定义颜色常量
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
// 设定模型
Model *model = NULL;
// 设定zbuffer
int *zbuffer = NULL;
// 设定宽高深
const int width = 400;
const int height = 400;
// 光照方向
Vec3f light_dir(1, 1, 1);
// 摄像机方向
Vec3f eye(0, 0, 10);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

// // 矩阵 to 向量
// Vec3f m2v(Matrix m)
// {
// 	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
// }

// // 向量 to 矩阵
// Matrix v2m(Vec3f v)
// {
// 	Matrix m(4, 1);
// 	m[0][0] = v.x;
// 	m[1][0] = v.y;
// 	m[2][0] = v.z;
// 	m[3][0] = 1.f;
// 	return m;
// }

struct GouraudShader : public IShader
{
	// 由顶点着色器写入，由片段着色器读取
	Vec3f varying_intensity;

	virtual Vec4f vertex(int iface, int nthvert)
	{
		// 从.obj文件中读取顶点
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
		// 将其转换为屏幕坐标
		gl_Vertex = Viewport * Projection * ModelView * gl_Vertex;
		// 获得漫反射照明强度
		varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir);
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor &color)
	{
		// 对当前像素进行强度插值
		float intensity = varying_intensity * bar;
		color = TGAColor(255, 255, 255) * intensity;
		// 我们不丢弃这个像素
		return false;
	}
};

struct Shader : public IShader
{
	// 由顶点着色器写入，由片段着色器读取
	Vec3f varying_intensity;
	mat<2, 3, float> varying_uv;

	virtual Vec4f vertex(int iface, int nthvert)
	{
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		// 获得漫反射照明强度
		varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_dir);
		// 从.obj文件中读取顶点
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
		// 将其转换为屏幕坐标
		return Viewport * Projection * ModelView * gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, TGAColor &color)
	{
		// 对当前像素进行强度插值
		float intensity = varying_intensity * bar;
		// 为当前像素插值uv
		Vec2f uv = varying_uv * bar;
		color = model->diffuse(uv) * intensity;
		// 我们不丢弃这个像素
		return false;
	}
};

// 主函数
int main(int argc, char **argv)
{
	if (2 == argc)
	{
		model = new Model(argv[1]);
	}
	else
	{
		model = new Model("obj/tinyobj/diablo3_pose.obj");
	}

	// 初始化矩阵
	lookat(eye, center, up);
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	projection(-1.f / (eye - center).norm());

	// 标准化光向量
	light_dir.normalize();

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

	Shader shader;
	for (int i = 0; i < model->nfaces(); i++)
	{
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; j++)
		{
			screen_coords[j] = shader.vertex(i, j);
		}
		triangle(screen_coords, shader, image, zbuffer);
	}

	image.flip_vertically(); // to place the origin in the bottom left corner of the image
	zbuffer.flip_vertically();
	image.write_tga_file("output.tga");
	zbuffer.write_tga_file("zbuffer.tga");

	delete model;
	return 0;
}