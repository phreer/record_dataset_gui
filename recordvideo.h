ifndef RECORDVIDEO_H
#define RECORDVIDEO_H


class RecordVideo : public QThread
{
public:
    RecordVideo();
protected:
    void run();
};

#endif // RECORDVIDEO_H
