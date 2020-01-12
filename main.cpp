//
// Furkan Yakal hw#3
//
#define GL_SILENCE_DEPRECATION
#include "Angel.h"

typedef vec4  color4;
typedef vec4  point4;

float xPosition=-4;
float yPosition=3.75;
float g = -0.001;

const float surfaceEffect = 0.01;

float xVelocitySetter = 0.00300;
float xVelocity=0.00300;
float yVelocity=0;

int colorNumber = 5; //initially magenta
//--------
// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  //0 black
    color4( 1.0, 0.0, 0.0, 1.0 ),  //1 red
    color4( 1.0, 1.0, 0.0, 1.0 ),  //2 yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  //3 green
    color4( 0.0, 0.0, 1.0, 1.0 ),  //4 blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  //5 magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  //6 white
    color4( 0.0, 1.0, 1.0, 1.0 )   //7 cyan
};
//////---------------------------------------SPHERE---------------------------//////
const int NumTimesToSubdivide = 5;
const int NumTriangles = 4096;
// (4 faces)^(NumTimesToSubdivide + 1)
const int NumVerticesSphere = 3 * NumTriangles;

point4 pointSphere[NumVerticesSphere];
vec3 normals[NumVerticesSphere];
vec2 textureSphere[NumVerticesSphere];

vec2  calculateuv(const point4& a) {
    double u = acos(a.x)/(2 * M_PI);
    double v = acos(a.y / sin(2 * M_PI* u)) / (2 * M_PI);
    return vec2(u, v);
}

int IndexSphere = 0;

void
triangle( const point4& a, const point4& b, const point4& c )
{
    vec3 normal = normalize( cross(b - a, c - b) );
    normals[IndexSphere] = normal; pointSphere[IndexSphere] = a; textureSphere[IndexSphere] = calculateuv(a);IndexSphere++;
    normals[IndexSphere] = normal; pointSphere[IndexSphere] = b; textureSphere[IndexSphere] = calculateuv(b) ;IndexSphere++;
    normals[IndexSphere] = normal; pointSphere[IndexSphere] = c; textureSphere[IndexSphere] = calculateuv(c) ;IndexSphere++;
}
point4
unit( const point4& p )
{
    float len = p.x*p.x + p.y*p.y + p.z*p.z;
    point4 t;
    if ( len > DivideByZeroTolerance ) {
        t = p / sqrt(len);
        t.w = 1.0;
    }
    return t;
}
void
divide_triangle( const point4& a, const point4& b,
                const point4& c, int count )
{
    if ( count > 0 ) {
        point4 v1 = unit( a + b );
        point4 v2 = unit( a + c );
        point4 v3 = unit( b + c );
        divide_triangle( a, v1, v2, count - 1 );
        divide_triangle( c, v2, v3, count - 1 );
        divide_triangle( b, v3, v1, count - 1 );
        divide_triangle( v1, v3, v2, count - 1 );
    }
    else {
        triangle( a, b, c );
    }
}
void
tetrahedron( int count )
{
    point4 v[4] = {
        vec4( 0.0, 0.0, 1.0, 1.0 ),
        vec4( 0.0, 0.942809, -0.333333, 1.0 ),
        vec4( -0.816497, -0.471405, -0.333333, 1.0 ),
        vec4( 0.816497, -0.471405, -0.333333, 1.0 )
    };
    divide_triangle( v[0], v[1], v[2], count );
    divide_triangle( v[3], v[2], v[1], count );
    divide_triangle( v[0], v[3], v[1], count );
    divide_triangle( v[0], v[2], v[3], count );
}
void sphere(){
    tetrahedron(NumTimesToSubdivide);
}
//----------------------------------------------------------------
//read images
GLubyte *readPPM(char* fileName) {
    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Can't open the file !\n");
    }
    int k, n, m;
    char c;
    int i;
    int red, green, blue;
    char ch[2];   // 2 char table
    // read the first 3 char of the file
    fread(ch, sizeof(char), 2, file);
    // Check the file is an PPM file
    if (ch[0] != 'P' || ch[1] != '3') {
        printf("Not a ppm file.");
        exit(1);
    }
    printf("%s is a PPM file\n", fileName);
    //remove unneccesary characters #
    fscanf(file, "%c", &c);
    
    while (c == '#') {
        fscanf(file, "%[^\n] ", ch);
        printf("%s\n", ch);
        fscanf(file, "%c", &c);
    }
    ungetc(c, file);
    fscanf(file, "%d %d %d", &n, &m, &k);
    
    printf("rows: %d columns: %d max value: %d\n", n, m, k);
    int nm = n * m;
    GLubyte* image;
    image = (GLubyte*)malloc(3 * sizeof(GLubyte) * nm);
    for (i = nm; i > 0; i--) {
        
        fscanf(file, "%d %d %d", &red, &green, &blue);
        //printf("%d %d %d", red, green, blue);
        image[3 * nm - 3 * i] = red;
        image[3 * nm - 3 * i + 1] = green;
        image[3 * nm - 3 * i + 2] = blue;
    }
    fclose(file);
    return image;
}

GLubyte* basketball;
GLubyte* earth;

void generateTexels(){
    char basketballppm[] = "basketball.ppm";
    char earthppm[] = "earth.ppm";
    basketball = readPPM(basketballppm);
    earth = readPPM(earthppm);
}

enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int  Axis = Xaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };
// ModelView, Projection, and ColorUpdater matrices uniform location

bool phongFlag = false;
bool gouraudFlag = false;
bool modifiedPhongFlag = true;
bool texturingFlag = false;

bool lightPositionFixed = true; //initially fixed

GLuint  ModelView, Projection, ColorUpdater, PhongFlag, GouraudFlag, ModifiedPhongFlag, TexturingFlag,AmbientProduct,DiffuseProduct,SpecularProduct,LightPosition,Shininess ;

GLuint vao;
GLuint program;
GLuint textures[2];

mat4 model_view;

//------
point4 light_position(-4.0,2.0,10.0, 1.0);

color4 light_ambient(0.2, 0.2, 0.2, 1.0); // L_a
color4 light_diffuse(1.0, 1.0, 1.0, 1.0); // L_d
color4 light_specular(1.0, 1.0, 1.0, 1.0); // L_s

color4 material_ambient(1.0, 0.0, 1.0, 1.0); // k_a
color4 material_diffuse(1.0, 0.8, 0.0, 1.0); // k_d
color4 material_specular(1.0, 0.8, 0.0, 1.0); // k_s

float  material_shininess;

color4 ambient_product;
color4 diffuse_product ;
color4 specular_product;

//------
// OpenGL initialization
void
init()
{
    sphere();
    generateTexels();
    
    program = InitShader( "vshader.glsl", "fshader.glsl" );
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    GLuint vTextureCoord = glGetAttribLocation(program, "vTextureCoord");
    
    glGenVertexArrays( 1,&vao );
    glBindVertexArray( vao);
    
    glGenTextures(2, textures);
    
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, basketball);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, earth);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(pointSphere) + sizeof(normals)+sizeof(textureSphere),NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(pointSphere), pointSphere );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(pointSphere), sizeof(normals), normals );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(pointSphere)+sizeof(normals), sizeof(textureSphere), textureSphere );
    
    // set up vertex arrays
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(pointSphere)));
    
    glEnableVertexAttribArray(vTextureCoord);
    glVertexAttribPointer(vTextureCoord, 2, GL_FLOAT, GL_FALSE, 0,BUFFER_OFFSET(sizeof(pointSphere) + sizeof(normals)));
    
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    ColorUpdater =glGetUniformLocation( program, "ColorUpdater" );
    
    AmbientProduct = glGetUniformLocation(program,"AmbientProduct");
    DiffuseProduct = glGetUniformLocation(program,"DiffuseProduct");
    SpecularProduct = glGetUniformLocation(program,"SpecularProduct");
    LightPosition = glGetUniformLocation(program,"LightPosition");
    
    ambient_product = light_ambient * material_ambient;
    diffuse_product = light_diffuse * material_diffuse;
    specular_product = light_specular * material_specular;
    
    glUniform4fv(AmbientProduct,1,ambient_product );
    glUniform4fv(DiffuseProduct,1,diffuse_product);
    glUniform4fv(SpecularProduct,1,specular_product);
    glUniform4fv(LightPosition,1,light_position);
    
    Shininess = glGetUniformLocation(program,"Shininess");
    material_shininess=20;
    glUniform1f(Shininess, material_shininess);
    
    PhongFlag = glGetUniformLocation(program, "PhongFlag");
    GouraudFlag = glGetUniformLocation(program, "GouraudFlag");
    ModifiedPhongFlag = glGetUniformLocation(program, "ModifiedPhongFlag");
    TexturingFlag = glGetUniformLocation(program, "TexturingFlag");
    
    glUniform1i(PhongFlag, phongFlag);
    glUniform1i(GouraudFlag, gouraudFlag);
    glUniform1i(ModifiedPhongFlag, modifiedPhongFlag);
    glUniform1i(TexturingFlag, texturingFlag);

    glUseProgram( program );
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_CULL_FACE);
    glClearColor(1.0, 1.0, 1.0, 1.0 );
}
void
display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    vec3 displacementSphere( xPosition, yPosition, -10.0 );
    model_view = (Scale(0.2, 0.2, 0.2) * Translate(displacementSphere)) *  RotateX(Theta[Xaxis])*RotateY(Theta[Yaxis]) ;
    
    if(!lightPositionFixed){
        glUniform4fv(LightPosition,1, Translate(displacementSphere)* RotateX(Theta[Xaxis])*RotateY(Theta[Yaxis]) *light_position);
    }
    
    glUniform4fv(ColorUpdater,1,vertex_colors[colorNumber]);
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    
    glutSwapBuffers();
}
//--------------------------------------------------------------------------------------------------------//
//Rearrenges the size and position of the object when the window size is changed.
void reshape( int w, int h )
{
    glViewport(0, 0, w, h);
    GLfloat aspect = GLfloat(w) / h;
    
    mat4 projection;

    projection = Perspective(45.0, aspect, 0.5, 6.0);
 
    glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);

}
//--------------------------------------------------------------------------------------------------------//
//Sets the keyboard functionalities
//When Q or q button is pressed program terminates
//When I or i button is pressed program resets the position of the current object
//When H or h button is pressed program prints the instructions to the console
void
keyboard( unsigned char key,int x, int y )
{
    if(key == 'Q' | key == 'q')
        exit(0);
    if(key == 'I' | key == 'i'){
        //when I or i is pressed reset the application to initial positions
        xPosition=-4;
        xVelocity=xVelocitySetter;
        yPosition=3.75;
        yVelocity=0;
        g=-0.001;
    }
    if(key == 'h' | key == 'H'){
        std::cout<< " This is a simple bouncing sphere application with different shading and texturing choices"<<std::endl;
        std::cout<< " You can control the horizontal velocity, objects drawing style, its color, its shading type, and its texturing"<<std::endl;
        std::cout<< " ------------------------------------------"<<std::endl;
        std::cout<< " By pressing Q or q you can exit from the program" << std::endl;
        std::cout<< " By pressing I or i you can reset the object positions" << std::endl;
        std::cout<< " By pressing right mouse button you can reach the menu" <<std::endl;
        std::cout<< " Under the menu you can control the displaying options" <<std::endl;
    }
}
//--------------------------------------------------------------------------------------------------------//
//Controls the animation inputs. At the end of each timer, updates the xPosition and yPosition.
//Velocity in the x axis remains constant.
//Velocity in the y axis decreases by the surfaceEffect in each hit to the ground. Gravitation g is added to yVelocity.
void timer( int p )
{
    xPosition = xPosition+xVelocity; //increment x position with respect to x velocity
    if(yPosition<-3.75) {
        yVelocity *= -1;
        yVelocity-=surfaceEffect;
        if(yVelocity<0){
            g=0;
            yVelocity=0;
            xVelocity=0;
        }
    }
    yVelocity = yVelocity + g;
    yPosition = yPosition + yVelocity;
    
    Theta[Xaxis]+=2;
    if(Theta[Xaxis]>=360)Theta[Xaxis]-=360;
    Theta[Yaxis]+=2;
    if(Theta[Yaxis]>=360)Theta[Yaxis]-=360;
    
    glutPostRedisplay();
    glutTimerFunc(10,timer,0);
}

//--------------------------------------------MENU RELATED------------------------------------------------//
void drawingModeMenu (int n){
    if(n==0)glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else if(n==1)glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    else if(n==2)glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}
void colorMenu(int n){
    colorNumber = n;
}
void velocityMenu (int n){
    if(n==0) xVelocitySetter=0.00100;
    else if(n==1)xVelocitySetter=0.00200;
    else if(n==2)xVelocitySetter=0.00300;
    else if(n==3)xVelocitySetter=0.00400;
    else if(n==4)xVelocitySetter=0.00500;
    xVelocity=xVelocitySetter;
}
void shadingMenu (int n){
    if(n==0){//gouraud
        gouraudFlag = true;
        phongFlag = false;
        modifiedPhongFlag = false;
        texturingFlag=false;
    }
    else if(n==1){//phong
        gouraudFlag = false;
        phongFlag = true;
        modifiedPhongFlag = false;
        texturingFlag=false;
        
    }else if(n==2){//modified-phong
        phongFlag = false;
        gouraudFlag = false;
        modifiedPhongFlag = true;
        texturingFlag=false;
     
    }else if(n==3){
        phongFlag = false;
        gouraudFlag = false;
        modifiedPhongFlag = false;
        texturingFlag=false;
    }
     glUniform1i(PhongFlag, phongFlag);
     glUniform1i(GouraudFlag, gouraudFlag);
     glUniform1i(ModifiedPhongFlag, modifiedPhongFlag);
     glUniform1i(TexturingFlag, texturingFlag);
     glutPostRedisplay();
}
void texturing(int n){
    glBindTexture(GL_TEXTURE_2D, textures[n]);//0 basketball //1 earth
    
    phongFlag = false;
    gouraudFlag = false;
    modifiedPhongFlag = false;
    texturingFlag=true;
    
    glUniform1i(PhongFlag, phongFlag);
    glUniform1i(GouraudFlag, gouraudFlag);
    glUniform1i(ModifiedPhongFlag, modifiedPhongFlag);
    glUniform1i(TexturingFlag, texturingFlag);
    glutPostRedisplay();
}

void lightSourcePositionMenu (int n){
    if(n==0){//fixed
        glUniform4fv(LightPosition,1,light_position);
        lightPositionFixed=true;
    }else if(n==1){ //moving
        lightPositionFixed=false;
    }
}
//change shininess coeffcient
void materialType (int n){
    if(n==0){ //plastic
        material_shininess= 5;
        glUniform1f(Shininess, material_shininess);
        
    }else if(n==1){ //metalic
        material_shininess=150;
        glUniform1f(Shininess, material_shininess);
    }
}
//set choosen ones to 0
void turnOffTheComponent (int n){
    if(n==0){ //ambient
        ambient_product=0;
        glUniform4fv(AmbientProduct,1,ambient_product );
    }else if(n==1){ //diffuse
        diffuse_product=0;
        glUniform4fv(DiffuseProduct,1,diffuse_product);
    }else if(n==2){ //specular
        specular_product=0;
        glUniform4fv(SpecularProduct,1,specular_product);
    }
}
//recalculate the choosen one
void turnOnTheComponent(int n){
    if(n==0){ //ambient
        ambient_product = light_ambient * material_ambient;
        glUniform4fv(AmbientProduct,1,ambient_product );
    }else if(n==1){ //diffuse
        diffuse_product = light_diffuse * material_diffuse;
        glUniform4fv(DiffuseProduct,1,diffuse_product);
    }else if(n==2){ //specular
        specular_product = light_specular * material_specular;
        glUniform4fv(SpecularProduct,1,specular_product);
    }
}

void mainMenu(int n){}

void CreateMenu(){
    int hvelocityMenu = glutCreateMenu(velocityMenu);
    glutAddMenuEntry("1", 0);
    glutAddMenuEntry("2", 1);
    glutAddMenuEntry("3", 2);
    glutAddMenuEntry("4", 3);
    glutAddMenuEntry("5", 4);
    
    int drawingMenu = glutCreateMenu(drawingModeMenu);
    glutAddMenuEntry("WIREFRAME LINES", 0);
    glutAddMenuEntry("POINTS", 1);
    glutAddMenuEntry("SOLID MODE", 2);
    
    int cMenu = glutCreateMenu(colorMenu);
    glutAddMenuEntry("Black", 0);
    glutAddMenuEntry("Red", 1);
    glutAddMenuEntry("Yellow", 2);
    glutAddMenuEntry("Green", 3);
    glutAddMenuEntry("Blue", 4);
    glutAddMenuEntry("Magenta", 5);
    glutAddMenuEntry("White", 6);
    glutAddMenuEntry("Cyan", 7);
    
    int sMenu = glutCreateMenu(shadingMenu);
    glutAddMenuEntry("Gouraud", 0);
    glutAddMenuEntry("Phong", 1);
    glutAddMenuEntry("Modified-Phong", 2);
    glutAddMenuEntry("NONE", 3);
    
    int lightSourceMenu = glutCreateMenu(lightSourcePositionMenu);
    glutAddMenuEntry("Fixed", 0);
    glutAddMenuEntry("Moving with the object", 1);
    
    int materialMenu = glutCreateMenu(materialType);
    glutAddMenuEntry("Plastic", 0);
    glutAddMenuEntry("Metalic", 1);
    
    int tOffComponents = glutCreateMenu(turnOffTheComponent);
    glutAddMenuEntry("Ambient", 0);
    glutAddMenuEntry("Difuse", 1);
    glutAddMenuEntry("Specular", 2);
    
    int tOnComponents = glutCreateMenu(turnOnTheComponent);
    glutAddMenuEntry("Ambient", 0);
    glutAddMenuEntry("Difuse", 1);
    glutAddMenuEntry("Specular", 2);
    
    int texMenu = glutCreateMenu(texturing);
    glutAddMenuEntry("Basketball", 0);
    glutAddMenuEntry("Earth", 1);
    
    glutCreateMenu(mainMenu);
    glutAddSubMenu("Horizontal Velocity", hvelocityMenu);
    glutAddSubMenu("Drawing Mode", drawingMenu);
    glutAddSubMenu("Color Type", cMenu);
    glutAddSubMenu("Shading Type", sMenu);
    glutAddSubMenu("Texture", texMenu);
    glutAddSubMenu("Light Position", lightSourceMenu);
    glutAddSubMenu("Material", materialMenu);
    glutAddSubMenu("Turn On Each Component", tOnComponents);
    glutAddSubMenu("Turn Off Each Component", tOffComponents);
  
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}
//--------------------------------------------------------------------------------------------------------//
int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode(  GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize( 512, 512 );
    glutInitWindowPosition( 50, 50 );
    glutCreateWindow( "410" );
    glewExperimental = GL_TRUE;
    glewInit();
    init();
    //-------------------------------callback functions-----------------------------------//
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc(keyboard);
    glutTimerFunc(5,timer,0);
    //------------------------------------------------------------------------------------//
    CreateMenu();
    glutMainLoop();
    return 0;
}



