#include "OpenGLWidget.hpp"

OpenGLWidget::OpenGLWidget( QWidget *parent ) 
: QOpenGLWidget(parent) {

}

OpenGLWidget::~OpenGLWidget() {}

void OpenGLWidget::initializeGL() {
    initializeOpenGLFunctions();
}

void OpenGLWidget::resizeGL( int w, int h ) {
    glViewport( 0,0, w,h );
}

void OpenGLWidget::paintGL() {
    
}

