#include "glwidget.h"
#include <QApplication>
#include <QGLFormat>

int main(int argc, char* argv[])
{
    QApplication a( argc, argv );

    // Specify an OpenGL 3.2 format using the Core profile.
    // That is, no old-school fixed pipeline functionality
    QGLFormat glFormat;
    glFormat.setVersion( 3, 2 );
    glFormat.setProfile( QGLFormat::CoreProfile );
    glFormat.setSampleBuffers( true );

    GLWidget w( glFormat );
    w.show();

    return a.exec();
}

