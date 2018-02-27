助教您好！
本文件夹下是基于Bezier曲面建模的渐进式光子映射大作业根目录。

images/目录下为最终生成的渲染效果图片；
obj/目录下为双三次Bezier曲面生成的obj文件；
src/目录下为本次实验的源代码，其中main函数引用的一些纹理文件，我已经打包放在textures/文件夹下，如果您需要测试代码，需要首先将此文件夹下的图片文件拷贝到代码文件夹src/Homework2/Homework2下；
	mesh/目录下为几种Object的头文件，包括Plane, Sphere, Double3Bezier；Mesh.h头文件集中include了这几个头文件；
	renderer/目录下为渲染器所在的头文件renderer.h，以及KDTree的实现文件utils.h。

编译环境为Win10 x64 + VS2015依赖于opencv3.1.0、Eigen3.
如果有任何问题，希望您能联系我，我的邮箱是zhouzp15@mails.tsinghua.edu.cn