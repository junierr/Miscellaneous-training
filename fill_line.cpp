#include <iostream>
#include <GL/glut.h>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include<time.h>
#include<windows.h>
using namespace std;


int hmin,hmax;                                 //记录扫描线开始和结束的位置


struct Line {                                   
    float dx,x,y,ym;                            
    Line(float x1,float yy1,float x2,float y2) {
        if(yy1==y2){                              //单独讨论横直线的情况
            this->y = yy1;
            this->ym = yy1;
            if(x1 < x2){
                dx = x1; x = x2;
            }else{
                dx =x2;x = x1;}
        }else if(y2<yy1){                        
            this -> x = x2;                    
            this ->y = y2;                     
            this -> ym = yy1;                  
        }else{
            this -> x = x1;
            this ->y = yy1;
            this -> ym = y2;
        }
        dx = (x2-x1)/(y2-yy1);
    }
};


typedef list<Line> TESTLIST;
vector<vector<Line>> con;    //有序表，类似优先队列
list<Line> AET;                    //滚动记录活动边表,这里将
                                         //该边表完整存储的意义不大所以采用滚动存储的方式
map<int, int> mapper;          //用于数据（y值）离散化处理
int x1,yy1,x2,y2;                     //描述构成直线的两个端点
int x0,yy0;                            //记录图形开始位置
float h_min,h_max;               //画线开始和结束的位置
int flag = 1;                         //用于记录用户点击的次数
int if_drawable = 1;              //当用户再次点击鼠标时不在更改信息
int window_size=600;           
vector<vector<Line>> con2;
int  level = 1;
/*
 使用鼠标【左键】进行绘制点，请您保证没有线是交叉的。
 点击鼠标【右键】绘制最后一个点。系统会自动将其与起始点相连。
 */



bool higher(const vector<Line> & l1, const vector<Line>& l2) {
    //将关键事件表中的line按照y值进行排序；
    //注意我们的画布是从上到下不断递增从左到右不断递增
    return l1[0].y < l2[0].y;//可以保证一定至少有一个不然map不会映射到
}


bool AET_lefter(const Line & l1, const Line & l2) {
    //将AET表中的line按照x值进行排序；
    return l1.x < l2.x;//可以保证一定至少有一个不然map不会映射到
}


bool lefter(const Line & l1, const Line & l2) {
    /*
     函数说明：将关键事件表中的line按照x值以及dx进行排序；
    */
    if(l1.x < l2.x){
        return 1;
    }else if (l1.x == l2.x){
        if(l1.dx<0&&l2.dx>0)
            return 1;
        else
            return 0;
    }else
        return 0;
}


void sort_con(){
    /*
     函数说明：对关键事件表进行排序处理
     其中y从小到大递增，x方向按照斜率和x大小由左到右排序
     */
    for (int i  = 0 ; i < con.size(); i++)
        if (con[i].size()>=2)
            sort(con[i].begin(),con[i].end(),lefter);
    for (int i = 0;i < con.size(); i++) {
        vector<Line> a;
        for (int j =0; j < con[i].size(); j++)
            a.push_back(con[i][j]);
        con2.push_back(a);                  //这里将事件表进行拷贝，另一种方式是将map的映射对应改变
    }
    sort(con.begin(), con.end(), higher);
}


void draw_lines(float x1,float yy1,float x2,float y2){
    glBegin(GL_LINES);
    glColor3f(1.0,1.0,0.0);
    glVertex2f(x1,window_size-yy1);
    glVertex2f(x2,window_size-y2);
    Sleep(3);
    glEnd();
    glFlush();
}



void lines_filling(){                           //真正的扫描线填充过程
    if (con.empty())                           //为了展示过程细节，部分功能没有使用函数ti
        return;
    int h_leveler = 0;                         //高度遍历器
    map<int,int>::iterator iter;             //定义一个迭代指针iter
    for(h_leveler = h_min;h_leveler <= h_max;h_leveler++){//开始扫描
        int id = mapper[h_leveler];
        if (!id) {                                 //说明没有到达关键节点，我们只需要进行绘制和更新即可；
            float xx = 0.0; flag = 1;        //flag用于控制每两组画一次线
            for(list<Line> ::iterator it=AET.begin();it!=AET.end();)
            {   if (flag%2==0) {            //偶次点画线
                    draw_lines(xx, h_leveler,it->x,h_leveler);
                    for (TESTLIST::iterator pl = AET.begin(); pl != AET.end();)
                    {
                        if (pl->ym == h_leveler)
                            AET.erase(pl);
                        pl++;               //这个负责删除的for循环在画线后执行可以避免留白情况
                    }
                        
                    it->x = it->x +it->dx;
                }
                else{
                    if (it->y != it->ym) {
                        xx =it->x;
                        it->x = it->x +it->dx;
                    }else
                    {
                        xx=x1;
                    }
                }
                flag++;
                it++;
            }
        }else{                                  //如果到了关键事件，那么加线、去线
            list<Line> ::iterator it;
            float xx = 0.0;int counter = 1;
            for(it=AET.begin();it!=AET.end();it++)
            {    Line temp= *it;
                if (counter%2==0)            
                    draw_lines(xx, h_leveler,temp.x,h_leveler);
                else
                    xx =temp.x;               
                counter++;
            }
            for (TESTLIST::iterator it = AET.begin(); it != AET.end();)
                if (it->ym == h_leveler)
                    AET.erase(it++);
                else
                    it++;                         //关键时间删除边
            for (int i =0 ; i < con2[id-1].size(); i++)
                if (con2[id-1][i].y == con2[id-1][i].ym)
                    continue;                   //如果是横线直接不用添加该横线
                else
                    AET.push_back(con2[id-1][i]);
            AET.sort(AET_lefter);          //维持滚动活动边表的有序性
        }}}


void InitEnvironment()                    
{   glClearColor(0.0,0.0,0.0,0);
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(7);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0,window_size,0,window_size);
}


void myDisplay(void)
{   glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
}

void OnMouse(int button,int state,int x,int y)
{if(button==GLUT_LEFT_BUTTON&&state==GLUT_DOWN&&if_drawable)//左键
    {if (flag ==1 &&if_drawable) {
            glColor3f(1,0,0);
            glBegin(GL_POINTS);
            glVertex2f(x,window_size-y);
            x0 = x;yy0 =y;
            x1 = x;yy1 = y;
            h_min = yy0;
            h_max = yy0;
            glEnd();
            glFlush();
            flag++;
        }else if(if_drawable){
            glColor3f(1,0,0);
            glBegin(GL_POINTS);
            glVertex2f(x,window_size-y);
            glEnd();
            x2 = x;y2 = y;
            glBegin(GL_LINES);
            glColor3f(1.0,0.0,0.0);
            glVertex2f(x1,window_size-yy1);
            glVertex2f(x2,window_size-y2);
            if (yy1 !=y2) {
                Line a(x1,yy1,x2,y2);
            int r_y = min (yy1,y2);
                if (yy1 < h_min)
                    h_min = yy1;
                if (y2 < h_min)
                    h_min = y2;
                if (yy1 > h_max)
                    h_max = yy1;
                if (y2 >h_max)
                    h_max = y2;
            int pos = mapper[r_y];
            if (pos==0) {           //说明该变量还没有离散化
                mapper[r_y] = level++;
                vector<Line> lines;
                lines.push_back(a);
                con.push_back(lines);}
            else
                con[pos-1].push_back(a);
            }
            x1 = x2; yy1 = y2;
            glEnd();
            glFlush();
        }
    }
    if(button==GLUT_RIGHT_BUTTON&&state==GLUT_DOWN&&if_drawable)
    {   //点击右键
        glColor3f(1,0,0);
        glBegin(GL_POINTS);
        glVertex2f(x,window_size-y);
        glEnd();
        x2 = x;y2 = y;
        glBegin(GL_LINES);
        glColor3f(1.0,0.0,0.0);
        glVertex2f(x1,window_size-yy1);
        glVertex2f(x2,window_size-y2);
        if (yy1 !=y2) {
            Line a(x1,yy1,x2,y2);
        int r_y = min (yy1,y2);
        if (yy1 < h_min)
            h_min = yy1;
        if (y2 < h_min)
            h_min = y2;
        if (yy1 > h_max)
            h_max = yy1;
        if (y2 >h_max)
            h_max = y2;
        int pos = mapper[r_y];
        if (pos==0) {           
            mapper[r_y] = level++;
            vector<Line> lines;
            lines.push_back(a);
            con.push_back(lines);}
        else
            con[pos-1].push_back(a);
        glEnd();
        glFlush();
        glBegin(GL_LINES);
        glColor3f(1.0,0.0,0.0);
        glVertex2f(x0,window_size-yy0);
        glVertex2f(x2,window_size-y2);
        glEnd();
        glFlush();
        Line aa(x0,yy0,x2,y2);
        r_y = min (yy0,y2);
        if (yy0 < h_min)
            h_min = yy0;
        if (y2 < h_min)
            h_min = y2;
        if (yy0 > h_max)
            h_max = yy0;
        if (y2 >h_max)
            h_max = y2;
        pos = mapper[r_y];
        if (pos==0) {        
            mapper[r_y] = level++;
            vector<Line> lines;
            lines.push_back(aa);
            con.push_back(lines);}
        else
            con[pos-1].push_back(aa);
        sort_con();
        lines_filling();
        if_drawable = 0;//鼠标点击结束
    }
}
}

int main(int argc, char *argv[])
{   
    glutInit(&argc, argv);   //初始化GLUT
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(400, 100);
    glutInitWindowSize(window_size, window_size);
    glutCreateWindow("filling_line");
    InitEnvironment();   //初始化
    glutMouseFunc(&OnMouse);  //注册鼠标事件
    glutDisplayFunc(&myDisplay);   //回调函数
    glutMainLoop();    //持续显示，当窗口改变会重新绘制图形
    return 0;
}
