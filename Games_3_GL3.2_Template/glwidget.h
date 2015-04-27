#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "glheaders.h" // Must be included before QT opengl headers
#include <QGLWidget>
#include <QWheelEvent>
#include <string>

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

void transform(char type);

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    GLWidget(const QGLFormat& format, QWidget* parent = 0);
public slots:
    void wheelEvent(QWheelEvent *event);

protected:
    virtual void initializeGL();
    virtual void resizeGL( int w, int h );
    virtual void paintGL();

    virtual void keyPressEvent( QKeyEvent* e );

private:
    bool prepareShaderProgram( const QString& vertexShaderPath,
                               const QString& fragmentShaderPath );

    QOpenGLShaderProgram m_shader;
    QOpenGLBuffer m_vertexBuffer;
};

#endif // GLWIDGET_H
