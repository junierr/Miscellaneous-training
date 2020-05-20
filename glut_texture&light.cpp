#include<gl/glut.h>//OpenGL实用工具库
#include<math.h>
#include <windows.h>
#include<stdlib.h>
#include<stdio.h>
#define PI 3.1415926535
#define BmpHeaderLength 54      //文件长度
#define WidthPosition  0x0012  //width值在文件中的位置
#define HeightPosition 0x0016   //height在文件中的位置

typedef struct{
    char *filename; //文件名
    FILE *fp;       //文件指针
    GLint width;    //宽
    GLint height;   //高
    GLubyte * pPixelData; //文件中 实际的像素信息
}FileDataInfo;  //文件数据信息

GLuint texGround;

FileDataInfo  LoadBMP(char *filename){
    FileDataInfo fileinfo;
    fileinfo.filename = filename;
    fileinfo.fp = fopen(fileinfo.filename,"rb");
    fseek(fileinfo.fp,WidthPosition,SEEK_SET);
    fread(&fileinfo.width,sizeof(fileinfo.width),1,fileinfo.fp);
    fseek(fileinfo.fp,HeightPosition,SEEK_SET);
    fread(&fileinfo.height,sizeof(fileinfo.height),1,fileinfo.fp);
    fseek(fileinfo.fp,BmpHeaderLength,SEEK_SET);
    
    int widthInBmpFile = fileinfo.width*3;
    //因为微软的bmp格式文件宽度 像素值 是 4 整数倍，不是的话，会在后面补上，
    //也就是说 （像素宽*3）*像素高 <= 文件大小-54； ---54是bmp文件头部大小。
    //所以在读取文件的时候需要调整内存大小，而不是 直接的(width*3)*height ;
    while(widthInBmpFile%4 != 0){
        widthInBmpFile++;
    }

    int pixelDataLen = widthInBmpFile *fileinfo.height;
    fileinfo.pPixelData = (GLubyte*)malloc(pixelDataLen);

    if(fileinfo.pPixelData == 0){
        printf("malloc() failed");
        free(fileinfo.pPixelData);
        exit(0);
    }
    fread(fileinfo.pPixelData,pixelDataLen,1,fileinfo.fp);
    fclose(fileinfo.fp);

    return fileinfo;
}


//释放资源
void destroy(FileDataInfo *p){
    free(p->pPixelData);
    fclose(p->fp);
}

int getTexture(FileDataInfo fileinfo){
    GLuint last_texture_ID,texture_ID=0;
    GLubyte * pTextureData;
    GLubyte * newData ;
    GLint max; 

    //OpenGL 低版本中，纹理大小不能超过 一定值。
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max);

    //为保证openGL向低版本的兼容性，超过了的话，需要将其缩小。
    if(fileinfo.width > max||fileinfo.height >max){
        //最大宽 和 高
        int maxWidth =256;
        int maxHeight = 256;
        int lines = (int)maxWidth*3;
        if(lines % 4 != 0){
            lines++;
        }
        int total = lines * maxHeight;
        newData = (GLubyte*)malloc(total);
        if(newData == 0){
            printf("newData=malloc() failed");
            return -1;
        }

        //缩小
        gluScaleImage(GL_RGB,fileinfo.width,fileinfo.height,GL_UNSIGNED_BYTE,fileinfo.pPixelData,maxWidth,maxHeight,GL_UNSIGNED_BYTE,newData);
        pTextureData = newData;
    }
    else{
        pTextureData = fileinfo.pPixelData;
    }
    //generate a texture ID
    glGenTextures(1,&texture_ID);
    if(texture_ID == 0){
        free(pTextureData);         
        printf("get textureID failed !");
        return -1;
    }

    glGetIntegerv(GL_TEXTURE_BINDING_2D,(GLint*)&last_texture_ID);
    glBindTexture(GL_TEXTURE_2D,texture_ID);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,fileinfo.width,fileinfo.height,0,GL_BGR_EXT,GL_UNSIGNED_BYTE,pTextureData);
    glBindTexture(GL_TEXTURE_2D,last_texture_ID);

return texture_ID;
}


static int spin = 0;
static int du=90,OriX=-1,OriY=-1;   //du是视点和x轴的夹角
static float r=1.5,h=0.0;   //r是视点绕y轴的半径，h是视点高度即在y轴上的坐标
static float c=PI/180.0;    //弧度和角度转换参数

GLfloat rote = 0.0;   //旋转角度
GLfloat anglex = 0.0; //X 轴旋转
GLfloat angley = 0.0; //Y 轴旋转
GLfloat anglez = 0.0; //Z 轴旋转
GLint WinW = 600;
GLint WinH = 600;
GLfloat oldx;
GLfloat oldy;
bool tr_w=false;
bool tr_a=false;
bool tr_s=false;
bool tr_d=false;
bool isoff = true;
bool isPos = true;
int ischange1 = 0;
bool isalp = false;
bool iswenli1 = false;

/* 点光源 */
GLfloat light0_pos[] = { 1.0, 1.0, 1.0, 0.0};//Position
GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };//color
GLfloat purple[] = { 1.0, 0, 1.0, 1.0 };//color


void key(unsigned char ch, int x, int y)
{
	switch (ch)
    {
    //改变材质
    case '0':{
        ischange1=0;
        break;
    }
    case '1':{
        ischange1=1;
        break;
    }
    case '2':{
        ischange1=2;
        break;
    }
    //控制透明
    case 'm':{
        isalp=!isalp;
        break;
    }
    //光照开关
    case 'p':{
        isoff=!isoff;
        break;
    }
    //光照左移
    case 'a': {
        light0_pos[0] -= 0.2f;
        break;
    }         
    //光照右移
    case 'd': {
        light0_pos[0] += 0.2f;
        break;
    }
    //光照上移
    case 'w': {
        light0_pos[1] += 0.2f;
        break;
    }
    //光照下移
    case 's': {
        light0_pos[1] -= 0.2f;
        break;
    }
    //光照前移
    case 'q': {
        light0_pos[2] += 0.2f;
        break;
    }
    //光照后移
    case 'e': {
        light0_pos[2] -= 0.2f;
        break;
    }
    case 'n': {
        iswenli1=!iswenli1;
        break;
    }
}
}

void init(void)
{
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_shininess[] = {50.0};
    GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat Light_Model_Ambient[] = {0.2, 0.2, 0.2, 1.0}; //
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Light_Model_Ambient); //
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
}

// /*实现窗口的重绘*/
void display(void)
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清除缓冲，GL_COLOR_BUFFER_BIT ：颜色缓冲标志位
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//控制视角移动
	glLoadIdentity();                                       //重置当前矩阵为4*4的单位矩阵
    gluLookAt(r*cos(c*du), h, r*sin(c*du), 0.0,0.0,0.0, 0.0,1.0,0.0);   //从视点看远点
    glColor3f(1.0,0.0,0.0);
    glutSolidTeapot(0.5f);

    //控制光源移动
    if(isoff){
        glDisable(GL_LIGHTING);
        glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
        glEnable(GL_LIGHTING);
    }
	else{
        glDisable(GL_LIGHTING);
        //glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
        //glEnable(GL_LIGHTING);
    }

    //材质变化
    if(ischange1==0){
        /* 黄铜 */
        GLfloat mat_ambient[] = { 0.329412, 0.223529, 0.027451, 1.000000};
        GLfloat mat_diffuse[] = { 0.780392, 0.568627, 0.113725, 1.000000};
        GLfloat mat_specular[] = { 0.992157, 0.941176, 0.807843, 1.000000};
        GLfloat mat_shininess[] = { 27.897400}; //材质RGBA镜面指数，数值在0～128范围内
        //GLfloat emission[4] = { 0.3, 0.2, 0.2, 1.0 };
        //glMaterialfv(GL_FRONT, GL_EMISSION, emission);
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    }
	else if(ischange1==1){
        /* 金 */
        GLfloat amb_Diff[4] = { 0, 0, 0, 0 };
        GLfloat specular[4] = { 1000, 100, 1, 1.0 };
        GLfloat shin[] = { 40 };
        glMaterialfv(GL_FRONT, GL_SPECULAR, specular);	//颜色
        glMaterialfv(GL_FRONT, GL_SHININESS, shin);	//镜面指数
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, amb_Diff);
    }
    else if(ischange1==2) {
        /* 珍珠 */
        GLfloat mat_ambient[] = {0.250000, 0.207250, 0.207250, 0.922000};
        GLfloat mat_diffuse[] = {1.000000, 0.829000, 0.829000, 0.922000};
        GLfloat mat_specular[] = {0.296648, 0.296648, 0.296648, 0.922000};
        GLfloat mat_shininess[] = {11.264000}; //材质RGBA镜面指数，数值在0～128范围内
        //GLfloat emission[4] = { 0.3, 0.2, 0.2, 1.0 };
        //glMaterialfv(GL_FRONT, GL_EMISSION, emission);
        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    }

    //透明显示
    if(isalp){
        glDisable(GL_DEPTH_TEST);
        glColor4f(1.0f, 1.0f, 1.0f, 0.3f);//颜色0.5 alpha值
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE);//混合函数
	    glEnable(GL_BLEND);//启用透明，注意不要开启深度测试,即不要有glEnable(GL_DEPTH_TEST)
    }
    else{
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    if(iswenli1){
        FileDataInfo groundFileInfo = LoadBMP("teapot.bmp");
        texGround = getTexture(groundFileInfo);
        glDisable(GL_TEXTURE_2D);
        
        glBindTexture(GL_TEXTURE_2D,texGround);
        // glTexCoord2f(0.0f,0.0f);    glVertex3f(-8.0f,-8.0f,0.0f);
        // glTexCoord2f(0.0,5.0f);     glVertex3f(-8.0f,8.0f,0.0f);
        // glTexCoord2f(5.0f,5.0f);    glVertex3f(8.0f,8.0f,0.0f);
        // glTexCoord2f(5.0f,0.0f);    glVertex3f(8.0f,-8.0f,0.0f);
        glEnable(GL_TEXTURE_2D);
        destroy(&groundFileInfo);
    }
    else{
        glDisable(GL_TEXTURE_2D);
    }

    glutSwapBuffers();                                      //交换两个缓冲区指针
	glFlush();
}

// /*负责在每次窗口移动或改变大小时重新定义视点和投影变换的方式*/
void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (w <= h)
        glOrtho(-1.5, 1.5, -1.5 * (GLfloat)h / (GLfloat)w, 1.5 * (GLfloat)h / (GLfloat)w, -10.0, 10.0);
    else
        glOrtho(-1.5 * (GLfloat)w / (GLfloat)h, 1.5 * (GLfloat)w / (GLfloat)h, -1.5, 1.5, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void onMouseMove(int x,int y)   //处理鼠标拖动
{
    du += x - OriX; //鼠标在窗口x轴方向上的增量加到视点与x轴的夹角上，就可以左右转
    h +=0.03*(y-OriY);  //鼠标在窗口y轴方向上的改变加到视点y的坐标上，就可以上下转
    if(h>1.0)   h=1.0;  //对视点y坐标作一些限制，不会使视点太奇怪
    else if(h<-1.0) h=-1.0;
    OriX = x,OriY = y;  //将此时的坐标作为旧值，为下一次计算增量做准备
}


void mouse(int button, int state, int x, int y)
{
	if(state == GLUT_DOWN)  //记录鼠标按下位置
        OriX = x,OriY = y;
	switch(button)
	{
	case GLUT_LEFT_BUTTON:
		if( state == GLUT_DOWN )
		{
			spin = (spin+30)%360;
			glutPostRedisplay();
		}
		break;
	default:
		break;
	}
}


int main(int argc, char **argv)
{
	glutInit(&argc, argv);//对GLUT库进行初始化
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);//设定显示模式
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(500, 500);
	glutCreateWindow(argv[0]);//创建该窗口


	init();
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(display);          //设置不断调用显示函数
	glutMouseFunc(mouse);
	glutMotionFunc(onMouseMove);
    glutKeyboardFunc(key);
	glutMainLoop();//启动GLUT的主事件循环, 程序结束前负责处理所有的GLUT事件
	return 0;
}

