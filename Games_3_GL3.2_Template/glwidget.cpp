#include "glwidget.h"

#include <QCoreApplication>
#include <QKeyEvent>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <QFile>
#include <QFileDevice>
#include <QWheelEvent>

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

#define VERT_SHADER ":/simple.vert"
#define FRAG_SHADER ":/simple.frag"

using namespace std;
unsigned int numTriangles; //Number of triangles
float r,g,b;

glm::mat4x4 viewMatrix;
glm::mat4 projectionMatrix;
glm::mat4 modelMatrix;
glm::mat4 mvp;

char translation;
glm::vec3 axis;

GLWidget::GLWidget( const QGLFormat& format, QWidget* parent )
    : QGLWidget( format, parent ),
      m_vertexBuffer( QOpenGLBuffer::VertexBuffer )
{
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

    r = 0.0f;
    g = 1.0f;
    b = 0.0f;

    ifstream ifs("bunny.stl", ios::binary | ios::in);
    char header[80];
    char num[4];
    ifs.read(header, 80);(0.0f,0.0f,1.0f);
    ifs.read(num,4);
    numTriangles =  *((unsigned int *)num);

    //unsigned int size = *numTriangles;
    float points[numTriangles*4*3];

    char buffer[12];
    float * vertex;

    for (int i= 0; i < numTriangles; i++)
    {
        ifs.read(buffer, 12);

        for (int j = 0; j < 3; j++)
        {
            ifs.read(buffer, 12);
            vertex = (float *)buffer;
            int offset = (i*12) + (j*4);
            points[offset] = vertex[0];
            points[offset+1] = vertex[1];
            points[offset+2] = vertex[2];
            points[offset+3] = 1.0f ;
        }
            char rubbish[2];
            ifs.read(rubbish,2);
    }



//    glm::mat4 matrix;
//    glm::translate(matrix,glm::vec3(100000.0f,0.0f,0.0f));

//    for (int i = 0; i < 4; i+=4)
//    {
//        glm::vec4 vec(points[i],points[i+1],points[i+2],points[i+3]);
//        cout << points[i] << " , " << points[i+1]<<  " , " << points[i+2] << " , " <<points[i+3];
//        glm::vec4 transformed = matrix * vec;
//        points[i]= transformed[0];
//        points[i+1]= transformed[1];
//        points[i+2]= transformed[2];
//        points[i+3]= transformed[3];
//        cout << transformed[i] << " , " << transformed[i+1]<<  " , " << transformed[i+2] << " , " << transformed[i+3];

//    }









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
    cout << 1;
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // We need us some vertex data. Start simple with a triangle ;-)
//    float points[] = { -0.5f, -0.5f, 0.0f, 1.0f,
//                        0.5f, -0.5f, 0.0f, 1.0f,
//                        0.0f,  0.5f, 0.0f, 1.0f};

    //float points[] = {};

    m_vertexBuffer.create();
    cout << 1;
    m_vertexBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    if ( !m_vertexBuffer.bind() )
    {
        qWarning() << "Could not bind vertex buffer to the context";
        return;
    }
    m_vertexBuffer.allocate( points, numTriangles * 3 * 4 * sizeof( float ) ); //change

    qDebug() << "Attempting vertex shader load from " << VERT_SHADER;
    qDebug() << "Attempting fragment shader load from " << FRAG_SHADER;

    // Prepare a complete shader program...
    if ( !prepareShaderProgram( VERT_SHADER, FRAG_SHADER) )
        std::runtime_error("Failed to load shader");
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

    glm::vec3 eye(0,0,1);
    glm::vec3 cent(0,0,0);
    glm::vec3 up(0,1,0);

    viewMatrix = glm::lookAt(eye,cent,up);
    projectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    modelMatrix = glm::mat4(1.0f);

    axis = glm::vec3(1.0f,0.0f,0.0f);
    translation = 'R';
    //modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.25f,0.0f));
    //modelMatrix = glm::rotate(modelMatrix,55.0f,glm::vec3(0.0f,0.0f,1.0f));
    //modelMatrix = glm::scale(modelMatrix,glm::vec3(1.0f,-2.0f,1.0f));
    mvp = projectionMatrix * viewMatrix * modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(m_shader.programId(),"mvp"), 1, GL_FALSE, &mvp[0][0]);
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta()/20;
    if (translation == 'R')
        modelMatrix = glm::rotate(modelMatrix,(float)numDegrees,axis);
    else if (translation == 'S')
        modelMatrix = glm::scale(modelMatrix,axis*((float)numDegrees/15));
    else \
        modelMatrix = glm::translate(glm::mat4(1.0f), axis*(0.05f*numDegrees));
    mvp = projectionMatrix * viewMatrix * modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(m_shader.programId(),"mvp"), 1, GL_FALSE, &mvp[0][0]);
    updateGL();

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

void GLWidget::keyPressEvent( QKeyEvent* e )
{
    switch ( e->key() )
    {
        case Qt::Key_Escape:
            QCoreApplication::instance()->quit();
            break;
        case Qt::Key_1:
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
        case Qt::Key_R:

            if (translation != 'R')
            {
                axis = glm::vec3(1.0f,0.0f,0.0f);
                translation = 'R';
            }
            else
            {
                if (axis == glm::vec3(1.0f,0.0f,0.0f))
                    axis = glm::vec3(0.0f,1.0f,0.0f);
                else if (axis == glm::vec3(0.0f,1.0f,0.0f))
                    axis = glm::vec3(0.0f,0.0f,1.0f);
                else
                    axis = glm::vec3(1.0f,0.0f,0.0f);
            }
        case Qt::Key_S:
            translation = 'S';
            if (translation != 'S')
            {
                axis = glm::vec3(1.0f,0.0f,0.0f);
            }
            else
            {
                if (axis == glm::vec3(1.0f,0.0f,0.0f))
                    axis = glm::vec3(0.0f,1.0f,0.0f);
                else if (axis == glm::vec3(0.0f,1.0f,0.0f))
                    axis = glm::vec3(0.0f,0.0f,1.0f);
                else
                    axis = glm::vec3(1.0f,0.0f,0.0f);
            }
        case Qt::Key_T:
            translation = 'T';
            if (translation != 'T')
            {
                axis = glm::vec3(1.0f,0.0f,0.0f);
            }
            else
            {
                if (axis == glm::vec3(1.0f,0.0f,0.0f))
                    axis = glm::vec3(0.0f,1.0f,0.0f);
                else if (axis == glm::vec3(0.0f,1.0f,0.0f))
                    axis = glm::vec3(0.0f,0.0f,1.0f);
                else
                    axis = glm::vec3(1.0f,0.0f,0.0f);
            }


        default:
            QGLWidget::keyPressEvent( e );
    }

    glUniform4f(glGetUniformLocation(m_shader.programId(),"fcolor"),r,g,b,1.0f);
    updateGL();
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
