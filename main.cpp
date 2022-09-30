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
const int width = 1080;
const int height = 1080;
// 光照方向
Vec3f light_dir(1, 0, 1);
// 摄像机方向
Vec3f eye(5, 5, 5);
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

// 主函数
int main(int argc, char **argv)
{
	if (2 == argc)
	{
		model = new Model(argv[1]);
	}
	else
	{
		model = new Model("obj/bun_zipper/bun_zipper.obj");
	}

	// 初始化矩阵
	lookat(eye, center, up);
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	projection(-1.f / (eye - center).norm());

	// 标准化光向量
	light_dir.normalize();

	TGAImage image(width, height, TGAImage::RGB);
	TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

	GouraudShader shader;
	for (int i = 0; i < model->nfaces(); i++)
	{
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; j++)
		{
			screen_coords[j] = shader.vertex(i, j);
		}
		triangle(screen_coords, shader, image, zbuffer);
	}

	image.flip_vertically();
	zbuffer.flip_vertically();
	image.write_tga_file("output.tga");
	zbuffer.write_tga_file("zbuffer.tga");

	delete model;
	return 0;
}

// int main(int argc, char **argv)
// {
// 	// 导入模型
// 	if (2 == argc)
// 	{
// 		model = new Model(argv[1]);
// 	}
// 	else
// 	{
// 		model = new Model("obj/bun_zipper.obj");
// 	}
// 	// 设定“画板”大小
// 	TGAImage image(width, height, TGAImage::RGB);
// 	// 绘制
// 	// 遍历所有面
// 	for (int i = 0; i < model->nfaces(); i++)
// 	{
// 		std::vector<int> face = model->face(i);
// 		Vec2i screen_coords[3];
// 		Vec3f world_coords[3];
// 		// 遍历，将所有的顶点储存在screen_coords中，同时将三角形边的向量储存在world_coords中
// 		for (int j = 0; j < 3; j++)
// 		{
// 			Vec3f v = model->vert(face[j]);
// 			screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
// 			world_coords[j] = v;
// 		}
// 		// 计算每一个三角形的法向量
// 		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
// 		// 归一化法向量
// 		n.normalize();
// 		// 计算光线强度
// 		float intensity = n * light_dir;
// 		// 如果光线强度大于0
// 		if (intensity > 0)
// 		{
// 			// 依据光线强度，绘制填充三角形
// 			triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
// 		}
// 	}
// 	// 原点在图像的左下角
// 	image.flip_vertically();
// 	// 设置输出文件名
// 	image.write_tga_file("output.tga");
// }