#include <QtOpenGLWidgets/QtOpenGLWidgets>
#include <QOpenGLFunctions>

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();

protected:
    void initializeGL() override;  // 相当于EGL初始化后调用
    void resizeGL(int w, int h) override;  // 窗口大小变化
    void paintGL() override;  // 每一帧绘制
};
