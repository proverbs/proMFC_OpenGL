#My OpenGL Homework
- 本项目为VS2015的项目，使用的是OpenGL3.3。
- 关于OpenGL3.3在vs2015上的配置，见我的博客：http://blog.csdn.net/qq379548839/article/details/52837874
- OpenGL3.3的参考资料：https://learnopengl-cn.github.io/
- 图片库使用FreeImage，目前还没有解决FreeImage静态编译的问题，所以需要使用freeimage的动态链接库。
- 模型加载使用Assimp，目前可以加载obj, stl, ply等格式的文件。更多支持的文件格式见http://assimp.sourceforge.net/main_features_formats.html

#使用说明
To play this program it is required to have the 2015 C++ Visual Redistributable installed for both x86 and x64. 

- vc_redist.x86.exe
- vc_redist.x64.exe


Due to some window-specific corner-cases the C++ redistributable installations might fail.
If so, check to do the following:

- If on Windows 7, make sure to have at least service pack 1 (SP1) installed in Windows update before installing the C++ redistributables.
- If on Windows 8, make sure to have at least the update KB2999226 installed before installing the C++ redistributables.
