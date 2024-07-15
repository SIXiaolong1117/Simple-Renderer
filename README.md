# Simple-Renderer

[ssloy/tinyrenderer](https://github.com/ssloy/tinyrenderer/wiki)的学习成果，逐步完善中。

学习笔记可以在[这里](https://SIXiaolong1117.github.io/categories/TinyRenderer%E5%AD%A6%E4%B9%A0%E7%AC%94%E8%AE%B0/)找到。

## 🤖模型获取

您可以到tinyrenderer的存储库中下载[OBJ模型及贴图](https://github.com/ssloy/tinyrenderer/tree/master/obj)。

或者使用其他模型（[例如](https://www.cc.gatech.edu/projects/large_models/)），需要注意的是我们的程序目前只支持OBJ格式的模型。

## 🛠️构建

要构建此项目，您需要：

```sh
git clone https://github.com/SIXiaolong1117/Simple-Renderer.git &&
cd Simple-Renderer &&
mkdir build &&
cd build &&
cmake .. &&
cmake --build . -j
```

使用：

```sh
./Simple-Renderer <输入模型OBJ>
# 例如
./Simple-Renderer ../obj/diablo3_pose/diablo3_pose.obj
./Simple-Renderer ../obj/diablo3_pose/diablo3_pose.obj ../obj/floor.obj
```

## ⚖️License

部分代码源自[ssloy/tinyrenderer](https://github.com/ssloy/tinyrenderer)，遵循[ssloy/tinyrenderer - LICENSE.txt](https://github.com/ssloy/tinyrenderer/blob/master/LICENSE.txt)。

其余部分，依据[MIT License](./LICENSE)开源。