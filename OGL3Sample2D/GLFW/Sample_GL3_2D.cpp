#include <iostream>
#include <bits/stdc++.h>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
#define SPACEBAR 32
struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;
typedef struct Point Point;
struct Point
{
    float x;
    float y;
    bool va;
};
struct fuse {
    VAO *vv;
    float c1;
    float c2;
    float high;
    float wid;
    float angle;
    float translate;
    float sy;
    int id;
    Point p1;
    Point p2;
};
typedef struct fuse fuse;
fuse bricks[100];
fuse bullets[100];
Point inter;
int fbwidth,fbheight;
int over=0;int zoom=0;int pan=0;
VAO *box1_circle,*box2_circle,*gun_semicircle;
int score=0;
struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;
int flagg[100]={0};
int flagforbullet[100]={0};
fuse box2_rec,box1_rec,gun_rec1;
fuse mirror[4];
float speed=0.0055f;
bool onSegment(Point p, Point q, Point r)
{
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
       return true;
 
    return false;
}
Point lineIntersection(Point p1,Point p2,Point p4,Point p5){
float Ax; float Ay;
float Bx; float By;
float Cx; float Cy;
float Dx; float Dy;
float X; float Y;
Point p3;

 Ax=p1.x; Ay=p1.y; Bx=p2.x; By=p2.y;
 Cx=p4.x; Cy=p4.y; Dx=p5.x; Dy=p5.y;
  float  distAB, theCos, theSin, newX, ABpos ;
// cout<<Ax<<" "<<Ay<<" "<<Bx<<" "<<By<<" "<<Cx<<" "<<Cy<<" "<<Dx<<" "<<Dy<<" ";
  //  Fail if either line is undefined.
  if (Ax==Bx && Ay==By || Cx==Dx && Cy==Dy){p3.va=false; return p3;}

  //  (1) Translate the system so that point A is on the origin.
  Bx-=Ax; By-=Ay;
  Cx-=Ax; Cy-=Ay;
  Dx-=Ax; Dy-=Ay;

  //  Discover the length of segment A-B.
  distAB=sqrt(Bx*Bx+By*By);

  //  (2) Rotate the system so that point B is on the positive X axis.
  theCos=Bx/(distAB*1.000f);
  theSin=By/(distAB*1.000f);
  newX=Cx*theCos+Cy*theSin;
  Cy  =Cy*theCos-Cx*theSin; Cx=newX;
  newX=Dx*theCos+Dy*theSin;
  Dy  =Dy*theCos-Dx*theSin; Dx=newX;

  //  Fail if the lines are parallel.
  if (Cy==Dy){ p3.va=false;return p3;}

  //  (3) Discover the position of the intersection point along line A-B.
  ABpos=Dx+(Cx-Dx)*Dy/((Dy-Cy)*1.000f);

  //  (4) Apply the discovered position to line A-B in the original coordinate system.
  X=Ax+ABpos*theCos;
  Y=Ay+ABpos*theSin;
  // cout<<X<<" "<<Y<<endl;
  p3.x=X;
  p3.y=Y;
  p3.va=true;

  //  Success.
  return p3;
   } 
 float x_intersection,y_intersection;
int intersect_point(Point p1,Point p2,Point p4,Point p5){
  float x0=p1.x, y0=p1.y,x1=p2.x, y1=p2.y;int i;
// Point p3;
  
    float s1_x, s1_y, s2_x, s2_y, x2=p4.x, y2=p4.y, x3=p5.x, y3=p5.y, q, p, r;
    
    s1_x = x1 - x0;
    s1_y = y1 - y0;
    s2_x = x3 - x2;     
    s2_y = y3 - y2;
    
    r=s1_x*s2_y - s2_x*s1_y;
    if(r==0){
      // p3.va=0;
      return 0;
    }

    p = (s1_x*(y0-y2) - s1_y*(x0-x2))/(r*1.0f);
    q = (s2_x*(y0-y2) - s2_y*(x0-x2))/(r*1.0f);

    if (p>=0 && p<=1 && q>=0 && q<=1)
    {
        x_intersection = x0 + (q * s1_x);
        y_intersection = y0 + (q * s1_y);
        // p3.va=1;
        return 1;
    }
    // p3.va=0;
    return 0;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
VAO *uy;
int orientation(Point p, Point q, Point r)
{
    // See http://www.geeksforgeeks.org/orientation-3-ordered-points/
    // for details of below formula.
    int val = (q.y - p.y) * (r.x - q.x) -
              (q.x - p.x) * (r.y - q.y);
 
    if (val == 0) return 0;  // colinear
 
    return (val > 0)? 1: 2; // clock or counterclock wise
}
 
// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool doIntersect(Point p1, Point q1, Point p2, Point q2)
{
    // Find the four orientations needed for general and
    // special cases
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);
 
    // General case
    if (o1 != o2 && o3 != o4)
        return true;
 
    // Special Cases
    // p1, q1 and p2 are colinear and p2 lies on segment p1q1
    if (o1 == 0 && onSegment(p1, p2, q1)) return true;
 
    // p1, q1 and p2 are colinear and q2 lies on segment p1q1
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;
 
    // p2, q2 and p1 are colinear and p1 lies on segment p2q2
    if (o3 == 0 && onSegment(p2, p1, q2)) return true;
 
     // p2, q2 and q1 are colinear and q1 lies on segment p2q2
    if (o4 == 0 && onSegment(p2, q1, q2)) return true;
 
    return false; // Doesn't fall in any of the above cases
}
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    //    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
            0,                  // attribute 0. Vertices
            3,                  // size (x,y,z)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
            1,                  // attribute 1. Color
            3,                  // size (r,g,b)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    return vao;
}







/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for(int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float circle_rot_dir = 1;
bool triangle_rot_status = true;
bool circle_rot_status = true;
int keys[12]={0};
float box1_rectangle_key_translate=0.0f;
float box2_rectangle_key_translate=0.0f;
float gun_translate=0.0f;
float gun_rotation=0.0f;
int bullet_number=-1;
fuse line;
float translate_bullet[100]={0};
double timme = glfwGetTime(), e_time;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
VAO *createBullet(float x,float y,float h,float w)
{
    VAO *rt;
    // float x1=x+(w/2);float y1=y+(h/2);float x2=x+(w/2);float y2=y-(h/2);float x3=x-(w/2);float y3=y-(h/2);float x4=x-(w/2);float y4=y+(h/2);
    static GLfloat vertex_buffer_data[10];
    vertex_buffer_data[0]=x;vertex_buffer_data[1]=y;vertex_buffer_data[2]=0.0f;
    vertex_buffer_data[3]=x;vertex_buffer_data[4]=y;vertex_buffer_data[5]=0.0f;
    vertex_buffer_data[6]=h;vertex_buffer_data[7]=w;vertex_buffer_data[8]=0.0f;
    static const GLfloat color_buffer_data[]={
        0,0,0, // color 1
        0,0,0, // color 1
        0,0,0, // color 1
    };
    rt = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
    return rt;
}
int bullet_maker_flag=0;

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_PRESS) {
      
        
            
                switch (key) {
            case GLFW_KEY_UP:
                if(zoom<4){
                zoom++;
                Matrices.projection = glm::ortho(-4.0f+zoom, 4.0f-zoom, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);}
                break;
            case GLFW_KEY_DOWN:
                if(zoom>0){
                zoom--;
                pan=0;
                Matrices.projection = glm::ortho(-4.0f+zoom, 4.0f-zoom, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);
                }
                break;
            case GLFW_KEY_LEFT:
                if(zoom>0){
                if(pan<zoom)
                   pan++;
                Matrices.projection = glm::ortho(-4.0f+zoom-pan, 4.0f-zoom-pan, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);
                }
                break;
            case GLFW_KEY_RIGHT:
                if(zoom>0){
                if(pan>-zoom)
                pan--;
                Matrices.projection = glm::ortho(-4.0f+zoom-pan, 4.0f-zoom-pan, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);
                 }
                break;
            case GLFW_KEY_N:
                if(speed<0.1f)
                  speed+=0.0010f;
                break;
            case GLFW_KEY_M:
                if(speed>0.0025f)
                  speed-=0.0010f;
                break;
            case GLFW_KEY_S:
                keys[4]=1;
                break;
            case GLFW_KEY_F:
                keys[5]=1;
                break;
            case GLFW_KEY_A:
                keys[7]=1;
                break;
            case GLFW_KEY_D:
                keys[6]=1;
                break;
            case SPACEBAR:
                 e_time = glfwGetTime(); // Time in seconds
                 if ((e_time - timme) <= 1.0) {
                 break;
                 } 
                 timme=e_time;
                 bullet_number++;
                // cout<<bullet_number<<endl;
                if(bullet_number>50){
                  bullet_number=0;
                  bullet_maker_flag=1;
                  for(int rr=0;rr<=50;rr++)
                    {
                      translate_bullet[rr]=0.0f;
                      flagforbullet[rr]=0;
                    }
                }
                flagforbullet[bullet_number]=1;
                bullets[bullet_number].angle=(gun_rotation*(M_PI/180.0));
                bullets[bullet_number].translate=gun_translate;
                bullets[bullet_number].sy=-3.80f;
                if(bullet_maker_flag==0)
                bullets[bullet_number].vv=createBullet(0.0f,0.0f,0.3f,0.0f);
                break;

            default:
                break;
        }
        if(key==GLFW_KEY_LEFT && mods==GLFW_MOD_ALT)
               {   if(box1_rectangle_key_translate>-2.5)
                    keys[0]=1;  
                  }
            if(key==GLFW_KEY_RIGHT && mods==GLFW_MOD_ALT){
                if(box1_rectangle_key_translate<5.5)
                keys[1]=1;
            }
            if(key==GLFW_KEY_LEFT && mods==GLFW_MOD_CONTROL)
             { if(box2_rectangle_key_translate>-5.5)
                keys[2]=1;
              }
            if(key==GLFW_KEY_RIGHT && mods==GLFW_MOD_CONTROL)
               { if(box2_rectangle_key_translate<2.5)
                keys[3]=1;
              }
                
    }
     else if (action == GLFW_RELEASE) {
           
        switch (key) {
            case GLFW_KEY_S:
                keys[4]=0;
                break;
            case GLFW_KEY_F:
                keys[5]=0;
                break;
            case GLFW_KEY_A:
                keys[7]=0;
                break;
            case GLFW_KEY_D:
                keys[6]=0;
                break;
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
         if(key==GLFW_KEY_LEFT && mods==GLFW_MOD_ALT)
                keys[0]=0;  
            if(key==GLFW_KEY_RIGHT && mods==GLFW_MOD_ALT)
                keys[1]=0;
            if(key==GLFW_KEY_LEFT && mods==GLFW_MOD_CONTROL)
                keys[2]=0;
            if(key==GLFW_KEY_RIGHT && mods==GLFW_MOD_CONTROL)
                keys[3]=0;
    }
}
float usee;
/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
            quit(0);
            break;
        default:
            break;
    }
}
double xpos, ypos;
int transfer[6]={0};
float x5,y5,usee1=0,pan1=0;
static void cursor_pos(GLFWwindow* window, double xpos, double ypos)
{
 x5=(xpos*8/(fbwidth*1.0))-4.0f;
 y5=((-8*ypos)/(fbheight*1.0))+4.0;
 if(box1_rectangle_key_translate>5.5)
  box1_rectangle_key_translate=5.49;
if(box2_rectangle_key_translate>2.5)
  box2_rectangle_key_translate=2.49;
if(transfer[0]==1){
   if(box1_rectangle_key_translate>-2.9&&box1_rectangle_key_translate<5.5)
  box1_rectangle_key_translate=x5+1.5;
}
if(transfer[1]==1){
  if(box2_rectangle_key_translate>-5.8&&box2_rectangle_key_translate<2.5)
  box2_rectangle_key_translate=x5-1.5;
}
if(transfer[2]==1)
  gun_translate=y5;
if(transfer[4]==1)
{
  if(zoom==0)
    pan=0;
if(zoom>0){
  if(pan<zoom)
    pan=x5-usee;
    if(pan>(-zoom))
      pan=x5-usee;
    if(-zoom-pan>0)
      pan=-zoom;
    if(zoom-pan<0)
      pan=zoom;

     if(zoom==0)
    pan1=0;
if(zoom>0){
  if(pan1<zoom)
    pan1=y5-usee1;
    if(pan1>(-zoom))
      pan1=y5-usee1;
    if(-zoom-pan1>0)
      pan1=-zoom;
    if(zoom-pan1<0)
      pan1=zoom;
  }
Matrices.projection = glm::ortho(-4.0f+zoom-pan, 4.0f-zoom-pan, -4.0f+zoom-pan1, 4.0f-zoom-pan1, 0.1f, 500.0f);
}
}
}



int createRectangle (GLfloat *vertex_buffer_data,float a,float b,float l,float h,float tilt,int cnt)
  {
    vertex_buffer_data[cnt]=a;
    cnt++;
    vertex_buffer_data[cnt]=b;
    cnt++;
    vertex_buffer_data[cnt]=0;
    cnt++;
    vertex_buffer_data[cnt]=a+l*cos(tilt*(M_PI/180));
    cnt++;
    vertex_buffer_data[cnt]=b+l*sin(tilt*(M_PI/180));
    cnt++;
    vertex_buffer_data[cnt]=0;
    cnt++;
    vertex_buffer_data[cnt]=a+h*sin(tilt*(M_PI/180));
    cnt++;
    vertex_buffer_data[cnt]=b+h*cos(tilt*(M_PI/180));
    cnt++;
    vertex_buffer_data[cnt]=0;
    cnt++;
    vertex_buffer_data[cnt]=a+h*sin(tilt*(M_PI/180))+l*cos(tilt*(M_PI/180));
    cnt++;
    vertex_buffer_data[cnt]=b+h*cos(tilt*(M_PI/180))+l*sin(tilt*(M_PI/180));
    cnt++;
    vertex_buffer_data[cnt]=0;
    cnt++;
    vertex_buffer_data[cnt]=a+l*cos(tilt*(M_PI/180));
    cnt++;
    vertex_buffer_data[cnt]=b+l*sin(tilt*(M_PI/180));
    cnt++;
    vertex_buffer_data[cnt]=0;
    cnt++;
    vertex_buffer_data[cnt]=a+h*sin(tilt*(M_PI/180));
    cnt++;
    vertex_buffer_data[cnt]=b+h*cos(tilt*(M_PI/180));
    cnt++;
    vertex_buffer_data[cnt]=0;
    cnt++;
    return cnt;
  }


VAO *segments[10];
void Numbers()
    {
      GLfloat vertex_buffer_data0[500],color_buffer_data0[500];
      GLfloat vertex_buffer_data1[500],color_buffer_data1[500];
      GLfloat vertex_buffer_data2[500],color_buffer_data2[500];
      GLfloat vertex_buffer_data3[500],color_buffer_data3[500];
      GLfloat vertex_buffer_data4[500],color_buffer_data4[500];
      GLfloat vertex_buffer_data5[500],color_buffer_data5[500];
      GLfloat vertex_buffer_data6[500],color_buffer_data6[500];
      GLfloat vertex_buffer_data7[500],color_buffer_data7[500];
      GLfloat vertex_buffer_data8[500],color_buffer_data8[500];
      GLfloat vertex_buffer_data9[500],color_buffer_data9[500];

      int cnt=0,i,m=1,l;
      cnt=0;
      for(i=0;i<=9;i++)
      {
        cnt=0;
        if(i==0) 
        {  
          cnt=createRectangle(vertex_buffer_data0,2,2.95,0.5,0.05,0,cnt);
          cnt=createRectangle(vertex_buffer_data0,2,2.5,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data0,2,2.05,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data0,2.45,2.5,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data0,2.45,2.05,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data0,2,2.05,0.5,0.05,0,cnt);
          for(l=0;l<cnt;l++)
          {
            color_buffer_data0[l]=0;
          }    
          segments[i]=create3DObject(GL_TRIANGLES, cnt/3, vertex_buffer_data0, color_buffer_data0, GL_FILL);
      //    printf("segments-%d\n",i);
        }
        if(i==1)
        {
          cnt=createRectangle(vertex_buffer_data1,2.45,2.5,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data1,2.45,2.05,0.05,0.5,0,cnt);
          for(l=0;l<cnt;l++)
          {
            color_buffer_data1[l]=0;
          }    
          segments[i]=create3DObject(GL_TRIANGLES, cnt/3, vertex_buffer_data1, color_buffer_data1, GL_FILL);

        }
        if(i==2)
        {
          cnt=createRectangle(vertex_buffer_data2,2,2.95,0.5,0.05,0,cnt);
          cnt=createRectangle(vertex_buffer_data2,2,2.05,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data2,2,2.5,0.5,0.05,0,cnt);
          cnt=createRectangle(vertex_buffer_data2,2.45,2.5,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data2,2,2.05,0.5,0.05,0,cnt);
          for(l=0;l<cnt;l++)
          {
            color_buffer_data2[l]=0;
          }    
          segments[i]=create3DObject(GL_TRIANGLES, cnt/3, vertex_buffer_data2, color_buffer_data2, GL_FILL);

        }
        if(i==3)
        {
          cnt=createRectangle(vertex_buffer_data3,2,2.95,0.5,0.05,0,cnt);
          cnt=createRectangle(vertex_buffer_data3,2,2.5,0.5,0.05,0,cnt);
          cnt=createRectangle(vertex_buffer_data3,2.45,2.5,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data3,2.45,2.05,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data3,2,2.05,0.5,0.05,0,cnt);
          for(l=0;l<cnt;l++)
          {
            color_buffer_data3[l]=0;
          }    
          segments[i]=create3DObject(GL_TRIANGLES, cnt/3, vertex_buffer_data3, color_buffer_data3, GL_FILL);
        }
        if(i==4)
        {
          cnt=createRectangle(vertex_buffer_data4,2,2.5,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data4,2,2.5,0.5,0.05,0,cnt);
          cnt=createRectangle(vertex_buffer_data4,2.45,2.5,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data4,2.45,2.05,0.05,0.5,0,cnt);
          for(l=0;l<cnt;l++)
          {
            color_buffer_data4[l]=0;
          }    
          segments[i]=create3DObject(GL_TRIANGLES, cnt/3, vertex_buffer_data4, color_buffer_data4, GL_FILL);

        }
        if(i==5)
        {
          cnt=createRectangle(vertex_buffer_data5,2,2.95,0.5,0.05,0,cnt);
          cnt=createRectangle(vertex_buffer_data5,2,2.5,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data5,2,2.5,0.5,0.05,0,cnt);
          cnt=createRectangle(vertex_buffer_data5,2.45,2.05,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data5,2,2.05,0.5,0.05,0,cnt);
          for(l=0;l<cnt;l++)
          {
            color_buffer_data5[l]=0;
          }    
          segments[i]=create3DObject(GL_TRIANGLES, cnt/3, vertex_buffer_data5, color_buffer_data5, GL_FILL);

        } 
        if(i==6)
        {
          cnt=createRectangle(vertex_buffer_data6,2,2.95,0.5,0.05,0,cnt);
          cnt=createRectangle(vertex_buffer_data6,2,2.5,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data6,2,2.05,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data6,2,2.5,0.5,0.05,0,cnt);
          cnt=createRectangle(vertex_buffer_data6,2.45,2.05,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data6,2,2.05,0.5,0.05,0,cnt);

          for(l=0;l<cnt;l++)
          {
            color_buffer_data6[l]=0;
          }    
          segments[i]=create3DObject(GL_TRIANGLES, cnt/3, vertex_buffer_data6, color_buffer_data6, GL_FILL);

        } 
        if(i==7)
        {
          cnt=createRectangle(vertex_buffer_data7,2,2.95,0.5,0.05,0,cnt);
          cnt=createRectangle(vertex_buffer_data7,2.45,2.5,0.05,0.5,0,cnt);
          cnt=createRectangle(vertex_buffer_data7,2.45,2.05,0.05,0.5,0,cnt);
          for(l=0;l<cnt;l++)
          {
            color_buffer_data7[l]=0;
          }    
          segments[i]=create3DObject(GL_TRIANGLES, cnt/3, vertex_buffer_data7, color_buffer_data7, GL_FILL);

        }
        if(i==8)
        {
         cnt=createRectangle(vertex_buffer_data8,2,2.95,0.5,0.05,0,cnt);
         cnt=createRectangle(vertex_buffer_data8,2,2.5,0.05,0.5,0,cnt);
         cnt=createRectangle(vertex_buffer_data8,2,2.05,0.05,0.5,0,cnt);
         cnt=createRectangle(vertex_buffer_data8,2,2.5,0.5,0.05,0,cnt);
         cnt=createRectangle(vertex_buffer_data8,2.45,2.5,0.05,0.5,0,cnt);
         cnt=createRectangle(vertex_buffer_data8,2.45,2.05,0.05,0.5,0,cnt);
         cnt=createRectangle(vertex_buffer_data8,2,2.05,0.5,0.05,0,cnt);

         for(l=0;l<cnt;l++)
         {
          color_buffer_data8[l]=0;
        }    
        segments[i]=create3DObject(GL_TRIANGLES, cnt/3, vertex_buffer_data8, color_buffer_data8, GL_FILL);

      } 
      if(i==9)
      {
        cnt=createRectangle(vertex_buffer_data9,2,2.95,0.5,0.05,0,cnt);
        cnt=createRectangle(vertex_buffer_data9,2,2.5,0.05,0.5,0,cnt);
        cnt=createRectangle(vertex_buffer_data9,2,2.5,0.5,0.05,0,cnt);
        cnt=createRectangle(vertex_buffer_data9,2.45,2.5,0.05,0.5,0,cnt);
        cnt=createRectangle(vertex_buffer_data9,2.45,2.05,0.05,0.5,0,cnt);
        cnt=createRectangle(vertex_buffer_data9,2,2.05,0.5,0.05,0,cnt);
        for(l=0;l<cnt;l++)
        {
          color_buffer_data9[l]=0;
        }    
        segments[i]=create3DObject(GL_TRIANGLES, cnt/3, vertex_buffer_data9, color_buffer_data9, GL_FILL);

      }   
    }
}



  




/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
              {
                transfer[0]=0;
                transfer[1]=0;
                transfer[2]=0;
                transfer[3]=0;
              }
            if(action == GLFW_PRESS)
              {
                if((-2+box1_rectangle_key_translate<=x5)&&(x5<=(-1+box1_rectangle_key_translate))&&(y5<=-2.90f))
                transfer[0]=1,transfer[3]=1;
                if((1+box2_rectangle_key_translate<=x5)&&(x5<=(2+box2_rectangle_key_translate))&&(y5<=-2.90f))
                transfer[1]=1,transfer[3]=1;
                if(x5<-3.56f&&(sqrt((x5+3.96)*(x5+3.96)+(y5-(gun_translate))*(y5-(gun_translate)))<=0.4))
                  transfer[2]=1,transfer[3]=1;
                if(x5<-3.56f||y5<=-2.90f)
                  transfer[3]=1;
                if(transfer[3]==0)
                {
                  float aa=-3.96;float aa1=gun_translate;
                  float ang=(atan((y5-aa1)/(x5-aa)))* (180 / 3.14159265f);
                  // cout<<atan(1)* (180 / 3.14159265f)<<endl;
                  gun_rotation=ang;
                  e_time = glfwGetTime(); // Time in seconds
                 if ((e_time - timme) <= 1.0) {
                 break;
                 } 
                 timme=e_time;
                 bullet_number++;
                // cout<<bullet_number<<endl;
                if(bullet_number>50){
                  bullet_number=0;
                  bullet_maker_flag=1;
                  for(int rr=0;rr<=50;rr++)
                    {
                      translate_bullet[rr]=0.0f;
                      flagforbullet[rr]=0;
                    }
                }
                flagforbullet[bullet_number]=1;
                bullets[bullet_number].angle=(gun_rotation*(M_PI/180.0));
                bullets[bullet_number].translate=gun_translate;
                bullets[bullet_number].sy=-3.80f;
                if(bullet_maker_flag==0)
                bullets[bullet_number].vv=createBullet(0.0f,0.0f,0.3f,0.0f);
                }
              }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                transfer[4]=0;
            }
            if(action==GLFW_PRESS)
            {
              transfer[4]=1;
             usee=x5;
             usee1=y5;
            }
            break;
        default:
            break;
    }
}
void scroll_call(GLFWwindow* window, double xoffset, double yoffset)
{
if(yoffset==1)
{
              if(zoom<4){
                zoom++;
                Matrices.projection = glm::ortho(-4.0f+zoom, 4.0f-zoom, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);}
}
if(yoffset==(-1))
{
              if(zoom>0){
                zoom--;
                pan=0;
                Matrices.projection = glm::ortho(-4.0f+zoom, 4.0f-zoom, -4.0f+zoom, 4.0f-zoom, 0.1f, 500.0f);
                }
}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    
     fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
       is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = 90.0f;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // set the projection matrix as perspective
    /* glMatrixMode (GL_PROJECTION);
       glLoadIdentity ();
       gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}




// Creates the circle object used in this sample code
VAO *createcircle (float radius,float x,float y,float z,int color_id)
{
    // GL3 accepts only Triangles. Quads are not supported
    VAO *cir;
    static  GLfloat vertex_buffer_data[(360*9)+10];
    static GLfloat color_buffer_data[(360*9)+10];
    float r=radius;
    float teta=(M_PI/180.0);
    for(int i=0;i<360;i++)
    {
        vertex_buffer_data[(9*i)+0]=x+(r*cos(teta));vertex_buffer_data[(9*i)+1]=y+(r*sin(teta));vertex_buffer_data[(9*i)+2]=z;
        vertex_buffer_data[(9*i)+3]=x+(r*cos(teta+(M_PI/180.0)));vertex_buffer_data[(9*i)+4]=y+(r*sin(teta+(M_PI/180.0)));vertex_buffer_data[(9*i)+5]=z;
        vertex_buffer_data[(9*i)+6]=x;vertex_buffer_data[(9*i)+7]=y;vertex_buffer_data[(9*i)+8]=0;
        teta+=M_PI/180.0;
        for(int j=0;j<9;j=j+3)
        {
            if(color_id==0)
            {color_buffer_data[(9*i)+j]=0;color_buffer_data[(9*i)+j+1]=1.0;color_buffer_data[(9*i)+j+2]=0.0;}
            else if(color_id==1)
            {color_buffer_data[(9*i)+j]=1.0;color_buffer_data[(9*i)+j+1]=0.4;color_buffer_data[(9*i)+j+2]=0.4;}
        }
    }

    // create3DObject creates and returns a handle to a VAO that can be used later
    cir = create3DObject(GL_TRIANGLES, 360*3, vertex_buffer_data, color_buffer_data, GL_FILL);
    // circle=cir;
    return cir;
}
float camera_rotation_angle = 90;
float circle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
VAO *createBrick(float x,float y,float h,float w,float z,int color_id,int counter,int br_flag)
{
    VAO *rt;
    float x1=x+(w/2.0f);float y1=y+(h/2.0f);float x2=x+(w/2.0f);float y2=y-(h/2.0f);float x3=x-(w/2.0f);
    float y3=y-(h/2.0f);float x4=x-(w/2.0f);float y4=y+(h/2.0f);
    static GLfloat vertex_buffer_data[19];
    vertex_buffer_data[0]=x1;vertex_buffer_data[1]=y1;vertex_buffer_data[2]=z;
    vertex_buffer_data[3]=x2;vertex_buffer_data[4]=y2;vertex_buffer_data[5]=z;
    vertex_buffer_data[6]=x3;vertex_buffer_data[7]=y3;vertex_buffer_data[8]=z;
    vertex_buffer_data[9]=x3;vertex_buffer_data[10]=y3;vertex_buffer_data[11]=z;
    vertex_buffer_data[12]=x4;vertex_buffer_data[13]=y4;vertex_buffer_data[14]=z;
    vertex_buffer_data[15]=x1;vertex_buffer_data[16]=y1;vertex_buffer_data[17]=z;
    if(br_flag==1){
        bricks[counter].c1=x;bricks[counter].c2=y;bricks[counter].high=h;bricks[counter].wid=w;
        bricks[counter].id=color_id;
    }
    static  GLfloat color_buffer_data[19];
    if(color_id==0)
    {
        color_buffer_data[0]=0.35;color_buffer_data[1]=0.65;color_buffer_data[2]=0.0;
        color_buffer_data[3]=0.35;color_buffer_data[4]=0.65;color_buffer_data[5]=0.0;
        color_buffer_data[6]=0.35;color_buffer_data[7]=0.65;color_buffer_data[8]=0.0;
        color_buffer_data[9]=0.35;color_buffer_data[10]=0.65;color_buffer_data[11]=0.0;
        color_buffer_data[12]=0.35;color_buffer_data[13]=0.65;color_buffer_data[14]=0.0;
        color_buffer_data[15]=0.35;color_buffer_data[16]=0.65;color_buffer_data[17]=0.0;
    }
    else if(color_id==1)
    {
        color_buffer_data[0]=1;color_buffer_data[1]=0.2;color_buffer_data[2]=0.2;
        color_buffer_data[3]=1;color_buffer_data[4]=0.2;color_buffer_data[5]=0.2;
        color_buffer_data[6]=1;color_buffer_data[7]=0.2;color_buffer_data[8]=0.2;
        color_buffer_data[9]=1;color_buffer_data[10]=0.2;color_buffer_data[11]=0.2;
        color_buffer_data[12]=1;color_buffer_data[13]=0.2;color_buffer_data[14]=0.2;
        color_buffer_data[15]=1;color_buffer_data[16]=0.2;color_buffer_data[17]=0.2;
    }
    else if(color_id==2)
    {
        color_buffer_data[0]=0.0;color_buffer_data[1]=0.0;color_buffer_data[2]=0.0;
        color_buffer_data[3]=0.0;color_buffer_data[4]=0.0;color_buffer_data[5]=0.0;
        color_buffer_data[6]=0.0;color_buffer_data[7]=0.0;color_buffer_data[8]=0.0;
        color_buffer_data[9]=0.0;color_buffer_data[10]=0.0;color_buffer_data[11]=0.0;
        color_buffer_data[12]=0.0;color_buffer_data[13]=0.0;color_buffer_data[14]=0.0;
        color_buffer_data[15]=0.0;color_buffer_data[16]=0.0;color_buffer_data[17]=0.0;
    }
    else if(color_id==3)
    {
        color_buffer_data[0]=0.75;color_buffer_data[1]=0.75;color_buffer_data[2]=0.75;
        color_buffer_data[3]=0.75;color_buffer_data[4]=0.75;color_buffer_data[5]=0.75;
        color_buffer_data[6]=0.75;color_buffer_data[7]=0.75;color_buffer_data[8]=0.75;
        color_buffer_data[9]=0.75;color_buffer_data[10]=0.75;color_buffer_data[11]=0.75;
        color_buffer_data[12]=0.75;color_buffer_data[13]=0.75;color_buffer_data[14]=0.75;
        color_buffer_data[15]=0.75;color_buffer_data[16]=0.75;color_buffer_data[17]=0.75;
    }
    rt = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    return rt;
}
void helping_fun(VAO *ert,float t_x,float t_y,float t_z,float rotation,float a1,float a2,float a3)
{
    glm::mat4 VP = Matrices.projection * Matrices.view;
    glm::mat4 MVP;  // MVP = Projection * View * Model
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translate_box1_rec = glm::translate (glm::vec3(t_x,t_y,t_z)); // glTranslatef
    glm::mat4 rotate_box1_rec = glm::rotate((float)rotation, glm::vec3(a1,a2,a3));  // rotate about vector (1,0,0)
    glm::mat4 box1_rec_Transform = translate_box1_rec*rotate_box1_rec ;
    Matrices.model *=box1_rec_Transform;
    MVP = VP * Matrices.model; // MVP = p * V * M
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(ert);
}
void draw ()
{
    // clear the color and depth in the frame buffer
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // use the loaded shader program
    // Don't change unless you know what you are doing
    glUseProgram (programID);

    // Eye - Location of camera. Don't change unless you are sure!!
    glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    // Target - Where is the camera looking at.  Don't change unless you are sure!!
    glm::vec3 target (0, 0, 0);
    // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
    glm::vec3 up (0, 1, 0);

    // Compute Camera matrix (view)
    // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
    //  Don't change unless you are sure!!
    Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

    // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
    //  Don't change unless you are sure!!
    glm::mat4 VP = Matrices.projection * Matrices.view;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // For each model you render, since the MVP will be different (at least the M part)
    //  Don't change unless you are sure!!
    glm::mat4 MVP;	// MVP = Projection * View * Model
    // Load identity to model matrix
            if(keys[0]==1){
               if(box1_rectangle_key_translate>-2.5)
                box1_rectangle_key_translate-=0.1f;
              else
                keys[0]=0;
              }
            if(keys[1]==1){
               if(box1_rectangle_key_translate<5.5)
                box1_rectangle_key_translate+=0.1f;
              else
                keys[1]=0;
              }
            if(keys[2]==1){
                if(box2_rectangle_key_translate>-5.5)
                box2_rectangle_key_translate-=0.1f;
              else
                keys[2]=0;
              }
            if(keys[3]==1){
                if(box2_rectangle_key_translate<2.5)
                box2_rectangle_key_translate+=0.1f;
              else
                keys[3]=0;
              }
            if(keys[4]==1)
                gun_translate+=0.1f;
            if(keys[5]==1)
                gun_translate-=0.1f;
            if(keys[6]==1)
                gun_rotation-=1.5f;
            if(keys[7]==1)
                gun_rotation+=1.5f;
    helping_fun(box1_rec.vv,box1_rectangle_key_translate,0.0f,0.0f,(float)(triangle_rotation*(M_PI/180.0)),0.0f,0.0f,1.0f);
    helping_fun(box2_rec.vv,box2_rectangle_key_translate,0.0f,0.0f,(float)(triangle_rotation*(M_PI/180.0)),0.0f,0.0f,1.0f);
    helping_fun(gun_rec1.vv,-3.80f,0.0f+gun_translate,0.0f,(float)(gun_rotation*(M_PI/180.0)),0.0f,0.0f,1.0f);
    helping_fun(box1_circle,-1.5f+box1_rectangle_key_translate,-3.0f,0.0f,(float)(70.0*(M_PI/180.0)),1.0f,0.0f,0.0f);
    helping_fun(box2_circle,1.5f+box2_rectangle_key_translate,-3.0f,0.0f,(float)(70.0*(M_PI/180.0)),1.0f,0.0f,0.0f);
    helping_fun(mirror[0].vv,1.0f,3.0f,0.0f,(float)(-45*(M_PI/180.0)),0.0f,0.0f,1.0f);
    helping_fun(mirror[1].vv,1.0f,-1.0f,0.0f,(float)(60*(M_PI/180.0)),0.0f,0.0f,1.0f);
    helping_fun(mirror[2].vv,3.0f,-2.0f,0.0f,(float)(45*(M_PI/180.0)),0.0f,0.0f,1.0f);
    helping_fun(mirror[3].vv,3.0f,2.0f,0.0f,(float)(-60*(M_PI/180.0)),0.0f,0.0f,1.0f);
    helping_fun(gun_semicircle,0.0f,gun_translate,0.0f,(float)(00.0*(M_PI/180.0)),0.0f,0.0f,1.0f);
    helping_fun(line.vv,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f);
    Matrices.model = glm::mat4(1.0f);
    int ew;
    glm::mat4 translatebrick;
    for(ew=0;ew<51;ew++)
    {
      float coll[2]={0};
      if((bricks[ew].c2+bricks[ew].translate)<-2.90f && flagg[ew]==1)
          {
            // cout<<"loop"<<endl;
            float x_cor=bricks[ew].c1;
            // cout<<x_cor<<endl;
            int tt=bricks[ew].id;
            // cout<<tt<<" "<<score<<endl;
            if(((-2.0+box1_rectangle_key_translate)<=x_cor)&&((-1.0+box1_rectangle_key_translate)>=x_cor))coll[0]=1;
            if(((1.0+box2_rectangle_key_translate)<=x_cor)&&((2.0+box2_rectangle_key_translate)>=x_cor))coll[1]=1;
             // cout<<coll[0]<<" "<<coll[1]<<" "<<tt<<endl;
            if((coll[0]==1 && coll[1]==1) && tt!=2)
              score-=5;
            else if(coll[0]==1 && tt == 1){score-=5;flagg[ew]=0;}
            else if(coll[0]==1 && tt == 0){score+=10;flagg[ew]=0;}
            else if(coll[1]==1 && tt == 1){score+=10;flagg[ew]=0;}
            else if(coll[1]==1 && tt == 0){score-=5;flagg[ew]=0;}
            else if((coll[1]==1 || coll[0]==1) && tt==2){over=1;}
            else{ score-=2;
            flagg[ew]=0;}
            bricks[ew].translate=0.0f;
            cout<<"1"<<endl;
          }
        if(flagg[ew]==1)
        {

            bricks[ew].translate-=speed;
            helping_fun(bricks[ew].vv,0.0f,bricks[ew].translate,0.0f,(float)(0.0f*(M_PI/180.0)),0.0f,0.0f,1.0f);

        }
        if(flagforbullet[ew]==1)
        {
            translate_bullet[ew]+=0.1f;
            // cout<<bullets[ew].angle<<endl;
            float rtt=bullets[ew].translate;
            helping_fun(bullets[ew].vv,bullets[ew].sy+(translate_bullet[ew]*(cos(bullets[ew].angle))),rtt+(translate_bullet[ew]*(sin(bullets[ew].angle))),0.0f,bullets[ew].angle,0.0f,0.0f,1.0f);
        bullets[ew].c1=bullets[ew].sy+((float)(0.30f+translate_bullet[ew])*(float)cos(bullets[ew].angle));
        bullets[ew].c2=rtt+((0.30f+translate_bullet[ew])*(float)sin(bullets[ew].angle));
        bullets[ew].p2.x=bullets[ew].c1;
        bullets[ew].p2.y=bullets[ew].c2;
        bullets[ew].p1.x=bullets[ew].sy+((float)(translate_bullet[ew])*(float)cos(bullets[ew].angle));
        bullets[ew].p1.y=rtt+((translate_bullet[ew])*(float)sin(bullets[ew].angle));
        }
    }
 for(ew=0;ew<51;ew++){
    if((((-4>bullets[ew].p1.x)||(4<bullets[ew].p1.x))||((-4>bullets[ew].p1.y)||(4<bullets[ew].p1.y)))&&flagforbullet[ew]==1)
    {
      flagforbullet[ew]=0;
      score-=8;
      cout<<"2"<<endl;
    }
  }
    for(ew=0;ew<50;ew++)
    {
        int eww;
        for(eww=0;eww<50;eww++)
        {
          if(flagg[ew]==1 && flagforbullet[eww]==1)
          { 

            float z1=bullets[eww].c1;float z2=bullets[eww].c2;float z3=bricks[ew].c1-0.1650f;float z4=bricks[ew].c1+0.1650f;
            float z5=bricks[ew].c2+bricks[ew].translate-0.1250f;float z6=bricks[ew].c2+bricks[ew].translate+0.1250f;
            //  if(ew==0)
            //   cout<<"Brick1: "<<z5<<z6<<endl;
            // if(eww==0)
            //   cout<<"Bullet1: "<<z1<<z2<<endl;   
            if(((z3<=z1)&&(z4>=z1))&&((z5<z2)&&(z6>z2)))
            {
              cout<<"3"<<endl;
              if(bricks[ew].id==2){
              flagforbullet[eww]=0;
              flagg[ew]=0;
              score+=20;}
              else
                score-=15;
            }
          }
        }
    }
    for(ew=0;ew<50;ew++)
    {
      if(flagforbullet[ew]==1 )
      {
        // if(doIntersect(mirror[0].p1, mirror[0].p2, bullets[ew].p1, bullets[ew].p2))
        // {
          if(intersect_point(mirror[0].p1, mirror[0].p2, bullets[ew].p1, bullets[ew].p2)==1){
          // cout<<mirror[0].p1.x<<" "<<mirror[0].p1.y<<" "<<mirror[0].p2.x<<" "<<mirror[0].p2.y<<" "<<bullets[ew].p1.x<<"#"<<bullets[ew].p1.y<<"#"<<
          // bullets[ew].p2.x<<"#"<<bullets[ew].p2.y<<"#";
            // cout<<inter.x<<"#"<<inter.y<<"#"<<inter.va<<endl;

           bullets[ew].angle=(float)(-90.0f*(M_PI/180.0))-bullets[ew].angle;
            bullets[ew].translate=y_intersection+(0.3f*sin(bullets[ew].angle));
            bullets[ew].sy=x_intersection+(0.3f*cos(bullets[ew].angle));
            translate_bullet[ew]=0.0f;
            bullets[ew].c1=bullets[ew].sy+((float)(0.30f+translate_bullet[ew])*(float)cos(bullets[ew].angle));
        bullets[ew].c2=bullets[ew].translate+((0.30f+translate_bullet[ew])*(float)sin(bullets[ew].angle));
        bullets[ew].p2.x=bullets[ew].c1;
        bullets[ew].p2.y=bullets[ew].c2;
        bullets[ew].p1.x=bullets[ew].sy+((float)(translate_bullet[ew])*(float)cos(bullets[ew].angle));
        bullets[ew].p1.y=bullets[ew].translate+((translate_bullet[ew])*(float)sin(bullets[ew].angle));
            
            // cout<<"0"<<endl;
        }
        // if(doIntersect(mirror[1].p1, mirror[1].p2, bullets[ew].p1, bullets[ew].p2))
        // {
          // cout<<doIntersect(mirror[1].p1, mirror[1].p2, bullets[ew].p1, bullets[ew].p2)<<endl;
          if(intersect_point(mirror[1].p1, mirror[1].p2, bullets[ew].p1, bullets[ew].p2)==1){
          // cout<<mirror[1].p1.x<<" "<<mirror[1].p1.y<<" "<<mirror[1].p2.x<<" "<<mirror[1].p2.y<<" "<<bullets[ew].p1.x<<"#"<<bullets[ew].p1.y<<"#"<<
          // bullets[ew].p2.x<<"#"<<bullets[ew].p2.y<<"#";
            // cout<<inter.x<<"#"<<inter.y<<"#"<<inter.va<<endl;
            bullets[ew].angle=(float)(120.0f*(M_PI/180.0))-bullets[ew].angle;
            bullets[ew].translate=y_intersection+0.3f*sin(bullets[ew].angle);
            bullets[ew].sy=x_intersection+0.3f*cos(bullets[ew].angle);
            
            translate_bullet[ew]=0.0f;
            bullets[ew].c1=bullets[ew].sy+((float)(0.30f+translate_bullet[ew])*(float)cos(bullets[ew].angle));
        bullets[ew].c2=bullets[ew].translate+((0.30f+translate_bullet[ew])*(float)sin(bullets[ew].angle));
        bullets[ew].p2.x=bullets[ew].c1;
        bullets[ew].p2.y=bullets[ew].c2;
        bullets[ew].p1.x=bullets[ew].sy+((float)(translate_bullet[ew])*(float)cos(bullets[ew].angle));
        bullets[ew].p1.y=bullets[ew].translate+((translate_bullet[ew])*(float)sin(bullets[ew].angle));
            
            // cout<<"1"<<endl;
        }
        // if(doIntersect(mirror[2].p1, mirror[2].p2, bullets[ew].p1, bullets[ew].p2))
        // {
          if(intersect_point(mirror[2].p1, mirror[2].p2, bullets[ew].p1, bullets[ew].p2)==1){
          
            bullets[ew].angle=(float)(90.0f*(M_PI/180.0))-bullets[ew].angle;
            bullets[ew].translate=y_intersection+0.3f*sin(bullets[ew].angle);
            bullets[ew].sy=x_intersection+0.3f*cos(bullets[ew].angle);
            translate_bullet[ew]=0.0f;
            bullets[ew].c1=bullets[ew].sy+((float)(0.30f+translate_bullet[ew])*(float)cos(bullets[ew].angle));
        bullets[ew].c2=bullets[ew].translate+((0.30f+translate_bullet[ew])*(float)sin(bullets[ew].angle));
        bullets[ew].p2.x=bullets[ew].c1;
        bullets[ew].p2.y=bullets[ew].c2;
        bullets[ew].p1.x=bullets[ew].sy+((float)(translate_bullet[ew])*(float)cos(bullets[ew].angle));
        bullets[ew].p1.y=bullets[ew].translate+((translate_bullet[ew])*(float)sin(bullets[ew].angle));
            
            // cout<<"2"<<endl;
        }
        // if(doIntersect(mirror[3].p1, mirror[3].p2, bullets[ew].p1, bullets[ew].p2))
        // {
          if(intersect_point(mirror[3].p1, mirror[3].p2, bullets[ew].p1, bullets[ew].p2)==1){
         
            bullets[ew].angle=(float)(-120.0f*(M_PI/180.0))-bullets[ew].angle;
            bullets[ew].translate=y_intersection+(0.3f*sin(bullets[ew].angle));
            bullets[ew].sy=x_intersection+(0.3f*cos(bullets[ew].angle));
            translate_bullet[ew]=0.0f;
            bullets[ew].c1=bullets[ew].sy+((float)(0.30f+translate_bullet[ew])*(float)cos(bullets[ew].angle));
        bullets[ew].c2=bullets[ew].translate+((0.30f+translate_bullet[ew])*(float)sin(bullets[ew].angle));
        bullets[ew].p2.x=bullets[ew].c1;
        bullets[ew].p2.y=bullets[ew].c2;
        bullets[ew].p1.x=bullets[ew].sy+((float)(translate_bullet[ew])*(float)cos(bullets[ew].angle));
        bullets[ew].p1.y=bullets[ew].translate+((translate_bullet[ew])*(float)sin(bullets[ew].angle));
            
            // cout<<"3"<<endl;
        }
      }
    }
    // Increment angles
    // float increments = 1;
    //camera_rotation_angle++; // Simulating camera rotation
    // triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
    // circle_rotation = circle_rotation + increments*circle_rot_dir*circle_rot_status;
    int temp=score;
int ff=0;    if(temp<0)temp=-1*score,ff=1;
    float tr=0;
    while(temp>0)
    {
      int kk=temp%10;
      temp=temp/10;
      helping_fun(segments[kk],1.0+tr,1.0f,0.0f,(float)(0*(M_PI/180.0)),0.0f,0.0f,1.0f);
      tr-=1.2;
    }
    if(ff==1)
      helping_fun(uy,3.6+tr,3.3,0,0,0,0,1);
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        //        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        //        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
       is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetCursorPosCallback(window, cursor_pos);
    glfwSetScrollCallback(window, scroll_call);


    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
    // Create the models


    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (1.0f, 0.8f, 1.0f, 1.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}
int main (int argc, char** argv)
{
    int width = 600;
    int height = 600;

    GLFWwindow* window = initGLFW(width, height);

    initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;
    Numbers();


    box1_rec.vv=createBrick(-1.5,-3.49,0.98,1.0,-1.0,0,1,0);
    box2_rec.vv=createBrick(1.5,-3.49,0.98,1.0,-1.0,1,1,0);
    mirror[0].vv=createBrick(0.0f,0.0f,0.1f,0.75,1.0,3,1,0);
    mirror[1].vv=createBrick(0.0f,0.0f,0.1f,0.75,1.0,3,1,0);
    mirror[2].vv=createBrick(0.0f,0.0f,0.1f,0.75,1.0,3,1,0);
    mirror[3].vv=createBrick(0.0f,0.0f,0.1f,0.75,1.0,3,1,0);
    mirror[0].p1.x=1.0f+(0.375f*cos((float)(-45*(M_PI/180.0))));
    mirror[0].p1.y=3.0f+(0.375f*sin((float)(-45*(M_PI/180.0))));
    mirror[1].p1.x=1.0f+(0.375f*cos((float)(60*(M_PI/180.0))));
    mirror[1].p1.y=-1.0f+(0.375f*sin((float)(60*(M_PI/180.0))));
    mirror[2].p1.x=3.0f+(0.375f*cos((float)(45*(M_PI/180.0))));
    mirror[2].p1.y=-2.0f+(0.375f*sin((float)(45*(M_PI/180.0))));
    mirror[3].p1.x=3.0f+(0.375f*cos((float)(-60*(M_PI/180.0))));
    mirror[3].p1.y=2.0f+(0.375f*sin((float)(-60*(M_PI/180.0))));
    
    mirror[0].p2.x=1.0f-(0.375f*cos((float)(-45*(M_PI/180.0))));
    mirror[0].p2.y=3.0f-(0.375f*sin((float)(-45*(M_PI/180.0))));
    mirror[1].p2.x=1.0f-(0.375f*cos((float)(60*(M_PI/180.0))));
    mirror[1].p2.y=-1.0f-(0.375f*sin((float)(60*(M_PI/180.0))));
    mirror[2].p2.x=3.0f-(0.375f*cos((float)(45*(M_PI/180.0))));
    mirror[2].p2.y=-2.0f-(0.375f*sin((float)(45*(M_PI/180.0))));
    mirror[3].p2.x=3.0f-(0.375f*cos((float)(-60*(M_PI/180.0))));
    mirror[3].p2.y=2.0f-(0.375f*sin((float)(-60*(M_PI/180.0))));
    box1_circle=createcircle(0.5f,0.0f,0.0f,0.0f,0);
    box2_circle=createcircle(0.5f,0.0f,0.0f,0.0f,1);
    gun_semicircle=createcircle(0.4f,-3.96f,0.0f,0.0f,0);
    gun_rec1.vv=createBrick(0.0f,0.0f,0.10f,1.0f,-1.0f,0,1,0);
    line.vv=createBullet(-4.0,-2.82,4.0,-2.82);
    uy=createBullet(-0.3,0,0.3,0);
    int j;
    for(j=0;j<52;j++)
        bricks[j].translate=0.0f;
    /* Draw in loop */
    int counter=-1;
    int ll=0;
    while (!glfwWindowShouldClose(window)) {  

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 3.0) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
         
            counter++;
            float sign;
            cout<<"Score is :"<<score<<endl;
            flagg[counter]=1;
            srand(time(NULL));
            if(rand()%2==1)sign=-1.0;
            else sign=1.0;
            srand(time(NULL));
            int brick_color=rand()%3;
            if(ll==0)
            {
              float checkk=(sign*(rand()%18)/5.0f);
              while(((checkk<1.375)&&(checkk>0.625))||((checkk<3.375)&&(checkk>2.625)))
                {
                  srand(time(NULL));
                  checkk=(sign*(rand()%18)/5.0f);
                }
            bricks[counter].vv=createBrick(checkk,3.750f,0.250f,0.330f,2.0,brick_color,counter,1);
            }
            last_update_time = current_time;
            // cout<<score<<endl;
            if(counter>50){
              counter=-1;ll=1;}
            }

        
        if(over==1)
    {
      exit(0);
      quit(window);
    }

    }
    glfwTerminate();
    //  exit(EXIT_SUCCESS);
}
