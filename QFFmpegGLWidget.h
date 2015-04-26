#ifndef QFFMPEGGLWidget_H
#define QFFMPEGGLWidget_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QVector3D>
#include <QMatrix4x4>
#include <QTime>
#include <QVector>
#include <QOpenGLTexture>



QT_FORWARD_DECLARE_CLASS(QOpenGLTexture)
QT_FORWARD_DECLARE_CLASS(QOpenGLShader)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

#define BUF_SIZE 2



class QFFmpegGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    QFFmpegGLWidget();
    ~QFFmpegGLWidget();

    void updateData(unsigned char*);
    void updateData(unsigned char**);

    void initShader(int w,int h);
public slots:

    void setTransparent(bool transparent);


protected:
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void initializeGL() Q_DECL_OVERRIDE;

private:
    QTimer *timer;

    //--------------------
    QOpenGLShader *m_vshaderA;
    QOpenGLShader *m_fshaderA;
    QOpenGLShaderProgram *m_programA;
    QOpenGLBuffer m_vboA;
    //-------------------
    QOpenGLTexture *m_texture;

    bool m_transparent;

    QColor m_background;

    bool isShaderInited;
    int width;
    int height;
    quint8 *buffer[BUF_SIZE];
    int bufIndex;
    void InitializeTexture( int id, int width, int height);
};

#endif
