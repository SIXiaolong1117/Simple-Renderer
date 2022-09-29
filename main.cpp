#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

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
const int width = 800;
const int height = 800;
const int depth  = 255;
// 光照方向
Vec3f light_dir(0, 0, -1);
// 摄像机方向
Vec3f camera(0, 0, 3);

// 矩阵 to 向量
Vec3f m2v(Matrix m)
{
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

// 向量 to 矩阵
Matrix v2m(Vec3f v)
{
	Matrix m(4, 1);
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}

// 视口矩阵
Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}

// 画线算法
void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color)
{
	// 是否转置（记得转回去）
	bool steep = false;
	// 如果图像“陡峭”，对图像进行转置
	if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y))
	{
		std::swap(p0.x, p0.y);
		std::swap(p1.x, p1.y);
		steep = true;
	}
	// 当 x0 > x1，对二者进行交换
	if (p0.x > p1.x)
	{
		std::swap(p0, p1);
	}
	// 从循环中取出。注意这里用的是int类型，然后通过一个derror和error来实现浮点数。
	int dx = p1.x - p0.x;
	int dy = p1.y - p0.y;
	int derror2 = std::abs(dy) * 2;
	int error2 = 0;
	int y = p0.y;
	// 一样的遍历算法，不多赘述
	for (int x = p0.x; x <= p1.x; x++)
	{
		if (steep)
		{
			// 如果转置了，记得转置回来（其实就是x和y反着set）
			image.set(y, x, color);
		}
		else
		{
			image.set(x, y, color);
		}
		error2 += derror2;
		if (error2 > dx)
		{
			y += (p1.y > p0.y ? 1 : -1);
			error2 -= dx * 2;
		}
	}
}

// 三角形绘制算法（扫线）
void triangle(Vec3i t0, Vec3i t1, Vec3i t2, TGAImage &image, float intensity, int *zbuffer)
{
	// 将顶点t0、t1、t2从低到高排序
	if (t0.y == t1.y && t0.y == t2.y)
		// 不关注退化三角形
		return;
	if (t0.y > t1.y)
	{
		std::swap(t0, t1);
	}
	if (t0.y > t2.y)
	{
		std::swap(t0, t2);
	}
	if (t1.y > t2.y)
	{
		std::swap(t1, t2);
	}
	// 总高度
	int total_height = t2.y - t0.y;
	// 遍历整个三角形
	for (int i = 0; i < total_height; i++)
	{
		// 确定现在是上下哪部分
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		// 不同的部分用不同的公式计算局部高度
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		//小心除以0
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
		Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
		Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;
		if (A.x > B.x)
		{
			std::swap(A, B);
		}
		for (int j = A.x; j <= B.x; j++)
		{
			float phi = B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x);
			Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;
			int idx = P.x + P.y * width;
			if (zbuffer[idx] < P.z)
			{
				zbuffer[idx] = P.z;
				image.set(P.x, P.y, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
			}
		}
	}
}

// // 重心坐标
// Vec3f barycentric(Vec2i *pts, Vec2i P)
// {
// 	// 计算叉积
// 	Vec3f u = cross(Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]), Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1]));
// 	// `pts`和`P`有整数值作为坐标，所以`abs(u[2])`<1意味着`u[2]`是0，这意味着三角形是退化的，在这种情况下，返回负坐标的东西。
// 	if (std::abs(u.z) < 1)
// 		return Vec3f(-1, 1, 1);
// 	// 返回重心坐标
// 	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
// }

// // 三角形光栅化（重心坐标）
// void triangle(Vec2i *pts, TGAImage &image, TGAColor color)
// {
// 	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
// 	Vec2i bboxmax(0, 0);
// 	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
// 	// 迭代三角形，寻找最小/最大坐标
// 	for (int i = 0; i < 3; i++)
// 	{
// 		bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
// 		bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));
// 		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
// 		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
// 	}
// 	Vec2i P;
// 	// 遍历，找出三角形内的点。
// 	// 遍历x
// 	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
// 	{
// 		// 遍历y
// 		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
// 		{
// 			// 重心坐标
// 			Vec3f bc_screen = barycentric(pts, P);
// 			// 是否在边界内
// 			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
// 				continue;
// 			// 绘制像素
// 			image.set(P.x, P.y, color);
// 		}
// 	}
// }

// // 二维光栅化（ybuffer）
// void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int ybuffer[])
// {
// 	if (p0.x > p1.x)
// 	{
// 		std::swap(p0, p1);
// 	}
// 	// 遍历
// 	for (int x = p0.x; x <= p1.x; x++)
// 	{
// 		float t = (x - p0.x) / (float)(p1.x - p0.x);
// 		int y = p0.y * (1. - t) + p1.y * t;
// 		// 判断ybuffer的大小，如果小于当前y
// 		if (ybuffer[x] < y)
// 		{
// 			// 当前y设为ybuffer
// 			ybuffer[x] = y;
// 			// 绘制像素
// 			image.set(x, 0, color);
// 		}
// 		// 否则，不绘制
// 	}
// }

// 主函数
int main(int argc, char **argv)
{
	if (2 == argc)
	{
		model = new Model(argv[1]);
	}
	else
	{
		model = new Model("obj/bun_zipper_res4.obj");
	}

	zbuffer = new int[width * height];
	for (int i = 0; i < width * height; i++)
	{
		zbuffer[i] = std::numeric_limits<int>::min();
	}

	{ // draw the model
		Matrix Projection = Matrix::identity(4);
		Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		Projection[3][2] = -1.f / camera.z;

		TGAImage image(width, height, TGAImage::RGB);
		for (int i = 0; i < model->nfaces(); i++)
		{
			std::vector<int> face = model->face(i);
			Vec3i screen_coords[3];
			Vec3f world_coords[3];
			for (int j = 0; j < 3; j++)
			{
				Vec3f v = model->vert(face[j]);
				screen_coords[j] = m2v(ViewPort * Projection * v2m(v));
				world_coords[j] = v;
			}
			Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
			n.normalize();
			float intensity = n * light_dir;
			if (intensity > 0)
			{
				triangle(screen_coords[0], screen_coords[1], screen_coords[2], image, intensity, zbuffer);
			}
		}

		image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
		image.write_tga_file("output.tga");
	}

	// { // dump z-buffer (debugging purposes only)
	// 	TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
	// 	for (int i = 0; i < width; i++)
	// 	{
	// 		for (int j = 0; j < height; j++)
	// 		{
	// 			zbimage.set(i, j, TGAColor(zbuffer[i + j * width], 1));
	// 		}
	// 	}
	// 	zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	// 	zbimage.write_tga_file("zbuffer.tga");
	// }
	delete model;
	delete[] zbuffer;
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
