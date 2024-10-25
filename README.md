# 计算机图形学（肯定是）

0. 本教程稍显麻烦的原因在于QtCreater创建时会生成一个build文件夹和CMakeLists.txt.user文件（个人设置文件），我们把它gitignore掉了，麻烦的部分在于怎么在本地重新创建这两个东西出来

   **每次提交时请检查，不要上传这两个文件！**

1. 打开QtCreater，它应该在你安装Qt的时候一起安装了。

2. 左上角创建新的项目，选择默认的第一个项目即可。![](README/20241023104531.png)

3. 创建过程中选择合适的创建路径，注意“构建套件“这步选择MSVC版（如出现黄色感叹号可以鼠标移动上去显示报错原因）![](README/20241023105327.png)

4. 创建完成后，另外新建一个文件夹，将[excalibare/CG_Qt_Ver](https://github.com/excalibare/CG_Qt_Ver)的code保存至本地，可以手动下载，不过个人推荐使用SourseTree（git图形化界面，百度即可获取）克隆

5. 将其中[excalibare/CG_Qt_Ver](https://github.com/excalibare/CG_Qt_Ver)的所有代码拷贝至QtCreater创建的项目中（会替换掉原有的文件）

6. 此时应该可以在QtCreater中运行，之后使用你喜欢的方法管理QtCreater创建项目的那个文件夹即可

7. 文件编码请选择UTF-8，打开项目时请选择对应的CMakes.txt打开

8. 顺便说一下，ctrl+i可以对选中代码格式化

## TODO list：

### 按钮（右侧空白区）：

1. 点击生成自定义矩形的按钮
2. 点击生成自定义⚪的按钮
3. 创建自定义图形（用下拉菜单选择）
4. 点击生成带箭头直线的按钮
5. 点击生成带箭头虚线的按钮
6. 在图形内/插入文字
7. 颜色选择（文字和图形）
8. （选做）插入图片

### 菜单栏（上方空白区）：

1. 撤销
2. 清空画布
3. 保存文件+读取文件

**先完成这些基本功能，之后再考虑美观与否**