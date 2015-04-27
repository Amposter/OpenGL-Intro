/*  Aashiq Parker
 *  PRKAAS003
 *  Games Assignment 2: OpenGL
 *  27 April 2015
 */

#include "glwidget.h"
#include <QCoreApplication>
#include <QKeyEvent>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <QFile>
#include <QFileDevice>
#include <QWheelEvent>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#define VERT_SHADER ":/simple.vert"
#define FRAG_SHADER ":/simple.frag"

using namespace std;

string fileName = ""; //Name of the file
float * points; //All the vertex points
unsigned int numTriangles; //Number of triangles
float r,g,b; //Color values

//Matrices to set up 'camera'
glm::mat4x4 viewMatrix;
glm::mat4 projectionMatrix;
glm::mat4 modelMatrix;
glm::mat4 mvp; //Final camera matrix mvp = projection*model*view

char translation; //Keeps track of current translation
glm::vec3 axis; //Keeps track of current axis
glm::vec3 x(1.0f,0.0f,0.0f);
glm::vec3 y(0.0f,1.0f,0.0f);
glm::vec3 z(0.0f,0.0f,1.0f);

GLWidget::GLWidget( const QGLFormat& format, QWidget* parent )
    : QGLWidget( format, parent ),
      m_vertexBuffer( QOpenGLBuffer::VertexBuffer )
{
        //Part of Question 2
        //Setting up the menubar to have a 'new' and 'open' option
       QMenuBar *bar = new QMenuBar(this);

       QMenu *fileMenu = new QMenu("File");

       QAction * actionOpen = fileMenu->addAction("Open");
       QAction * actionNew = fileMenu->addAction("New");

       QObject::connect(actionOpen,SIGNAL(triggered()),this,SLOT(openFile())); //Connecting the options to methods
       QObject::connect(actionNew,SIGNAL(triggered()),this,SLOT(newFile()));

       bar->addMenu(fileMenu);
   }


void GLWidget::initializeGL()
{
    // Resolve OpenGL functions
    glewExperimental = true;
    GLenum GlewInitResult = glewInit();
    if (GlewInitResult != GLEW_OK) {
        const GLubyte* errorStr = glewGetErrorString(GlewInitResult);
        size_t size = strlen(reinterpret_cast<const char*>(errorStr));
        qDebug() << "Glew error "
                 << QString::fromUtf8(
                        reinterpret_cast<const char*>(errorStr), size);
    }
    //Set the initial color of the object
    r = 0.0f;
    g = 1.0f;
    b = 0.0f;

    // get context opengl-version
    qDebug() << "Widget OpenGl: " << format().majorVersion() << "." << format().minorVersion();
    qDebug() << "Context valid: " << context()->isValid();
    qDebug() << "Really used OpenGl: " << context()->format().majorVersion() << "." << context()->format().minorVersion();
    qDebug() << "OpenGl information: VENDOR:       " << (const char*)glGetString(GL_VENDOR);
    qDebug() << "                    RENDERDER:    " << (const char*)glGetString(GL_RENDERER);
    qDebug() << "                    VERSION:      " << (const char*)glGetString(GL_VERSION);
    qDebug() << "                    GLSL VERSION: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    QGLFormat glFormat = QGLWidget::format();
    if ( !glFormat.sampleBuffers() )
        qWarning() << "Could not enable sample buffers";

    // Set the clear color to black
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

    // we need a VAO in core!
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    m_vertexBuffer.create();
    m_vertexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_vertexBuffer.bind() )
    {
        qWarning() << "Could not bind vertex buffer to the context";
        return;
    }
    m_vertexBuffer.allocate( points, numTriangles * 3 * 4 * sizeof( float ) ); //change

    if (fileName == "") //Check so that the shaders are only bound once
    {
        qDebug() << "Attempting vertex shader load from " << VERT_SHADER;
        qDebug() << "Attempting fragment shader load from " << FRAG_SHADER;

        // Prepare a complete shader program...
        if ( !prepareShaderProgram( VERT_SHADER, FRAG_SHADER) )
            std::runtime_error("Failed to load shader");
    }
    // Bind the shader program so that we can associate variables from
    // our application to the shaders
    if ( !m_shader.bind() )
    {
        qWarning() << "Could not bind shader program to context";
        return;
    }
    // Enable the "vertex" attribute to bind it to our currently bound
    // vertex buffer.
    m_shader.setAttributeBuffer( "vertex", GL_FLOAT, 0, 4 );
    m_shader.enableAttributeArray( "vertex" );
    glUniform4f(glGetUniformLocation(m_shader.programId(),"fcolor"),r,g,b,1.0f);

 /***************************************************************************************************************************************
     * Question 1: Setting up the camera */

    glm::vec3 eye(0,0,4); //The position of the camera
    glm::vec3 cent(0,0,0); //The target
    glm::vec3 up(0,1,0); //Direction for motion upwards

    //Set up the model,view and projection matrices
    viewMatrix = glm::lookAt(eye,cent,up);
    projectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    modelMatrix = glm::mat4(1.0f);//Where transformations go, no transformations at the start so it's just the identity matrix

    axis = glm::vec3(1.0f,0.0f,0.0f);//Set the default axis and transformation to rotate in the x-axis
    translation = 'R';
    mvp = projectionMatrix * viewMatrix * modelMatrix; //Finally, create the camera matrix
    glUniformMatrix4fv(glGetUniformLocation(m_shader.programId(),"mvp"), 1, GL_FALSE, &mvp[0][0]); //Pass it to the shader
}

/**************************************************************************************************************************************/






/***************************************************************************************************************************************
 * Question 2, Part 1: Loading the STL file */
void drawSTL(string filename)
{
     fileName = filename; //Setting the name of the file for later use

     ifstream ifs(filename, ios::binary | ios::in); //Loading in the stl file
     if (!ifs) //Check that it's open
     {
         cerr << "File could not be opened";
         return;
     }

     //Specification of the stl file follows the one provided by Wikipedia

     //Space for the header and count of triangles
     char header[80];
     char num[4];

     //Read in the values
     ifs.read(header, 80);(0.0f,0.0f,1.0f);
     ifs.read(num,4);

     numTriangles =  *((unsigned int *)num);

     //Set up the pointer size so the points can be added
     points =  new float[numTriangles*4*3];


     //Space for each vertex
     char buffer[12];
     float * vertex;

     //The loops reads in the vertices for each triangle, then slots each point into the pointer
     for (int i= 0; i < numTriangles; i++)
     {
         ifs.read(buffer, 12); //Read in vertex

         for (int j = 0; j < 3; j++) //Loop 3 times since each triangle has 3 vertices
         {
             ifs.read(buffer, 12);
             vertex = (float *)buffer; //Convert to floats
             int offset = (i*12) + (j*4); //Calculate the offset position in the pointer

             //Set each point in the pointer;
             points[offset] = vertex[0];
             points[offset+1] = vertex[1];
             points[offset+2] = vertex[2];
             points[offset+3] = 1.0f ;
         }
             //Get rid of the two bytes of waste between each triangle
             char rubbish[2];
             ifs.read(rubbish,2);
     }

     ifs.close();
}
/**************************************************************************************************************************************/






/***************************************************************************************************************************************
 * Question 3: Part 1, Method to apply the current transformation to the bunny */
void GLWidget::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta()/20; //Get the value and direction of the scroll

    //Check which transformation it is and apply the appropriate one
    if (translation == 'R')
        modelMatrix = glm::rotate(modelMatrix,(float)numDegrees,axis);
    else if (translation == 'S')
        modelMatrix = glm::scale(modelMatrix, (axis*(float)numDegrees/25.0f) + glm::vec3(1.0f));
    else if (translation == 'T')
        modelMatrix *= glm::translate(glm::mat4(1.0f), axis*(0.005f*numDegrees));

    mvp = projectionMatrix * viewMatrix * modelMatrix; //Update the mvp camera matrix
    glUniformMatrix4fv(glGetUniformLocation(m_shader.programId(),"mvp"), 1, GL_FALSE, &mvp[0][0]);//Update the matrix in the shader class
    updateGL(); //Update the scren
}

/**************************************************************************************************************************************/





//Methods for opening and creating a new version of the file

void GLWidget::openFile(void)
{
    QString directory = QFileDialog::getOpenFileName(this,tr("Open File"),".","Allfiles (*.stl)"); //Open dialog to locate the file
    drawSTL(directory.toStdString());//Call the draw method
    initializeGL();
    updateGL();
}

void GLWidget::newFile(void)
{
    if (fileName != "") //Just reload the current file, check that its not empty
    {
        drawSTL(fileName);
        initializeGL();
        updateGL();
    }
}

void GLWidget::resizeGL( int w, int h )
{
    // Set the viewport to window dimensions
    glViewport( 0, 0, w, qMax( h, 1 ) );
}

void GLWidget::paintGL()
{

    // Clear the buffer with the current clearing color
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Draw stuff
   glDrawArrays( GL_TRIANGLES, 0, numTriangles*3);
}

/***************************************************************************************************************************************
 * Question 4: Changing the color of the object on key presses */

void GLWidget::keyPressEvent( QKeyEvent* e )
{
    switch ( e->key() )
    {
        case Qt::Key_Escape:
            QCoreApplication::instance()->quit();
            break;
        case Qt::Key_1: //Listen for key presses '1' - '5' and change to a different colour
            r = 1.0f;
            g = 0.0f;
            b = 0.0f;
            break;
        case Qt::Key_2:
            r = 0.0f;
            g = 0.0f;
            b = 1.0f;;
            break;
        case Qt::Key_3:
            r = 0.5f;
            g = 0.5f;
            b = 0.0f;
            break;
        case Qt::Key_4:
            r = 0.0f;
            g = 0.5f;
            b = 0.5f;
            break;
        case Qt::Key_5:
            r = 0.5f;
            g = 0.0f;
            b = 0.5f;
            break;
   /**************************************************************************************************************************************/

        //Part of question 3: Listen for presses to change the transformation
        case Qt::Key_R:
            transform('R');
            break;
        case Qt::Key_S:
            transform('S');
            break;
        case Qt::Key_T:
            transform('T');
            break;
        default:
            QGLWidget::keyPressEvent( e );
    }

    glUniform4f(glGetUniformLocation(m_shader.programId(),"fcolor"),r,g,b,1.0f);
    updateGL();
}

//Method to update the transformation type
void transform(char type)
{
    if (translation != type) //If it is the first time the transformation has been called, set the axis to x
    {
        translation = type;
        axis = x;
    }
    else //Else change to the next axis
    {
        if (axis == x)
            axis = y;
        else if (axis == y)
            axis = z;
        else if (axis == z)
            axis = x;
    }

}

bool GLWidget::prepareShaderProgram( const QString& vertexShaderPath,
                                     const QString& fragmentShaderPath )
{
    // First we load and compile the vertex shader...
    bool result = m_shader.addShaderFromSourceFile( QOpenGLShader::Vertex, vertexShaderPath );
    if ( !result )
        qWarning() << m_shader.log();

    // ...now the fragment shader...
    result = m_shader.addShaderFromSourceFile( QOpenGLShader::Fragment, fragmentShaderPath );
    if ( !result )
        qWarning() << m_shader.log();

    // ...and finally we link them to resolve any references.
    result = m_shader.link();
    if ( !result )
        qWarning() << "Could not link shader program:" << m_shader.log();

    return result;
}
